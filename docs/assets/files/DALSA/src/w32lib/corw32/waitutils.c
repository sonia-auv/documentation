/****************************************************************************** 
Copyright (c) 2004 Coreco inc / 2010 DALSA Corp.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

  - Redistributions of source code must retain the above copyright notice, 
    this list of conditions and the following disclaimer. 

  - Redistributions in binary form must reproduce the above copyright notice, 
    this list of conditions and the following disclaimer in the documentation 
    and/or other materials provided with the distribution. 

  - Neither the name of the DALSA nor the names of its contributors 
    may be used to endorse or promote products derived from this software 
    without specific prior written permission. 


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
OF THE POSSIBILITY OF SUCH DAMAGE.

===============================================================================

waitutils.c			 											

Description:
   Posix wait object handling compatibility layer with Win32.

Platform:
	-Generic Posix.

History:
   1.00 March 15, 2004, parhug

$Log: waitutils.c $
Revision 1.12  2006/05/12 08:48:14  PARHUG
Change order of regular and async code path operations in UnlockSharedWaiter
in order to remove a race condition in SMP systems.
Revision 1.11  2006/04/21 09:56:13  PARHUG
Added LockSharedWaiter, UnlockSharedWaiter (and _Async versions) to protect the shared waiter list
during its destruction. On SMP systems the different threads and async signal handlers 
were simultaneoulsy accessing the list at high frame rates.
Revision 1.10  2006/03/07 10:02:02  PARHUG
Add critical section back in to the CleanUp of the multiple waiters.
Re-sequence the order that shared waiters are added / removedfrom waiting handles.
Change WaitMultipleObjects to use a handle allocated on the stack and not entered into the handle registry.
Add functions to use this unregistered handle for shared waits.
(All this to improve performance by removing malloc usage and handle registration on every shared wait).
Revision 1.9  2005/10/12 15:34:45  PARHUG
Add more handle validation.
Revision 1.8  2005/07/22 16:33:27  PARHUG
Merge with 1.7.1.2
Revision 1.7.1.2  2005/05/11 10:53:09  PARHUG
Remove use of critical sections for updating handle flags.
Alter logic in WaitForMultipleObjects to not require critical sections.
Alter logic in WaitForSingleObject to handle events better and to yield the scheduler to prevent observed thread starvation.
Fix the way Posix semaphores are used si they recover from being
 interrupted by signals.
Revision 1.7  2005/04/22 10:27:33  PARHUG
Fixed handling of closed handles (NullObject).
Always clear signal bit for kernel notfication events regardless of reference count.
Check for invalid handles in WaitForMultipleObjects.
Revision 1.6  2005/03/08 17:42:47  PARHUG
Add reference counting to WaitForMultipleObjects and change UpdateHandleStatus
to not clear a signalled event until all waiters have awoken.
Revision 1.5  2005/01/19 11:12:08  parhug
Fix WaitMultipleObjects to avoid a race condition by moving the EnterCriticalSection to an earlier point.
Revision 1.4  2004/10/14 09:47:25  parhug
Fixes for shared wait object handling (semaphore or event).
Revision 1.3  2004/10/06 17:19:02  parhug
Update for new handle model. Fixes for WaitForMultipleObjects.
Revision 1.2  2004/08/31 09:25:13  parhug
Fixed WaitForMultipleObjects.
Revision 1.1  2004/08/19 12:26:11  parhug
Initial revision

*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __CELL__
#include <cordef.h>
#endif
#include <corlists.h>
#include <linuxobjs.h>

#define USE_SEMTIMEDWAIT 1

extern int pthread_yield(void);

static LONG sharedWaitAccessCount = 0;
static pthread_mutex_t sharedWaitAccessMutex = PTHREAD_MUTEX_INITIALIZER;

#define YIELD_WAIT( a ) { if (((a) == -1) && (errno == EINTR)) sched_yield(); }
static int SemTryWait( HANDLE handle );


//****************************************************************************
//  Function Name : LockSharedWaiter
//
// Description :
//    This function locks the shared waiter update mechanism for the standard
//    code path. A mutex is used to ensure only one thread can access the 
//    contents of the shared wait handle list. This prevents collision between
//    a thread signalling a shared waiter handle while the list is being 
//    destroyed.
//    A signal handler uses the *_Async version of this function since it 
//    cannot used a mutex. If the Async code path is using the lock, this
//    function busy waits until it is done. (The Async code path only signals
//    shared waiters and will never be the one cleaning up the shared waiter
//    list).
//
// Input  : (none)
// Output : (none)
//
// Return Value : (int) 
//    0 means shared waiter update mechanism is locked.
//    (other) means shared waiter update mechanism is not locked
//
//****************************************************************************
int LockSharedWaiter(void)
{
   int status = 0;
   LONG value = 0;
   
   // Lock the mutex so no one else can take it.
   if ( (status = pthread_mutex_lock(&sharedWaitAccessMutex)) == 0)
   {
      // We have the lock here.   
      // Now, check if the Async code path has locked access (i.e. the signal handler).  
      if ( (value = InterlockedIncrement(&sharedWaitAccessCount)) == 1)
      {
         // Async path does not own the lock.
         return status;
      }

      // The Async code path (signal handler) owns the flag. Wait for it.
      do
      {
         struct timespec sleeptime;
         int timeout = 0;
         int status;
            
         pthread_yield();
         
         sleeptime.tv_sec = 0;
         sleeptime.tv_nsec = 500000;  // 500 usec.
         status = nanosleep( &sleeptime, NULL );
         YIELD_WAIT(status);
         value = sharedWaitAccessCount;
         if (timeout++ > 1000)
         {
				// More than 0.5 seconds have elapsed in simple lock contention.
				// Just take the lock. 
				break;
         }
      } while(value > 1);
   }
   return status;
}

//****************************************************************************
//  Function Name : UnlockSharedWaiter
//
// Description :
//    This function unlocks the shared waiter update mechanism for the standard
//    code path. No check is made to ensure it was previously locked.
//
// Input  : (none)
// Output : (none)
//
// Return Value : (int) 
//    0 means shared waiter update mechanism is unlocked.
//    (other) means shared waiter update mechanism is not locked
//
//****************************************************************************
int UnlockSharedWaiter(void)
{
   InterlockedDecrement(&sharedWaitAccessCount);
   pthread_mutex_unlock(&sharedWaitAccessMutex);
   return 0;
}

//****************************************************************************
//  Function Name : LockSharedWaiter_Async
//
// Description :
//    This function locks the shared waiter update mechanism for the Async
//    code path. A shared global is atomically incremented and its value
//    indicates the lock status. 
//    If it is 1, the lock is acquired.
//
//    Note : This function is intended for use in a signal handler. A signal
//       handler may not block and may call only a few functions safely. The
//       Sapera API uses the signal handler only for signalling a semaphore
//       in reponse to a SIGIO signal from a driver. If the access mechanism 
//       is already locked, then the shared waiter list is being destroyed in
//       response to a satisifed wait and we can skip the notification since 
//       the semaphore has been signalled and will be picked up on the next
//       wait.
//
// Input  : (none)
// Output : (none)
//
// Return Value : (int) 
//    0 means shared waiter update mechanism is locked.
//    -1 means shared waiter update mechanism is not locked
//
//****************************************************************************
int LockSharedWaiter_Async(void)
{
   if (InterlockedIncrement(&sharedWaitAccessCount) == 1) 
   {
      // The Async path owns the lock.
      return 0;
   }
   else
   {
      //  The Async path does not own the lock.
      return -1;
   }
}

//****************************************************************************
//  Function Name : UnlockSharedWaiter_Async
//
// Description :
//    This function unlocks the shared waiter update mechanism for the Async
//    code path. The shared global is atomically decremented. No check is 
//    made on the value.
//
// Input  : (none)
// Output : (none)
//
// Return Value : (none) 
//
//****************************************************************************
void UnlockSharedWaiter_Async(void)
{
   InterlockedDecrement(&sharedWaitAccessCount);
}


//****************************************************************************
//  Function Name : UpdateHandleStatus
//
// Description :
//    This function updates the signalled state of a handle based on the
//    handle type.
//    Auto-reset events, events that have been "pulsed", and kernel 
//    notification events all have their signalled flag cleared if they
//    were set on entrance to this function.
//
// Input  : 
//    handle : Event object handle (of type HANDLE).
//
// Output : (none)
//
// Return Value : (none) 
//
//****************************************************************************
static inline void UpdateHandleStatus( HANDLE handle )
{
	if ( HANDLE_IS_VALID(handle) )
	{
		if ( VALID_HANDLE(Event, handle) )
		{
			if ( HANDLE_IS_SIGNALLED(handle) )
			{
				if ( (HANDLE_REF(handle)->eventType == AutoReset) || \
					((HANDLE_REF(handle)->state & STATE_SIGNAL_PULSED) != 0)  )
				{
               // Clear the event signalled state. (Only 1 thread can awaken here anyway).
					HANDLE_REF(handle)->state &= ~(STATE_SIGNAL_SET | STATE_SIGNAL_PULSED);
				}
			}
		}
		else if ( VALID_HANDLE(Semaphore, handle) )
		{
         // Clear the event signalled state. (Only 1 thread can awaken here anyway).
			if ( HANDLE_REF(handle)->eventType == KernelNotification)
			{
				// Async-safe event is a KernelNotification event 
				// provided as a posix semaphore.
				if ( (HANDLE_REF(handle)->state & STATE_SIGNAL_SET) != 0)
				{
					int value = 0;
					sem_getvalue(HANDLE_REF(handle)->psemaphore, &value);
					if (value == 0)
					{
						HANDLE_REF(handle)->state &= ~STATE_SIGNAL_SET;
					}
				}
			}
		}
		if ( HANDLE_IS_CLOSING(handle) )
		{
			// Close is pending, try to close it again
			CloseHandle(handle);
		}
	}
}


static inline int old_GetWaitStatus( HANDLE handle )
{
	int result = WAIT_FAILED;

	if ( !HANDLE_IS_OPEN(handle) )
	{
		result = WAIT_ABANDONED;
	}
	else
	{
		if ( HANDLE_IS_SIGNALLED(handle) )
		{
			if ( HANDLE_REF(handle)->eventType == KernelNotification)
			{
				SemTryWait(handle);
			}
			result = WAIT_OBJECT_0;
		}
	}
	return result;
}

static inline int GetWaitStatus( HANDLE handle )
{
	int result = WAIT_FAILED;

	if ( !HANDLE_IS_OPEN(handle) )
	{
		result = WAIT_ABANDONED;
	}
	else
	{
		if ( HANDLE_REF(handle)->eventType == KernelNotification)
		{
			if ( sem_trywait(HANDLE_REF(handle)->psemaphore) == 0)
			{
				result = WAIT_OBJECT_0;
				HANDLE_REF(handle)->state |= STATE_SIGNAL_SET;  
			}
		}
		else
		{
			if ( HANDLE_IS_SIGNALLED(handle) )
			{
				result = WAIT_OBJECT_0;
			}
		}
	}
	return result;
}


//****************************************************************************
//  Function Name : FindHandleEntry
//
// Description :
//    This function searches the input handle array for a handle matching the
// input handle object. 
//
// Input  : 
//    handlelist : Aray of Event object handles (of type HANDLE).
//    handle     : Event object handle (of type HANDLE).
//    total      : Maximum number of handles in the list.
//
// Output : (none)
//
// Return Value : (int) 
//    The index into the handle list array corresponding to the location 
//    of "handle". It it is not found, "total" is returned.
//
//****************************************************************************
static inline int FindHandleEntry( volatile HANDLE *handleList, HANDLE handle, int total)
{
	int i;

	for (i = 0; i < total; i++)
	{
		if ( handleList[i] == handle)
			break;
	}
	return i;
}

//****************************************************************************
//  Function Name : CleanUpMultipleWaiters
//
// Description :
//    This function destroys the shared waiter list on behalf of the 
//    WaitMultiple* function. It finds all instances of the particular shared 
//    wait object in the multiWaitHandles array in each of the handles in the 
//    input list and removes it.
//
//    Since any handle in the list can be in more than on shared wait at any
//    one time, the shared wait object is removed from the handles list and
//    any remaining shared wait objects are shuffled leaving the other waits
//    intact. 
//
//    The shared waiter access locking mechanism is used to prevent corruption
//    of the multiWaitHandles array and to prevent the signalling of a shared 
//    handle in one thread after another thread has destroyed it. (Think SMP).
//
// Input  : 
//    bCritSectionHeld : Flag indicating if each handles critical section is 
//                       already held on input.
//    end      : The clean up only needs to go from the beginning of the 
//               handles list to this index.
//    total    : The total number of handles in the handles list.
//    pHandles : The array of handles used in the WaitMultiple* call.
//    linkHandle : The shared handle that links all the handles together 
//                 for the WaitMultiple* call.
//
// Output : (none)
//
// Return Value : (int) 
//    The index into the handle list array corresponding to the location 
//    of "handle". It it is not found, "total" is returned.
//
//****************************************************************************
static void CleanUpMultipleWaiters( BOOL bCritSectionHeld, int end, LONG total, HANDLE *pHandles, HANDLE linkHandle)
{
	int i, j, entry;
	volatile HANDLE *list;

   LockSharedWaiter();
	for (i = 0; i < end; i++)
	{
		// Make sure the handle is still valid.
		if ( HANDLE_IS_VALID(pHandles[i]) )
		{
         if ( !bCritSectionHeld )
         {
            EnterCriticalSection( &HANDLE_REF(pHandles[i])->cs);
         }

			// Find the linkHandle pointer in the list for this handle.
			list = (volatile HANDLE *)&HANDLE_REF(pHandles[i])->multiWaitHandles[0];
			entry = FindHandleEntry( list, linkHandle, total);

			// Now remove the entry and adjust the count.
         InterlockedDecrement( (LONG *)&HANDLE_REF(pHandles[i])->numMultiWaiters);
			for ( j = entry; j <= HANDLE_REF(pHandles[i])->numMultiWaiters; j++)
			{
				list[j] = list[j+1];
			}
         list[HANDLE_REF(pHandles[i])->numMultiWaiters] = NULL;

         // Decrement the reference count for this handle (no longer waiting).
         DECREMENT_HANDLE_REFCOUNT(pHandles[i]);
      
         if ( !bCritSectionHeld )
         {
            LeaveCriticalSection( &HANDLE_REF(pHandles[i])->cs);
         }

		}
	}
   UnlockSharedWaiter();
}



static int SemWait( HANDLE handle )
{
   int status = -1;
   if ( HANDLE_IS_VALID(handle))
   {
      if ( HANDLE_REF(handle)->bPosixSemaphore)
      {
         do 
         {
            if (HANDLE_IS_VALID(handle) )
            {
					if (HANDLE_REF(handle)->eventType == KernelNotification)
               {
						// Async-safe event is a KernelNotification event 
						// provided as a posix semaphore.
						if ( (HANDLE_REF(handle)->state & STATE_SIGNAL_SET) != 0)
						{
							int value = 0;
							sem_getvalue(HANDLE_REF(handle)->psemaphore, &value);
							if (value == 0)
							{
								HANDLE_REF(handle)->state &= ~STATE_SIGNAL_SET;
							}
							status = 0;
						}
						else
						{
							status = sem_wait( HANDLE_REF(handle)->psemaphore);
							YIELD_WAIT(status);
						}
					}
					else
					{
						status = sem_wait( HANDLE_REF(handle)->psemaphore);
						YIELD_WAIT(status);
					}
					if (status != 0)
					{  
						int value = 0;
						sem_getvalue(HANDLE_REF(handle)->psemaphore, &value);
						if (value == 0)
						{
							HANDLE_REF(handle)->state &= ~STATE_SIGNAL_SET;
						}
						else
						{
							status = 0;
						}
					}
					
            }
            else
            {
               status = 0;
            }
         }
         while (status == -1 && errno == EINTR);
      }
      else
      {
         struct sembuf sop;

         sop.sem_num = HANDLE_REF(handle)->semnum;
         sop.sem_op  = -1;
         sop.sem_flg = 0;
         do
         {
            if ( HANDLE_IS_VALID(handle) )
            {
               status = semop( HANDLE_REF(handle)->semid, &sop, 1);
               YIELD_WAIT(status);
           }
            else
            {
               status = 0;
            }
         }while (status == -1 && errno == EINTR);
      }
   }
   return status;
}


static int SemWaitInterruptible( HANDLE handle )
{
   int status = -1;
   if ( HANDLE_IS_VALID( handle))
   {
      if ( HANDLE_REF(handle)->bPosixSemaphore)
      {
         status = sem_wait( HANDLE_REF(handle)->psemaphore);
      }
      else
      {
         struct sembuf sop;

         sop.sem_num = HANDLE_REF(handle)->semnum;
         sop.sem_op  = -1;
         sop.sem_flg = 0;
         status = semop( HANDLE_REF(handle)->semid, &sop, 1);
         if(status == -1)
            status = errno;
      }
   }
   return status;
}

 
static int SemTryWait( HANDLE handle )
{
   int status = -1;
   if (HANDLE_IS_VALID(handle))
   {
      if ( HANDLE_REF(handle)->bPosixSemaphore)
      {
         do 
         {
            if (HANDLE_IS_VALID(handle) )
            {
					status = sem_trywait( HANDLE_REF(handle)->psemaphore);
					YIELD_WAIT(status);
            }
            else
            {
               status = 0;
            }
         } while (status == -1 && errno == EINTR);
			if (status == 0)
			{
				if (HANDLE_REF(handle)->eventType == KernelNotification)
				{
					// Async-safe event is a KernelNotification event 
					// provided as a posix semaphore.
					if ( (HANDLE_REF(handle)->state & STATE_SIGNAL_SET) != 0)
					{
						int value = 0;
						sem_getvalue(HANDLE_REF(handle)->psemaphore, &value);
						if (value == 0)
						{
							HANDLE_REF(handle)->state &= ~STATE_SIGNAL_SET;
						}
					}
				}
			}
			else
			{  
				int value = 0;
				sem_getvalue(HANDLE_REF(handle)->psemaphore, &value);
				if (value == 0)
				{
					HANDLE_REF(handle)->state &= ~STATE_SIGNAL_SET;
				}
				else
				{
					status = 0;
				}
			}
      }
      else
      {
         struct sembuf sop;

         sop.sem_num = HANDLE_REF(handle)->semnum;
         sop.sem_op  = -1;
         sop.sem_flg = IPC_NOWAIT;
         do
         {
            if ( HANDLE_IS_VALID(handle) )
            {
               status = semop( HANDLE_REF(handle)->semid, &sop, 1);
					YIELD_WAIT(status);
            }
            else
            {
               status = 0;
            }
         } while (status == -1 && ( (errno == EINTR) || (errno == EAGAIN)));
      }
   }
   return status;
}

static int SemTryWaitInterruptible( HANDLE handle )
{
   int status = -1;
   if ( HANDLE_IS_VALID(handle))
   {
      if ( HANDLE_REF(handle)->bPosixSemaphore)
      {
         status = sem_trywait( HANDLE_REF(handle)->psemaphore);
      }
      else
      {
         struct sembuf sop;

         sop.sem_num = HANDLE_REF(handle)->semnum;
         sop.sem_op  = -1;
         sop.sem_flg = IPC_NOWAIT;
         status = semop( HANDLE_REF(handle)->semid, &sop, 1);
         if(status == -1)
            status = errno;
      }
   }
   return status;
}

//****************************************************************************
// Helper functions for the shared object used by WaitForMultipleObjects.
// It is always valid and is always a semaphore so we remove some of the checking
// and hope no one uses it incorrectly (internally).

static int _InitSharedWaitSemaphore( PHANDLEOBJ semaphore )
{
   int status = -1;
   if (semaphore != NULL)
   {
      memset(semaphore, 0, sizeof(HANDLEOBJ));
      status = sem_init( &semaphore->semaphore, 0, 0);
      if ( status == 0)
      {
         semaphore->type = Semaphore;
         semaphore->psemaphore = (sem_t *)&semaphore->semaphore;
         semaphore->sem_currentcount = 0;
         semaphore->sem_maxcount = 1;
         semaphore->refCount = 1; // We are open
         semaphore->state = STATE_OPEN;
         semaphore->bPosixSemaphore = TRUE;
      }
   }
   return status;
}

static int _CleanupSharedWaitSemaphore( PHANDLEOBJ semaphore )
{
   int status = -1;
   if (semaphore != NULL)
   {
      status = sem_destroy( &semaphore->semaphore);
   }
   return status;
}

static int SemWait_NoCheck( PHANDLEOBJ handleobj )
{
   int status = -1;
   if ( handleobj->bPosixSemaphore)
   {
      do 
      {
         status = sem_wait( handleobj->psemaphore);
         YIELD_WAIT(status);
      } while ((status == -1) && (errno == EINTR));
   }
   else
   {
      struct sembuf sop;

      sop.sem_num = handleobj->semnum;
      sop.sem_op  = -1;
      sop.sem_flg = 0;
      do
      {
         status = semop( handleobj->semid, &sop, 1);
         YIELD_WAIT(status);
      } while (status == -1 && errno == EINTR);
   }
   return status;
}

static int SemTryWait_NoCheck( HANDLE handle )
{
   int status = -1;
   if ( HANDLE_REF(handle)->bPosixSemaphore)
   {
      do 
      {
         status = sem_trywait( HANDLE_REF(handle)->psemaphore);
         YIELD_WAIT(status);
      } while (status == -1 && errno == EINTR);
   }
   else
   {
      struct sembuf sop;

      sop.sem_num = HANDLE_REF(handle)->semnum;
      sop.sem_op  = -1;
      sop.sem_flg = IPC_NOWAIT;
      do
      {
         status = semop( HANDLE_REF(handle)->semid, &sop, 1);
         YIELD_WAIT(status);
      } while (status == -1 && errno == EINTR);
   }
   return status;
}

static int SemTimedWait_NoCheck( HANDLE handle, LONG millisecs )
{
   int status = -1;
   if ( HANDLE_REF(handle)->bPosixSemaphore)
   {
		struct timeval ts;
		struct timespec tmout;
		//struct timezone tz;
		LONG usecs;
		LONG newusecs;

		//gettimeofday(&ts, &tz);
		GetCurrentClockTime( &ts);

		usecs = millisecs/1000;
		newusecs = ts.tv_usec + (usecs % 1000000);
		tmout.tv_sec = ts.tv_sec + (usecs / 1000000) + (newusecs / 1000000);
		tmout.tv_nsec = (newusecs % 1000000) * 1000;
		
      do 
      {
         status = sem_timedwait( HANDLE_REF(handle)->psemaphore, &tmout);
         YIELD_WAIT(status);
      } while (status == -1 && errno == EINTR);
   }
   else
   {
      struct sembuf sop;

      sop.sem_num = HANDLE_REF(handle)->semnum;
      sop.sem_op  = -1;
      sop.sem_flg = IPC_NOWAIT;
      do
      {
         status = semop( HANDLE_REF(handle)->semid, &sop, 1);
         YIELD_WAIT(status);
      } while (status == -1 && errno == EINTR);
   }
   return status;
}



static LONG WaitForSingleSharedObject( HANDLE handle, LONG millisecs)
{
   LONG result = WAIT_FAILED;
#if !USE_SEMTIMEDWAIT
   struct timespec sleeptime, remainder;
   LONG adjustedms;
   int status;
#endif

	if (VALID_HANDLE(Semaphore, handle))
	{
      INCREMENT_HANDLE_REFCOUNT(handle);
      if (millisecs == INFINITE)
      {
         result = SemWait_NoCheck(HANDLE_REF(handle));
      }
      else if (millisecs == 0)
      {
         result = SemTryWait_NoCheck(handle);
      }
      else
      {
        result = WAIT_TIMEOUT;
 #if USE_SEMTIMEDWAIT
			result = SemTimedWait_NoCheck( handle, millisecs); 
 #else
        do 
         {
				adjustedms = (millisecs > 5) ? 5 : millisecs;
				//sleeptime.tv_sec = millisecs / 1000;
				sleeptime.tv_sec = 0;
				sleeptime.tv_nsec = (adjustedms - (sleeptime.tv_sec * 1000)) * 1000000;

				if ( SemTryWait_NoCheck(handle) == 0)
            {
               result = WAIT_OBJECT_0;
               break;
            }
            else
            {
               remainder.tv_sec = 0;
               remainder.tv_nsec = 0;
               status = nanosleep( &sleeptime, &remainder );
               YIELD_WAIT(status);
               millisecs -= ((sleeptime.tv_sec - remainder.tv_sec)*1000  + (sleeptime.tv_nsec - remainder.tv_nsec)/1000000);
            }
         } while (millisecs > 0);
#endif
      }
      DECREMENT_HANDLE_REFCOUNT(handle);
   }
   else
   {
      result = WAIT_FAILED;
   }
   return result;
}


// Non-portable version of WaitForSingleObject : uses struct timeval for timeout.
//
// If pTimeout is NULL and the wait flag is set - wait forever (INFINITE timeout).
//
LONG WaitForSingleObject_NP( HANDLE handle, BOOL bWait, struct timeval *pTimeout)
{
	struct timespec sleeptime, remainder;
	BOOL bWaitForever = FALSE;
	LONG usecs = 0;
	LONG newusecs = 0;
	LONG result = WAIT_FAILED;
	handleType_t savedHandleType;

	// Check if our thread has been cancelled. No need to proceed.
	pthread_testcancel();

	if (!HANDLE_IS_VALID(handle))
   {
		return result;
   }

	// Check if it is already signalled.
	result = GetWaitStatus(handle);
	if ( result != WAIT_FAILED )
	{
		UpdateHandleStatus(handle);
		return result;
	}
	savedHandleType = HANDLE_REF(handle)->type;

	// Set up the timeout (if bWait is TRUE and pTimeout is NULL - wait forever)
	sleeptime.tv_sec = 0;
	sleeptime.tv_nsec = 0;
	if (bWait && pTimeout != NULL)
	{
		sleeptime.tv_sec = pTimeout->tv_sec;
		sleeptime.tv_nsec = pTimeout->tv_usec * 1000;
		usecs = pTimeout->tv_sec * 1000000 + pTimeout->tv_usec;
	}
	else
	{
		bWaitForever = TRUE;
	}


	// Wait based on the handle type being used.
	switch (savedHandleType)
	{
		case Mutex:
			INCREMENT_HANDLE_REFCOUNT(handle);
			if ( !bWait )
			{
				result = WAIT_TIMEOUT;
				if (pthread_mutex_trylock( &HANDLE_REF(handle)->mutex) == 0)
				{
					result = WAIT_OBJECT_0;
				}
			}
			else if (bWaitForever)
			{
				if ( pthread_mutex_lock( &HANDLE_REF(handle)->mutex) == 0)
				{
						result = WAIT_OBJECT_0;
				}

			}
			else
			{
				result = WAIT_TIMEOUT;
				while ((usecs > 0) && VALID_HANDLE(savedHandleType,handle))
				{
					if ( pthread_mutex_trylock( &HANDLE_REF(handle)->mutex) == 0)
					{
						result = WAIT_OBJECT_0;
						break;
					}
					remainder.tv_sec = 0;
					remainder.tv_nsec = 0;
					nanosleep( &sleeptime, &remainder );
					usecs -= ((sleeptime.tv_sec - remainder.tv_sec)*1000000  + (sleeptime.tv_nsec - remainder.tv_nsec)/1000);
				}
			}
			// Check that the handle is still valid.
			if (!VALID_HANDLE( savedHandleType, handle) )
			{
				result = WAIT_ABANDONED;
			}
			else
			{
				DECREMENT_HANDLE_REFCOUNT(handle);
			}
			break;

			// File and device handles will behave as an Event (a condition variable).
		case DeviceObject:
		case FileObject:
		case Event:
		case Thread:
			{
				if ( bWaitForever )
				{
					// Acquire the mutex in order to wait on the condition variable.
					if ( pthread_mutex_lock( &HANDLE_REF(handle)->mutex) == 0 )
					{
						// Check if it is already signalled.
						INCREMENT_HANDLE_REFCOUNT(handle);
						result = GetWaitStatus(handle);
						
						while ( result == WAIT_FAILED )
						{
							// Wait for it to be signalled.
							pthread_cond_wait( &HANDLE_REF(handle)->event, &HANDLE_REF(handle)->mutex);

							// Check that the handle is still valid.
							if (VALID_HANDLE( savedHandleType, handle) )
							{
								result = GetWaitStatus(handle);
							}
							else
							{
								result = WAIT_ABANDONED;
								pthread_mutex_unlock( &HANDLE_REF(handle)->mutex);  
								return result;
							}
						}
						if (result == WAIT_OBJECT_0)
						{
							UpdateHandleStatus(handle);
						}
						DECREMENT_HANDLE_REFCOUNT(handle);
						pthread_mutex_unlock( &HANDLE_REF(handle)->mutex);
						pthread_testcancel();
						Sleep(0);
					}
				}
				else
				{
					struct timeval ts;
					struct timespec tmout;
					//struct timezone tz;

					//gettimeofday(&ts, &tz);
					GetCurrentClockTime( &ts);
               //ts.tv_usec += sleeptime.tv_nsec / 1000;
               //ts.tv_sec += sleeptime.tv_sec + (ts.tv_usec / 1000000);
               //ts.tv_usec += (usecs % 1000000);
               //ts.tv_sec += (usecs / 1000000);

					newusecs = ts.tv_usec + (usecs % 1000000);
					tmout.tv_sec = ts.tv_sec + (usecs / 1000000) + (newusecs / 1000000);
					tmout.tv_nsec = (newusecs % 1000000) * 1000;

					// Acquire the mutex in order to do a timed wait on the condition variable.
					if ( pthread_mutex_lock( &HANDLE_REF(handle)->mutex) == 0 )
					{
						// Check if it is already signalled.
						INCREMENT_HANDLE_REFCOUNT(handle);
						result = GetWaitStatus(handle);
						
						while (result == WAIT_FAILED)
						{
							// Wait for it to be signalled (with a timeout).
							int waitresult = 0;
                     waitresult = pthread_cond_timedwait( &HANDLE_REF(handle)->event, &HANDLE_REF(handle)->mutex, &tmout);
                     
                     // Check that the handle is still valid.
                     if (VALID_HANDLE( savedHandleType, handle) )
                     {
                        if ( waitresult == 0)
                        {
                           result = GetWaitStatus(handle);
                        }
                        else
                        {
                           if (waitresult == ETIMEDOUT)
                           {
                              result = WAIT_TIMEOUT;
                           }
                           else
                           {
                              // Some other error. (just time out as well).
                              result = GetWaitStatus(handle);
                              if (result == WAIT_FAILED)
                              {
											result = WAIT_TIMEOUT;
										}
                           }
                        }
                     }
							if ( result == WAIT_FAILED)
							{
								if (!VALID_HANDLE( savedHandleType, handle) )
								{
									pthread_mutex_unlock(&HANDLE_REF(handle)->mutex);   
									return WAIT_ABANDONED;
								}
							}
						}
						if (result == WAIT_OBJECT_0)
						{
							UpdateHandleStatus(handle);
						}
						DECREMENT_HANDLE_REFCOUNT(handle);
						pthread_mutex_unlock(&HANDLE_REF(handle)->mutex);
						pthread_testcancel();
						//Sleep(0);
					}
				}
				// Check that the handle is still valid.
				if (!VALID_HANDLE( savedHandleType, handle) )
				{
					// Handle type changed (memory freed up?)
					result = WAIT_ABANDONED;
				}
			}
			break;

			// Semaphores (support shared with namedMutex).
		case Semaphore:
		case NamedSemaphore:
		case NamedMutex:
			{
				INCREMENT_HANDLE_REFCOUNT(handle);
				if ( bWaitForever )
				{
					if ( SemWait(handle) == 0)
					{
						result = WAIT_OBJECT_0;
					}
					else
					{
						if ( !HANDLE_IS_OPEN(handle) )
						{
							result = WAIT_ABANDONED;
						}
					}
				}
				else
				{
					result = WAIT_TIMEOUT;

					do 
					{
						if ( SemTryWait(handle) == 0)
						{
							if ( !HANDLE_IS_OPEN(handle) )
							{
								result = WAIT_ABANDONED;
							}
							else
							{
								result = WAIT_OBJECT_0;
							}
							break;
						}
						else
						{
							if ( !HANDLE_IS_OPEN(handle) )
							{
								result = WAIT_ABANDONED;
							}
						}
						remainder.tv_sec = 0;
						remainder.tv_nsec = 0;
						nanosleep( &sleeptime, &remainder );
						usecs -= ((sleeptime.tv_sec - remainder.tv_sec)*1000000  + (sleeptime.tv_nsec - remainder.tv_nsec)/1000);
					} while (usecs > 0);
				}
				// Check that the handle is still valid.
				if (!VALID_HANDLE( savedHandleType, handle) )
				{
					result = WAIT_ABANDONED;
				}
				else
				{
					DECREMENT_HANDLE_REFCOUNT(handle);
				}
			}
			break;


			// System V semaphore shared between processes.
		case SharedEvent:
			break;

			// System V semaphore opened by name, shared between processes
			// (not supported in Linux 2.4 at all yet).
		case NamedSharedEvent:
			break;

			// Closed objects become NullObjects until their handle is reclaimed.
		case NullObject:
			{
				result = WAIT_ABANDONED;
			}
			break;
		
		default:
			break;
	 }
	// Check if our thread has been cancelled. No need to proceed.
	pthread_testcancel();
	
	return result;
}


LONG WaitForSingleObject( HANDLE handle, LONG millisecs)
{
	int status;
	struct timespec sleeptime, remainder;
	LONG adjustedms;
	LONG result = WAIT_FAILED;
	handleType_t savedHandleType;

	// Check if our thread has been cancelled. No need to proceed.
	pthread_testcancel();

	if (!HANDLE_IS_VALID(handle))
   {
		return result;
   }

	// Check if it is already signalled.
	result = GetWaitStatus(handle);
	if ( result != WAIT_FAILED )
	{
		UpdateHandleStatus(handle);
		return result;
	}
	savedHandleType = HANDLE_REF(handle)->type;

	// Wait based on the handle type being used.
	switch (savedHandleType)
	{
		case Mutex:
			INCREMENT_HANDLE_REFCOUNT(handle);
			if ( millisecs == 0)
			{
				result = WAIT_TIMEOUT;
				if (pthread_mutex_trylock( &HANDLE_REF(handle)->mutex) == 0)
				{
					result = WAIT_OBJECT_0;
				}
			}
			else if (millisecs == INFINITE)
			{
				if ( pthread_mutex_lock( &HANDLE_REF(handle)->mutex) == 0)
				{
						result = WAIT_OBJECT_0;
				}

			}
			else
			{
				// Poll the mutex (at 5 ms max intervals) - mutex primitive has no timeout.
				result = WAIT_TIMEOUT;
				while ((millisecs > 0) && VALID_HANDLE(savedHandleType,handle))
				{
					adjustedms = (millisecs > 5) ? 5 : millisecs;
					sleeptime.tv_sec = millisecs / 1000;
					sleeptime.tv_nsec = (adjustedms - (sleeptime.tv_sec * 1000)) * 1000000;
					if ( pthread_mutex_trylock( &HANDLE_REF(handle)->mutex) == 0)
					{
						result = WAIT_OBJECT_0;
						break;
					}
					remainder.tv_sec = 0;
					remainder.tv_nsec = 0;
					status = nanosleep( &sleeptime, &remainder );
					YIELD_WAIT(status);
					millisecs -= ((sleeptime.tv_sec - remainder.tv_sec)*1000  + (sleeptime.tv_nsec - remainder.tv_nsec)/1000000);
				}
			}
			// Check that the handle is still valid.
			if (!VALID_HANDLE( savedHandleType, handle) )
			{
				result = WAIT_ABANDONED;
			}
			else
			{
				DECREMENT_HANDLE_REFCOUNT(handle);
			}
			break;

			// File and device handles will behave as an Event (a condition variable).
		case DeviceObject:
		case FileObject:
		case Event:
		case Thread:
			{
				if ( millisecs == 0)
				{
					result = GetWaitStatus(handle);
					if (result == WAIT_OBJECT_0)
					{
						UpdateHandleStatus(handle);
					}
				}
				else if ( millisecs == INFINITE)
				{
					// Acquire the mutex in order to wait on the condition variable.
					if ( pthread_mutex_lock( &HANDLE_REF(handle)->mutex) == 0 )
					{
						// Check if it is already signalled.
						INCREMENT_HANDLE_REFCOUNT(handle);
						result = GetWaitStatus(handle);
						
						while ( result == WAIT_FAILED )
						{
							// Wait for it to be signalled.
							pthread_cond_wait( &HANDLE_REF(handle)->event, &HANDLE_REF(handle)->mutex);

							// Check that the handle is still valid.
							if (VALID_HANDLE( savedHandleType, handle) )
							{
								result = GetWaitStatus(handle);
							}
							else
							{
								result = WAIT_ABANDONED;
								pthread_mutex_unlock( &HANDLE_REF(handle)->mutex);  
								return result;
							}
							if ( result == WAIT_FAILED)
							{
								sched_yield();
							}
						}
						if (result == WAIT_OBJECT_0)
						{
							UpdateHandleStatus(handle);
						}
						DECREMENT_HANDLE_REFCOUNT(handle);
						pthread_mutex_unlock( &HANDLE_REF(handle)->mutex);
					}
				}
				else
				{
					struct timeval ts;
					struct timespec tmout;
					//struct timezone tz;
					int usecs;

					//gettimeofday(&ts, &tz);
					GetCurrentClockTime( &ts);
					usecs = ((millisecs % 1000)*1000) + ts.tv_usec;
               ts.tv_usec = (usecs % 1000000);
					ts.tv_sec += (millisecs / 1000) + (usecs / 1000000);
					tmout.tv_sec = ts.tv_sec;
					tmout.tv_nsec = ts.tv_usec * 1000;

					// Acquire the mutex in order to do a timed wait on the condition variable.
					if ( pthread_mutex_lock( &HANDLE_REF(handle)->mutex) == 0 )
					{
						// Check if it is already signalled.
						INCREMENT_HANDLE_REFCOUNT(handle);
						result = GetWaitStatus(handle);
						
						while (result == WAIT_FAILED)
						{
							// Wait for it to be signalled (with a timeout).
							int waitresult = 0;
                     waitresult = pthread_cond_timedwait( &HANDLE_REF(handle)->event, &HANDLE_REF(handle)->mutex, &tmout);
                     
                     // Check that the handle is still valid.
                     if (VALID_HANDLE( savedHandleType, handle) )
                     {
                        if ( waitresult == 0)
                        {
                           result = GetWaitStatus(handle);
                        }
                        else
                        {
                           if (waitresult == ETIMEDOUT)
                           {
                              result = WAIT_TIMEOUT;
                           }
                           else
                           {
                              // Some other error (spurious wakeup).
										result = GetWaitStatus(handle);
                            }
                        }
                     }
							if ( result == WAIT_FAILED)
							{
								if (!VALID_HANDLE( savedHandleType, handle) )
								{
									pthread_mutex_unlock(&HANDLE_REF(handle)->mutex);   
									return WAIT_ABANDONED;
								}
								sched_yield();
							}
						}
						if (result == WAIT_OBJECT_0)
						{
							UpdateHandleStatus(handle);
						}
						DECREMENT_HANDLE_REFCOUNT(handle);
						pthread_mutex_unlock(&HANDLE_REF(handle)->mutex);
						pthread_testcancel();
					}
				}
				// Check that the handle is still valid.
				if (!VALID_HANDLE( savedHandleType, handle) )
				{
					// Handle type changed (memory freed up?)
					result = WAIT_ABANDONED;
				}
			}
			break;

			// Semaphores (support shared with namedMutex).
		case Semaphore:
		case NamedSemaphore:
		case NamedMutex:
			{
				INCREMENT_HANDLE_REFCOUNT(handle);
				if (millisecs == 0)
				{
					if ( SemTryWait(handle) == 0)
					{
						if ( !HANDLE_IS_OPEN(handle) )
						{
							result = WAIT_ABANDONED;
						}
						else
						{
							result = WAIT_OBJECT_0;
						}
						break;
					}
					else
					{
						if ( !HANDLE_IS_OPEN(handle) )
						{
							result = WAIT_ABANDONED;
						}
					}
				}
				else if (millisecs == INFINITE)
				{
					if ( SemWait(handle) == 0)
					{
						result = WAIT_OBJECT_0;
					}
					else
					{
						if ( !HANDLE_IS_OPEN(handle) )
						{
							result = WAIT_ABANDONED;
						}
					}
				}
				else
				{
					result = WAIT_TIMEOUT;
					// Poll the semaphore (at 1 ms max intervals) - semaphore primitive has no timeout.
					int microsecs = millisecs * 1000;
					adjustedms = (microsecs > 999) ? 999 : microsecs;
					sleeptime.tv_sec = 0;
					sleeptime.tv_nsec = adjustedms * 1000;
					do
					{

						if ( SemTryWait(handle) == 0)
						{
							if ( !HANDLE_IS_OPEN(handle) )
							{
								result = WAIT_ABANDONED;
							}
							else
							{
								result = WAIT_OBJECT_0;
							}
							break;
						}
						else
						{
							if ( !HANDLE_IS_OPEN(handle) )
							{
								result = WAIT_ABANDONED;
							}
						}
						remainder.tv_sec = 0;
						remainder.tv_nsec = 0;
						status = nanosleep( &sleeptime, &remainder );
						YIELD_WAIT(status);  
						millisecs -= ((sleeptime.tv_sec - remainder.tv_sec)*1000  + (sleeptime.tv_nsec - remainder.tv_nsec)/1000000);
					} while (millisecs > 0);
				}
				// Check that the handle is still valid.
				if (!VALID_HANDLE( savedHandleType, handle) )
				{
					result = WAIT_ABANDONED;
				}
				else
				{
					DECREMENT_HANDLE_REFCOUNT(handle);
				}
			}
			break;


			// System V semaphore shared between processes.
		case SharedEvent:
			break;

			// System V semaphore opened by name, shared between processes
			// (not supported in Linux 2.4 at all yet).
		case NamedSharedEvent:
			break;

			// Closed objects become NullObjects until their handle is reclaimed.
		case NullObject:
			{
				result = WAIT_ABANDONED;
			}
			break;
		
		default:
			break;
	 }
	// Check if our thread has been cancelled. No need to proceed.
	pthread_testcancel();
	
	return result;
}
LONG WaitForMultipleObjects( LONG count, HANDLE *handles, BOOL bWaitAll, LONG millisecs)
{
	int i;
	LONG status = WAIT_FAILED;
	HANDLE handle = NULL;
   void *temp = NULL;
   //HANDLE object = NULL; // Shared semaphore.
   PHANDLEOBJ object = NULL; // Storage for shared semaphore.

	// Check if we are doing "Wait for All" mode. Simple.
	if ( bWaitAll || (count == 1))
	{
		for (i = 0; i < count; i++)
		{
			status = WaitForSingleObject( handles[i], millisecs);
			if ( (status != WAIT_OBJECT_0) && (status != WAIT_ABANDONED_0))
			{
				return status;
			}
		}
		return status;
	}

	// "Wait for Any" mode (most difficult case).
	// Create a semaphore to wait on and link in each handle to it.
	
#if 0 
   if ( _InitSharedWaitSemaphore( &object) != 0)
   {
		status = WAIT_FAILED; 
      return status;
   }
   else
   {
      temp = (void *)&object;
      handle = (HANDLE) &temp;
   }
#else
   handle = CreateSemaphore( NULL, 0, MAX_POSIX_SEMAPHORE_COUNT, NULL);
   if (!VALID_HANDLE(Semaphore, handle))
   {
		//printf("FAILED\n"); 
		status = WAIT_FAILED;
		return status;
	}
	object = HANDLE_REF(handle);
#if 0
   if ((handle == NULL) || (handle == INVALID_HANDLE_VALUE))
   {
		status = WAIT_FAILED;
		return status;
	}
	object = HANDLE_REF(handle);
	if ((object == NULL) || (object == INVALID_HANDLE_VALUE))
	{
		printf("FAILED\n"); 
		status = WAIT_FAILED;
		return status;
	}
#endif
#endif

	if ( (handle != NULL) && (handle != INVALID_HANDLE_VALUE))
	{
		// Run through all the handles, check their status and Acquire the critical section.
		LockSharedWaiter(); 
		for (i = 0; i < count; i++)
		{
         if ( HANDLE_IS_VALID(handles[i]))
         {
				// Increment the reference count to indicate we are using this handle.
            INCREMENT_HANDLE_REFCOUNT(handles[i]);

            // Add the new multiple waiter handle to the list of multiple waiter handles in this handle.
            if ( HANDLE_REF(handles[i])->numMultiWaiters < (MULTI_WAIT_COUNT_MAX-1) )
            {
	            EnterCriticalSection( &HANDLE_REF(handles[i])->cs);
               HANDLE_REF(handles[i])->multiWaitHandles[HANDLE_REF(handles[i])->numMultiWaiters] = handle;
               InterlockedIncrement( (LONG *)&HANDLE_REF(handles[i])->numMultiWaiters);
               LeaveCriticalSection( &HANDLE_REF(handles[i])->cs);
            }
            else
            {
               // This handle is in too many calls to WaitForMultipleObjects - fail.
               status = WAIT_FAILED;

               // Clean up everything up to this point.
 				   UnlockSharedWaiter();
              CleanUpMultipleWaiters( FALSE, i, count, handles, handle);
				  #if 0
					_CleanupSharedWaitSemaphore(&object);
				  #else
					CloseHandle(handle);
				  #endif
                return status;
            }
          }
      }
 		UnlockSharedWaiter(); 

		// Re-check the individual handles in case there was a race with the
		// async path (signal handler).
		for (i = 0; i < count; i++)
		{	
			// See if this handle has already been signalled.
         status = WaitForSingleObject( handles[i], 0);

         if ( (status == WAIT_OBJECT_0) || (status == WAIT_ABANDONED_0))
         {
            status += i;

            // Clean up everything up to this point.
            CleanUpMultipleWaiters( FALSE, count, count, handles, handle);
            #if 0
            _CleanupSharedWaitSemaphore(&object);
           #else
               CloseHandle(handle);
            #endif
            return status;
         }
         else
         {
            if (!HANDLE_IS_VALID(handles[i]))
            {
               // This handle is invalid - fail them all.
               status = WAIT_FAILED;

                // Clean up everything up to this point.
               CleanUpMultipleWaiters( FALSE, count, count, handles, handle);
            #if 0
               _CleanupSharedWaitSemaphore(&object);
            #else
               CloseHandle(handle);
            #endif
             return status;
            }
         }
      }

		// Perform the wait on the shared object.
		status = WaitForSingleSharedObject( handle, millisecs);

		if ( status == WAIT_OBJECT_0 )
		{
			// Run through all of the objects to see which one signalled.
			for (i = 0; i < count; i++)
			{
				status = WaitForSingleObject( handles[i], 0);
				if ( (status == WAIT_OBJECT_0) || (status == WAIT_ABANDONED_0))
				{
					status += i;

					// Clean up everything up to this point.
					CleanUpMultipleWaiters( FALSE, count, count, handles, handle);
			#if 0
               _CleanupSharedWaitSemaphore(&object);
         #else
               CloseHandle(handle);
          #endif
					return status;
				}
			}

			status = WAIT_FAILED; // Error - fall through.
		}

		// Failure or timeout here. Clean up.
		CleanUpMultipleWaiters( FALSE, count, count, handles, handle);
	#if 0
      _CleanupSharedWaitSemaphore(&object);
   #else
      CloseHandle(handle);
   #endif
      return status;
	}
	return status;
}



