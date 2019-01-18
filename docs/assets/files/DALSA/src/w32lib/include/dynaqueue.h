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

File : dynaqueue.h
	Dynamic queue handling.


******************************************************************************/

/*! \file dynaqueue.h
\brief Dynamic queue handling functions.

*/

#ifndef __DYNAQUEUE_H__
#define __DYNAQUEUE_H__

#include "cordef.h"
#include "pthread.h"
#include <sys/time.h>

typedef struct _DYNAMIC_FIFO
{
	INT32			Size;
	INT32			Head;
	INT32			Tail;
	ULONG_PTR	Data[1];
} DYNAMIC_FIFO, *PDYNAMIC_FIFO;

// Macros for accessing dynamic FIFOs.

#define ZERO(a,len) memset( (void *)(a), 0, (len))

#define INIT_DYNAMIC_FIFO(Fifo, size) \
	do \
   { \
      PDYNAMIC_FIFO f = (Fifo); \
      int s = (size); \
      ZERO(f,(sizeof(DYNAMIC_FIFO )+s*sizeof(ULONG_PTR))); \
      f->Size = s; \
   } while(0)

// Push to head of fifo
#define PUSH_FIFO( f, Value )                               \
	do                                                       \
	{                                                        \
      PDYNAMIC_FIFO Fifo = (f);                      \
      int head = Fifo->Head;                                \
      int size = Fifo->Size;                                \
		Fifo->Data[head] = Value;                             \
		Fifo->Head = ( (head + 1) == size ) ? 0 : (head + 1); \
	} while(0)

// Pop from tail of fifo
#define POP_FIFO( f, pValue)                                \
	do                                                       \
	{                                                        \
      PDYNAMIC_FIFO Fifo = (f);                      \
      int tail = Fifo->Tail;                                \
      int size = Fifo->Size;                                \
		*(pValue) = Fifo->Data[tail];                         \
		Fifo->Tail = ( (tail + 1) == size ) ? 0 : (tail + 1); \
	} while(0)

#define PEEK_FIFO( f, pValue)                                \
	do                                                       \
	{                                                        \
      PDYNAMIC_FIFO Fifo = (f);                      \
      int tail = Fifo->Tail;                                \
		*(pValue) = Fifo->Data[tail];                         \
	} while(0)

// Push to tail of fifo.
#define PUSH_LIFO( f, Value )                               \
	do                                                       \
	{                                                        \
      PDYNAMIC_FIFO Fifo = (f);                      \
      int tail = Fifo->Tail;                                \
      int size = Fifo->Size;                                \
		Fifo->Tail = ( (tail - 1) < 0 ) ? (size - 1) : (tail - 1); \
		Fifo->Data[Fifo->Tail] = Value;                             \
	} while(0)

// Pop from head of fifo
#define POP_HEAD( f, pValue)                                \
	do                                                       \
	{                                                        \
      PDYNAMIC_FIFO Fifo = (f);                      \
      int head = Fifo->Head;                                \
      int size = Fifo->Size;  												\
		Fifo->Head = ( (head - 1) < 0 ) ? (size - 1) : (head - 1); \
		*(pValue) = Fifo->Data[Fifo->Head];                         \
	} while(0)

#define FIFO_DATA_LEVEL( f ) \
   ({ \
      int __ret = 0; \
      PDYNAMIC_FIFO Fifo = (f); \
      int head = Fifo->Head; \
      int tail = Fifo->Tail; \
      __ret = (head >= tail ) ? (head - tail) : (Fifo->Size - (tail - head)); \
      __ret; \
   })

#define FIFO_FREE_SPACE( f ) \
   ({ \
      int __ret = 0; \
      PDYNAMIC_FIFO Fifo = (f); \
      int head = Fifo->Head; \
      int tail = Fifo->Tail; \
      __ret = (head >= tail ) ? (Fifo->Size - (head - tail) - 1) : (tail - head - 1); \
      __ret; \
   })


typedef enum
{
	Overwrite = 1, /* Adding elements to a full queue overwrites (loses) the oldest elements */
	Block     = 2	/* Adding elements to a full queue fails. */
} QueueMode;



typedef struct _DQUEUE
{
	CRITICAL_SECTION   cSection;
#if defined(POSIX_HOSTPC)
	pthread_condattr_t wait_signal_cvattr;
   pthread_cond_t     wait_signal_cv;
   pthread_mutex_t    wait_signal_mutex;
	BOOL					 dataReady;
#else
	HANDLE				 waitEvent; 
#endif
	QueueMode			 mode;
	int					 valid;
	PDYNAMIC_FIFO		 dFifo;
} DQUEUE, *PDQUEUE;

#define QUEUE_NO_ERROR			0		// No error
#define QUEUE_ERROR_INVALID	-1		// Queue is invalid.
#define QUEUE_ERROR_FULL		-2		// Queue is full and queue mode is "Block"
#define QUEUE_ERROR_NULL		-3		// Element to be added to queue is NULL.


//=============================================================================
// Queue Object Creation/Destruction prototypes
//
//=============================================================================
PDQUEUE DQueueCreate( int maxElements, QueueMode mode);
int     DQueueDestroy( PDQUEUE pQueue );
void	  DQueueQuery( PDQUEUE pQueue, PUINT32 qLength, PUINT32 numQueued, PVOID *qNext);
int	  DQueuePost( PDQUEUE pQueue, void *element);
int	  DQueuePostFront( PDQUEUE pQueue, void *element);
void   *DQueuePendEx( PDQUEUE pQueue, struct timeval *pTimeout);
void   *DQueuePend( PDQUEUE pQueue, int timeout_ms);

#endif 


