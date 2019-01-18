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

linuxobjs.h			 											

Description:
   Private object definitions for Win32 Compatibility layer for Posix.

Platform:
	-Posix (generic layer).

$Log: linuxobjs.h $
Revision 1.9  2006/04/21 09:51:40  PARHUG
Revision 1.8  2006/03/07 09:48:42  PARHUG
SignalSharedWaiter uses only semaphores now and they are validated elsewhere and are not in the HANDLE registry.
Revision 1.7  2005/10/12 15:33:00  PARHUG
Validate handle in macros HANDLE_IS_SIGNALLED, HANDLE_IS_OPEN, and HANDLE_IS_CLOSING.
Revision 1.6  2005/04/22 10:30:40  PARHUG
Added prototype for SetKernelEventObject.
Revision 1.5  2005/03/08 17:23:22  PARHUG
Added SignalSharedWaiter and changed the operation of SignalSharedWaiters to signal all the waiters in the array in reverse order.
Revision 1.4  2004/10/14 09:50:37  parhug
Added SignalSharedWaiters. Also a small fix for efficiency.
Revision 1.3  2004/10/07 11:23:56  BOUERI
- Remove warning.
Revision 1.2  2004/10/06 17:01:03  parhug
Update for new handle model.
Revision 1.1  2004/08/19 12:26:08  parhug
Initial revision

*******************************************************************************/

#ifndef _LINUXOBJS_H_
#define _LINUXOBJS_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#include <unistd.h>
#include <sys/time.h>
#include <sched.h>

// For Non-Linux systems.......
//#include <sys/sem.h>

#include <linux/sem.h>

#ifndef __CELL__
#include <cordef.h>
#endif

#if _SEM_SEMUN_UNDEFINED
/* arg for semctl system calls. */
union semun
{
	int val;                           //<= value for SETVAL
	struct semid_ds *buf;              //<= buffer for IPC_STAT & IPC_SET
	unsigned short int *array;         //<= array for GETALL & SETALL
	struct seminfo *__buf;             //<= buffer for IPC_INFO
	void *__pad;
};
#endif

// These are defined in sys/sem.h but we need linux/sem.h to get SEMMSL!!!
// Put prototypes here for now !!
/* Semaphore control operation.  */
extern int semctl (int __semid, int __semnum, int __cmd, ...) __THROW;

/* Get semaphore.  */
extern int semget (key_t __key, int __nsems, int __semflg) __THROW;

/* Operate on semaphore.  */
extern int semop (int __semid, struct sembuf *__sops, size_t __nsops) __THROW;

extern key_t ftok (__const char *__pathname, int __proj_id) __THROW;

///////////////////////////////////////////////////////////////////////////////////////
// General definitions
#include <corposix.h>

#define MAX_HANDLE_OBJECTS				1024
#define HANDLE_CLOSE_RETRY_TIMEOUT	16
#define MAX_POSIX_SEMAPHORE_COUNT	8
//==============================================================================
// Handle state settings.
#define STATE_OPEN				0x00000001
#define STATE_CLOSING			0x00000002
#define STATE_SIGNAL_SET		0x00000004
#define STATE_SIGNAL_PULSED	0x00000008

//==============================================================================
// General handle macros.
#define RELEASE_HANDLE(a)	\
	do \
	{ \
		PHANDLEOBJ macrohandle = (a); \
		if ( (macrohandle != NULL) && (macrohandle != INVALID_HANDLE_VALUE) ) \
		{ \
			free(macrohandle); \
		} \
	} while(0);

#define GET_NEW_HANDLE(hType,a)	\
	do \
	{ \
		PHANDLEOBJ macrohandle = (PHANDLEOBJ)malloc(sizeof(HANDLEOBJ)); \
		if ( macrohandle != NULL) \
		{ \
			macrohandle->type = hType; \
		} \
		(a) = macrohandle; \
	} while(0);

#define VALID_HANDLEOBJ(hType,a)	(((a) != INVALID_HANDLE_VALUE) && ((a) != NULL) && ( (a)->type == (hType)))

#define HANDLEOBJ_IS_SIGNALLED(h)	( ((h)->state & (STATE_OPEN)) && ((h)->state & (STATE_SIGNAL_SET | STATE_SIGNAL_PULSED)))
#define HANDLEOBJ_IS_OPEN(h)			(((h)->state & (STATE_OPEN | STATE_CLOSING)) != 0)
#define HANDLEOBJ_IS_CLOSING(h)		(((h)->state & STATE_CLOSING) != 0)

#define INCREMENT_HANDLEOBJ_REFCOUNT(h) InterlockedIncrement((LONG *)&((h)->refCount))
#define DECREMENT_HANDLEOBJ_REFCOUNT(h) InterlockedDecrement((LONG *)&((h)->refCount))

#define HANDLE_IS_VALID(a)	(((a) != NULL) && ((a) != INVALID_HANDLE_VALUE) && IsRegisteredHandle((a)) )
#define VALID_HANDLE(hType,a)	(HANDLE_IS_VALID((a)) && ( HANDLE_REF((a))->type == (hType)))

#define HANDLE_IS_SIGNALLED(h)	(HANDLE_IS_VALID((h)) && (HANDLE_REF((h))->state & (STATE_OPEN)) && (HANDLE_REF((h))->state & (STATE_SIGNAL_SET | STATE_SIGNAL_PULSED)))
#define HANDLE_IS_OPEN(h)			(HANDLE_IS_VALID((h)) && ((HANDLE_REF((h))->state & (STATE_OPEN | STATE_CLOSING)) != 0))
#define HANDLE_IS_CLOSING(h)		(HANDLE_IS_VALID((h)) && ((HANDLE_REF((h))->state & STATE_CLOSING) != 0))

#define INCREMENT_HANDLE_REFCOUNT(h) InterlockedIncrement((LONG *)&(HANDLE_REF((h))->refCount))
#define DECREMENT_HANDLE_REFCOUNT(h) InterlockedDecrement((LONG *)&(HANDLE_REF((h))->refCount))


//==============================================================================
// Prototypes
BOOL CloseFileObject( PHANDLEOBJ handle);
BOOL CloseHandleObject( PHANDLEOBJ handle);
BOOL ReleaseSemaphoreObject( PHANDLEOBJ hSemaphore, LONG lReleaseCount, LONG *pPreviousCount);
BOOL SetEventObject( PHANDLEOBJ hEvent);
BOOL SetKernelEventObject( PHANDLEOBJ hEvent);
BOOL IsRegisteredHandle( HANDLE handle);
HANDLE CreateThreadHandle(void);

int  LockSharedWaiter(void);
int  UnlockSharedWaiter(void);
int  LockSharedWaiter_Async(void);
void UnlockSharedWaiter_Async(void);


//==============================================================================
// In-line functions for semaphores (private).

static inline int SemSetValue( PHANDLEOBJ handle, int value)
{
   if (handle != NULL)
   {
      // Set current semaphore value (semval)
      if ( handle->bPosixSemaphore)
      {
         // Cannot be set. Is initialized sith sem_init at creation time.
         return 0;
      }
      else
      {
         union semun arg;
         arg.val = value;
         return semctl( handle->semid, handle->semnum, SETVAL, arg); // Set current semaphore value.
      }
   }
   return 0;
}

static inline int SemGetValue( PHANDLEOBJ handle)
{
   if (handle != NULL)
   {
      // Get current semaphore value (semval)
      if ( handle->bPosixSemaphore)
      {
         int value;
         sem_getvalue( handle->psemaphore, &value);
         return value;
      }
      else
      {
         return semctl( handle->semid, handle->semnum, GETVAL, (union semun)0);
      }
   }
   return 0;
}

static inline int SemPostObject( PHANDLEOBJ handle)
{
   if (handle != NULL)
   {
      if ( handle->bPosixSemaphore)
      {
         return sem_post( handle->psemaphore);
      }
      else
      {
         int status;
         struct sembuf sop;
   
         sop.sem_num = handle->semnum;
         sop.sem_op  = 1;
         //sop.sem_flg = IPC_NOWAIT;
			sop.sem_flg = 0; 
         do
         {
            status = semop( handle->semid, &sop, 1);
         } while ( (status != 0) && ((errno == EINTR) && (errno == EAGAIN)));
         return status;
      }
   }
   return -1;
}

static inline int SemPost( HANDLE handle)
{
	return SemPostObject(HANDLE_REF(handle));
}

static inline BOOL SignalSharedWaiter( HANDLE handle)
{
   // Signal the shared waiters (they are semaphores)
   if (HANDLE_IS_VALID(handle))
   {
     return SemPostObject(HANDLE_REF(handle));
   }
   return FALSE;
}

static inline BOOL SignalSharedWaiters( PHANDLEOBJ handleobj)
{
   if ( LockSharedWaiter() == 0)
   {
      // Shared waiters are not locked
      if (handleobj->numMultiWaiters > 0)
      {
         // Event is blocked on WaitForMultipleObjects.
         int i;
         int num = handleobj->numMultiWaiters;
         for (i = (num-1); i >= 0; i--)
         {
            SignalSharedWaiter( handleobj->multiWaitHandles[i]);
         }
         UnlockSharedWaiter();
         return TRUE;
      }
      UnlockSharedWaiter();
   }
   // None signalled.
   return FALSE;
}


#endif

