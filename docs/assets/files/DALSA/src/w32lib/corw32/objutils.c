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

objutils.c			 											

Description:
   Posix object handling compatibility layer with Win32.

Platform:
	-Generic Posix.

History:
   1.00 October 15, 2003, parhug

$Log: objutils.c $
Revision 1.13  2006/05/12 08:46:28  PARHUG
Fix up handling of named objects (semaphore / mutex). Fall back to standard
semaphore if named object cannot be used.
Revision 1.12  2006/04/28 10:16:23  PARHUG
KernelNotification event is now a semaphore, updated the CloseHandleObject function.
Revision 1.11  2006/03/07 09:53:20  PARHUG
Add a mutex to protect handle registry updates.
Change the protections on the file used for named objects (via ftok)
 so that it can be deleted after a crash from another user/group/protection environment.
Revision 1.10  2005/10/12 15:34:09  PARHUG
Change OpenMutex so that initial state is owned (signalled).
Revision 1.9  2005/07/22 16:32:28  PARHUG
Merge with 1.8.1.3
Revision 1.8.1.3  2005/05/25 09:12:51  PARHUG
Change ReferanceHandle function. Kernel to user notification now uses a posix semaphore.
Revision 1.8.1.2  2005/05/11 10:48:06  PARHUG
Change to use Posix semaphores for un-name semaphores and for the kernel notification event.
Adjust reference count check when closing events.
Revision 1.8  2005/04/22 13:24:40  PARHUG
Add SetKernelEventObject in CloseHandleObject to properly signal outstanding waiters
on handle close.
Revision 1.7  2005/03/21 11:55:05  PARHUG
Change initialization file object and device object handles to enable reference counting.
Revision 1.6  2005/03/08 17:36:34  PARHUG
Added CloseRegisteredHandle and changed CloseHandle to use it.
Altered logic to mark the handle closed but only free it if its reference count is 0 (no one waiting on it).
Revision 1.5  2005/01/03 14:07:19  parhug
In OpenMutex, for the case of POSIX_SEMAPHORES, move the call to SemGetValue until 
after the handle is initialized (specifically, after handle->psemaphore has been set up).
Revision 1.4  2004/10/14 10:00:23  parhug
Add use of SignalSharedWaiters so we can use either Semaphores or Events as the shared object.
Revision 1.3  2004/10/07 11:01:35  BOUERI
- Fix CloseHandle to save the handle before remove it form the list.
Revision 1.2  2004/10/06 17:17:58  parhug
Update for new handle model. Merge with new thread-type for handle.
Revision 1.1  2004/08/19 12:26:09  parhug
Initial revision

*******************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <corposix.h>

#include <semaphore.h>
#include <GDrv_Common.h>
#include <linuxobjs.h>

#include <limits.h>

#ifndef NEW_SAPERA
#define priority_class 	filedes
#define thread_priority file_attributes
#endif

struct _HandleRegistry
{
	int numHandles;
	struct _HandleRegistry *next;
	void *Handle[1];
};

static PHANDLEOBJ	NullHandle = NULL;
static int SizeHandleRegistry = MAX_HANDLE_OBJECTS;
static struct _HandleRegistry *HandleRegistry = NULL;
static pthread_mutex_t registryAccessMutex = PTHREAD_MUTEX_INITIALIZER;

static PHANDLEOBJ CreateSysVSemaphore( const char *name, LONG initialCount, LONG maximumCount);
static PHANDLEOBJ CreatePosixSemaphore( const char *name, LONG initialCount, LONG maximumCount);

//==============================================================================
// Static globals for process local control of a set of semaphore objects.
static int MaxSemMapEntries = 0;
static unsigned long *SemEntryMap = NULL;
static LONG lclObjectSemId = -1;
static LONG lclObjectInit = 0;
static pthread_mutex_t lclObjectMutex = PTHREAD_MUTEX_INITIALIZER;


static key_t GetSemaphoreKey( const char *name)
{
	key_t semkey;
	char *fileName;
   BOOL fPrivateKey = FALSE;

	// Check if the semaphore key is named or not.
	if ( name == NULL)
	{
		// Normally we would use IPC_PRIVATE as the key but it is shared among all processes
		// with no way to determine which process is using which semaphore in the set
		// (Max SEMMSL semaphores per set, max SEMMNI identifiers per system)
		//
		// So here, we make a key based on the process id for the private use of this process.
		pid_t pid;

		fileName = (char *)malloc( 32*sizeof(char));
		if ( fileName != NULL)
		{
			pid = getpid();
			sprintf(fileName,"/tmp/CORECO_SEM_%d", (unsigned int)pid);
         fPrivateKey = TRUE;
		}
	}
	else
	{
		// Name needs to be a valid, accessible, file name !!!
		// (By default, we assume it is in the /tmp directory)
		fileName = (char *)malloc( strlen(name)+8);
		if ( fileName != NULL)
		{
			sprintf(fileName,"/tmp/%s", name);
		}
	}

	// We now have a valid file name to base the semaphore object on.
	if ( fileName == NULL)
	{
		// No file name. (Means memory allocation failed - we will die soon).
		semkey = IPC_PRIVATE;
	}
	else
	{
		if ( !CheckFileExists(fileName, TRUE) )
		{
			// File does not exist. Create one to use here.
			int f;
			f = open(fileName, (O_RDWR |O_CREAT));
			if ( f <= 0)
			{
				// Cannot create a file for the SysV semaphore key to use.
				// (Therefore - no named semaphore for now).
				semkey = IPC_PRIVATE;
			}
			else
			{
				semkey = ftok( fileName, 1);
				if (semkey < 0)
				{
					// Cannot create this key. Use the private (non-shared) one.
					semkey = IPC_PRIVATE;
				}
				// Make the file accessible to all.
				fchmod(f, ((S_IRUSR | S_IWUSR) | (S_IRGRP | S_IWGRP) | (S_IROTH | S_IWOTH)));

				// Unlink the file so it disappears when we exit.
            if (fPrivateKey)
            {
			      unlink(fileName);
            }
            else
            {
               close(f);
            }
         }
		}
		else
		{
			semkey = ftok( fileName, 1);
			if (semkey < 0)
			{
				// Cannot create this key. Use the private (non-shared) one.
				semkey = IPC_PRIVATE;
			}
		}
		free(fileName);
	}
	return semkey;
}

void CorIpcInit(void)
{
	// Initialize the IPC mechanism.
	// (to be called from library _init routine)
	// Create a number of semaphores so that this process can use them at will.
	// Keep track in a static variable (global) so that they can be reused.
	// (An array of bitmasks).
	long initFlag = -1;
	key_t semkey;
	int semid;
	int numSems = 32 * (SEMMSL / 32);

	pthread_mutex_lock( &lclObjectMutex);
	initFlag = InterlockedExchange( (LONG *)&lclObjectInit, (LONG)1);
	if ( initFlag == 0)
	{
		// Get the default semaphore key for this process Id;
		semkey = GetSemaphoreKey(NULL);

		// Create the Process local semaphores.
		semid = semget( semkey, numSems, (IPC_CREAT | 0600));
		if ( semid != -1)
		{
			// New process local set created. Set up the globals to control it.
			MaxSemMapEntries = SEMMSL/32;
			SemEntryMap = (unsigned long *)malloc(MaxSemMapEntries * sizeof(unsigned long));
			if ( SemEntryMap == NULL)
			{
				// Initialization of the global control structures failed.
				// Delete the semaphore set so we can try it again later.
				semctl( semid, 0, IPC_RMID, (union semun)0);
				lclObjectInit = 0;
			}
			else
			{
				// Initialze the globals for the process local semaphore set.
				memset( SemEntryMap, 0, MaxSemMapEntries*sizeof(unsigned long));
				lclObjectSemId = semid;
			}
		}
		else
		{
			// Semaphore set creation Failed.
			// There is nothing that can be done here.
			lclObjectInit = 0;

			// Log an error about this (somewhere).
			//????????????????????
		}
	}
	pthread_mutex_unlock( &lclObjectMutex);
}

void CorIpcClose(void)
{
	// Cleanup the IPC mechanism.
	// (to be called from library _fini routine)
	long initFlag = -1;
	int status;

	pthread_mutex_lock( &lclObjectMutex);
	initFlag = InterlockedExchange( (LONG *)&lclObjectInit, (LONG)1);
	if ( initFlag == 1)
	{
		// Already initialized (good).
		// Since we are closing, we need to clean this up.
		if ( lclObjectSemId != -1)
		{
			// Remove the semaphore set.
			status = semctl( lclObjectSemId, 0, IPC_RMID, (union semun)0);
			if ( status != 0)
			{
				// Log a message somewhere about "errno"
			}
		}
	}
	if ( SemEntryMap != NULL)
	{
		free(SemEntryMap);
	}
	pthread_mutex_unlock( &lclObjectMutex);
}


static int SemDestroy( PHANDLEOBJ handle)
{
	int status = 0;

	// Clean up the semaphore information (if found).
	if ( VALID_HANDLEOBJ( Semaphore, handle) ||
		  VALID_HANDLEOBJ( NamedSemaphore, handle) ||
		  VALID_HANDLEOBJ( NamedMutex, handle) )
	{
		if ( handle->bPosixSemaphore)
		{
			// Posix (pthreads) semaphores (unnamed semaphores ONLY !!!)
			status = sem_destroy( &handle->semaphore);
		}
		else
		{
			// SystemV semaphores
			// Delete the semaphore set for a named object.
			// Leave the process-local one alone.

			if ( VALID_HANDLEOBJ( Semaphore, handle) )
			{
				// Clear the resource bit for this process local semaphore.
				int i, j;
				i = handle->semnum / 32;
				j = handle->semnum % 32;
				SemEntryMap[i] &= ~(1 << j);
			}
			else
			{
				// These can be shared between processes and are backed by file names (for sem_key).
				// There is no way to know if some other process is using it.
				// So .... since we re-use the named mutexes a lot in our API 
				// We will make policy that 
				//     a) "mutex" protects access and should be preserved (not deleted)
				//        so other processes can still use it to protect access.
				//     b) "semaphore" is for signalling an event (between processes if "Named")
				//        so it is useful to know that one end died and took it with it (perhaps?).
				// 
				// Destroy the semaphore set for this named semaphore. (keep for named mutex)
				//if ( VALID_HANDLEOBJ( NamedSemaphore, handle) || VALID_HANDLEOBJ( NamedMutex, handle))
				if ( VALID_HANDLEOBJ( NamedSemaphore, handle))
				{
					status = semctl( handle->semid, handle->semnum, IPC_RMID, (union semun)0);            
					if ((status != 0) && ((errno == EIDRM) || (errno == EINVAL)))
					{
						status = 0;
					}
				} 
			}
		}
	}
	return status;
}

static inline BOOL CloseEventMutexObject( PHANDLEOBJ handle )
{
	BOOL status = FALSE;

	// Check that the reference count has become zero.
	EnterCriticalSection( &handle->cs);
	if ( handle->refCount == 0)
	{
		// Destroy the mutex.
		if ( pthread_mutex_destroy( &handle->mutex) == 0)
		{
			if ( pthread_mutexattr_destroy( &handle->mutex_attributes) == 0)
			{
				// Destroy the condition variable (event).
				if ( pthread_cond_destroy( &handle->event) == 0)
				{
					if ( pthread_condattr_destroy( &handle->event_attributes) == 0)
					{
						status = TRUE;
						handle->state = 0;
					}
				}
			}
		}
	}
	LeaveCriticalSection( &handle->cs);
	return status;
}

void CreateObjectHandle( PHANDLEOBJ *pHandle, handleType_t type )
{
	int i;
	PHANDLEOBJ handle = NULL;

	// Get a handle.
	GET_NEW_HANDLE( type, handle);

	// Set up handle if successful.
	if ( handle != NULL)
	{
		// Set up critical section.
		InitializeCriticalSection( &handle->cs);

		// Set up Mutex (and attributes)
		if ( pthread_mutexattr_init( &handle->mutex_attributes) == 0)
		{
#if _USE_UNIX98
			pthread_mutexattr_settype( &handle->mutex_attributes, PTHREAD_MUTEX_RECURSIVE_NP);
#endif
			if ( pthread_mutex_init( &handle->mutex, &handle->mutex_attributes) == 0)
			{
				// Set up the condition variable.
				if ( pthread_condattr_init( &handle->event_attributes) == 0)
				{
#if defined(USE_MONOTONIC_CLOCK)
						pthread_condattr_setclock( &handle->event_attributes, CLOCK_MONOTONIC);
#endif
					if ( pthread_cond_init( &handle->event, &handle->event_attributes) == 0)
					{
						// Eveything is OK and we are set up properly.
						handle->refCount = 0;
						handle->state = 0;
						handle->list.prev = NULL;
						handle->list.next = NULL;
						handle->eventType = EventNone;
						handle->sem_currentcount = 0;
						handle->sem_maxcount = SEM_VALUE_MAX;
						handle->file_attributes = 0;
						handle->fp = NULL;
						handle->numMultiWaiters = 0;
						handle->thread_id = 0;
						for (i = 0; i < MULTI_WAIT_COUNT_MAX; i++)
						{
							handle->multiWaitHandles[i] = NULL;
						}
					}
					else
					{
						// Clean up.
						pthread_condattr_destroy( &handle->event_attributes);
						pthread_mutex_destroy( &handle->mutex);
						pthread_mutexattr_destroy( &handle->mutex_attributes);
						DeleteCriticalSection( &handle->cs);
						RELEASE_HANDLE(handle);
						handle = NULL;
					}
				}
				else
				{
					// Clean up
					pthread_mutex_destroy( &handle->mutex);
					pthread_mutexattr_destroy( &handle->mutex_attributes);
					DeleteCriticalSection( &handle->cs);
					RELEASE_HANDLE(handle);
					handle = NULL;
				}
			}
			else
			{
				// Clean up.
				pthread_mutexattr_destroy( &handle->mutex_attributes);
				DeleteCriticalSection( &handle->cs);
				RELEASE_HANDLE(handle);
				handle = NULL;
			}
		}
		else
		{
			// Clean up
			DeleteCriticalSection( &handle->cs);
			RELEASE_HANDLE(handle);
			handle = NULL;
		}
	}
	*pHandle = handle;
	return;
}

void InitHandleRegistry(void)
{
   pthread_mutex_lock( &registryAccessMutex); 
   
	// Initialize the NullHandle object.
	CreateObjectHandle( &NullHandle, NullObject);

	// Allocate N entries for the handle registry.
	HandleRegistry = (struct _HandleRegistry *)malloc((SizeHandleRegistry-1) * sizeof(void *) + sizeof(struct _HandleRegistry));

	if (HandleRegistry != NULL)
	{
		int i;
		HandleRegistry->numHandles = SizeHandleRegistry;
		HandleRegistry->next = NULL;
		for (i = 0; i < SizeHandleRegistry; i++)
		{
			HandleRegistry->Handle[i] = INVALID_HANDLE_VALUE;
		}
	}
   pthread_mutex_unlock( &registryAccessMutex);
}

void FreeHandleRegistry(void)
{
	// Free the handle registry and all handles therein.
	struct _HandleRegistry *prev;
	struct _HandleRegistry *registry = HandleRegistry;

   pthread_mutex_lock( &registryAccessMutex); 
	while (registry != NULL)
	{
		int i;
		for (i = 0; i < registry->numHandles; i++)
		{
			if (registry->Handle[i] != INVALID_HANDLE_VALUE)
			{
				if ( registry->Handle[i] != (void *)NullHandle)
				{
					CloseHandleObject(registry->Handle[i]);
				}
			}
		}
		prev = registry;
		registry = registry->next;
		free(prev);
	}

	// Delete NullHandle
	DestroyObjectHandle(NullHandle);
   pthread_mutex_unlock( &registryAccessMutex); 
}

static void ExtendHandleRegistry(void)
{
	// Allocate additional entries fo the handle registry,
	// Free the handle registry and all handles therein.
	struct _HandleRegistry *new;
	struct _HandleRegistry *registry = HandleRegistry;

   if ( registry != NULL )
	{
		while (registry->next != NULL)
		{
			registry = registry->next;
		}

		new = (struct _HandleRegistry *)malloc((SizeHandleRegistry-1) * sizeof(void *) + sizeof(struct _HandleRegistry));

		if (new != NULL)
		{
			int i;
			new->numHandles = SizeHandleRegistry;
			new->next = NULL;
			for (i = 0; i < SizeHandleRegistry; i++)
			{
				new->Handle[i] = INVALID_HANDLE_VALUE;
			}

			registry->next = new;
		}

	}
}


static HANDLE RegisterHandle( PHANDLEOBJ handle)
{
	// Find the next free entry in the handle registry, insert the handle object,
	// and return the handle pointer.
	struct _HandleRegistry *registry = HandleRegistry;

	if ( registry != NULL && handle != INVALID_HANDLE_VALUE)
	{
		int i;
		pthread_mutex_lock( &registryAccessMutex); 
		do
		{
			for (i = 0; i < registry->numHandles; i++)
			{
				if ( registry->Handle[i] != INVALID_HANDLE_VALUE )
				{
					// Clean up NullHandles along the way.
					if ( registry->Handle[i] == (void *)NullHandle)
					{
						registry->Handle[i] = INVALID_HANDLE_VALUE;
					}
				}
				else
				{
					registry->Handle[i] = (void *)handle;
					pthread_mutex_unlock( &registryAccessMutex); 
					return (HANDLE)&registry->Handle[i];
				}
			}
			registry = registry->next;
		} while (registry != NULL);

		// Dropped through, no free handle slots. Extend it.
		ExtendHandleRegistry();
		pthread_mutex_unlock( &registryAccessMutex); 
		return RegisterHandle(handle);
	}
	return INVALID_HANDLE_VALUE;
}

static BOOL UnRegisterHandle( HANDLE handle)
{
	// Find the handle and point it to the NullHandle.
	struct _HandleRegistry *registry = HandleRegistry;

	if ( registry != NULL)
	{
		int i;
		pthread_mutex_lock( &registryAccessMutex); 
		do
		{
			for (i = 0; i < registry->numHandles; i++)
			{
				if ( registry->Handle[i] != INVALID_HANDLE_VALUE )
				{
               // Clean up NullHandles along the way.
					//if ( registry->Handle[i] == (void *)NullHandle)
					//{
					//	registry->Handle[i] = INVALID_HANDLE_VALUE;
					//}
					//else
					{
						if ( registry->Handle[i] == (void *)HANDLE_REF(handle))
						{
							registry->Handle[i] = (void *)NullHandle;
							pthread_mutex_unlock( &registryAccessMutex); 
							return TRUE;
						}
					}
				}
			}
			registry = registry->next;
		} while (registry != NULL);
		pthread_mutex_unlock( &registryAccessMutex); 
	}
	// Handle not found !!!
	return FALSE;
}

BOOL IsRegisteredHandle( HANDLE handle)
{
	// Find the handle and point it to the NullHandle.
	struct _HandleRegistry *registry = HandleRegistry;

	if ( registry != NULL)
	{
		int i;
		do
		{
			for (i = 0; i < registry->numHandles; i++)
			{
				if ( registry->Handle[i] != INVALID_HANDLE_VALUE )
				{
					if ( handle == (void *)&registry->Handle[i])
					{
						if ( registry->Handle[i] == (void *)HANDLE_REF(handle))
						{
							return TRUE;
						}
					}
				}
			}
			registry = registry->next;
		} while (registry != NULL);
	}
	// Handle not found !!!
	return FALSE;
}

PHANDLEOBJ CloseRegisteredHandle( HANDLE handle)
{
	// Find the handle and point it to the NullHandle and return what was there.
	PHANDLEOBJ closedHandle = INVALID_HANDLE_VALUE;
	struct _HandleRegistry *registry = HandleRegistry;

	if ( registry != NULL)
	{
		int i;
		pthread_mutex_lock( &registryAccessMutex); 
		do
		{
			for (i = 0; i < registry->numHandles; i++)
			{
				if ( registry->Handle[i] != INVALID_HANDLE_VALUE )
				{
					if ( handle == (void *)&registry->Handle[i])
					{
						if ( registry->Handle[i] == (void *)HANDLE_REF(handle))
						{
							closedHandle = HANDLE_REF(handle);
							registry->Handle[i] = (void *)NullHandle;
							pthread_mutex_unlock( &registryAccessMutex); 
							return closedHandle;
						}
					}
				}
			}
			registry = registry->next;
		} while (registry != NULL);
		pthread_mutex_unlock( &registryAccessMutex); 
	}
	// Handle not found !!!
	return closedHandle;
}

BOOL DestroyObjectHandle( PHANDLEOBJ handle )
{
   int result = -1;

	if ( (handle == INVALID_HANDLE_VALUE) || (handle == NULL) )
		return FALSE;

	// Check that the reference count has become zero.
	if ( handle->refCount == 0)
	{
		EnterCriticalSection( &handle->cs);
      
		SignalSharedWaiters(handle);

      // Destroy the mutex.
		if ( pthread_mutex_destroy( &handle->mutex) == 0)
		{
			if ( pthread_mutexattr_destroy( &handle->mutex_attributes) == 0)
			{
				// Destroy the condition variable (event).
				if ( pthread_cond_destroy( &handle->event) == 0)
				{
					if ( pthread_condattr_destroy( &handle->event_attributes) == 0)
					{
						// Destroy any created semaphores (if necessary).
						result = SemDestroy(handle);
					}
				}
			}
		}
		LeaveCriticalSection( &handle->cs);
	}
	if (result == 0)
	{		
		DeleteCriticalSection( &handle->cs);
		RELEASE_HANDLE(handle);
		handle = NULL;
      return TRUE;
	}
	return FALSE;
}


BOOL CloseHandleObject( PHANDLEOBJ handle)
{
	int timeout = HANDLE_CLOSE_RETRY_TIMEOUT;
	BOOL status = FALSE;

	if ( (handle == INVALID_HANDLE_VALUE) || (handle == NULL) )
		return status;

	switch (handle->type)
	{
		case DeviceObject:
		case FileObject:
			if ( HANDLEOBJ_IS_OPEN(handle) )
			{
				if ( CloseFileObject(handle) )
				{
					// File was closed at O/S level.
					handle->state |= STATE_CLOSING;
					DECREMENT_HANDLEOBJ_REFCOUNT(handle);
					//if (handle->refCount > 1)
					if (handle->refCount > 0)
					{
						// Someone is waiting on this now-closed handle.
						// Signal them and yield.
					
						SetEventObject(handle);
						while( (handle->refCount > 0) && (timeout-- > 0))
						{
							Sleep(0);
						}
					}
               if (handle->refCount == 0)
               {
						status = DestroyObjectHandle(handle);
               }
				}
			}
			break;

		case Thread:
			if ( HANDLEOBJ_IS_OPEN(handle) )
			{
				handle->state |= STATE_CLOSING;
            if(DECREMENT_HANDLEOBJ_REFCOUNT(handle) == 0)
            {
   				status = DestroyObjectHandle(handle);
            }
         }
         break;

			// Mutex and Event handling are intertwined.
		case Mutex:
		case Event:
			{
				if ( HANDLEOBJ_IS_OPEN(handle) )
				{
					handle->state |= STATE_CLOSING;
					DECREMENT_HANDLEOBJ_REFCOUNT(handle);
					
					while ((handle->refCount > 0) && (timeout-- > 0))
					{
						SetEventObject(handle);
						Sleep(0);	
					}
               if ( handle->refCount == 0)
               {
						status = DestroyObjectHandle(handle);
               }
				}
			}
			break;

			// Semaphores (shared with NamedMutex)
		case Semaphore:
		case NamedSemaphore:
		case NamedMutex:
			{
				if ( HANDLEOBJ_IS_OPEN(handle) )
				{
					handle->state |= STATE_CLOSING;
					DECREMENT_HANDLEOBJ_REFCOUNT(handle);
					//if (handle->refCount > 1)
					if ( handle->eventType == KernelNotification)
					{
						SetKernelEventObject(handle);
                  Sleep(0);
					}
               else
               {
                  while ((handle->refCount > 0) && (timeout-- > 0))
                  {
                     // Someone is waiting on this now-closed handle.
                     // Signal them and yield.
                     SemPostObject(handle);
                     Sleep(0);
                  }
               }
               if (handle->refCount == 0)
               {
						status = DestroyObjectHandle(handle);
               }
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

		default:
			break;
	}
	return status;
}

BOOL CloseHandle( HANDLE inhandle)
{
	PHANDLEOBJ handle = CloseRegisteredHandle(inhandle);
	return CloseHandleObject(handle);
}

// This function is for files (and possibly devices) only!
HANDLE	CreateFile( char *filename, ULONG desiredAccess, ULONG shareMode,
							void *securityAttributes, ULONG creationDisposition,
							ULONG fileAttributes, HANDLE hTemplate)
{
	PHANDLEOBJ handle = INVALID_HANDLE_VALUE;
	int flags = 0;
	char mode[4];

	// See if "filename" exists as a device "special" file.
	if ( CheckDeviceExists( filename, FALSE) )
	{
		// Device exists and is accessible to us in Read-Only mode.
		// Check "desiredAccess" to see if we need to check for R/W mode as well.
		if ( (desiredAccess & (GENERIC_WRITE | GENERIC_ALL)) != 0 )
		{
			if ( CheckDeviceExists( filename, TRUE) )
			{
				// R/W is also available. Set up flags for R/W access.
				flags = O_RDWR | O_NOCTTY;
			}
			else
			{
				// Device is not writable and R/W mode selected.
				return INVALID_HANDLE_VALUE;
			}
		}
		else
		{
			// Open file. Set up flags for read-only access.
			flags = O_RDONLY | O_NOCTTY;
		}

		// Allocate and set up the handle for this device.
		CreateObjectHandle( &handle, DeviceObject);
		if ( VALID_HANDLEOBJ( DeviceObject, handle ) )
		{
			handle->filedes = open( filename, flags);
			if ( handle->filedes < 0 )
			{
				RELEASE_HANDLE(handle);
				return INVALID_HANDLE_VALUE;
			}
			handle->refCount = 1;
			handle->state = STATE_OPEN;
			handle->file_attributes = flags;
			return RegisterHandle(handle);
		}
		else
		{
			return INVALID_HANDLE_VALUE;
		}
	}

	// We are dealing now with regular files (it cannot be a device because
	// it must already exist - we cannot create one here).
	mode[0] = '\0';

	// See if the file exists as either a regular file or as a device.
	if ( CheckFileExists( filename, FALSE ) )
	{
		// File exists and is accessible to us in Read-Only mode.
		// Check "desiredAccess" to see if we need to check for R/W mode as well.
		// Also check if CREATE_NEW is set (cannot create new if it exists).
		if (	((desiredAccess & (GENERIC_WRITE | GENERIC_ALL)) != 0 ) && \
				((creationDisposition & CREATE_NEW) == 0))
		{
			if ( CheckFileExists( filename, TRUE) )
			{
				// R/W is also available. Open file and set up the HANDLE structure for R/W access.
				CreateObjectHandle( &handle, FileObject);
				if (VALID_HANDLEOBJ(FileObject,handle))
				{
					if ((creationDisposition & (CREATE_ALWAYS | TRUNCATE_EXISTING)) != 0)
					{
						// Open and truncate the file (overwriting file).
						sprintf(mode,"w+");
					}
					if ((creationDisposition & (OPEN_EXISTING | OPEN_ALWAYS)) != 0)
					{
						// Open and the file.
						sprintf(mode,"a+");
					}
					handle->fp = fopen( filename, mode);
					handle->refCount = 1;
					handle->state = STATE_OPEN;
					return RegisterHandle(handle);
				}
				else
				{
					return INVALID_HANDLE_VALUE;
				}
			}
			else
			{
				// Device is not writable and R/W mode selected or we are asked to
				// create an existing file if it doesn't exist (and it does).
				return INVALID_HANDLE_VALUE;
			}
		}
		else
		{
			// Open file and setup the HANDLE structure for Read-only access.
			CreateObjectHandle( &handle, FileObject);
			if (VALID_HANDLEOBJ(FileObject,handle))
			{
				if ((creationDisposition & OPEN_EXISTING) != 0)
				{
					// Open and the file.
					handle->fp = fopen( filename, "r");
					handle->refCount = 1;
					handle->state = STATE_OPEN;
					return RegisterHandle(handle);
				}
			}
			return INVALID_HANDLE_VALUE;
		}
	}
	else
	{
		// "filename" does not exist as either a device or a regular file.
		// See if we are to create it and in which modes.

		if ( (creationDisposition & OPEN_EXISTING) != 0)
		{
			// File does not exist so cannot satisy OPEN_EXISTING option.
			return INVALID_HANDLE_VALUE;
		}

		// We will be creating a new file. Set up the mode (read-only is meaningless)

		CreateObjectHandle( &handle, FileObject);
		if (VALID_HANDLEOBJ(FileObject,handle))
		{
			handle->fp = fopen( filename, "w+");
			if ( handle->fp == NULL)
			{
				RELEASE_HANDLE(handle);
				return INVALID_HANDLE_VALUE;
			}
			handle->refCount = 1;
			handle->state = STATE_OPEN;
			return RegisterHandle(handle);
		}
		return INVALID_HANDLE_VALUE;
	}
}


HANDLE CreateEvent( LPSECURITY_ATTRIBUTES pSpecial, BOOL bManualReset, BOOL bInitialState, const char *name)
{
	PHANDLEOBJ handle = NULL;

	// Check the pointer in the first argument. If it is "-1" then this event
	// is to be used for signalling between the kernel driver and the user application
	// using SIGIO. (Until FASYNC has been fully implemented).
	// (It is implemented as a Posix semaphore).
	if ( (void *)pSpecial == (void *)(-1))
	{
		handle = CreatePosixSemaphore( NULL, 0, MAX_POSIX_SEMAPHORE_COUNT);
		if (handle != NULL)
		{
			handle->eventType = KernelNotification;
			handle->sigid = SIGNAL_FOR_KERNEL_EVENT; 	// For now - add RT signals later!!!
			handle->thread_id = pthread_self();
		}
	}
	else
	{
		// Allocate a handle and perform the basic setup. (Event and mutex are generic).
		CreateObjectHandle( &handle, Event);

		// Set up the event type.
		if ( handle != NULL)
		{
			handle->eventType = (bManualReset) ? ManualReset : AutoReset;
			handle->refCount = 1; // We are open
			handle->state = STATE_OPEN;
			if (bInitialState)
			{
				handle->state |= STATE_SIGNAL_SET;
			}
		}
	}
	return (handle == NULL) ? INVALID_HANDLE_VALUE : RegisterHandle(handle);
}


HANDLE CreateMutex( LPSECURITY_ATTRIBUTES pIgnored, BOOL bInitialOwner, const char *name)
{
	PHANDLEOBJ handle = NULL;
	int initialCount = (bInitialOwner) ? 0 : 1;

	// Check if the mutex is named or not.
	if ( name != NULL)
	{
#ifdef USE_POSIX_SEMAPHORES
		mode_t mode = 0664;
		sem_t *Sem = NULL;

		// Try to create the name mutex as a named semaphore with a count of 1.
		// NOTE : This is a recent addition to Linux Pthreads so it may not be
		//  implemented on the running platform.
		Sem = sem_open( (const char *)name, (int)O_CREAT, mode, initialCount);
		if ( (Sem != (void *)-1) && (Sem != NULL))
		{
			// Created ok. unlink it so that it disappears on program exit.
			sem_unlink(name);

			// Make the handle a named mutex
			// (This is the same as a named semaphore with a maximum count of 1).
			CreateObjectHandle( &handle, NamedMutex);

			handle->psemaphore = Sem;
			handle->sem_currentcount = initialCount;
			handle->sem_maxcount = 1;
			handle->refCount = 1; // We are open
		}
		else
		{
			handle = INVALID_HANDLE_VALUE;
		}
#else
		// Use SystemV semaphores.
		handle = CreateSysVSemaphore( name, initialCount, initialCount);
		if ( handle != INVALID_HANDLE_VALUE)
		{
			handle->type = NamedMutex;
		}
#endif
	}

	// Create an unnamed mutex if no name specified or the named mutex creation failed.
	if ( (name == NULL) || ((name != NULL) && (handle == INVALID_HANDLE_VALUE)) )
	{
		// Make the handle an unamed mutex.
		CreateObjectHandle( &handle, Mutex);
		if ( handle != NULL)
		{
			handle->refCount = 1;
			handle->state = STATE_OPEN;
			if ( bInitialOwner)
			{
				// Acquire the mutex.
				pthread_mutex_lock( &handle->mutex);
			}
		}
	}
	return (handle == NULL) ? INVALID_HANDLE_VALUE : RegisterHandle(handle);
}

HANDLE OpenMutex ( LONG desiredAccess, BOOL bInheritHandle, const char *name)
{
	PHANDLEOBJ handle = INVALID_HANDLE_VALUE;
#ifdef USE_POSIX_SEMAPHORES
	sem_t *Sem = NULL;
	mode_t mode = 0664;

	// This needs to be a system V semaphore!!!
	Sem = sem_open( name, O_RDWR, mode, 0);
	if ( (Sem != (void *)-1)  && (Sem != NULL))
	{
		// Create a new handle for this semaphore (it is a "NamedSemaphore");
		CreateObjectHandle( &handle, NamedMutex);

		// Make the handle a semaphore and initialize the semaphore information.
		if ( handle != NULL)
		{
			int value = 0;
			sem_getvalue( Sem, &value);
			handle->sem_maxcount = 1;	// Mutex has a max count of 1.
			handle->refCount = 1; 	// We are open
			handle->psemaphore = Sem;
			handle->state = STATE_OPEN;
			handle->sem_currentcount = SemGetValue(handle);
		}
	}
	return (handle == NULL) ? INVALID_HANDLE_VALUE : RegisterHandle(handle);
#else
	// Just do a create here, if it exists it will open it. If it does not exist it will create it.
	// (Don't know what the standard is - creating it can't hurt).
	handle = CreateSysVSemaphore( name, 1, 1);
	if ( handle != INVALID_HANDLE_VALUE)
	{
      if ( lclObjectInit == 1)
      {
         if ( handle->semid == lclObjectSemId)
         {
            // Named SysV sempahore creation failed.
            // If is part of the base semaphore set  now.
            handle->type = Semaphore;
         }
         else
         {
        	 handle->type = NamedMutex;
         }
      }
      else
      {
         handle->type = NamedMutex;
      }
	}
	if ( handle == NULL || handle == INVALID_HANDLE_VALUE )
	{
		return INVALID_HANDLE_VALUE;
	}
	else
	{
		return RegisterHandle(handle);
	}
#endif
}



static PHANDLEOBJ CreatePosixSemaphore( const char *name, LONG initialCount, LONG maximumCount)
{
	PHANDLEOBJ handle = NULL;
	int status;

	// Check if the semaphore is named or not.
	if ( name != NULL)
	{
		mode_t mode = 0664;
		sem_t *Sem = NULL;

		// Try to create the named semaphore
		// NOTE : This is a recent addition to Linux Pthreads so it may not be
		//  implemented on the running platform.
		Sem = sem_open( (const char *)name, (int)O_CREAT, mode, initialCount);
		if ( (Sem != (void *)-1) && (Sem != NULL))
		{
			// Created ok. unlink it so that it disappears on program exit.
			sem_unlink(name);

			// Initialize the semaphore
			status = sem_init(Sem, 0, (int)initialCount);
			if ( status == 0)
			{
				// Make the handle a named mutex
				// (This is the same as a named semaphore with a maximum count of 1).
				CreateObjectHandle( &handle, NamedSemaphore);
				handle->psemaphore = Sem;
				handle->sem_currentcount = initialCount;
				handle->sem_maxcount = maximumCount;
				handle->refCount = 1; // We are open
			}
			else
			{
				sem_close(Sem);
				handle = INVALID_HANDLE_VALUE;
			}
		}
		else
		{
			handle = INVALID_HANDLE_VALUE;
		}
	}

	// Create an unnamed sempahore if no name specified or the named semaphore creation failed.
	if ( (name == NULL) || ((name != NULL) && (handle == INVALID_HANDLE_VALUE)) )
	{
		// Make the handle an unamed semaphore and initialize the semaphore information.
		CreateObjectHandle( &handle, Semaphore);
		if ( handle != NULL)
		{
			status = sem_init( &handle->semaphore, 0, (int)initialCount);
			if ( status == 0)
			{
				handle->psemaphore = (sem_t *)&handle->semaphore;
				handle->sem_currentcount = initialCount;
				handle->sem_maxcount = maximumCount;
				handle->refCount = 1; // We are open
				handle->state = STATE_OPEN;
				handle->bPosixSemaphore = TRUE;
			}
			else
			{
				// Creation failed. Clean it up.
				DestroyObjectHandle( handle);
				handle = INVALID_HANDLE_VALUE;
			}
		}
	}

	return (handle == NULL) ? INVALID_HANDLE_VALUE : handle;
}

static PHANDLEOBJ CreateSysVSemaphore( const char *name, LONG initialCount, LONG maximumCount)
{
	PHANDLEOBJ handle = NULL;
	key_t	semkey;
	BOOL processLocal = TRUE;
	int semid, semnum, i, j;

	// Check if the semaphore is named or not.
	if ( name != NULL)
	{
		// Get a semaphore key for the name entered.
		semkey = GetSemaphoreKey( name);
		if ( semkey != IPC_PRIVATE)
		{
			processLocal = FALSE;
				// Create a single named semaphore to share with all processes.
			semid = semget( semkey, 1, (IPC_CREAT | 0666));
			if ( semid != -1)
			{
				CreateObjectHandle( &handle, NamedSemaphore);
				handle->sem_currentcount = initialCount;
				handle->sem_maxcount = maximumCount;
				handle->refCount = 1; // We are open
				handle->state = STATE_OPEN;
				handle->semid = semid;
				handle->semnum = 0;
				handle->bPosixSemaphore = FALSE;
				SemSetValue( handle, initialCount);
			}
			else
			{
				handle = INVALID_HANDLE_VALUE;
			}
			return handle;
		}
	}

	// We are to use the Process-Local semaphore set.
	// Set it up if it is not already initialized.

	if (lclObjectInit == 0)
	{
		CorIpcInit();
	}

	// Check that this was successfull.
	if ( lclObjectInit == 1)
	{
		// Get the next available semaphore number in the process localset.
		// Scan array for a new entry -> error if no more entries (too many resources for process).
		unsigned long map, bit;

		pthread_mutex_lock( &lclObjectMutex);
		semnum = -1;
		for (i = 0; i < MaxSemMapEntries; i++)
		{
			map = SemEntryMap[i];
			if ( map != 0xffffffff)
			{
				// Find the first free location in this entry of the map.
				bit = map & 0x01;
				j = 0;
				while( (bit == 1) && (j < 32))
				{
					map >>= 1;
					bit = map & 0x01;
					j++;
				}
				semnum = 32*i + j;
				break;
			}
		}
		if ( (semnum >= 0) && (semnum < MaxSemMapEntries*32))
		{
			// Allocate this semaphore.
			SemEntryMap[i] |= 1 << j;
		}
		else
		{
			// Too many resources at this time.
			handle = INVALID_HANDLE_VALUE;
		}
		pthread_mutex_unlock( &lclObjectMutex);
	}
	else
	{
		handle = INVALID_HANDLE_VALUE;
	}

	// Create the semaphore object if possible.
	if ( (lclObjectSemId != -1) && (handle != INVALID_HANDLE_VALUE))
	{
		CreateObjectHandle( &handle, Semaphore);
		if ( handle != INVALID_HANDLE_VALUE)
		{
			handle->sem_currentcount = initialCount;
			handle->sem_maxcount = maximumCount;
			handle->refCount = 1; // We are open
			handle->state = STATE_OPEN;
			handle->semid = lclObjectSemId;
			handle->semnum = semnum;
			handle->bPosixSemaphore = FALSE;
			SemSetValue( handle, initialCount);
		}
		else
		{
			// Object creation failed. Clear the bit in the resource mask.
			i = semnum / 32;
			j = semnum % 32;
			SemEntryMap[i] &= ~(1 << j);
			handle = INVALID_HANDLE_VALUE;
		}
	}

	return handle;
}



HANDLE CreateSemaphore( LPSECURITY_ATTRIBUTES pIgnored, LONG initialCount, LONG maximumCount, const char *name)
{
	PHANDLEOBJ handle;

	if ( name == NULL)
	{
		handle = CreatePosixSemaphore( name, initialCount, maximumCount);
	}
	else
	{
		handle = CreateSysVSemaphore( name, initialCount, maximumCount);
	}

   return RegisterHandle(handle);
}


HANDLE OpenSemaphore ( LONG desiredAccess, BOOL bInheritHandle, const char *name)
{
	PHANDLEOBJ handle = INVALID_HANDLE_VALUE;
#ifdef USE_POSIX_SEMAPHORES
	sem_t *Sem = NULL;
	mode_t mode = 0664;

	Sem = sem_open( name, O_RDWR, mode, 0);
	if ( (Sem != (void *)-1)  && (Sem != NULL))
	{
		// Create a new handle for this semaphore (it is a "NamedSemaphore");
		CreateObjectHandle( &handle, NamedSemaphore);

		// Make the handle a semaphore and initialize the semaphore information.
		if ( handle != NULL)
		{
			int value = 0;
			sem_getvalue( Sem, &value);
			handle->sem_currentcount = value;
			handle->sem_maxcount = SEM_VALUE_MAX;
			handle->refCount = 1; 	// We are open
			handle->psemaphore = Sem;
			handle->state = STATE_OPEN;
		}
	}
	return (handle == NULL) ? INVALID_HANDLE_VALUE : RegisterHandle(handle);
#else
	// Just do a create here, if it exists it will open it. If it does not exist it will create it.
	// (Don't know what the standard is - creating it can't hurt).
	handle = CreateSysVSemaphore( name, 0, 1);
	return RegisterHandle(handle);
#endif
}

HANDLE CreateThreadHandle()
{

	PHANDLEOBJ handle = NULL;

	// Allocate a handle and perform the basic setup. (Event and mutex are generic).
	CreateObjectHandle( &handle, Thread);

	// Set up the event type.
	if ( handle != NULL)
	{
		handle->eventType = Synchronisation;
		handle->refCount = 1; // We are open
		handle->state = STATE_OPEN;
      handle->thread_id = pthread_self();
		handle->priority_class = PRIORITY_CLASS_NOT_SET;
		handle->thread_priority = THREAD_PRIORITY_NORMAL;
	}

	return (handle == NULL) ? INVALID_HANDLE_VALUE : RegisterHandle(handle);
}

BOOL ReferanceHandle(HANDLE handle)
{
	// Add threadID to kernel to user notification event (actually a Posix semaphore).
   if(VALID_HANDLE(Semaphore, handle) && HANDLE_REF(handle)->eventType == KernelNotification)
   {
      HANDLE_REF(handle)->thread_id = pthread_self();
      return TRUE;
   }
   return FALSE;
}

