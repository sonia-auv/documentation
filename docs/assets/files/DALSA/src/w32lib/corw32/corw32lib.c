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

corw32lib.c			 											

Description:
   Posix compatibility layer with Win32 (Entry point + general lib functions).

Platform:
	-Generic Posix.

History:
   1.00 May 10, 2004, parhug

$Log: corw32lib.c $
Revision 1.9  2006/03/07 10:06:43  PARHUG
Added GetLastError.
Revision 1.8  2005/07/22 16:28:56  PARHUG
Merge with 1.7.1.2
Revision 1.7.1.2  2005/05/11 10:21:27  PARHUG
Change kernel notification event to a semaphore (better compatibility with the signal handler).
Revision 1.7  2005/04/19 15:25:06  PARHUG
Change debug messages in LoadLibrary to filter out missing CXM (plug-in) module messages (not an error).
Revision 1.6  2005/03/15 12:18:28  PARHUG
Changed FreeLibrary to return a BOOL (as required).
Revision 1.5  2005/01/07 11:36:58  parhug
Added GetProcAddress, LoadLibrary, FreeLibrary, GetEnvironmentVariable as functions.
Revision 1.4  2004/12/20 12:40:50  parhug
Properly initialize sigaction structure at library load. Properly remove sigaction settings on library unload.
Revision 1.3  2004/10/14 09:56:05  parhug
Fix signal handler to work properly (alternate stack not required).
Revision 1.2  2004/10/06 16:53:26  parhug
- Use the "SO" constructor/destructor instead of <_init()/_fini()>
- Add signal handler with an alternate signal stack to handle SIGIO (with sigaction) from our kernel drivers.
Revision 1.1  2004/08/19 12:26:06  parhug
Initial revision

*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <sys/time.h>
#include <sched.h>

#include <sys/termios.h>

#include <corposix.h>
#include <GDrv_Common.h>
#include <linuxobjs.h>

//??????????????debug??????????????????
LONG g_signalsendcount = 0;
LONG g_signalrecvcount = 0;

void GetSignalCount( PLONG send, PLONG recv)
{
	if (send != NULL) *send = g_signalsendcount;
	if (recv != NULL) *recv = g_signalrecvcount;
}

void CorW32IncrementSignalSendCount()
{
	InterlockedIncrement( &g_signalsendcount);
}
void CorW32IncrementSignalRecvCount()
{
	InterlockedIncrement( &g_signalrecvcount);
}

//????????????debug????????????????????

static int			AttachCount = 0;
static void *alt_stack_ptr = NULL;

#ifndef __CELL__
void InitHandleRegistry(void);
int  FreeHandleRegistry(void);

void _w32_init(void) __attribute__((constructor));
void _w32_fini(void) __attribute__((destructor));

//------------------------------------------------------------------------------
// Signal handler for notification events from the kernel drivers.
//
// In this model, the sigaction member "si_value" contains a pointer to the
// handle that signalled. This handle is checked for validity before being
// used in case the signal did not come from us using this model.
//
static void _CorSignalHandler( int s, siginfo_t *si, void *context)
{
	HANDLE handle;

	// Make sure the info structure is not NULL.
	if ( si != NULL )
	{
		handle = si->si_ptr;
		// Validate the handle.
		if (IsRegisteredHandle(handle))
		{
			if (HANDLE_REF(handle)->sigid == s)
			{
				CorW32IncrementSignalSendCount(); //????????????????????????????????
				SetKernelEventObject( HANDLE_REF(handle));
				//printf("handler 0x%lx\n", (unsigned long)HANDLE_REF(handle)); 
			}
		}
	}
}


/*----------------------------------------------------------------------------*/
/* Library entry point */
void _w32_init(void)
{
	stack_t ss = {0};
	
#ifdef COR_TRACE_LIBINIT
	printf("libCorW32 Init called - AttachCount = %d\n", AttachCount);
#endif
 	// See if this is already in use.
	if (!AttachCount++)
	{
		struct sigaction saction = {0};

		// First time in (for this program). Init the IPC subsystem.
		// CorIpcInit();
		// !No! - this just creates a SysV semaphore set for use IF needed later.
		// To save space, leave this out and if a semaphore set is needed later, 
		//	this will get called, otherwise it will not.
		
		// Initialize the handle registry for this process.
		InitHandleRegistry();

		// Initialize the signal handler settings for async events from kernel drivers.
		sigemptyset( &saction.sa_mask);
		//saction.sa_flags = SA_RESTART | SA_NOCLDSTOP | SA_NODEFER | SA_SIGINFO;
		saction.sa_flags = SA_RESTART | SA_NOCLDSTOP | SA_SIGINFO;   //*** Remove re-entrancy ***
		
		// Initialize the alternate stack for signal handling
		alt_stack_ptr = malloc(SIGSTKSZ);

		if (alt_stack_ptr != NULL)
		{
			// Install the signal handler to use the alternate stack.
			ss.ss_sp = alt_stack_ptr;
			ss.ss_size = SIGSTKSZ;
			ss.ss_flags = 0;
			if ( sigaltstack( &ss, NULL) == 0)
			{
				// Add flag for using the alternate stack.
				saction.sa_flags |= SA_ONSTACK;				
			}
			else
			{
				// Alternate stack setup failed. Revert.
				free(alt_stack_ptr);
				alt_stack_ptr = NULL;
			}
		}
		
		// Install a sigaction handler for SIGNAL_FOR_KERNEL_EVENT with the
		// flags as already set up.
		saction.sa_sigaction = _CorSignalHandler;
		sigaction( SIGNAL_FOR_KERNEL_EVENT, &saction, NULL);
	}
}

void _w32_fini(void)
{
#ifdef COR_TRACE_LIBINIT
	printf("libCorW32 Finit called - AttachCount = %d\n", AttachCount);
#endif
	// See if this device is in use.
	if (!--AttachCount)
	{
		struct sigaction saction = {0};

		// Disable the signal handler for async events from kernel drivers.
		// (i.e.)Install a sigaction handler for SIGIO.
		sigemptyset( &saction.sa_mask);
		saction.sa_handler = SIG_IGN;
		sigaction( SIGNAL_FOR_KERNEL_EVENT, &saction, NULL);

		// Free the alternate stack (if allocated)
		if (alt_stack_ptr != NULL)
		{
			free(alt_stack_ptr);
			alt_stack_ptr = NULL;
		}

		// Free the handle registry for this process.
		FreeHandleRegistry();

		// Last one out (for this program) closes the IPC subsystem down.
		CorIpcClose();

	}
}
#endif

FARPROC GetProcAddress( HMODULE hModule, char* lpProcName )
{
	return dlsym( hModule, lpProcName);
}

#define DEBUG_DYNAMIC_LOADING 1
HMODULE LoadLibrary( const char* lpFileName )
{
#if DEBUG_DYNAMIC_LOADING
	// Debug - resolve all library names now.
	void *ptr = dlopen( lpFileName, (RTLD_NOW));
	if (ptr == NULL)
   {
   	char *errstring = dlerror();
      // Trap missing plug-in error - it is OK for it to be mising.
      if (errstring != NULL)
      {
      	if ( strstr(errstring, "cannot open shared object file") == NULL)
      	{
				fprintf(stderr, "LoadLibrary - loading %s returns %s\n", lpFileName, errstring);
      	}
      }
   }
   return ptr;
#else
	// Normal - resolve library names as they are accessed.
	return dlopen( lpFileName, (RTLD_LAZY));
#endif
}

BOOL FreeLibrary( HMODULE hModule )
{
	if ( dlclose(hModule) == 0)
	{
		return TRUE;
	}
	return FALSE;
}

int GetEnvironmentVariable( const char* name, char* value, unsigned int len)
{
	int rlen = 0;
	char *ptr = NULL;
	ptr = getenv(name);
	if (ptr == NULL)
	{
		if (value != NULL)
		{
			value[0] = '\0';
		}
	}
	else
	{
		if (value != NULL)
		{
			int count = MIN(len-1, strlen(ptr));
			strncpy( value, ptr, count);
			value[count] = '\0';
			rlen = count;
		}
	}
	return rlen;
}

int GetLastError(void)
{
   return errno;
}

void SetLastError(int err)
{
	errno=0;
}

int GetSystemDirectory( char *buf, int size)
{
	strncpy(buf, "/usr/local/lib", (size_t)size);
	return (int)strlen(buf);
}

//====================================================================
//Conio-like Functions
//====================================================================

int kbhit(void)
{
  int cnt = 0;
  int error;
  unsigned char c;
  static struct termios Otty, Ntty;


  tcgetattr( 0, &Otty);
  Ntty = Otty;

  Ntty.c_iflag          = 0;       /* input mode                */
  Ntty.c_oflag          = 0;       /* output mode               */
  Ntty.c_lflag         &= ~ICANON; /* raw mode */
  Ntty.c_cc[VMIN]       = CMIN;    /* minimum time to wait      */
  Ntty.c_cc[VTIME]      = CTIME;   /* minimum characters to wait for */

  if (0 == (error = tcsetattr(0, TCSANOW, &Ntty))) 
  {
    //error += ioctl(0, FIONREAD, &cnt);
    cnt = read(0, &c, 1);
    error += tcsetattr(0, TCSANOW, &Otty);
  }


  return ( error == 0 ? cnt : -1 );
}

char getch()
{
  int cnt = 0;
  int error;
  unsigned char c[4];
  static struct termios Otty, Ntty;


  tcgetattr( 0, &Otty);
  Ntty = Otty;

  Ntty.c_iflag          = 0;       /* input mode                */
  Ntty.c_oflag          = 0;       /* output mode               */
  Ntty.c_lflag         &= ~ICANON; /* raw mode */
  Ntty.c_cc[VMIN]       = CMIN;    /* minimum time to wait      */
  Ntty.c_cc[VTIME]      = CTIME;   /* minimum characters to wait for */

  if (0 == (error = tcsetattr(0, TCSANOW, &Ntty))) 
  {
    //error += ioctl(0, FIONREAD, &cnt);
    cnt = read(0, c, 1);
    error += tcsetattr(0, TCSANOW, &Otty);
  }

  return c[0];
}

//====================================================================
// Work-arounds for gclibc version variations 
//====================================================================

int cor_sscanf( const char *str, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	return vsscanf(str, format, args);
}

void strcpy_safe( char *dest, size_t count, const char *src )
{
	if (count > 0)
	{
		strncpy( dest, src, count);
		dest[count-1] = 0;
	}
	return;
}
