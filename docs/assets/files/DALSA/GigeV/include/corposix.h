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

File : corposix.h

Description:
   Win32 Compatibility layer definitions for Posix.

Platform:
	-Posix (generic layer).

History:
   1.00 September 26th, 2003, parhug

$Log: corposix.h $
Revision 1.14  2006/01/31 15:53:26  PARHUG
Add GetLastError and CheckFilePath prototypes
Revision 1.13  2005/11/15 12:11:15  PARHUG
Add alias' for _snprintf and _vsnprintf.
Revision 1.12  2005/11/11 16:39:34  PARHUG
Add defines for aliasing  _sprintf and _vsprintf.
Revision 1.11  2005/10/28 09:18:36  PARHUG
Add alias for Windows functions - lstrcpyn, lstrcmpi
Revision 1.10  2005/03/01 21:27:49  parhug
Fixed prototype for GetPrivateProfileSection.
Revision 1.9  2005/01/19 11:17:28  parhug
Add MAX_PATH definition (if required).
Revision 1.8  2005/01/07 14:27:35  parhug
Moved some new prototypes (used to be #defines) inside the "#ifdef __cplusplus } #endif"  at the end of the file.
Revision 1.7  2005/01/07 11:06:31  parhug
Added prototypes for new functions GetEnvironmentVariable, GetTickCount.
Changed macros LoadLibrary, GetProcAddress, FreeLibrary, DeleteFile to function prototypes.
Revision 1.6  2004/11/18 12:08:41  BOUERI
- Added function DeleteFile().
Revision 1.5  2004/11/03 16:46:17  parhug
Added macros for WINAPI, __max, __min and prototypes for _strrev and GetFileSize.
Revision 1.4  2004/10/06 12:09:14  parhug
Removed sigset and oldsigset from the HANDLEOBJ structure.
Revision 1.3  2004/09/29 17:25:06  BOUERI
- Added thread handle.
Revision 1.2  2004/09/29 08:52:22  parhug
Changed handle model / new functions.
Revision 1.1  2004/08/19 09:44:11  parhug
Initial revision

*******************************************************************************/

#ifndef _CORPOSIX_H_
#define _CORPOSIX_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <unistd.h>

///////////////////////////////////////////////////////////////////////////////////////
// General definitions
#include "corenv.h"
#define COR_LINUX 1
#include "posixcmn.h"
#ifndef __CELL__
#include "cordef.h"
#endif

// Re-definitions of WIN32'isms to Posix.
#define	FILE_BEGIN		SEEK_SET
#define	FILE_CURRENT	SEEK_CUR
#define	FILE_END			SEEK_END

#define FILESYSTEM_DELIMITER	"/"

//==============================================================================
// Defines for emulation of Win32 file / device access.
//
#ifdef __CELL__
#define DWORD unsigned int
#endif
#define INVALID_HANDLE_VALUE     ((HANDLE)~0UL)
#define INVALID_FILE_SIZE        ((DWORD)~0UL)
#define INVALID_SET_FILE_POINTER ((DWORD)~0UL)
#define INVALID_FILE_ATTRIBUTES  ((DWORD)~0UL)

#define FILE_READ_DATA            0x0001    /* file & pipe */
#define FILE_LIST_DIRECTORY       0x0001    /* directory */
#define FILE_WRITE_DATA           0x0002    /* file & pipe */
#define FILE_ADD_FILE             0x0002    /* directory */
#define FILE_APPEND_DATA          0x0004    /* file */
#define FILE_ADD_SUBDIRECTORY     0x0004    /* directory */
#define FILE_CREATE_PIPE_INSTANCE 0x0004    /* named pipe */
#define FILE_READ_EA              0x0008    /* file & directory */
#define FILE_READ_PROPERTIES      FILE_READ_EA
#define FILE_WRITE_EA             0x0010    /* file & directory */
#define FILE_WRITE_PROPERTIES     FILE_WRITE_EA
#define FILE_EXECUTE              0x0020    /* file */
#define FILE_TRAVERSE             0x0020    /* directory */
#define FILE_DELETE_CHILD         0x0040    /* directory */
#define FILE_READ_ATTRIBUTES      0x0080    /* all */
#define FILE_WRITE_ATTRIBUTES     0x0100    /* all */
#define FILE_ALL_ACCESS           0x1ff

#define FILE_GENERIC_READ         (FILE_READ_DATA | FILE_READ_ATTRIBUTES | FILE_READ_EA )
#define FILE_GENERIC_WRITE        (FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA |  FILE_APPEND_DATA)
#define FILE_GENERIC_EXECUTE      (FILE_EXECUTE | FILE_READ_ATTRIBUTES )

// File attribute flags
#define FILE_SHARE_READ			0x00000001L
#define FILE_SHARE_WRITE		0x00000002L
#define FILE_SHARE_DELETE		0x00000004L

#define FILE_ATTRIBUTE_READONLY         0x00000001L
#define FILE_ATTRIBUTE_HIDDEN           0x00000002L
#define FILE_ATTRIBUTE_SYSTEM           0x00000004L
#define FILE_ATTRIBUTE_LABEL            0x00000008L  /* Not in Windows API */
#define FILE_ATTRIBUTE_DIRECTORY        0x00000010L
#define FILE_ATTRIBUTE_ARCHIVE          0x00000020L
#define FILE_ATTRIBUTE_NORMAL           0x00000080L
#define FILE_ATTRIBUTE_TEMPORARY        0x00000100L
#define FILE_ATTRIBUTE_ATOMIC_WRITE     0x00000200L
#define FILE_ATTRIBUTE_XACTION_WRITE    0x00000400L
#define FILE_ATTRIBUTE_COMPRESSED       0x00000800L
#define FILE_ATTRIBUTE_OFFLINE		0x00001000L

// File creation flags
#define FILE_FLAG_WRITE_THROUGH    0x80000000UL
#define FILE_FLAG_OVERLAPPED 	   0x40000000L
#define FILE_FLAG_NO_BUFFERING     0x20000000L
#define FILE_FLAG_RANDOM_ACCESS    0x10000000L
#define FILE_FLAG_SEQUENTIAL_SCAN  0x08000000L
#define FILE_FLAG_DELETE_ON_CLOSE  0x04000000L
#define FILE_FLAG_BACKUP_SEMANTICS 0x02000000L
#define FILE_FLAG_POSIX_SEMANTICS  0x01000000L
#define CREATE_NEW              0x01
#define CREATE_ALWAYS           0x02
#define OPEN_EXISTING           0x04
#define OPEN_ALWAYS             0x08
#define TRUNCATE_EXISTING       0x0c

#define GENERIC_READ               0x80000000
#define GENERIC_WRITE              0x40000000
#define GENERIC_EXECUTE            0x20000000
#define GENERIC_ALL                0x10000000

//===============================================================================
// Critical Section definitions.

#define CRITICAL_SECTION_ACCESS_TIMEOUT_VALUE	100

typedef enum
{
	Undefined,
	Fast,
	ErrorChecking,
	Recursive
} MutexType_t;

typedef struct _CRITICAL_SECTION
{
	MutexType_t				type;				// Type of Critical Section (fast, errorchecking, recursive)
	long						refCount;		// Reference count for recursive Critical section types.
	long						waitCount;		// Count of threads waiting for Critical section.
	pthread_t				ownerThread;	// Owning thread (NULL for none).
	pthread_cond_t			cvWaiter;		// Condition variable for notification of waiting threads.
	pthread_condattr_t	cvWaiterAttributes;	// Condition variable atributes.
	pthread_mutex_t		pCsMutex;				// Mutex to hold for the critical section.
	pthread_mutexattr_t	pCsMutexAttributes;	// Mutex attributes.
	int 						savedThreadCancelState;	// Cancel state of thread before mutex acquired.
} CRITICAL_SECTION, *PCRITICAL_SECTION, *LPCRITICAL_SECTION;

//==================================================================
// Definitions for Win32 Handle Object emulation.

#define MULTI_WAIT_COUNT_MAX	256

typedef enum
{
	NullObject,
	DeviceObject,
	FileObject,
	Mutex,
	Event,
	Semaphore,
	NamedMutex,
	NamedSemaphore,
	SharedEvent,
	NamedSharedEvent,
   Thread
} handleType_t;

typedef enum
{
	EventNone,
	Synchronisation,
	Notification,
	ManualReset,
	AutoReset,
	KernelNotification
} eventType_t;

typedef void *HANDLE, **PHANDLE;


typedef struct _HANDLEOBJ
{
	int						state;
	int						refCount;
	handleType_t			type;
	LIST_ENTRY				list;
	BOOL						bPosixSemaphore; // Flag indicating Posix semaphore in use (Buggy in 2.4 kernel)
	sem_t						*psemaphore;	// Pointer to Posix semaphore.
	sem_t						semaphore;		// Storage for Posix semaphore.
	int						semid;			// SysV semaphore set ID. (always works).
	int						semnum;			// SysV semaphore number within set. (always works).
	int						sem_currentcount;
	int						sem_maxcount;
	pthread_mutex_t		mutex;
	pthread_mutexattr_t	mutex_attributes;
	pthread_cond_t			event;
	pthread_condattr_t	event_attributes;
	eventType_t				eventType; 		// Auto reset vs others
   pthread_t            thread_id;
#ifdef NEW_SAPERA
	int 						priority_class;  // Next version of Sapera.
	int						thread_priority; // Next version of Sapera.
#endif
	int						filedes;
	int						file_attributes;
	int						sigid;			// Signal to be used (SIGIO for now).
	FILE						*fp;
	CRITICAL_SECTION		cs;			// For exclusive when modifying the handle.
	int numMultiWaiters;
	HANDLE *multiWaitHandles[MULTI_WAIT_COUNT_MAX];
} HANDLEOBJ, *PHANDLEOBJ;

#define HANDLE_REF(a) ((PHANDLEOBJ)(*(volatile PHANDLEOBJ *)(a)))

// Handle state settings.
#define STATE_OPEN				0x00000001
#define STATE_CLOSING			0x00000002
#define STATE_SIGNAL_SET		0x00000004
#define STATE_SIGNAL_PULSED	0x00000008

//====================================================================
// IPC subsystem init/cleanup functions.
void CorIpcInit(void);
void CorIpcClose(void);

//====================================================================
// Posix-Win32 Compatibility layer functions
//
// Equivalent functions differing in name only
#define wsprintf            sprintf
#define wvsprintf           vsprintf
#define _sprintf            sprintf
#define _vsprintf           vsprintf
#define _snprintf           snprintf
#define _vsnprintf          vsnprintf
#define GetCurrentThreadId  pthread_self
#define GetCurrentProcessId getpid
#define lstrlen  strlen
#define lstrcpy  strcpy
#define lstrcpyn strncpy
#define lstrcat  strcat
#define lstrcmpi stricmp

#define CopyMemory(dst,src,size)		memmove((dst),(src),(size))
#define MoveMemory(dst,src,size)		memmove((dst),(src),(size))
#define ZeroMemory(ptr,size)			memset((ptr),0,(size))
#define FillMemory(ptr,size,value)	memset((ptr),(value),(size))
#define OutputDebugString(str)

#define IsBadReadPtr(ptr,size) 0
#define IsBadWritePtr(ptr,size) 0

// typedefs assumed native in Win32.
#ifndef WINAPI
 #define WINAPI
#endif
#ifndef MAX_PATH
 #define MAX_PATH 512
#endif
typedef int32_t *LPDWORD;
typedef void *HPALETTE, *HGLOBAL;
typedef struct
{
	int32_t	left;
	int32_t	top;
	int32_t	right;
	int32_t	bottom;
} RECT, *LPRECT, *PRECT;

typedef struct
{
	u_int8_t	peRed;
	u_int8_t	peGreen;
	u_int8_t	peBlue;
	u_int8_t	peFlags;
} PALETTEENTRY, *PPALETTEENTRY, *LPPALETTENTRY;

typedef struct
{
	u_int16_t	palVersion;
	u_int16_t	palNumEntries;
	PALETTEENTRY	palPalEntry[1];
} LOGPALETTE, *PLOGPALETTE, *LPLOGPALETTE;

// Remove these
#ifndef __cplusplus
	#ifndef max
	#define max(a,b) ((a) < (b)) ? (b) : (a)
	#endif
	#ifndef MAX
	#define MAX max
	#endif
	#ifndef __max
	#define __max max
	#endif
	#ifndef min
	#define min(a,b) ((a) > (b)) ? (b) : (a)
	#endif
	#ifndef MIN
	#define MIN min
	#endif
	#ifndef __min
	#define __min min
	#endif
#endif
//============================================================================
//
#ifdef __cplusplus
extern "C" {
#endif
//============================================================================
// INI file access functions.
int	GetPrivateProfileString( const char *section, const char *key, char *defaultString, char *outstring, int outstringsize, const char *filename);
int	GetPrivateProfileInt( const char *section, const char *key, int defaultValue, const char *filename);
float	GetPrivateProfileFloat( const char *section, const char *key, float defaultValue, const char *filename);

unsigned long GetPrivateProfileSectionNames( char *sectionNames, unsigned long nSize, const char *filename);
unsigned long GetPrivateProfileSection( char *section, char *sectionData, unsigned long nSize, const char *filename);
BOOL WritePrivateProfileSection( char *section, char *sectionData, const char *filename);

BOOL WritePrivateProfileString( const char *section, const char *key, char *string, const char * filename);
BOOL WritePrivateProfileInt( const char *section, const char * key, int value, const char * filename);
BOOL WritePrivateProfileFloat( const char *section, const char * key, float value, const char * filename);

int cor_sscanf( const char *str, const char *format, ...);
void strcpy_safe( char *dest, size_t count, const char *src );

// String functions.
int strnicmp (const char *pszS1, const char *pszS2, size_t stMaxlen);
int stricmp (const char *pszS1, const char *pszS2);
char *_strrev( char *str);
int GetEnvironmentVariable( const char* name, char* value, unsigned int len);
int GetLastError(void);
void SetLastError(int e);
int GetSystemDirectory( char *buf, int size);

char * itoa ( int value, char * str, int base );

// Conio functions
int kbhit(void);
char getch();

//==============================================================================
// File / device object handling.
typedef void*   PSECURITY_ATTRIBUTES,*LPSECURITY_ATTRIBUTES;
typedef void*	PSECURITY_DESCRIPTOR;
typedef void*	LPOVERLAPPED;

BOOL CheckFilePath( const char *inFile, char *outFile);
BOOL CheckFileExists( const char *inFile, BOOL rwmode);
BOOL CheckDeviceExists( const char *inFile, BOOL rwmode);
HANDLE	CreateFile( char *filename, ULONG desiredAccess, ULONG shareMode,
							void *securityAttributes, ULONG creationDisposition,
							ULONG fileAttributes, HANDLE hTemplate);
BOOL CloseFile( HANDLE handle);
BOOL ReadFile( HANDLE hFile, void *buffer, ULONG bytesToRead,
					ULONG *bytesRead, void *overlapped);
BOOL WriteFile( HANDLE hFile, void *buffer, ULONG bytesToWrite,
						ULONG *bytesWritten, void *overlapped);
BOOL DeviceIoControl( HANDLE handle, ULONG ioctlCode,
						void *inputBuffer, ULONG inputBufSize,
						void *outputBuffer, ULONG outputBufSize,
						ULONG *bytesTransfered, void *overlap);

ULONG SetFilePointer( HANDLE hFile, LONG offsetLow, LONG *offsetHigh, ULONG method);
BOOL	IsEndOfFile( HANDLE hFile);
BOOL	FlushFileBuffers( HANDLE hFile );	//?????? Is this one needed ????????
ULONG GetFileSize( HANDLE hFile, ULONG *lpFileSizeHigh);
BOOL DeleteFile( const char* lpFileName);
//#define DeleteFile remove

//==============================================================================
// General handle functions and macros.
void CreateObjectHandle( PHANDLEOBJ *pHandle, handleType_t type );
BOOL DestroyObjectHandle( PHANDLEOBJ handle );
BOOL CloseHandle( HANDLE handle);

//==============================================================================
// Critical Section handling functions.
void InitializeCriticalSection( LPCRITICAL_SECTION lpCriticalSection );
void EnterCriticalSection( LPCRITICAL_SECTION lpCriticalSection );
void LeaveCriticalSection( LPCRITICAL_SECTION lpCriticalSection );
void DeleteCriticalSection( LPCRITICAL_SECTION lpCriticalSection );
BOOL TryEnterCriticalSection( LPCRITICAL_SECTION lpCriticalSection);
void SetCriticalSectionMutexType( LPCRITICAL_SECTION lpCriticalSection, MutexType_t type);

//==============================================================================
// Event / Semaphore / Mutex Primitives.
#define MUTEX_ALL_ACCESS           FILE_ALL_ACCESS
#define EVENT_ALL_ACCESS           FILE_ALL_ACCESS
#define SEMAPHORE_ALL_ACCESS       FILE_ALL_ACCESS
#define SYNCHRONIZE                (FILE_ALL_ACCESS - 1)

HANDLE CreateMutex( LPSECURITY_ATTRIBUTES lpMutexAttributes,
    				BOOL bInitialOwner,
    				const char *lpName
    				);

HANDLE CreateSemaphore(	LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
    					LONG lInitialCount,
    					LONG lMaximumCount,
    					const char *lpName
    					);

HANDLE CreateEvent( LPSECURITY_ATTRIBUTES lpEventAttributes,
    				BOOL bManualReset,
    				BOOL bInitialState,
    				const char *lpName
    				);

HANDLE OpenMutex( LONG dwDesiredAccess,
    				BOOL bInitialOwner,
    				const char *lpName
    				);

HANDLE OpenSemaphore( LONG dwDesiredAccess,
					BOOL bInheritHandle,
    				const char *lpName
    				);

HANDLE OpenEvent( LONG dwDesiredAccess,
					BOOL bInheritHandle,
    				const char *lpName
    				);

BOOL SetEvent( HANDLE hEvent );

BOOL ResetEvent( HANDLE hEvent );

BOOL PulseEvent( HANDLE hEvent );

BOOL ReleaseSemaphore(	HANDLE hSemaphore,
						LONG lReleaseCount,
						LONG *lpPreviousCount
    				);

BOOL ReleaseMutex( HANDLE hMutex );

BOOL ReferanceHandle(HANDLE handle);

//==============================================================================
// Generic Wait functions for the unified object primitives.
#define WAIT_FAILED			-1
#define WAIT_ABANDONED		0x00000080L
#define WAIT_ABANDONED_0	WAIT_ABANDONED
#define WAIT_OBJECT_0		0x00000000L
#define WAIT_TIMEOUT			0x00000102L
#define WAIT_IO_COMPLETION	0x000000c0L
#define INFINITE				-1

LONG WaitForSingleObject( HANDLE hHandle, LONG dwMilliseconds );
LONG WaitForMultipleObjects( LONG count, HANDLE *hHandles, BOOL bWaitAll, LONG dwMilliseconds );

// Non-portable version (uses struct timeval).
LONG WaitForSingleObject_NP( HANDLE handle, BOOL bWait, struct timeval *pTimeout);

//==============================================================================
// Interlocked functions.
LONG InterlockedCompareExchangeValue( LONG *destination, LONG exchangeVal, LONG comperand);
void *InterlockedCompareExchange( void **destination, void *exchangeVal, void *comperand);
LONG InterlockedExchange( LONG *target, LONG value);
LONG InterlockedExchangeAdd( LONG *addend, LONG increment);
LONG	InterlockedIncrement( LONG *addend);
LONG	InterlockedDecrement( LONG *addend);

//==============================================================================
// Thread functions
unsigned long _beginthread( unsigned int (*threadfunc)(void *),
							unsigned int stacksize, void *arglist );
unsigned long _beginthreadex( void *security, unsigned int stacksize,
					unsigned int (*threadfunc)(void *), void *arglist,
					unsigned int initflag, unsigned int *threadid);
void _endthread(void);
void _endthreadex( unsigned int retval);

#define THREAD_PRIORITY_ERROR_RETURN	-1

#define THREAD_PRIORITY_IDLE				1		// Always 1 (lowest value allowed)
#define THREAD_PRIORITY_LOWEST			2		// Added to PriorityClass value.
#define THREAD_PRIORITY_BELOW_NORMAL	3		// Added to PriorityClass value.  
#define THREAD_PRIORITY_NORMAL			4		// Added to PriorityClass value.
#define THREAD_PRIORITY_ABOVE_NORMAL	5		// Added to PriorityClass value.
#define THREAD_PRIORITY_HIGHEST			6		// Added to PriorityClass value.
#define THREAD_PRIORITY_TIME_CRITICAL	15		// Always 15 (unless RT PriorityClass).

#define THREAD_PRIORITY_MINIMUM	THREAD_PRIORITY_IDLE	
#define THREAD_PRIORITY_MAXIMUM	THREAD_PRIORITY_TIME_CRITICAL 	

BOOL   SetThreadPriority( HANDLE hThread, int priority);
int    GetThreadPriority( HANDLE hThread);
HANDLE GetCurrentThread(void);

#define REALTIME_PRIORITY_OFFSET		4

#define PRIORITY_CLASS_NOT_SET		0
#define IDLE_PRIORITY_CLASS			1
#define BELOW_NORMAL_PRIORITY_CLASS	3	
#define NORMAL_PRIORITY_CLASS			5
#define ABOVE_NORMAL_PRIORITY_CLASS	7
#define HIGH_PRIORITY_CLASS			9			
#define REALTIME_PRIORITY_CLASS		15

BOOL   SetPriorityClass( HANDLE hThread, UINT32 priorityClass);
UINT32 GetPriorityClass( HANDLE hThread);

//==============================================================================
// Timing and delay functions.
BOOL QueryPerformanceFrequency( LARGE_INTEGER *pLfreq);
BOOL QueryPerformanceCounter( LARGE_INTEGER *pLcount);
unsigned int Sleep( int millisecs);
unsigned long GetTickCount( void );
void GetCurrentClockTime( struct timeval *tv);
//==============================================================================
// Dynamic library object access functions.
typedef void* HMODULE;
typedef void* HINSTANCE;
typedef void*	FARPROC;

//#define GetProcAddress(hModule, name)	dlsym( (hModule), (name))
//#define LoadLibrary(name)	dlopen( (name), (RTLD_LAZY))
//#define FreeLibrary(name)	dlclose( (name) )


FARPROC GetProcAddress( HMODULE hModule, char* lpProcName );
HMODULE LoadLibrary( const char* lpFileName );
BOOL FreeLibrary( HMODULE hModule );

#define GetModuleHandle LoadLibrary

//???????????????????????????
void CorW32IncrementSignalRecvCount();
void CorW32IncrementSignalSendCount();
void GetSignalCount( LONG *send, LONG *recv);
//??????????????????????????


#ifdef __cplusplus
}
#endif


#endif

