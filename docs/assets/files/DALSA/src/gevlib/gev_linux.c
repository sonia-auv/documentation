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

/*! \file gev_linux.c
\briefLinux specific calls. Used to isolate operating system.

	These calls must be re-implemented on any new platform.
	They have mainly to do with multi-threading.
	This ensure it is 'relatively' easy to port this reference
	implementation to a new platform.

*/


//====================================================================
// INCLUDE FILES
//====================================================================


#include "gevoslib.h"	  // include all GEV Lib definitions
#include "gevapi.h"
#include "gevapi_internal.h"
//====================================================================
// IMPLEMENTATION FOR Linux
//====================================================================

//! Platform specific call to initialize the socket API
/*!
	This function is used to initialize the socket API.
	\return TRUE if successful, FALSE otherwise
	\note None
*/
BOOL _InitSocketAPI ()
{
	BOOL fStatus = TRUE;


	return fStatus;
}


//! Platform specific call to close the socket API
/*!
	This function is used to fully close socket API
	\return TRUE if successful, FALSE otherwise
	\note None
*/
BOOL _CloseSocketAPI ()
{
	BOOL fStatus = TRUE;

	return fStatus;
}

//! Platform specific call to get the maximum number of network interfaces in the system.
int _GetMaxNetworkInterfaces( void)
{
	char          ipbuf[1024];
	struct ifconf ipconfig;
	int           s;
	int           numAdapters;

	// Get a socket to talk to the network driver 
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if(s < 0)
	{
		perror("socket");
		return 1;
	}

	// Query the available interfaces/adapters. 
	ipconfig.ifc_len = sizeof(ipbuf);
	ipconfig.ifc_buf = ipbuf;
	if ( ioctl(s, SIOCGIFCONF, &ipconfig) < 0 )
	{
		GevPrint(GEV_LOG_ERROR, __FILE__, __LINE__, "_GetMacAddress : SIOCGIFCONF failed!\n");
		return FALSE;
	}

	// Check the number of adapters against the one requested.
	numAdapters = ipconfig.ifc_len / sizeof(struct ifreq);
	return numAdapters;
}

//! Platform specific call to retrieve MAC address of device
/*!
	This function is used to obtain the MAC and IP address of the network adapter
	\param [in] indexAdapter: Search Index of network adapter (0..(N-1))
	\param [out] pMacHigh: Pointer to upper 16-bit of MAC address
	\param [out] pMacLow: Pointer to upper 32-bit of MAC address
	\param [out] pIpAddr: Pointer to IP address associated with selected network adapter
	\param [out] pAdapterIndex: Pointer to hold internal adapter index
	\return TRUE if successful, FALSE otherwise
	\note  This function must be called before calling _SetIpAddress().
*/
BOOL _GetMacAddress (int indexAdapter, UINT16 *pMacHigh, UINT32 *pMacLow, UINT32 *pIpAddr, UINT32 *pAdapterIndex)
{
	char          ipbuf[1024];
	struct ifconf ipconfig;
	struct ifreq *ipreq;
	int           s;
	int           numAdapters;
	UINT32        ipAddr = 0;

	// Get a socket to talk to the network driver 
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if(s < 0)
	{
		perror("socket");
		return 1;
	}

	// Query the available interfaces/adapters. 
	ipconfig.ifc_len = sizeof(ipbuf);
	ipconfig.ifc_buf = ipbuf;
	if ( ioctl(s, SIOCGIFCONF, &ipconfig) < 0 )
	{
		GevPrint(GEV_LOG_ERROR, __FILE__, __LINE__, "_GetMacAddress : SIOCGIFCONF failed!\n");
		return FALSE;
	}

	// Check the number of adapters against the one requested.
	numAdapters = ipconfig.ifc_len / sizeof(struct ifreq);
	ipreq       = ipconfig.ifc_req;
	
	if (indexAdapter < numAdapters)
	{
		// Get the request structure for the indicated adapter index
		// (using the internal adapter index - not the order of detection index).
		struct ifreq *item = &ipreq[indexAdapter];
		struct ifreq temp_item;
		temp_item = *item;	

		if ( ioctl(s, SIOCGIFINDEX, &temp_item) < 0 )		
		{
			GevPrint(GEV_LOG_ERROR, __FILE__, __LINE__, "_GetMacAddress : SIOCGIFINDEX failed!\n");
			return FALSE;
		}
		else
		{
			if (pAdapterIndex != NULL)
			{ 
				*pAdapterIndex = temp_item.ifr_ifindex;
			}	
		}

		// Get the required information for the particular requested adapter.
		// Get the IP address
		struct sockaddr_in *sa = (struct sockaddr_in *)&item->ifr_addr;
		ipAddr = ntohl(*((u_int32_t *)&sa->sin_addr));
		if (pIpAddr != NULL)
		{
			*pIpAddr = ipAddr;
		}

		// Get the MAC address
		if ( ioctl(s, SIOCGIFHWADDR, item) < 0 )
		{
			GevPrint(GEV_LOG_ERROR, __FILE__, __LINE__, "_GetMacAddress : SIOCGIFHWADDR failed!\n");
			return FALSE;
		}
		else
		{
			struct sockaddr *eth = (struct sockaddr *) &item->ifr_ifru.ifru_hwaddr;
			unsigned long *low = (unsigned long *)&eth->sa_data[2];
			unsigned short *high = (unsigned short*)&eth->sa_data[0];
			//printf("%s : MAC = 0x%04x, 0x%08x", ntohs(*high), ntohl(*low));
			GevPrint(GEV_LOG_INFO, __FILE__, __LINE__,"Interface %8s : IP %3d.%3d.%3d.%3d : MAC = %02x:%02x:%02x:%02x:%02x:%02x\n", 
				item->ifr_name,
				((ipAddr >> 24)&0xff), ((ipAddr >> 16)&0xff), ((ipAddr >> 8)&0xff), (ipAddr&0xff),
				((ntohs(*high)>> 8)&0x00ff), (ntohs(*high)&0x00ff),
				((ntohl(*low)>> 24)&0x00ff), ((ntohl(*low)>> 16)&0x00ff), ((ntohl(*low)>> 8)&0x00ff), (ntohl(*low)&0x00ff));

			if ((pMacHigh != NULL) && (pMacLow != NULL))
			{
				*pMacHigh = *high;
				*pMacLow = *low;
			}
		}
		return TRUE;
	}

	return FALSE;
}

//! Platform specific call to retrieve MTU setting fpr NIC interface
/*!
	This function is used to obtain the MAC and IP address of the network adapter
	\param [in] indexAdapter: Internal index of network adapter (0..(N-1))
	\param [out] pMtu: Pointer to return MTU setting in (int).
	\return TRUE if successful, FALSE otherwise
	\note  This function must be called before calling _SetIpAddress().
*/
BOOL _GetMTUSetting (int indexAdapter, int *pMtu)
{
	char           ipbuf[1024];
	struct ifconf  ipconfig;
	struct ifreq  *ipreq;
	int            s;
	int            i;
	int            numAdapters;
	int            mtu_size = 1500;

	// Get a socket to talk to the network driver 
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if(s < 0)
	{
		perror("socket");
		return 1;
	}

	// Query the available interfaces/adapters. 
	ipconfig.ifc_len = sizeof(ipbuf);
	ipconfig.ifc_buf = ipbuf;
	if ( ioctl(s, SIOCGIFCONF, &ipconfig) < 0 )
	{
		GevPrint(GEV_LOG_ERROR, __FILE__, __LINE__, "_GetMacAddress : SIOCGIFCONF failed!\n");
		return FALSE;
	}

	// Check the number of adapters against the one requested.
	numAdapters = ipconfig.ifc_len / sizeof(struct ifreq);
	ipreq       = ipconfig.ifc_req;
	
	for (i = 0 ; i < numAdapters; i++)
	{
		// Get the request structure for the indicated adapter index
		// (using the internal adapter index - not the order of detection index).
		struct ifreq *item = &ipreq[i];
		struct ifreq temp_item;
		temp_item = *item;	

		if ( ioctl(s, SIOCGIFINDEX, &temp_item) < 0 )		
		{
			GevPrint(GEV_LOG_ERROR, __FILE__, __LINE__, "_GetMTUSetting : SIOCGIFINDEX failed!\n");
			return FALSE;
		}
		else
		{
			if (indexAdapter == temp_item.ifr_ifindex)
			{
				// Found the adapter we are after.
				ioctl(s, SIOCGIFMTU, &temp_item);
				mtu_size = temp_item.ifr_ifru.ifru_mtu;
				break;
			}
		}
	}
	if (pMtu != NULL)
	{
		*pMtu = mtu_size;
		return TRUE;
	}
	return FALSE;
}
//! Return last socket error
/*!
	This function is used retreive the last socket error value.
	\return Socket status code
	\note None
*/
int _GetSocketError ()
{
	return GetLastError();
	return 0;
}

//====================================================================
// HELPER
//====================================================================

//! Delay in ms
/*!
	This function is wait for a specified amount of time (in ms).
	\param [in] delay_ms Delay to wait in ms
	\return None
	\note None
*/
void _Wait (UINT32 delay_ms)
{
	Sleep (delay_ms);
}


//! Retreive a 64-bit timestamp in us
/*!
	This function returns a 64-bit timestamp representing the
	time in us since the system booted. GigE Vision specification
	does not specify the unit of the timestamp counter, but it
	does requires the counter to be 64-bit wide.
	\param [out] pHighTime Pointer to upper 32-bit of timestamp value
	\param [out] pLowTime Pointer to lower 32-bit of timestamp value
	\return TRUE if successful, FALSE otherwise
	\note This implementation relies on Pentium high performance timer
*/
BOOL _GetTimestamp (UINT32 *pHighTime, UINT32 *pLowTime)
{
	BOOL fStatus = TRUE;
	LARGE_INTEGER currentTime = {0};
	LARGE_INTEGER frequency = {0};

	// Check for valid parameters
	if ((pHighTime == NULL) || (pLowTime == NULL))
	{
		fStatus = FALSE;
	}

	// Retrieve frequency of high performance counter
	if (fStatus == TRUE)
	{
		// In practice, this call should be performed only once
		// during initialization.
		fStatus = QueryPerformanceFrequency (&frequency);
	}

	// Retrieve current time of high performance counter
	if (fStatus == TRUE)
	{
		fStatus = QueryPerformanceCounter (&currentTime);
	}

	// Convert timestamp into us
	if (fStatus == TRUE)
	{
		currentTime.QuadPart /= (frequency.QuadPart / 1000000);
	}

	// Return the timestamp
	if (fStatus == TRUE)
	{
		*pHighTime = currentTime.HighPart;
		*pLowTime = currentTime.LowPart;
	}
	else
	{
		// Performance counter not supported, force timestamp to 0.
		*pHighTime = 0;
		*pLowTime = 0;
	}

	return fStatus;
}

//! Convert a 32-bit value for output as a LittleEndian feature.
/*!
	This function converts a 32-bit value so that the API will output it
	as a LittleEndian feature.
	
	Note : The API assumes that all writes are to BigEndian (Network order)
	destinations. So, to output LittleEndian we need to swap it from host
	order to network order so the API write function will generate a LittleEndian
	on output.
	(A little confusing - yes).
	
	\param [in] cpu_data
	\return le_data
	\note None
*/
UINT32 _Convert_to_LEFeature_Order(UINT32 cpu_data)
{
	return htonl(cpu_data);
}

//! Convert an input LittleEndian feature to a host 32-bit value.
/*!
	This function converts an input 32-bit LittleEndian value from the API so 
	that it is handled properly by the API.
	
	Note : The API assumes that all reads are from BigEndian (Network order)
	destinations. So, to interpret an input LittleEndian we need to swap it from host
	order to network order after the API read function to generate the proper
	representation for the CPU.
	(A little confusing - yes).
	
	\param [in] cpu_data
	\return le_data
	\note None
*/
UINT32 _Convert_from_LEFeature_Order(UINT32 cpu_data)
{
	return htonl(cpu_data);
}

//! Convert 32-bit (CPU ordering) to little endian
/*!
	This function converts a 32-bit value to little endian.
	\param [in] cpu_data
	\return le_data
	\note None
*/
UINT32 _CPU_to_LE32 (UINT32 cpu_data)
{
	return __cpu_to_le32(cpu_data);
}
//! Convert 32-bit little endian to CPU ordering.
/*!
	This function converts a 32-bit little endian value to cpu order.
	\param [in] le_data
	\return cpu_data
	\note None
*/
UINT32 _LE32_to_CPU (UINT32 le_data)
{
	return __le32_to_cpu(le_data);
}
//! Convert 32-bit (CPU ordering) to big endian
/*!
	This function converts a 32-bit value to big endian.
	\param [in] cpu_data
	\return be_data
	\note None
*/
UINT32 _CPU_to_BE32 (UINT32 cpu_data)
{
	return __cpu_to_be32(cpu_data);
}
//! Convert 32-bit big endian to CPU ordering.
/*!
	This function converts a 32-bit big endian value to cpu order.
	\param [in] le_data
	\return cpu_data
	\note None
*/
UINT32 _BE32_to_CPU (UINT32 be_data)
{
	return __be32_to_cpu(be_data);
}

//====================================================================
// CRITICAL SECTION
//====================================================================

//! Create a critical section object
/*!
	This function creates and initialize a critical section object.
	\param [out] pCriticalSection Pointer to the critical section object
	\return TRUE if successful, FALSE otherwise.
	\note None
*/
BOOL _InitCriticalSection (_CRITICAL_SECTION *pCriticalSection)
{
	InitializeCriticalSection( pCriticalSection);
	return TRUE;
}

//! Release a critical section object
/*!
	This function releases resources associated with a critical
	section object.
	\param [in] pCriticalSection Pointer to the critical section object
	\return TRUE if successful, FALSE otherwise.
	\note None
*/
BOOL _ReleaseCriticalSection (_CRITICAL_SECTION *pCriticalSection)
{
	DeleteCriticalSection( pCriticalSection);
	return TRUE;
}

//! Enter in a critical section
/*!
	This function is used to take control of the critical section.
	\param [in] pCriticalSection Pointer to the critical section object
	\return TRUE if successful, FALSE otherwise.
	\note None
*/
BOOL _EnterCriticalSection (_CRITICAL_SECTION *pCriticalSection)
{
	EnterCriticalSection( pCriticalSection);
	return TRUE;
}

//! Leave a critical section
/*!
	This function releases control of the critical section
	\param [in] pCriticalSection Pointer to the critical section object
	\return TRUE if successful, FALSE otherwise.
	\note None
*/
BOOL _LeaveCriticalSection (_CRITICAL_SECTION *pCriticalSection)
{
	LeaveCriticalSection( pCriticalSection);
	return TRUE;
}


//====================================================================
// EVENT
//====================================================================

//! Create a manual reset event object
/*!
	This function is used to create an event object
	\param [out] pEvent Pointer to the event object
	\return TRUE if successful, FALSE otherwise.
	\note The event must be manually set and clear by the program.
*/
BOOL _CreateEvent (_EVENT *pEvent)
{
	BOOL fStatus = TRUE;

	// Validate parameters
	if (pEvent == NULL)
	{
		fStatus = FALSE;
	}

	// Create an unnamed manual reset event under Windows
	if (fStatus == TRUE)
	{
		*pEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
		if (*pEvent == NULL)
		{
			GevPrint(GEV_LOG_ERROR, __FILE__, __LINE__, "ERROR: Unable to create event\n\b");
			fStatus = FALSE;
		}
	}

	return fStatus;
}


//! Destroy an event object
/*!
	This function is used to destroy an event object
	\param [in] pEvent Pointer to the event object
	\return TRUE if successful, FALSE otherwise.
	\note None
*/
BOOL _DestroyEvent (_EVENT *pEvent)
{
	BOOL fStatus;

	fStatus = CloseHandle (*pEvent);
	*pEvent = NULL;
	return fStatus;
}


//! Waits until event gets signaled
/*!
	This function wait for the event to get signaled or for 
	the timeout to occur.
	\param [in] pEvent Pointer to the event object
	\param [in] timeout Timeout before event gets signaled in ms
	\return TRUE if event is signaled, FALSE on timeout.
	\note None
*/
BOOL _WaitForEvent (_EVENT *pEvent, UINT32 timeout)
{
	BOOL fStatus = TRUE;
	UINT32 waitStatus;

	// Validate parameters
	if (pEvent == NULL)
	{
		fStatus = FALSE;
	}

	// Wait for event to get signaled
	if (fStatus == TRUE)
	{
		waitStatus = WaitForSingleObject (*pEvent, timeout);
		if (waitStatus == WAIT_FAILED)
		{
			GevPrint(GEV_LOG_ERROR, __FILE__, __LINE__, "ERROR: WaitForSingleObject failed in _WaitForEvent\n\b");
			fStatus = FALSE;
		}
		else if (waitStatus != WAIT_OBJECT_0)
		{
			fStatus = FALSE;	// timeout expired
		}
	}

	return fStatus;
}


//! Clear event object
/*!
	This function is used clear the event object
	\param [in] pEvent Pointer to the event object
	\return TRUE if successful, FALSE otherwise.
	\note None
*/
BOOL _ClearEvent (_EVENT *pEvent)
{
	BOOL fStatus = TRUE;

	// Validate parameters
	if (pEvent != NULL)
	{
		fStatus = ResetEvent( *pEvent);
	}
	else
	{
		fStatus = FALSE;
	}

	return fStatus;
}


//! Set event object
/*!
	This function is used to signal the event object
	\param [in] pEvent Pointer to the event object
	\return TRUE if successful, FALSE otherwise.
	\note None
*/
BOOL _SetEvent (_EVENT *pEvent)
{
	BOOL fStatus = TRUE;

	// Validate parameters
	if (pEvent != NULL)
	{
		fStatus = SetEvent( *pEvent);
	}
	else
	{
		fStatus = FALSE;
	}

	return fStatus;
}


//====================================================================
// MUTEX
//====================================================================

//! Create a mutex object
/*!
	This function is used to create a mutex object
	\param [out] pMutex Pointer to the mutex object
	\return TRUE if successful, FALSE otherwise.
	\note The mutex is intially unowned.
*/
BOOL _CreateMutex (_MUTEX *pMutex)
{
	BOOL fStatus = TRUE;

	// Validate parameters
	if (pMutex == NULL)
	{
		fStatus = FALSE;
	}

	// Create an unnamed manual reset event under Windows
	if (fStatus == TRUE)
	{
		*pMutex = CreateMutex (NULL, FALSE, NULL);
		if (*pMutex == NULL)
		{
			GevPrint(GEV_LOG_ERROR, __FILE__, __LINE__, "ERROR: Unable to create mutex\n\b");
			fStatus = FALSE;
		}
	}

	return fStatus;
}


//! Destroy a mutex object
/*!
	This function is used to destroy a mutex object
	\param [in] pMutex Pointer to the mutex object
	\return TRUE if successful, FALSE otherwise.
	\note None
*/
BOOL _DestroyMutex (_MUTEX *pMutex)
{
	BOOL fStatus;

	fStatus = CloseHandle (*pMutex);
	*pMutex = NULL;
	return fStatus;
}

//! Acquire mutex
/*!
	This function is used to acquire a mutex.
	\param [in] pMutex Pointer to the mutex object
	\param [in] timeout Timeout before event gets signaled in ms
	\return TRUE if successful, FALSE otherwise.
	\note None
*/
BOOL _AcquireMutex (_MUTEX *pMutex, UINT32 timeout)
{
	BOOL fStatus = TRUE;
	UINT32 waitStatus;

	// Validate parameters
	if (pMutex == NULL)
	{
		fStatus = FALSE;
	}
	//printf("_AcquireMutex : mutex is = 0x%x\n", (unsigned int)*pMutex); //????????????????????????????

	// Wait for event to get signaled
	if (fStatus == TRUE)
	{
		waitStatus = WaitForSingleObject (*pMutex, timeout);
		if (waitStatus == WAIT_FAILED)
		{
			GevPrint(GEV_LOG_ERROR, __FILE__, __LINE__, "ERROR: WaitForSingleObject failed in _AcquireMutex\n\b");
			fStatus = FALSE;
		}
		else if (waitStatus != WAIT_OBJECT_0)
		{
	//printf("_AcquireMutex : waitStatus = %d\n", waitStatus); //????????????????????????????
			fStatus = FALSE;	// timeout expired
		}
	}

	return fStatus;
}

//! Release mutex
/*!
	This function is used to release an acquired mutex.
	\param [in] pMutex Pointer to the mutex object
	\return TRUE if successful, FALSE otherwise.
	\note None
*/
BOOL _ReleaseMutex (_MUTEX *pMutex)
{
	return ReleaseMutex (*pMutex);
}


//====================================================================
// THREADING
//====================================================================

//! Create a thread
/*!
	This function is used to create a new thread
	\param [in] fct Function defining the thread
	\param [in] context Context to pass to the thread
	\param [in] thread priority
	\param [out] pThread Pointer to a thread object
	\return TRUE if successful, FALSE otherwise.
	\note None
*/
BOOL _CreateThread (unsigned _stdcall fct(void *), void *context, int priority, _THREAD *pThread)
{
	BOOL fStatus = TRUE;

	// Validate parameters
	if (pThread == NULL)
	{
		fStatus = FALSE;
	}

	if (fStatus == TRUE)
	{
		*pThread = (_THREAD)_beginthreadex (NULL, 0, fct, context, 0, NULL);
		if (*pThread == (_THREAD) -1)
		{
			GevPrint(GEV_LOG_ERROR, __FILE__, __LINE__, "ERROR: Unable to create thread\n\b");
			fStatus = FALSE;
		}
	}

	if (fStatus == TRUE)
	{
		fStatus = SetThreadPriority (*pThread, priority);
		if (fStatus == FALSE)
		{
			GevPrint(GEV_LOG_ERROR, __FILE__, __LINE__, "ERROR: Unable to set thread priority\n\b");
			fStatus = TRUE; // Silently ignore thread prioirity warnings -> need to use "sudo" in Linux.
		}
	}

	return fStatus;
}


//! Wait for a thread to get signaled
/*!
	This function wait for thread to get signaled or for timeout to expire.
   Signalled threads have exitted and have their handle objects closed.
	\param [in] pThread Pointer to thread object
	\param [in] timeout Timeout (in ms) for thread to exit
	\return TRUE if successful, FALSE otherwise.
	\note A thread is signaled when it exits
*/
BOOL _WaitForThread (_THREAD *pThread, UINT32 timeout)
{
	BOOL fStatus = TRUE;
	UINT32 waitStatus;

	// Validate parameters
	if ((pThread == NULL) || (*pThread == NULL))	// Ensure we don't have an ununitialized thread
	{
		fStatus = FALSE;
	}

	// Wait for thread to complete
	if (fStatus == TRUE)
	{
		waitStatus = WaitForSingleObject (*pThread, timeout);
		if (waitStatus == WAIT_FAILED)
		{
			GevPrint(GEV_LOG_ERROR, __FILE__, __LINE__, "ERROR: WaitForSingleObject failed in _WaitForThread\n\b");
			fStatus = FALSE;
		}
		else if (waitStatus != WAIT_OBJECT_0)
		{
			GevPrint(GEV_LOG_INFO, __FILE__, __LINE__, "INFO: WaitForSingleObject timed out in _WaitForThread\n\b");
			fStatus = FALSE;	// timeout expired
		}
		else
		{
			// Its all good - close the thread handle.
			CloseHandle(*pThread);
		}
	}

	return fStatus;
}

int _GetNumCpus()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}


//====================================================================
// Timing Helper Functions
//====================================================================
BOOL _IsTimedOut( struct timeval *due_time)
{
   struct timeval now;
   unsigned long msec_due;
   unsigned long msec;
   
   // Convert the the due time to a millisecond counter.
    msec_due = (due_time->tv_sec * 1000) + (due_time->tv_usec / 1000);
  
   // Get the current time and turn it into a millisecond counter.
   gettimeofday( &now, NULL);
   msec = (now.tv_sec * 1000) + (now.tv_usec / 1000);
    
   // Check if we are past the due time.
   //
   // (Note : This wraps every 50 days or so. Function may be wrong once 
   //	        every 50 days. So - we make the maximum timeout 
   //	        1024*1024*1024 msec (12 days)).
   if ( msec >= msec_due)
   {
		return TRUE;
	}
	else
	{
		// May have wrapped (check -w.r.t. limit of max timeout interval).
		if ( (msec_due - msec) > 0x40000000)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
}

void _GetTimeOut( int interval_ms, struct timeval *due_time)
{
	if (due_time != NULL)
	{
		struct timeval now;
		unsigned long msec_now;
		unsigned long msec_due;
		
		// Get the current time (as msecs)
		gettimeofday( &now, NULL);
		msec_now = (now.tv_sec * 1000) + (now.tv_usec / 1000);
		
		// Add the interval.
		msec_due = msec_now + interval_ms;
		
		// See if it wrapped (near the 32-bit limit).
		due_time->tv_sec = (msec_due / 1000);
		due_time->tv_usec = 1000*(msec_due - (1000 * due_time->tv_sec));
	}
}

//====================================================================
// Logging Helper Function
//====================================================================

#if 1
int GevPrint( int level, const char *file, unsigned int line, const char *fmt, ...)
{
	int status = 0;

	if ( GevGetLogLevel() <= (UINT32)level )
	{
		return status;
	}
	else
	{
		va_list marker;	
		char filename[256];
		int num = 0;
		int len = 0;
		int i = 0;

		char *logtext[] = {"<FAT>", "<ERR>", "<WRN>", "<INFO>", "<TRACE>"};

		len = strlen(file);
		for (i = (len-1); i >= 0; i--)
		{
			if (file[i] == '/')
				break;
		}
		num = ((len-i) > (sizeof(filename)-1)) ? (sizeof(filename) - 1) : (len-i);
		strncpy(filename, &file[i+1], num);
		filename[num+1] = '\0';

		va_start(marker,fmt);
		fprintf(stdout, "%s:%s:(%d):", logtext[level], filename, line);
		vfprintf(stdout, fmt, marker);
		va_end(marker);
	}
	return status;
}
#else
int GevPrint( int level, const char *fmt, ...)
{
	int status = 0;
	va_list marker;

	if ( GevGetLogLevel() <= (UINT32)level )
	{
		return status;
	}
	
	va_start(marker,fmt);
	vfprintf(stdout, fmt, marker);
	va_end(marker);
	return status;
}
#endif

static int AttachCount = 0;
/*----------------------------------------------------------------------------*/
/* Library entry point */
void _gevapi_init(void)
{
#ifdef COR_TRACE_LIBINIT
	printf("libGevApi Init called - AttachCount = %d\n", AttachCount);
#endif
 	// See if this is already in use.
	if (!AttachCount++)
	{
	}
}

void _gevapi_fini(void)
{
#ifdef COR_TRACE_LIBINIT
	printf("libGevApi Fini called - AttachCount = %d\n", AttachCount);
#endif
	// See if this device is in use.
	if (!--AttachCount)
	{
	}
}


