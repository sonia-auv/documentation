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

archutils.c			 										

Description:
   Architecture specific utility functions. (In assembler).

Platform:
	-Generic Posix.

History:
   1.00 October 29, 2003, parhug

$Log: archutils.c $
Revision 1.3  2006/03/07 10:07:37  PARHUG
Changed to include "linux/types.h" in kernel mode (required by 2.6 kernel).
Revision 1.2  2004/09/08 10:41:28  BOUERI
- Change the include to support the driver compilation.
Revision 1.1  2004/08/19 12:26:04  parhug
Initial revision

*******************************************************************************/
#define COR_LINUX 1
#if defined(__KERNEL__)
	#include <linux/types.h>
#else
	#include <sys/types.h>
#endif
#ifndef __CELL__
#include <cordef.h>
#endif

// Definition to use C wrapper functions (i386 and powerpc) or
// assembly lanuage functions (i386 only) for Interlocked function emulation.
#define USE_ASSEMBLY_LANGUAGE_INTERLOCK_FUNCTIONS	0

//=============================Intel-specific functions======================

#if defined( __i386__ )
//============================== Inline functions for GNU C on i386==========

static inline long interlocked_cmpxchg( LONG *dest, LONG xchg, LONG compare )
{
    long ret;
    __asm__ __volatile__( "lock; cmpxchgl %2,(%1)"
                          : "=a" (ret) : "r" (dest), "r" (xchg), "0" (compare) : "memory" );
    return ret;
}

static inline void *interlocked_cmpxchg_ptr( void **dest, void *xchg, void *compare )
{
    void *ret;
    __asm__ __volatile__( "lock; cmpxchgl %2,(%1)"
                          : "=a" (ret) : "r" (dest), "r" (xchg), "0" (compare) : "memory" );
    return ret;
}

static inline long interlocked_xchg( LONG *dest, LONG val )
{
    long ret;
    __asm__ __volatile__( "lock; xchgl %0,(%1)"
                          : "=r" (ret) : "r" (dest), "0" (val) : "memory" );
    return ret;
}

static inline void *interlocked_xchg_ptr( void **dest, void *val )
{
    void *ret;
    __asm__ __volatile__( "lock; xchgl %0,(%1)"
                          : "=r" (ret) : "r" (dest), "0" (val) : "memory" );
    return ret;
}

static inline long interlocked_xchg_add( LONG *dest, LONG incr )
{
    long ret;
    __asm__ __volatile__( "lock; xaddl %0,(%1)"
                          : "=r" (ret) : "r" (dest), "0" (incr) : "memory" );
    return ret;
}

//============================== Inline functions for GNU C on x86_64==========
#elif defined(x86_64)
static inline LONG interlocked_cmpxchg( LONG *dest, LONG xchg, LONG compare )
{
    LONG ret;
    __asm__ __volatile__( "lock; cmpxchgl %2,(%1)"
                          : "=a" (ret) : "r" (dest), "r" (xchg), "0" (compare) : "memory" );
    return ret;
}

static inline void *interlocked_cmpxchg_ptr( void **dest, void *xchg, void *compare )
{
    void *ret;
    __asm__ __volatile__( "lock; cmpxchgq %2,(%1)"
                          : "=a" (ret) : "r" (dest), "r" (xchg), "0" (compare) : "memory" );
    return ret;
}

static inline LONG interlocked_xchg( LONG *dest, LONG val )
{
    LONG ret;
    __asm__ __volatile__( "lock; xchgl %0,(%1)"
                          : "=r" (ret) : "r" (dest), "0" (val) : "memory" );
    return ret;
}

static inline void *interlocked_xchg_ptr( void **dest, void *val )
{
    void *ret;
    __asm__ __volatile__( "lock; xchgq %0,(%1)"
                          : "=r" (ret) : "r" (dest), "0" (val) : "memory" );
    return ret;
}
#if 0
static inline long interlocked_xchg_add( long *dest, long incr )
{
    long ret;
    long i = (long)incr;
    __asm__ __volatile__( "lock; xaddq %0,(%1)"
                          : "=r" (ret) : "r" (dest), "0" (i) : "memory" );
    return ret;
}
#endif
#if 0
    __asm__ __volatile__( "lock; xadd %0,(%1)"
                          : "=r" (ret), "=r" (dest) : "0" (incr), "r" (dest) );

#endif
#if 0 
//static inline LONG interlocked_xchg_add( LONG *dest, LONG incr )
static inline LONG interlocked_xchg_add( LONG *dest, LONG incr )
{
    long ret = (long)incr;
    __asm__ __volatile__( "movq %%r8, %0; lock; xaddl %%r8, %1; movq %0, %%r8"
                          : "=r" (ret) : "r" (*dest) : "memory" );
    return (LONG)ret;
}
#else
static inline LONG interlocked_xchg_add( LONG *dest, LONG incr )
{
    LONG ret;
    __asm__ __volatile__( "lock; xaddl %0,(%1)"
                          : "=r" (ret) : "r" (dest), "0" (incr) : "memory" );
    return ret;
}
#endif
#elif defined(arm_family)
//=========================================================================
// ARM family architecture - no interlocked instructions so emulate it with
// a global "interlock instruction" mutex.
static pthread_mutex_t interlockedOperationMutex = PTHREAD_MUTEX_INITIALIZER;
#else
#error Interlocked functions not defined for specified CPU/compiler architecture. (__i386__, x86_64, arm_family not specified).
#endif

//=========================== Interlock Functions =========================

#if !defined(arm_family) && (defined(__powerpc__) || !USE_ASSEMBLY_LANGUAGE_INTERLOCK_FUNCTIONS)

//===========================Wrapper Versions==============================
// C-wrappers around the assembly functions defined above.

//=========================================================================
//	LONG InterlockedCompareExchangeValue( PLONG dest, LONG xchg, LONG compare )
//
LONG InterlockedCompareExchangeValue( LONG *dest, LONG xchg, LONG compare )
{
    return (LONG)interlocked_cmpxchg( dest, xchg, compare );
}
//=========================================================================
//	void *InterlockedCompareExchange( void **dest, void *xchg, void *compare )
//
void *InterlockedCompareExchange( void **dest, void *xchg, void *compare )
{
    return interlocked_cmpxchg_ptr( dest, xchg, compare );
}

//========================================================================
// LONG InterlockedExchange( PLONG dest, LONG val );
//
LONG InterlockedExchange( LONG *dest, LONG val )
{
    return (LONG) interlocked_xchg( dest, val );
}

//=========================================================================
//	LONG InterlockedExchangeAdd( PLONG dest, LONG incr )
//
LONG InterlockedExchangeAdd( LONG *dest, LONG incr )
{
    return (LONG) interlocked_xchg_add( dest, incr );
    //return 0;
}

//=========================================================================
//	LONG InterlockedIncrement( PLONG dest )
//
LONG InterlockedIncrement( LONG *dest )
{
    return (LONG) interlocked_xchg_add( dest, 1 ) + 1;
}

//=========================================================================
//	LONG InterlockedDecrement( PLONG dest )
//
LONG InterlockedDecrement( LONG *dest )
{
    return (LONG) interlocked_xchg_add( dest, -1 ) - 1;
}

#elif !defined(arm_family)
//=====================Assembly Language Functions========================
//
//=========================================================================
// LONG InterlockedCompareExchange( PLONG dest, LONG xchg, LONG compare )
//
	__asm__ (".align 4	\n\t"
				".globl	InterlockedCompareExchange \n\t"
				"InterlockedCompareExchange: \n\t"
				"movl 12(%esp),%eax\n\t"
				"movl 8(%esp),%ecx\n\t"
				"movl 4(%esp),%edx\n\t"
				"lock; cmpxchgl %ecx,(%edx)\n\t"
				"ret $12");

//=========================================================================
// LONG InterlockedExchange( PLONG dest, LONG val )
//
	__asm__ (".align 4	\n\t"
				".globl	InterlockedExchange \n\t"
				"InterlockedExchange: \n\t"
				"movl 8(%esp),%eax\n\t"
				"movl 4(%esp),%edx\n\t"
				"lock; xchgl %eax,(%edx)\n\t"
				"ret $8");

//=========================================================================
// LONG InterlockedExchangeAdd( PLONG dest, LONG incr )
//
	__asm__ (".align 4	\n\t"
				".globl	InterlockedExchangeAdd \n\t"
				"InterlockedExchangeAdd: \n\t"
				"movl 8(%esp),%eax\n\t"
				"movl 4(%esp),%edx\n\t"
				"lock; xaddl %eax,(%edx)\n\t"
				"ret $8");

//=========================================================================
// LONG InterlockedIncrement( PLONG dest )
//
	__asm__ (".align 4	\n\t"
				".globl	InterlockedIncrement \n\t"
				"InterlockedIncrement: \n\t"
				"movl 4(%esp),%edx\n\t"
				"movl $1,%eax\n\t"
				"lock; xaddl %eax,(%edx)\n\t"
				"incl %eax\n\t"
				"ret $4");

//=========================================================================
//		LONG InterlockedDecrement( PLONG dest )
//
	__asm__ (".align 4	\n\t"
				".globl	InterlockedDecrement \n\t"
				"InterlockedDecrement: \n\t"
				"movl 4(%esp),%edx\n\t"
				"movl $-1,%eax\n\t"
				"lock; xaddl %eax,(%edx)\n\t"
				"decl %eax\n\t"
				"ret $4");
#else
//=========================================================================
// ARM family architecture - no interlocked instructions so emulate it with
// a global "interlock instruction" mutex.
//=========================================================================
//	LONG InterlockedCompareExchangeValue( PLONG dest, LONG xchg, LONG compare )
//
LONG InterlockedCompareExchangeValue( LONG *dest, LONG xchg, LONG compare )
{
	LONG result = 0;
   pthread_mutex_lock( &interlockedOperationMutex); 
	if (dest != NULL)
	{
		if ( *dest == compare)
		{
			result = *dest;
			*dest = xchg;
		}
	}
   pthread_mutex_unlock( &interlockedOperationMutex); 
   return result;
}
//=========================================================================
//	void *InterlockedCompareExchange( void **dest, void *xchg, void *compare )
//
void *InterlockedCompareExchange( void **dest, void *xchg, void *compare )
{
	void *result = NULL;
   pthread_mutex_lock( &interlockedOperationMutex); 
	if (dest != NULL)
	{
		if ((*dest != NULL) && (xchg != NULL) && (compare != NULL))
		{
			if ( *dest == compare)
			{
				result = *dest;
				*dest = xchg;
			}
		}
	}
   pthread_mutex_unlock( &interlockedOperationMutex); 
   return result;
}

//========================================================================
// LONG InterlockedExchange( PLONG dest, LONG val );
//
LONG InterlockedExchange( LONG *dest, LONG val )
{
	LONG result = 0;
   pthread_mutex_lock( &interlockedOperationMutex); 
	if (dest != NULL)
	{
		result = *dest;
		*dest = val;
	}
   pthread_mutex_unlock( &interlockedOperationMutex); 
   return result;
}

//=========================================================================
//	LONG InterlockedExchangeAdd( PLONG dest, LONG incr )
//
LONG InterlockedExchangeAdd( LONG *dest, LONG incr )
{
	LONG result = 0;
   pthread_mutex_lock( &interlockedOperationMutex); 
	if (dest != NULL)
	{
		result = *dest;
		*dest += incr;
	}
   pthread_mutex_unlock( &interlockedOperationMutex); 
   return result;
}

//=========================================================================
//	LONG InterlockedIncrement( PLONG dest )
//
LONG InterlockedIncrement( LONG *dest )
{	
	LONG result = 0;
   pthread_mutex_lock( &interlockedOperationMutex); 
	if (dest != NULL)
	{
		*dest += 1;
		result = *dest;
	}
   pthread_mutex_unlock( &interlockedOperationMutex); 
   return result;
}

//=========================================================================
//	LONG InterlockedDecrement( PLONG dest )
//
LONG InterlockedDecrement( LONG *dest )
{
	LONG result = 0;
   pthread_mutex_lock( &interlockedOperationMutex); 
	if (dest != NULL)
	{
		*dest -= 1;
		result = *dest;
	}
   pthread_mutex_unlock( &interlockedOperationMutex); 
   return result;
}

#endif

