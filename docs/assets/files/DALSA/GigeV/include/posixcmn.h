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

posixcmn.h			 									

Description:
   Win32 Compatibility layer definitions for Posix - shared with kernel level.

Platform:
	-Posix (user and kernel level).

History:
   1.00 December 23, 2003, parhug

$Log: posixcmn.h $
Revision 1.7  2006/03/27 11:46:15  PARHUG
Fix FIELD_OFFSET macro.
Revision 1.6  2005/10/13 16:36:15  PARHUG
Added STATUS_ALERTED
Revision 1.5  2004/11/03 16:48:02  parhug
Added "linux/types.h" for kernel compilation tog et definitions for "s64" and "u64".
Revision 1.4  2004/09/14 09:39:52  parhug
Added PCHAR definition.
Revision 1.3  2004/09/02 15:46:52  parhug
Moved errno.h so both user and kernel levels see it.
Revision 1.2  2004/08/31 13:15:43  parhug
Remove some kernel include files and redefine some macros.
Revision 1.1  2004/08/19 09:44:10  parhug
Initial revision

*******************************************************************************/

#ifndef _POSIXCMN_H_
#define _POSIXCMN_H_

#ifdef __KERNEL__

//================================
// Kernel-only definitions.
#include <linux/types.h>
typedef s64 LONGLONG, *PLONGLONG;
typedef u64 ULONGLONG, *PULONGLONG;
typedef s32 LONG, *PLONG;
typedef u32 ULONG, *PULONG;
typedef u32 ULONG32, *PULONG32;

#else

#include <linux/types.h>
typedef int64_t LONGLONG, *PLONGLONG;
typedef u_int64_t ULONGLONG, *PULONGLONG;
typedef int32_t LONG, *PLONG;
typedef u_int32_t ULONG, *PULONG;
typedef u_int32_t ULONG32, *PULONG32;

#endif

#define POINTER_32


///////////////////////////////////////////////////////////////////////////////
// Linux/Unix errors mapped into NT status returns (compatibility)
#include <linux/errno.h>

#define STATUS_SUCCESS                   0
#define STATUS_INSUFFICIENT_RESOURCES    -ENOMEM
#define STATUS_NO_MEMORY                 -ENOMEM

#define STATUS_INVALID_PARAMETER         -EINVAL
#define STATUS_ACCESS_VIOLATION          -EFAULT

#define STATUS_IO_TIMEOUT                -ETIMEDOUT
#define STATUS_TIMEOUT                   -ETIME

#define STATUS_BUFFER_TOO_SMALL          -ENOBUFS
#define STATUS_INVALID_DEVICE_REQUEST    -EBADRQC

#define STATUS_DEVICE_BUSY               -EBUSY

#define STATUS_NOT_IMPLEMENTED           -ENOSYS
#define STATUS_UNSUCCESSFUL              -EFAULT

#define STATUS_ADAPTER_HARDWARE_ERROR    -EIO
#define STATUS_DEVICE_DOES_NOT_EXIST     -ENODEV
#define STATUS_ALERTED                   -EINTR
/* Possibly relevant errors
EPERM = operation not permitted
ENOENT = No such file or directory
ESRCH = no such process
EINTR = interrupted system call
EIO = I/O error
ENXIO = no such device or address
E2BIG = arg list too long

EAGAIN = EWOULDBLOCK = try again
ENOMEM = no memory
EACCESS = permission denied
EFAULT
EBUSY = device / resource busy
ENODEV = no such device
EINVAL = invalid argument
ESPIPE = illegal seek
ENOSYS = function not implemented
ENODATA = no data available
ETIME = timer expired
ETIMEDOUT = connection timed out
ENOBUFS = no buffer space available
*/


///////////////////////////////////////////////////////////////////////////////
// General definitions

#ifndef NULL
	#define NULL            0
#endif

#ifndef far
	#define far
#endif

#ifndef TRUE
	#define TRUE                    1
	#define FALSE                   0
#endif

#define NT_SUCCESS(a) ((a) == 0)
#define NT_ERROR(a) ((a) != 0)

// Win32 kernel source "-isms" to work around.
#define IN
#define OUT
#define MAXIMUM_FILENAME_LENGTH 256

typedef int BOOL, *PBOOL;
typedef char *PCHAR;

typedef union _LARGE_INTEGER
{
	struct
	{
		ULONG LowPart;
		LONG HighPart;
	};
	struct
	{
		ULONG LowPart;
		LONG HighPart;
	}u;
	LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;
//
//  Structure for double linked list.
//	List head has (*prev == NULL)
//  as link words.
//

typedef struct _LIST_ENTRY
{
   struct _LIST_ENTRY *next;
   struct _LIST_ENTRY *prev;
} LIST_ENTRY, *PLIST_ENTRY;

//
//  Linked list structure.
//

typedef struct _SINGLE_LIST_ENTRY
{
    struct _SINGLE_LIST_ENTRY *next;
} SINGLE_LIST_ENTRY, *PSINGLE_LIST_ENTRY;


//
// Calculate the byte offset of a field in a structure of type "type".
//

#define FIELD_OFFSET(type, field)    ((LONG)(int *)&(((type *)0)->field))


//
// Calculate the address of the base of the structure given its type, and an
// address of a field within the structure.
//

#define CONTAINING_RECORD(address, type, field) ((type *)( \
                                                  (char *)(address) - \
                                                  (unsigned long)(&((type *)0)->field)))

#ifdef __KERNEL__
// Interlocked functions.
LONG InterlockedCompareExchangeValue( LONG *destination, LONG exchangeVal, LONG comperand);
void *InterlockedCompareExchange( void **destination, void *exchangeVal, void *comperand);
LONG InterlockedExchange( LONG *target, LONG value);
LONG InterlockedExchangeAdd( LONG *addend, LONG increment);
LONG	InterlockedIncrement( LONG *addend);
LONG	InterlockedDecrement( LONG *addend);


//
// Linux compiler directives made to look like NT ones.
//

// Remove this definition of inline - inline is broken kernel compilation
// unless using the attribute(__always_inline) technique. The simple
// definition used originally is highly compiler version dependent and
// was here to be able to re-ise the Windows driver code.
//
// #define __forceinline inline

#define __forceinline 

#endif

#endif

