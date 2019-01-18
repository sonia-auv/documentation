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

csection.c			 										

Description:
   Critical Section emulation functions.

Platform:
	-Generic Posix.

History:
   1.00 October 1st, 2003, parhug

$Log: csection.c $
Revision 1.2  2006/03/07 10:06:28  PARHUG
Restructure the wait for entering the critical section. This will work better if multiple threads are in contention. 
Revision 1.1  2004/08/19 12:26:06  parhug
Initial revision

*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>

#ifdef __CELL__
#include "corposix.h"
#else
#include <cordef.h>
#endif

/**========================================================================**/
/*
* #define constants.
*/

/**========================================================================**/
/*
* Function globals.
*/

/**========================================================================**/
/*
* Function prototypes.
*/

/**============================== CODE ====================================**/

static void ReInitializeCriticalSection( LPCRITICAL_SECTION cs)
{
	MutexType_t save;
	int kind;

	// Save the critical section tyep.
	save = cs->type;

	// Delete the critical section.
	DeleteCriticalSection( cs);

	// Initialize the type and reference counts.
	cs->type = save;
	cs->ownerThread = 0;
	cs->waitCount = 0;
	cs->refCount = 0;

	// Initialize the condition variable.
	pthread_condattr_init( &cs->cvWaiterAttributes );
	pthread_cond_init( &cs->cvWaiter, &cs->cvWaiterAttributes);

	// Initialize the mutex.
	switch (cs->type)
	{
		case Fast:
			kind = PTHREAD_MUTEX_ADAPTIVE_NP;
			break;
		case ErrorChecking:
			kind = PTHREAD_MUTEX_ERRORCHECK_NP;
			break;
		case Recursive:
		default:
			kind = PTHREAD_MUTEX_RECURSIVE_NP;
			break;
	}

	pthread_mutexattr_init( &cs->pCsMutexAttributes);
#if _USE_UNIX98
	pthread_mutexattr_settype( &cs->pCsMutexAttributes, kind);
#endif
	pthread_mutex_init( &cs->pCsMutex, &cs->pCsMutexAttributes);

	return;
}

void SetCriticalSectionMutexType( LPCRITICAL_SECTION cs, MutexType_t type)
{
	cs->type = type;
	ReInitializeCriticalSection(cs);
	return;
}

void InitializeCriticalSection( LPCRITICAL_SECTION cs)
{
	if ( cs != NULL)
	{
		// Initialize the type and reference counts.
		cs->type = Recursive;
		cs->ownerThread = 0;
		cs->waitCount = 0;
		cs->refCount = 0;

		// Initialize the condition variable.
		pthread_condattr_init( &cs->cvWaiterAttributes );
		pthread_cond_init( &cs->cvWaiter, &cs->cvWaiterAttributes);

		// Initialize the mutex.
		pthread_mutexattr_init( &cs->pCsMutexAttributes);
	#if _USE_UNIX98
		pthread_mutexattr_settype( &cs->pCsMutexAttributes, PTHREAD_MUTEX_RECURSIVE_NP);
	#endif
		pthread_mutex_init( &cs->pCsMutex, &cs->pCsMutexAttributes);
	}
	return;
}

void DeleteCriticalSection( LPCRITICAL_SECTION cs)
{
	int  timeout = 0;

	if ( cs != NULL)
	{
		if (cs->type != Undefined)
		{
			// Enter the critical section. (Cannot delete it if someone else is holding it).
			EnterCriticalSection(cs);

			// Set up to cancel everyone waiting.
			cs->type = Undefined;
			cs->ownerThread = 0;
			while ( (cs->waitCount > 0) && (timeout < CRITICAL_SECTION_ACCESS_TIMEOUT_VALUE) )
			{
				timeout++;
				pthread_cond_broadcast( &cs->cvWaiter);
				sched_yield(); // Yield the time-slice so waiters can be awakened.
			}
			cs->waitCount = 0;
			cs->refCount = 0;

			// Everyone is out (or timed out). Delete the thing.
			pthread_cond_destroy( &cs->cvWaiter);
			pthread_condattr_destroy( &cs->cvWaiterAttributes);

			pthread_mutex_unlock( &cs->pCsMutex);
			pthread_mutex_destroy( &cs->pCsMutex);
			pthread_mutexattr_destroy( &cs->pCsMutexAttributes);

			// Restore the thread cancel state.
			pthread_setcancelstate( cs->savedThreadCancelState, NULL);
		}
	}
	pthread_testcancel();
	return;
}

static BOOL ObtainCriticalSection( LPCRITICAL_SECTION cs )
{
	// Set the thread cancellation state.
	pthread_setcancelstate( PTHREAD_CANCEL_DISABLE, &cs->savedThreadCancelState);

	// Increment the reference count and set the owner information.
	cs->refCount++;
	cs->ownerThread = pthread_self();
	return TRUE;
}

void EnterCriticalSection( LPCRITICAL_SECTION cs)
{
   int saveState;
   int acquired = 0;

   if (cs != NULL)
   {
		if (cs->type != Undefined)
		{
			// Works best for Recursive mutexes.
			if ( pthread_mutex_lock( &cs->pCsMutex) == 0)
			{
				// We have acquired the mutex and can operate on the data structure.
				// If reference count is 0, we get it.
				// If we are the current owner, we get it.
			  if ( cs->refCount == 0)
				{
					if ( ObtainCriticalSection( cs) )
					{
						// We have the critical section.
						pthread_mutex_unlock( &cs->pCsMutex);
						return;
					}

				}
				else
				{
					if ( cs->ownerThread == pthread_self() )
					{
						// We already have it.
						cs->refCount++;
						pthread_mutex_unlock( &cs->pCsMutex);
						return;
					}
				}

				// FALLTHOUGH
				// Set up to wait on the critical section.
				cs->waitCount++;
				pthread_setcancelstate( PTHREAD_CANCEL_DISABLE, &saveState );
				while( cs->refCount != 0)
				{
					// Wait on the condition variable to get the critical section.
					pthread_cond_wait( &cs->cvWaiter, &cs->pCsMutex);

					// We have been signalled and now own the mutex (again.)
					// Check if the critical section hase been deleted.
					if ( cs->type == Undefined )
					{
						// It is deleted.
						pthread_setcancelstate( saveState, NULL);
						return;
					}
				}
				pthread_setcancelstate( saveState, NULL);
				if ( ObtainCriticalSection( cs) )
				{
					// We have successfully waited for the critical section.
					cs->waitCount--;
					pthread_mutex_unlock( &cs->pCsMutex);
				}
			}
      }
      else
      {
         // Can only be here if current thread already owns the mutex.
         // Assume it is the same as a recursive lock of the critical section.
         cs->refCount++;
      }
   }
   else
   {
		pthread_testcancel();
	}
   return;
}

BOOL TryEnterCriticalSection( LPCRITICAL_SECTION cs)
{
	BOOL status = FALSE;
	
	if (cs != NULL)
	{
		if (cs->type != Undefined)
		{
			// Acquire mutex (non-blocking)
			if ( pthread_mutex_trylock( &cs->pCsMutex) == 0)
			{
				// If reference count is 0, we get it.
				// If we are the current owner, we get it.
				if ( cs->refCount == 0)
				{
					cs->refCount++;
					cs->ownerThread = pthread_self();
					pthread_mutex_unlock( &cs->pCsMutex);
					status = TRUE;
				}
				else
				{
					if ( cs->ownerThread == pthread_self() )
					{
						// We get the critical section (already have it)
						cs->refCount++;
						pthread_mutex_unlock( &cs->pCsMutex);
						status = TRUE;
					}
				}
			}
		}
	}
	else
	{
		// Did not get the critical section. Get out.
		pthread_testcancel();
	}
	return status;
}

void LeaveCriticalSection( LPCRITICAL_SECTION cs)
{
	if (cs != NULL)
	{
		if (cs->type != Undefined)
		{
			// Acquire the mutex.
			if ( pthread_mutex_lock( &cs->pCsMutex) == 0)
			{
				// See if we own the critical section
				// (we should - but who knows what evil lurks in the minds of programmers).
				if ( cs->ownerThread == pthread_self() )
				{
					// We own it. See if this is the last reference (in case of recursion).
					if ( --cs->refCount == 0 )
					{
						// Last reference. We can signal anyone waiting.
						if (cs->waitCount > 0)
						{
							pthread_cond_signal( &cs->cvWaiter );
						}
						// Restore the thread cancel state.
						pthread_setcancelstate( cs->savedThreadCancelState, NULL);
					}
				}
				pthread_mutex_unlock( &cs->pCsMutex);
			}
		}
	}
	else
	{
		pthread_testcancel();
	}
	return;
}


