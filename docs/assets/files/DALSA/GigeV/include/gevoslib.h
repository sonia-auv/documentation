/***************************************************************** 
Copyright (c) 2008-2012, Teledyne DALSA Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions 
are met:
	-Redistributions of source code must retain the above copyright 
	notice, this list of conditions and the following disclaimer. 
	-Redistributions in binary form must reproduce the above 
	copyright notice, this list of conditions and the following 
	disclaimer in the documentation and/or other materials provided 
	with the distribution. 
	-Neither the name of DALSA Coreco nor the names of its 
	contributors may be used to endorse or promote products derived 
	from this software without specific prior written permission. 

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
*******************************************************************/

/*! \file gevoslib.h
\brief Header file for GEV O/S independence wrapper library.

  This header files includes definitions of function to be provided
  when supporting the GEV API under various operating systems.

*/

#ifndef _GEVOSLIB_H_
#define _GEVOSLIB_H_			//!< used to avoid multiple inclusion

//====================================================================
// INCLUDES
//====================================================================

#include "corenv.h"				//!< Environment definitions.

#if COR_WIN32
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gev_windows.h"		//!< Windows-specific definitions
#endif

#if COR_LINUX
#include "gev_linux.h"			//!< Linux-specific definitions
#endif

//====================================================================
// PROTOTYPES
//====================================================================

//--------------------------------------------------------------------
// OS-specific wrapper

// Socket API
BOOL _InitSocketAPI ();
BOOL _CloseSocketAPI ();
int _GetSocketError ();

// IP stack and network card
int  _GetMaxNetworkInterfaces( void);
BOOL _GetMacAddress (int indexAdapter, UINT16 *pMacHigh, UINT32 *pMacLow, UINT32 *pIpAddr, UINT32 *pAdapterIndex);
BOOL _SetIPAddress (UINT32 ipClient, UINT32 ipMask);
BOOL _GetMTUSetting (int indexAdapter, int *pMtu);

// Event function prototypes
BOOL _CreateEvent (_EVENT *pEvent);
BOOL _DestroyEvent (_EVENT *pEvent);
BOOL _WaitForEvent (_EVENT *pEvent, UINT32 timeout);
BOOL _ClearEvent (_EVENT *pEvent);
BOOL _SetEvent (_EVENT *pEvent);

// Mutex function prototype.
BOOL _CreateMutex (_MUTEX *pMutex);
BOOL _DestroyMutex (_MUTEX *pMutex);
BOOL _AcquireMutex (_MUTEX *pMutex, UINT32 timeout);
BOOL _ReleaseMutex (_MUTEX *pMutex);

// Critical section prototypes
BOOL _InitCriticalSection (_CRITICAL_SECTION *pCriticalSection);
BOOL _ReleaseCriticalSection (_CRITICAL_SECTION *pCriticalSection);
BOOL _EnterCriticalSection (_CRITICAL_SECTION *pCriticalSection);
BOOL _LeaveCriticalSection (_CRITICAL_SECTION *pCriticalSection);

// Thread function prototypes
//BOOL _CreateThread (unsigned int fct(void *), void *context, int priority, _THREAD *pThread);
BOOL _CreateThread (unsigned int _stdcall fct(void *), void *context, int priority, _THREAD *pThread);
BOOL _WaitForThread (_THREAD *pThread, UINT32 timeout);
int _GetNumCpus();

// Timeout handling.
BOOL _IsTimedOut( struct timeval *due_time);
void _GetTimeOut( int interval_ms, struct timeval *due_time);

// Miscellaneous
void _Wait (UINT32 delay_ms);
BOOL _GetTimestamp (UINT32 *pHighTime, UINT32 *pLowTime);
UINT32 _Convert_to_LEFeature_Order(UINT32 cpu_data);
UINT32 _Convert_from_LEFeature_Order(UINT32 cpu_data);
UINT32 _CPU_to_LE32 (UINT32 cpu_data);
UINT32 _LE32_to_CPU (UINT32 le_data);
UINT32 _CPU_to_BE32 (UINT32 cpu_data);
UINT32 _BE32_to_CPU (UINT32 be_data);


#endif // _GEVOSLIB_H_

