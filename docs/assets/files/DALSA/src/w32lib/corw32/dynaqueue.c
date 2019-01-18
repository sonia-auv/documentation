/****************************************************************************** 
Copyright (c) 2010, DALSA Corp.
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

File : dynaqueue.c
	Dynamic queue handling.


******************************************************************************/

/*! \file dynaqueue.c
\brief Dynamic queue handling functions.

*/

#include "corposix.h"
#include "dynaqueue.h"

//=============================================================================
// Queue Object signalling helper functions.
//
//=============================================================================
//
// Signal a change in the queue.
//! .
//
/*!
	This function is used to signal the queue that data has arrived.
	\param [in] pQueue	Pointer to the queue object (data structure).
	\return None.
*/
static void _DQueueSetEvent( PDQUEUE pQueue )
{
	if (pQueue != NULL)
	{
#if !defined(POSIX_HOSTPC)
		SetEvent( pQueue->waitEvent);	
#else
		pthread_mutex_lock(&pQueue->wait_signal_mutex);
		pQueue->dataReady = TRUE; 
		//pthread_cond_signal(&pQueue->wait_signal_cv);  
		pthread_cond_broadcast(&pQueue->wait_signal_cv);  
		pthread_mutex_unlock(&pQueue->wait_signal_mutex); 
#endif
	}
}

// Wait for a change in the queue.
//! .
//
/*!
	This function is used to wait for the queue's event mechanism to be 
	signalled.
	\param [in] pQueue	Pointer to the queue object (data structure).
	\param [in] pTimeout	Pointer to a "struct timeval" for the timeout.
								(Timeout is infinite if this is NULL).
	\return None.
*/

static int _DQueueWaitForEvent( PDQUEUE pQueue, struct timeval *pTimeout )
{
	if (pQueue != NULL)
	{
#if !defined(POSIX_HOSTPC)
		LONG millisecs = INFINITE;
		if ( pTimeVal != NULL )
		{
			millisecs = (pTimeval->tv_sec * 1000) + (pTimeval->tv_usec / 1000);
		}
		return (int)WaitForSingleObject( pQueue->waitEvent, millisecs);
#else
		int waitresult = 0;
		if (!pQueue->dataReady)
		{
			if (pTimeout != NULL)
			{
				struct timeval ts;
				struct timespec tmout;
				long int usecs, newusecs;

				//gettimeofday(&ts, NULL);
				GetCurrentClockTime(&ts);
				usecs = pTimeout->tv_sec * 1000000 + pTimeout->tv_usec;
				newusecs = ts.tv_usec + (usecs % 1000000);
				tmout.tv_sec = ts.tv_sec + (usecs / 1000000) + (newusecs / 1000000);
				tmout.tv_nsec = (newusecs % 1000000) * 1000;
				
				// Do the timed wait.
				pthread_mutex_lock(&pQueue->wait_signal_mutex); 
				waitresult = pthread_cond_timedwait( &pQueue->wait_signal_cv, &pQueue->wait_signal_mutex, &tmout);
				while ((waitresult == EINTR) && (!pQueue->dataReady))
				{
					waitresult = pthread_cond_timedwait( &pQueue->wait_signal_cv, &pQueue->wait_signal_mutex, &tmout);
				}
				pthread_mutex_unlock(&pQueue->wait_signal_mutex); 
	 		}
			else
			{
				// Wait forever.
				pthread_mutex_lock(&pQueue->wait_signal_mutex); 
				waitresult = pthread_cond_wait(&pQueue->wait_signal_cv, &pQueue->wait_signal_mutex); 
				while ((waitresult == EINTR) && (!pQueue->dataReady))
				{
					waitresult = pthread_cond_wait(&pQueue->wait_signal_cv, &pQueue->wait_signal_mutex); 
				}
				pthread_mutex_unlock(&pQueue->wait_signal_mutex); 
			}
		}
		//if ((waitresult == 0) || pQueue->dataReady)
		if (pQueue->dataReady)
		{
			return WAIT_OBJECT_0;
		}
		else 
		{
			return WAIT_TIMEOUT;
		}
#endif
	}
	return WAIT_FAILED;
}


//=============================================================================
// Queue Object Creation/Destruction functions
//
//=============================================================================
//
// Create a queue.
//! .
//
/*!
	This function is used to create a queue to contain a maximum of N pointers
	to memory. The operational mode of the queue is also specified.
	\param [in] maxElements Maximum number of objects that can be queued.
	\param [in] mode			QueueMode value for the queue (Overwrite / Block mode)
	\return A pointer to a valid DQUEUE object on success, NULL on failure.
	\note Pointers are 32 bits (in 32-bit environments) and 
         64 bits (in 64-bit environments).
*/
PDQUEUE DQueueCreate( int maxElements, QueueMode mode)
{
	PDQUEUE pQueue = NULL;

	pQueue = (PDQUEUE)malloc(sizeof(DQUEUE));
	if ((pQueue != NULL) && (maxElements > 0))
	{
		size_t size =  sizeof(DYNAMIC_FIFO) + (maxElements+1)*sizeof(ULONG_PTR);

		// Allocate the dynamic fifo space.
		pQueue->dFifo = (PDYNAMIC_FIFO) malloc(size);
		if ( pQueue->dFifo != NULL )
		{
			// Set up the queue's internals.
			// Set up the FIFO.
			INIT_DYNAMIC_FIFO( pQueue->dFifo, (maxElements+1));
	
			// Set up the access controls.
			pQueue->mode = (mode == Block) ? Block : Overwrite;
			InitializeCriticalSection( &pQueue->cSection );
#if !defined(POSIX_HOSTPC)
			pQueue->waitEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
#else
			pthread_mutex_init( &pQueue->wait_signal_mutex, NULL); 
			pthread_condattr_init( &pQueue->wait_signal_cvattr);
#if defined(USE_MONOTONIC_CLOCK)
			pthread_condattr_setclock( &pQueue->wait_signal_cvattr, CLOCK_MONOTONIC);
#endif		 
			pthread_cond_init( &pQueue->wait_signal_cv, &pQueue->wait_signal_cvattr); 
			pthread_mutex_unlock(&pQueue->wait_signal_mutex); 
			pQueue->dataReady = FALSE;
#endif
			pQueue->valid = TRUE;
		}
		else
		{
			// Allocation failed. Cleanup.
			free(pQueue);
			pQueue = NULL;
		}
	}
	return pQueue;
}


//
// Destroy a queue.
//! .
//
/*!
	This function is used to destroy a queue.
	\param [in] qQueue	Pointer to the queue object to be destroyed.
	\return 0 on success, -1 on error.
	\note None
*/
int DQueueDestroy( PDQUEUE pQueue )
{
	int ret = -1;

	if (pQueue != NULL)
	{
		// Signal that we are destroying the queue.
		EnterCriticalSection( &pQueue->cSection );
		pQueue->valid = FALSE;
		_DQueueSetEvent( pQueue );
		LeaveCriticalSection( &pQueue->cSection );

		// Close the wait event and free up the queue resources.
#if !defined(POSIX_HOSTPC)
		CloseHandle(pQueue->waitEvent);
#else
		pthread_mutex_destroy(&pQueue->wait_signal_mutex); 
		pthread_cond_destroy(&pQueue->wait_signal_cv); 
#endif
		if (pQueue->dFifo != NULL)
		{
			free(pQueue->dFifo);
		}
		DeleteCriticalSection( &pQueue->cSection );
		free(pQueue);
		ret = 0;
	}
	return ret;
}

//
// Query information about a queue.
//! .
//
/*!
	This function is used to query information about a queue.
	\param [in]  qQueue       Pointer to the queue object to be destroyed.
	\param [out] qLength	     Pointer to hold the total number of elements allowed in the queue.
	\param [out] numQueued    Pointer to hold the current number of elements in the queue.
	\param [out] qNext        Pointer to hold the pointer to the next element in the queue (for peeking).
	\return None.
	\note If any of the pointers for the returned data are NULL, no data will be returned for that 
         data item. If the queue is not valid, it will be reported as a zero length, empty queue, 
         for zero sized elements with the next element being NULL.
*/

void	DQueueQuery( PDQUEUE pQueue, PUINT32 qLength, PUINT32 numQueued, PVOID *qNext)
{
	if (pQueue == NULL)
	{
		if (qLength != NULL) *qLength = 0;
		if (numQueued != NULL) *numQueued = 0;
		if (qNext != NULL) *qNext = NULL;
	}
	else
	{
		EnterCriticalSection(&pQueue->cSection);
		if (!pQueue->valid)
		{
			// Queue is being destroyed.
			if (qLength != NULL) *qLength = 0;
			if (numQueued != NULL) *numQueued = 0;
			if (qNext != NULL) *qNext = NULL;
		}
		else
		{
			// Fill in the valid return entries.
			if (qLength != NULL) 
			{
				*qLength = pQueue->dFifo->Size - 1;
			}
			if (numQueued != NULL) 
			{
				*numQueued = FIFO_DATA_LEVEL(pQueue->dFifo);
			}
			if (qNext != NULL) 
			{
				ULONG_PTR value = 0;
				PEEK_FIFO( pQueue->dFifo, &value);
				*qNext = (void *)value;
			}
		}
		LeaveCriticalSection(&pQueue->cSection);
	}
}


//
// Post an element to a queue.
//! .
//
/*!
	This function is used to post an element to the head of a queue (in FIFO order).
	\param [in] pQueue        Pointer to the queue object.
	\param [in] element	     Pointer to the element to be added to the queue.
	\return Possible error returns are :
				QUEUE_NO_ERROR				0   
				QUEUE_ERROR_INVALID		-1 : Queue is invalid.
				QUEUE_ERROR_FULL			-2	: Queue is full and queue mode is "Block"
				QUEUE_ERROR_NULL			-3 : Element to be added to queue is NULL.
	\note If the queue is full and the queue mode is Overwrite, the element at the tail 
         of the queue will be lost. This would be the oldest data item if the queue is
         always accessed in FIFO mode. If the queue is accessed in FIFO and LIFO mode
         (for regular and high priority elements), care should be taken when using the
         Overwrite mode.
*/

int	DQueuePost( PDQUEUE pQueue, void *element)
{
	int status = QUEUE_ERROR_INVALID;

	if (pQueue != NULL)
	{
		if (pQueue->valid)
		{
			if ( element != NULL )
			{
				EnterCriticalSection(&pQueue->cSection);
				if ( FIFO_FREE_SPACE( pQueue->dFifo) > 0 )
				{
					PUSH_FIFO( pQueue->dFifo, (ULONG_PTR)element);
					_DQueueSetEvent( pQueue );
					status = QUEUE_NO_ERROR;
				}
				else
				{
					if (pQueue->mode == Overwrite)
					{
						ULONG_PTR dummy;
						POP_FIFO( pQueue->dFifo, &dummy);
						PUSH_FIFO( pQueue->dFifo, (ULONG_PTR)element);
						_DQueueSetEvent( pQueue );
						status = QUEUE_NO_ERROR;
					}
					else
					{
						status = QUEUE_ERROR_FULL;
					}
				}
				LeaveCriticalSection(&pQueue->cSection);
			}
			else
			{
				status = QUEUE_ERROR_NULL;
			}
		}
	}
	return status;
}

//
// Post an element to the front of a queue.
//! .
//
/*!
	This function is used to post an element to the tail of a queue (in LIFO order).
	\param [in] pQueue        Pointer to the queue object.
	\param [in] element	     Pointer to the element to be added to the queue.
	\return Possible error returns are :
				QUEUE_NO_ERROR				0   
				QUEUE_ERROR_INVALID		-1 : Queue is invalid.
				QUEUE_ERROR_FULL			-2	: Queue is full and queue mode is "Block"
				QUEUE_ERROR_NULL			-3 : Element to be added to queue is NULL.
	\note If the queue is full and the queue mode is Overwrite, the element at the head 
         of the queue will be lost. This would be the oldest data item if the queue is
         always accessed in LIFO mode. If the queue is accessed in FIFO and LIFO mode
         (for regular and high priority elements), care should be taken when using the
         Overwrite mode.
*/

int	DQueuePostFront( PDQUEUE pQueue, void *element)
{
	int status = QUEUE_ERROR_INVALID;

	if (pQueue != NULL)
	{
		if (pQueue->valid)
		{
			EnterCriticalSection(&pQueue->cSection);
			if ( FIFO_FREE_SPACE( pQueue->dFifo) != 0 )
			{
				if ( element != NULL)
				{
					PUSH_LIFO( pQueue->dFifo, (ULONG_PTR)element);
					_DQueueSetEvent( pQueue );
					status = QUEUE_NO_ERROR;
				}
				else
				{
					status = QUEUE_ERROR_NULL;
				}
			}
			else
			{
				if (pQueue->mode == Overwrite)
				{
					ULONG_PTR dummy;
					POP_HEAD( pQueue->dFifo, &dummy);
					PUSH_LIFO( pQueue->dFifo,(ULONG_PTR)element);
					_DQueueSetEvent( pQueue );
					status = QUEUE_NO_ERROR;
				}
				else
				{
					status = QUEUE_ERROR_FULL;
				}
			}
			LeaveCriticalSection(&pQueue->cSection);
		}
	}
	return status;
}


//
// Get an element from a queue.
//! 
//
/*!
	This function is used to obtain an element from the head of a queue (with timeout).
   It uses the struct timeval as the timeout.
	\param [in] qQueue		Pointer to the queue object.
	\param [in] pTimeout		Pointer to a timeval structure for the time to wait for an empty queue to have data. (NULL for INFINITE).
	\return Pointer to the element popped from the queue 
			  (NULL if a timeout occurs or the queue is invalid).

	\note A timeout of 0 can be used to operate in non-blocking mode.
*/

void *DQueuePendEx( PDQUEUE pQueue, struct timeval *pTimeout)
{
	void *element = NULL;

	if (pQueue != NULL)
	{
		if (pQueue->valid)
		{
			EnterCriticalSection(&pQueue->cSection);
			if ( FIFO_DATA_LEVEL( pQueue->dFifo ) > 0 )
			{
				ULONG_PTR value = 0;
				POP_FIFO( pQueue->dFifo, &value);
				element = (void *)value;
				LeaveCriticalSection(&pQueue->cSection);
			}
			else
			{
				// Queue is empty. Wait for it to get data.
				pQueue->dataReady = FALSE;

				LeaveCriticalSection(&pQueue->cSection);
				if (_DQueueWaitForEvent( pQueue, pTimeout) == 0)
				{
					// Wait succeeded see if queue is still valid.
					if (pQueue->valid)
					{
						EnterCriticalSection(&pQueue->cSection);
						if ( FIFO_DATA_LEVEL( pQueue->dFifo ) > 0 )
						{
							ULONG_PTR value = 0;
							POP_FIFO( pQueue->dFifo, &value);
							element = (void *)value;
						}
						LeaveCriticalSection(&pQueue->cSection);
					}
				}
			}
		}
	}
	return element;
}


//
// Get an element from a queue.
//! 
//
/*!
	This function is used to obtain an element from the head of a queue (with timeout)
	\param [in] qQueue		Pointer to the queue object.
	\param [in] timeout_ms	Time (in msec) to wait for an empty queue to have data. (-1 for INFINITE).
	\return Pointer to the element popped from the queue 
			  (NULL if a timeout occurs or the queue is invalid).

	\note A timeout of 0 can be used to operate in non-blocking mode.
*/

void *DQueuePend( PDQUEUE pQueue, int timeout_ms)
{
	struct timeval timeout;
	struct timeval *pTimeout = NULL;

	if (timeout_ms != -1)
	{
		timeout.tv_sec  = timeout_ms/1000;
		timeout.tv_usec = (timeout_ms - (timeout.tv_sec * 1000)) * 1000;
		pTimeout = &timeout;
	}
	return DQueuePendEx( pQueue, pTimeout);
}


