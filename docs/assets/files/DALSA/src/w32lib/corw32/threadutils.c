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

threadutils.c

Description:
   Posix thread compatibility layer with Win32.

Platform:
	-Generic Posix.

History:
   1.00 May 6, 2004, parhug

$Log: threadutils.c $
Revision 1.4  2006/03/07 09:56:32  PARHUG
On thread exit, don't delete the TLS key (threadKey) until after the exit event is signalled.
(The threadKey identifies / protects the handle we are signalling on,
deleting the key frees the thread-local-storage we are using for the handle).
Revision 1.3  2004/10/25 10:27:37  BOUERI
- Remove the close handle when thread exit. (Patch)
Revision 1.2  2004/10/06 17:00:01  BOUERI
- Added thread event.
Revision 1.1  2004/08/19 12:26:10  parhug
Initial revision

*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sched.h>

#include <corposix.h>

#include <linuxobjs.h>

#ifndef NEW_SAPERA
#define priority_class 	filedes
#define thread_priority file_attributes
#endif


// INTERNAL DEFINE
typedef unsigned int (*THREAD_CONSTRUCT)(void *);
typedef void (*THREAD_DECTRUCT)(void *);

typedef struct
{
   THREAD_CONSTRUCT pFct; // IN
   void *arglist;         // IN
	pthread_attr_t	attr;   // IN
   sem_t semaphor;        // Sync
   HANDLE threadHandle;   // OUT
} COR_BEGIN_THREAD, *PCOR_BEGIN_THREAD;

// Thread local storage data
typedef struct
{
   HANDLE 			threadHandle;
	pthread_t		thread;
	pthread_attr_t	attr;
	int				priorityClass;
	int				priority;
} COR_TLS_DATA, *PCOR_TLS_DATA;


static pthread_key_t threadKey = {0};
static LONG initCount = 0;


static void _exitProc(void *data)
{
   LONG curentCount;
	COR_TLS_DATA *tls = data;

	if (tls != NULL)
	{
		// Get thread handle and close it.
		HANDLE threadHandle = tls->threadHandle;
		if(VALID_HANDLE(Thread,threadHandle) &&
		   pthread_equal(HANDLE_REF(threadHandle)->thread_id, pthread_self()) )
		{
		   SetEvent(threadHandle);
		}

		// Free the TLS data
		free(tls);

		// See if the key is no longer being used.
		curentCount = InterlockedDecrement(&initCount);
		if(curentCount == 0)
		{
		   pthread_key_delete(threadKey);
		}
	}
}

static unsigned int _intProc(void * thPrm)
{
   HANDLE handle;
   THREAD_CONSTRUCT pFct;
   BOOL success = TRUE;
   void *arglist;
   PCOR_BEGIN_THREAD pThreadPrm = (PCOR_BEGIN_THREAD)thPrm;
   LONG curentCount;
	COR_TLS_DATA *tls = NULL;


   // Save info
   pFct = pThreadPrm->pFct;
   arglist = pThreadPrm->arglist;

   handle = CreateThreadHandle();
   if(!VALID_HANDLE(Thread, handle))
   {
      success = FALSE;
   }

   // Init KEY
   if(success == TRUE)
   {
      curentCount = InterlockedIncrement(&initCount);
      if(curentCount == 1)
      {
         if(pthread_key_create(&threadKey, _exitProc) != 0)
         {
            success = FALSE;
         }
      }
   }

   if(success == FALSE)
   {
      if(!VALID_HANDLE(Thread, handle))
      {
         CloseHandle(handle);
         curentCount = InterlockedDecrement(&initCount);
         if(curentCount == 0)
         {
            pthread_key_delete(threadKey);
         }
      }
      sem_post(&pThreadPrm->semaphor);
      return 1;
   }

	// Create thread local storage (TLS) data.
	tls = (COR_TLS_DATA *)malloc(sizeof(COR_TLS_DATA));
	tls->threadHandle = handle;
	tls->attr = pThreadPrm->attr;
	tls->priorityClass = NORMAL_PRIORITY_CLASS;
	tls->priority = THREAD_PRIORITY_NORMAL;

   // Save thread local data (TLS) parameter structure in the key
   if(success == TRUE)
   {
      if( pthread_setspecific(threadKey, tls) != 0)
      {
         success = FALSE;
      }
   }

   pThreadPrm->threadHandle = handle;

   sem_post(&pThreadPrm->semaphor);
   // pThreadPrm in no longer valid after signaling the semaphor

   return pFct(arglist);

}

unsigned long _beginthread( THREAD_CONSTRUCT threadfunc,
							unsigned int stacksize, void *arglist )
{
	unsigned int threadid;
	return _beginthreadex( NULL, stacksize, threadfunc, arglist, 0, &threadid);

}


unsigned long _beginthreadex( void *security, unsigned int stacksize,
					THREAD_CONSTRUCT threadfunc, void *arglist,
					unsigned int initflag, unsigned int *threadid)
{
	pthread_t		thread;
	pthread_attr_t	attr;
	size_t			curstacksize = (size_t)0;
	size_t			newstacksize = (size_t)0;
   LONG           millisecs = 15;
   COR_BEGIN_THREAD threadPrm;
   HANDLE handle = INVALID_HANDLE_VALUE;
   struct timespec sleeptime;
	int status;


	pthread_attr_init( &attr);
	pthread_attr_getstacksize( &attr, &curstacksize);
	if ( curstacksize < stacksize )
	{
		newstacksize = (size_t)stacksize;
		pthread_attr_setstacksize( &attr, newstacksize);
	}

	// Set up attributes for thread creation.
 	// System scope so realtime threads will be big time high priority
	// Always use the detached state. We don't do any "join" operations.
	pthread_attr_setscope( &attr, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED);

	// Store info in thread context.
   threadPrm.pFct = threadfunc;
   threadPrm.arglist = arglist;
   threadPrm.threadHandle = INVALID_HANDLE_VALUE;
   threadPrm.attr = attr;

   if( sem_init (&threadPrm.semaphor, 0, 0) == -1)
   {
		return (unsigned long) INVALID_HANDLE_VALUE;
   }

	status = pthread_create( &thread, &attr, (void *)_intProc, &threadPrm);

   sleeptime.tv_sec = 0;
	sleeptime.tv_nsec = 1000000;
   while (millisecs >= 0)
	{
      struct timespec remainder = {0};
      if(sem_trywait(&threadPrm.semaphor) == 0)
      {
         break;
      }
		nanosleep( &sleeptime, &remainder );
		millisecs -= ((sleeptime.tv_sec - remainder.tv_sec)*1000  + (sleeptime.tv_nsec - remainder.tv_nsec)/1000000);
   }

   sem_destroy(&threadPrm.semaphor);

   handle = threadPrm.threadHandle;

	if (status == 0)
	{
      if(handle == INVALID_HANDLE_VALUE)
      {
         pthread_kill(thread,SIGKILL);
      }
		else if ( threadid != NULL)
		{
         *threadid = (unsigned int)thread;
		}
		return (unsigned long) handle;
	}
	else
	{
		*threadid = 0;
		return (unsigned long) INVALID_HANDLE_VALUE;
	}
}

void _endthread(void)
{
	pthread_exit(0);
}

void _endthreadex( unsigned int retval)
{
	pthread_exit((void *)&retval);
}

/*============================================================================
Thread Priority setting / calculations

Map the pThreads dynamic min/max priority value range into the Windows
range (1-31). Based on the "Priority Class".
If the "PriorityClass" is not set - linearly map the priorities between
max and min.

Windows defines (for various the levels) are :

1  IDLE_PRIORITY_CLASS          THREAD_PRIORITY_IDLE
2  IDLE_PRIORITY_CLASS          THREAD_PRIORITY_LOWEST
3  IDLE_PRIORITY_CLASS          THREAD_PRIORITY_BELOW_NORMAL
4  IDLE_PRIORITY_CLASS          THREAD_PRIORITY_NORMAL
5  IDLE_PRIORITY_CLASS          THREAD_PRIORITY_ABOVE_NORMAL
6  IDLE_PRIORITY_CLASS          THREAD_PRIORITY_HIGHEST
15 IDLE_PRIORITY_CLASS          THREAD_PRIORITY_TIME_CRITICAL


1  BELOW_NORMAL_PRIORITY_CLASS  THREAD_PRIORITY_IDLE
4  BELOW_NORMAL_PRIORITY_CLASS  THREAD_PRIORITY_LOWEST
5  BELOW_NORMAL_PRIORITY_CLASS  THREAD_PRIORITY_BELOW_NORMAL
6  BELOW_NORMAL_PRIORITY_CLASS  THREAD_PRIORITY_NORMAL
7  BELOW_NORMAL_PRIORITY_CLASS  THREAD_PRIORITY_ABOVE_NORMAL
8  BELOW_NORMAL_PRIORITY_CLASS  THREAD_PRIORITY_HIGHEST
15 BELOW_NORMAL_PRIORITY_CLASS  THREAD_PRIORITY_TIME_CRITICAL


1  NORMAL_PRIORITY_CLASS        THREAD_PRIORITY_IDLE
6  NORMAL_PRIORITY_CLASS        THREAD_PRIORITY_LOWEST
7  NORMAL_PRIORITY_CLASS        THREAD_PRIORITY_BELOW_NORMAL
8  NORMAL_PRIORITY_CLASS        THREAD_PRIORITY_NORMAL
9  NORMAL_PRIORITY_CLASS        THREAD_PRIORITY_ABOVE_NORMAL
10 NORMAL_PRIORITY_CLASS        THREAD_PRIORITY_HIGHEST
15 NORMAL_PRIORITY_CLASS        THREAD_PRIORITY_TIME_CRITICAL


1  ABOVE_NORMAL_PRIORITY_CLASS  THREAD_PRIORITY_IDLE
8  ABOVE_NORMAL_PRIORITY_CLASS  THREAD_PRIORITY_LOWEST
9  ABOVE_NORMAL_PRIORITY_CLASS  THREAD_PRIORITY_BELOW_NORMAL
10 ABOVE_NORMAL_PRIORITY_CLASS  THREAD_PRIORITY_NORMAL
11 ABOVE_NORMAL_PRIORITY_CLASS  THREAD_PRIORITY_ABOVE_NORMAL
12 ABOVE_NORMAL_PRIORITY_CLASS  THREAD_PRIORITY_HIGHEST
15 ABOVE_NORMAL_PRIORITY_CLASS  THREAD_PRIORITY_TIME_CRITICAL

1  HIGH_PRIORITY_CLASS          THREAD_PRIORITY_IDLE
10 HIGH_PRIORITY_CLASS          THREAD_PRIORITY_LOWEST
11 HIGH_PRIORITY_CLASS          THREAD_PRIORITY_BELOW_NORMAL
12 HIGH_PRIORITY_CLASS          THREAD_PRIORITY_NORMAL
13 HIGH_PRIORITY_CLASS          THREAD_PRIORITY_ABOVE_NORMAL
14 HIGH_PRIORITY_CLASS          THREAD_PRIORITY_HIGHEST
15 HIGH_PRIORITY_CLASS          THREAD_PRIORITY_TIME_CRITICAL

16 REALTIME_PRIORITY_CLASS      THREAD_PRIORITY_IDLE
17 REALTIME_PRIORITY_CLASS      -7
18 REALTIME_PRIORITY_CLASS      -6
19 REALTIME_PRIORITY_CLASS      -5
20 REALTIME_PRIORITY_CLASS      -4
21 REALTIME_PRIORITY_CLASS      -3
22 REALTIME_PRIORITY_CLASS      THREAD_PRIORITY_LOWEST
23 REALTIME_PRIORITY_CLASS      THREAD_PRIORITY_BELOW_NORMAL
24 REALTIME_PRIORITY_CLASS      THREAD_PRIORITY_NORMAL
25 REALTIME_PRIORITY_CLASS      THREAD_PRIORITY_ABOVE_NORMAL
26 REALTIME_PRIORITY_CLASS      THREAD_PRIORITY_HIGHEST
27 REALTIME_PRIORITY_CLASS      3
28 REALTIME_PRIORITY_CLASS      4
29 REALTIME_PRIORITY_CLASS      5
30 REALTIME_PRIORITY_CLASS      6
31 REALTIME_PRIORITY_CLASS      THREAD_PRIORITY_TIME_CRITICAL

Basically - below 16 is not "realtime", above 15 is "realtime".

If the priority class has been explcitily set :
Map these into the pThreads min / max as follows :

Levels 0  -> 15 map into min -> (min + 3*(max - min)/4)
Levels 16 -> 31 map into (min + 3*(max - min)/4) -> max

If the priority class has not been explicitly set:
just map the thread priorities linearly into min->max.

============================================================================*/

static int _CalculateLocalPriority(int min, int max, int desiredPriority, int priorityClass)
{
	int merged_priority = 0;
	float delta = 0.0;
	float priority = 0.0;
	float fmin = 0.0;

	if (desiredPriority < -7)
	{
		desiredPriority = -7;
	}

	if (priorityClass == PRIORITY_CLASS_NOT_SET)
	{
		// Simple linear mapping of priorities.
		// Check inputs
		if (desiredPriority < 0)
		{
			desiredPriority += THREAD_PRIORITY_TIME_CRITICAL;
		}
		else
		{
			desiredPriority *= 2;
		}
		fmin = (float)min;
		delta = (float)(max - min);

	}
	else
	{
		// Adjust the priority based on special input cases.
		if (priorityClass == REALTIME_PRIORITY_CLASS)
		{
			delta = (float)(max - min) * 0.25;
			fmin = (float)min + 0.75 * (float)(max - min);
			if (desiredPriority == THREAD_PRIORITY_IDLE)
			{
				//desiredPriority = THREAD_PRIORITY_TIME_CRITICAL;
				desiredPriority = THREAD_PRIORITY_IDLE;
			}
			else if (desiredPriority < 0)
			{
				//desiredPriority = REALTIME_PRIORITY_CLASS + REALTIME_PRIORITY_OFFSET + THREAD_PRIORITY_NORMAL + desiredPriority;
				desiredPriority = REALTIME_PRIORITY_OFFSET + THREAD_PRIORITY_NORMAL + desiredPriority;
			}
			else
			{
				//desiredPriority = REALTIME_PRIORITY_CLASS + REALTIME_PRIORITY_OFFSET + desiredPriority;
				desiredPriority = REALTIME_PRIORITY_OFFSET + desiredPriority;
			}
		}
		else
		{
			delta = (float)(max - min) * 0.75;
			fmin = (float)min;
			if (desiredPriority < 0)
			{
				desiredPriority = THREAD_PRIORITY_NORMAL;
			}
		}
	}

	// Get a merged Priority (adjust for out of boounds)
	merged_priority = desiredPriority + priorityClass;

	if (merged_priority > (THREAD_PRIORITY_TIME_CRITICAL + REALTIME_PRIORITY_CLASS))
	{
		merged_priority = THREAD_PRIORITY_TIME_CRITICAL;
	}
	else if (merged_priority < THREAD_PRIORITY_IDLE)
	{
		merged_priority = THREAD_PRIORITY_IDLE;
	}

	// Calcualte the priority (mapped into current prioirty range).
	priority = (float)(merged_priority - THREAD_PRIORITY_IDLE) / (float)THREAD_PRIORITY_MAXIMUM;
	priority = fmin + priority*delta;
	merged_priority = (int)priority;
	return merged_priority;
}

BOOL   SetThreadPriority( HANDLE hThread, int priority)
{
   if( VALID_HANDLE(Thread,hThread) && (priority >= 1) && (priority <= THREAD_PRIORITY_TIME_CRITICAL) )
	{
		int sched;
		int min = 0;
		int max = 32;
		int new_priority = 1;
		struct sched_param param = {0};
		COR_TLS_DATA *tls = pthread_getspecific(threadKey);

		if (tls == NULL)
		{
			// Setting priority for a thread from outside the thread.
			sched = sched_getscheduler(getpid());
		}
		else
		{
			// Setting priority for a thread from inside the thread.
			pthread_attr_getschedpolicy(&tls->attr, &sched);

		}

		if (priority == THREAD_PRIORITY_TIME_CRITICAL)
		{
			// As a special case - always try to get the most out of the TIME_CRITICAL setting.
			// 
			min = sched_get_priority_min(SCHED_RR);
			max = sched_get_priority_max(SCHED_RR);
			HANDLE_REF(hThread)->priority_class = REALTIME_PRIORITY_CLASS;

			param.sched_priority = max;
			pthread_setschedparam( HANDLE_REF(hThread)->thread_id, SCHED_RR, &param);
			HANDLE_REF(hThread)->thread_priority = max;
		}
		else
		{
			min = sched_get_priority_min(sched);
			max = sched_get_priority_max(sched);
			// Check if max is 0 ----> this means SCHED_OTHER and dynamically changing priorities based on demand.
			if (max != 0)
			{
				new_priority = _CalculateLocalPriority(min, max, priority, HANDLE_REF(hThread)->priority_class);
				param.sched_priority = new_priority;
				pthread_setschedparam( HANDLE_REF(hThread)->thread_id, sched, &param);
				HANDLE_REF(hThread)->thread_priority = priority;
			}
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

HANDLE GetCurrentThread( void )
{
	HANDLE hThread = NULL;
	COR_TLS_DATA *tls = pthread_getspecific(threadKey);

	if (tls == NULL)
	{
		// Current thread not created with our thread creation model.
		// (Possibly a main program thread - just return NULL).
		return hThread;
	}
	else
	{
		hThread = tls->threadHandle;
	}

	return hThread;

}

int    GetThreadPriority( HANDLE hThread)
{
   if(VALID_HANDLE(Thread,hThread))
	{
		return HANDLE_REF(hThread)->thread_priority;
	}
	else
	{
		return THREAD_PRIORITY_ERROR_RETURN;
	}
}

BOOL   SetPriorityClass( HANDLE hThread, UINT32 priorityClass)
{
   if((VALID_HANDLE(Thread,hThread)) && ((priorityClass >= 0) && (priorityClass <= REALTIME_PRIORITY_CLASS)) )
	{
		HANDLE_REF(hThread)->priority_class = priorityClass;
		return SetThreadPriority( hThread, HANDLE_REF(hThread)->thread_priority);
	}
	else
	{
		return FALSE;
	}
}
UINT32 GetPriorityClass( HANDLE hThread)
{
   if(VALID_HANDLE(Thread,hThread))
	{
		return HANDLE_REF(hThread)->priority_class;
	}
	else
	{
		return 0;
	}
}


