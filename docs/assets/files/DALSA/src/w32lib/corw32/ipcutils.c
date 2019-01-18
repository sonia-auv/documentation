/****************************************************************************** 
Copyright (c) 2003 Coreco inc / 2010 DALSA Corp.
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

ipcutils.c			 										

Description:
   Inter-Process Communications (IPC) Posix compatibility layer with Win32.

Platform:
	-Generic Posix.

History:
   1.00 October 15, 2003, parhug

$Log: ipcutils.c $
Revision 1.9  2006/04/21 09:50:48  PARHUG
Added SignalSharedWaiters_Async so the I/O signal handler can signal semaphores using
the Async code path in the "shared waiter lock" mechanism for SMP safety.
Revision 1.8  2006/03/27 12:27:30  PARHUG
Kernel event object is now a semaphore.
Revision 1.7  2005/07/22 16:31:29  PARHUG
Merge with 1.6.1.2
Revision 1.6.1.2  2005/05/11 10:23:57  PARHUG
Remove use of critical sections in handle flag updates.
Change the use of the conditiion variables to properly lock/unlock the associated mutex 
(this is the proper way to do handle flag updates).
Revision 1.6  2005/03/15 12:17:34  PARHUG
Added SetKernelEventObject in order to not use a critical section within the signal handler
(where kernel notification events are signalled).
Revision 1.5  2005/03/08 17:28:17  PARHUG
Change handling of shared waiters. Now signal the object first and then use SignalSharedWaiters to signal 
all of the shared objects set up in WaitForMultipleObjects.
Revision 1.4  2004/10/14 09:53:02  parhug
Added SignalSharedWaiters so we can support semaphores or events as the common object.
Revision 1.3  2004/10/06 16:59:30  parhug
Update for new handle model
Revision 1.2  2004/08/31 09:23:07  parhug
Update for use with shared objects used in WaitForMultipleObjects call.
Revision 1.1  2004/08/19 12:26:08  parhug
Initial revision

*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <corposix.h>
#include <linuxobjs.h>

/**============================== CODE ====================================**/

static BOOL SignalSharedWaiters_Async( volatile PHANDLEOBJ handleobj)
{
	// *** DO NOT LOCK *** the shared waiters lock from the Async code path
	// There is no easy way to do it properly so instead we build, destroy, 
	// and traverse the multi-waiter list in a "lock-free" - friendly manner.
	//
	// Here - we could have checked if it was locked in the main code path and avoid
	// avoided the conflict (recovering elsewhere)
	// BUT
	// It DID NOT WORK !!! - so 
	// *** DO NOT LOCK ***
	// 
   //if ( LockSharedWaiter_Async() == 0)
   {
      // Shared waiters were locked by us (Async code path), we can access them now.
      if (handleobj->numMultiWaiters > 0)
      {
         // Event is blocked on WaitForMultipleObjects.
         int i;
         int num = handleobj->numMultiWaiters;
         for (i = (num-1); i >= 0; i--)
         {
            SignalSharedWaiter( handleobj->multiWaitHandles[i]);
         }
      }
      //UnlockSharedWaiter_Async();
      return TRUE;
      
   }
   //UnlockSharedWaiter_Async();
   // None signalled.
   return FALSE;
}

HANDLE OpenEvent( LONG desiredAccess, BOOL bInheritHandle, const char *name)
{
	// Not suported.
	return NULL;
}

BOOL SetEventObject( PHANDLEOBJ hEvent)
{
	BOOL status = FALSE;
 
	pthread_mutex_lock( &hEvent->mutex);
	hEvent->state |= STATE_SIGNAL_SET;

	if ( pthread_cond_broadcast( &hEvent->event) == 0)
	{
		status = TRUE;
	}
   SignalSharedWaiters(hEvent);
	
	pthread_mutex_unlock( &hEvent->mutex);

	return status;
}

BOOL SetKernelEventObject( volatile PHANDLEOBJ hEvent)
{
	BOOL status = FALSE;

   // Kernel event object is a semaphore now. 
  	hEvent->state |= STATE_SIGNAL_SET;  
	//?????InterlockedIncrement( &hEvent->sem_currentcount);  //????????????debug??????
   sem_post(&hEvent->semaphore);
   status = SignalSharedWaiters_Async(hEvent);

	return status;
}

BOOL SetEvent( HANDLE hEvent)
{
	BOOL status = FALSE;

	// Check if our thread has been cancelled. No need to proceed.
	pthread_testcancel();

	if ( (hEvent == INVALID_HANDLE_VALUE) || (hEvent == NULL))
		return status;

	if ( VALID_HANDLE( Event, hEvent) || VALID_HANDLE( FileObject, hEvent) || VALID_HANDLE( DeviceObject, hEvent) || VALID_HANDLE( Thread, hEvent))
	{
 		return SetEventObject( HANDLE_REF(hEvent));
	}
	else
	{
		// "Event" for kernel notifier is actually a posix sempahore.
		if ( VALID_HANDLE( Semaphore, hEvent) )
		{
			if (HANDLE_REF(hEvent)->eventType == KernelNotification)
			{
				return SetKernelEventObject( HANDLE_REF(hEvent));
			}
		}
	}
	return status;
}


BOOL ResetEvent( HANDLE hEvent)
{
	BOOL status = FALSE;

	// Check if our thread has been cancelled. No need to proceed.
	pthread_testcancel();

	if ( (hEvent == INVALID_HANDLE_VALUE) || (hEvent == NULL))
		return status;

	if ( VALID_HANDLE( Event, hEvent) || VALID_HANDLE( FileObject, hEvent) || VALID_HANDLE( DeviceObject, hEvent) || VALID_HANDLE( Thread, hEvent) )
	{
		pthread_mutex_lock( &HANDLE_REF(hEvent)->mutex);
		HANDLE_REF(hEvent)->state &= ~(STATE_SIGNAL_SET | STATE_SIGNAL_PULSED);
		status = TRUE;
		pthread_mutex_unlock( &HANDLE_REF(hEvent)->mutex);
	}
	return status;
}

BOOL PulseEvent( HANDLE hEvent )
{
	BOOL status = FALSE;
	int timeout = 10;
	volatile int *refPtr;

	// Check if our thread has been cancelled. No need to proceed.
	pthread_testcancel();

	if ( (hEvent == INVALID_HANDLE_VALUE) || (hEvent == NULL))
		return status;

	if ( VALID_HANDLE( Event, hEvent) || VALID_HANDLE( FileObject, hEvent) || VALID_HANDLE( DeviceObject, hEvent) || VALID_HANDLE( Thread, hEvent))
	{
		pthread_mutex_lock( &HANDLE_REF(hEvent)->mutex);
		HANDLE_REF(hEvent)->state |= STATE_SIGNAL_PULSED;

		// Signal everyone waiting (based on refCount).
		if ( pthread_cond_broadcast( &HANDLE_REF(hEvent)->event) == 0)
		{
			status = TRUE;
		}
      SignalSharedWaiters(HANDLE_REF(hEvent));
		pthread_mutex_unlock( &HANDLE_REF(hEvent)->mutex);

      // Check that refCount becomes 1 (no one waiting) and then clear the event.
		refPtr = (volatile int *)&HANDLE_REF(hEvent)->refCount;
		while( (*refPtr > 1) && (timeout-- > 0) )
		{
			Sleep(0);
		}
	
      status = (*refPtr > 1) ? FALSE : TRUE;
	
	}
	return status;

}

BOOL ReleaseMutex( HANDLE hMutex)
{
	BOOL status = FALSE;

	// Check if our thread has been cancelled. No need to proceed.
	pthread_testcancel();

	if ( (hMutex == INVALID_HANDLE_VALUE) || (hMutex == NULL))
		return status;

	if ( VALID_HANDLE( Mutex, hMutex) || VALID_HANDLE( NamedMutex, hMutex) )
	{
		// Still need to release ownership of the mutex.
		if ( VALID_HANDLE( Mutex, hMutex))
		{
			if (pthread_mutex_unlock( &HANDLE_REF(hMutex)->mutex) == 0)
			{
				status = TRUE;
			}
		}
		else if ( VALID_HANDLE( NamedMutex, hMutex) )
		{
			// Named mutex implemented as a SystemV semaphore with a "key_t" generated
			// by the mutex name.
			LONG prev;
			status = ReleaseSemaphore( hMutex, 1, &prev);
		}

      SignalSharedWaiters( HANDLE_REF(hMutex) );
	}
	return status;
}


BOOL ReleaseSemaphoreObject( PHANDLEOBJ hSemaphore, LONG lReleaseCount, LONG *pPreviousCount)
{
	BOOL status = FALSE;

	hSemaphore->sem_currentcount = SemGetValue( hSemaphore);

	if ( pPreviousCount != NULL)
	{
		*pPreviousCount = hSemaphore->sem_currentcount;
	}
	if ( (lReleaseCount > 0) && (lReleaseCount < hSemaphore->sem_maxcount) )
	{
		hSemaphore->sem_currentcount = lReleaseCount;
	}

   if (SemPostObject(hSemaphore) == 0)
	{
		status = TRUE;
	}

   SignalSharedWaiters(hSemaphore);
	
	return status;
}

BOOL ReleaseSemaphore( HANDLE hSemaphore, LONG lReleaseCount, LONG *pPreviousCount)
{
	BOOL status = FALSE;

	// Check if our thread has been cancelled. No need to proceed.
	pthread_testcancel();

	if ( (hSemaphore == INVALID_HANDLE_VALUE) || (hSemaphore == NULL))
		return status;

	if ( VALID_HANDLE( Semaphore, hSemaphore) ||
		  VALID_HANDLE( NamedSemaphore, hSemaphore) ||
		  VALID_HANDLE( NamedMutex, hSemaphore) )
	{
   	if ( HANDLE_REF(hSemaphore)->eventType == KernelNotification)
      {
			return SetKernelEventObject( HANDLE_REF(hSemaphore));
      }
      else
      {
         return ReleaseSemaphoreObject( HANDLE_REF(hSemaphore), lReleaseCount, pPreviousCount);
      }
	}
	return status;
}
