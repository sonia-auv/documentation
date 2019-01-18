/****************************************************************************** 
Copyright (c) 2008-2015, Teledyne DALSA Inc.
All rights reserved.

File : gevgenapi.c
	Public API for GenApi on top of GEV C library.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions 
are met:
	-Redistributions of source code must retain the above copyright 
	notice, this list of conditions and the following disclaimer. 
	-Redistributions in binary form must reproduce the above 
	copyright notice, this list of conditions and the following 
	disclaimer in the documentation and/or other materials provided 
	with the distribution. 
	-Neither the name of Teledyne DALSA nor the names of its contributors 
	may be used to endorse or promote products derived 
	from this software without specific prior written permission. 

===============================================================================
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
******************************************************************************/
/*! \file gevgenapi.c
\brief GEV API definitions.

*/

#include "gevgenapi.h"
#include "gevapi_internal.h"

using namespace GenApi;


//======================================================================
// CGevPort (IPort) class implementation.
GenApi::EAccessMode CGevPort::GetAccessMode() const
{
	// If the driver is open, return RW (= read/write), 
	// otherwise NA (= not available)
	if (GevCameraIsOpen(m_handle))
	{
		if (GevCameraIsWritable(m_handle))
		{
			return GenApi::RW;
		}
		else
		{
			return GenApi::RO;
		}
	}
	else
	{
		return GenApi::NA;
	}
}

void CGevPort::Read(void *pBuffer, int64_t Address, int64_t Length)
{
	GEV_STATUS status = GEVLIB_OK;
	if(!m_handle)
		throw RUNTIME_EXCEPTION("Invalid device");
	if(Length == 4)
	{
		UINT32 reg;
		status = Gev_ReadReg( m_handle, (UINT32) Address,&reg);
		if( status == GEVLIB_OK)
		{
			*(UINT32 *)pBuffer = ntohl(reg);
		}
		else
			throw RUNTIME_EXCEPTION("Error 0x%08x reading register address 0x%llx",status, Address);
	}
	else
	{
		status = Gev_ReadMem( m_handle, (UINT32) Address, (char *)pBuffer,  (UINT32)Length);
		if( status)
			throw RUNTIME_EXCEPTION("Error 0x%08x reading memory address 0x%llx length %lld",status, Address, Length);
	}
}

void CGevPort::Write(const void *pBuffer, int64_t Address, int64_t Length)
{
	GEV_STATUS status = GEVLIB_OK;
	if(!m_handle)
		throw RUNTIME_EXCEPTION("Invalid device");
	if((Length == 4) && !GevIsStringAddress(Address))
	{
		UINT32 reg = htonl(*(UINT32 *)pBuffer);
		status = Gev_WriteReg(m_handle, (UINT32) Address, reg);
		if(status)
			throw RUNTIME_EXCEPTION("Error 0x%08x writing register address 0x%llx",status, Address);
	}
	else
	{
		status = Gev_WriteMem( m_handle, (UINT32) Address, (char *)pBuffer,  (UINT32)Length);
		if( status)
			throw RUNTIME_EXCEPTION("Error 0x%08x writing memory address 0x%llx length %lld",status, Address, Length);
	}
}
//======================================================================



#ifdef __cplusplus
extern "C" {
#endif

static BOOL m_APIInit = FALSE;

// Internal info for detected NIC and GEV devices.
#define GEV_DEFAULT_NUM_DEVICES		32
static UINT32 						m_numNetIF = 0;
static PGEV_NETWORK_INTERFACE m_pNetIF   = NULL;
static UINT32 						m_numDevices = 0;
static UINT32 						m_maxDevices = 0;
static PGEV_DEVICE_INTERFACE	m_pDevice = NULL;
static _CRITICAL_SECTION		m_csUpdateLock;
static GEV_CAMERA_HANDLE		*m_openCameraList = NULL;
static int							m_numOpenCameras = 0;
static BOOL							m_bEnableAutoDiscovery = TRUE;

// Internal defaults for API device controls.
static UINT32 m_discoveryTimeout			= 1000;
static UINT32 m_library_version			= 1;
static UINT32 m_library_logLevel			= 0;
static UINT32 m_numRetries					= 3;
static UINT32 m_command_timeout_ms		= 2000;
static INT32  m_enumeration_port			= 39999;		// Port to use for enumerating cameras.
static UINT16 m_gvcp_port_range_start	= 40000;		// Start value for range of ports to be used to connect to cameras.
static UINT16 m_gvcp_port_range_end		= 49999;		// End value for range of ports to be used to connect to cameras.
static UINT32 m_streamPktSize				= 1500;
static UINT32 m_streamPktDelay			= 0;


// Table for managing ports assigned to devices.
static UINT32 *m_portTable		 = NULL;
static int     m_portTableSize = 0;

static UINT16 _GetFreePort()
{
	int i, j;
	UINT16 port = 0;

	if (m_portTable != NULL)
	{
		for (i = 0; i < m_portTableSize; i++)
		{
			if (m_portTable[i] != 0xffffffff)
			{
				UINT32 portMap = m_portTable[i];

				for (j = 0; j < 32; j++)
				{
					if ((portMap >> j) == 0)
					{
						port = m_gvcp_port_range_start + 32*i + j;
						portMap |= (1 << j);
						m_portTable[i] = portMap;
						return port;
					}
				}
			}
		}
	}
	return port;
}

static void _PutFreePort(UINT16 port)
{
	int i, j;
	UINT32 mask;

	if (m_portTable != NULL)
	{
		i = (port - m_gvcp_port_range_start)/32;
		j = (port - m_gvcp_port_range_start)%32;

		mask = (1 << j);
		m_portTable[i] &= ~mask;
	}
	return;
}

static GEV_STATUS	_GetAvailablePorts( PUINT16 ports)
{
	GEV_STATUS status = GEVLIB_ERROR_NULL_PTR ;

	if (ports != NULL)
	{
		status = GEVLIB_ERROR_INSUFFICIENT_MEMORY;
		int i = 0;

		if (m_portTable == NULL)
		{
			// Create port table - 1 bit per port.
			m_portTableSize = (m_gvcp_port_range_end - m_gvcp_port_range_start + 31)/32;
			m_portTable = (UINT32 *)malloc(m_portTableSize*sizeof(UINT32));

			if (m_portTable == NULL)
			{
				return status;
			}
			memset(m_portTable, 0, m_portTableSize*sizeof(UINT32));
		}

		for (i = 0; i < GEV_NUM_PORTS_PER_HANDLE; i++)
		{
			ports[i] = _GetFreePort();
		}
		status = GEVLIB_OK;
	}
	return status;
}

static GEV_STATUS	_ReleasePorts( PUINT16 ports)
{
	GEV_STATUS status = GEVLIB_ERROR_NULL_PTR;

	if (ports != NULL)
	{
		status = GEVLIB_OK;
		int i = 0;

		for (i = 0; i < GEV_NUM_PORTS_PER_HANDLE; i++)
		{
			if (ports[i] != 0)
			{
				_PutFreePort(ports[i]);
			}
		}
	}
	return status;
}

//=============================================================================
// API Initialization and Management 
//
//! Initialize the GigE Vision API.
//
/*!
	This function is used initialize the API.
	\return status
	\note None
*/
GEV_STATUS	GevApiInitialize(void)
{
	GEV_STATUS status = GEVLIB_OK;
	if (!m_APIInit)
	{
		// Init socket API
		_InitSocketAPI ();

		// Initialize the critical section for updating the library state.
		_InitCriticalSection( &m_csUpdateLock );

		// Allocate the initial storage for camera lists.
		m_maxDevices = GEV_DEFAULT_NUM_DEVICES;
		m_numDevices = 0;
		if (m_pDevice != NULL)
		{	
			free(m_pDevice);
			m_pDevice = NULL;
		}
		m_pDevice = (PGEV_DEVICE_INTERFACE) malloc(m_maxDevices * sizeof(GEV_DEVICE_INTERFACE));

		if (m_openCameraList != NULL)
		{
			free(m_openCameraList);
			m_openCameraList = NULL;
		}
		m_openCameraList = (GEV_CAMERA_HANDLE *) malloc(2*m_maxDevices * sizeof(GEV_CAMERA_HANDLE));
		memset(m_openCameraList, 0, 2*m_maxDevices * sizeof(GEV_CAMERA_HANDLE));


		m_numNetIF = 0;
		if (m_pNetIF != NULL)
		{
			free(m_pNetIF);
			m_pNetIF = NULL;
		}

		// Initialize the default library options.
		m_discoveryTimeout		= 1000;
		m_library_version			= 1;
		m_numRetries				= 3;
		m_command_timeout_ms		= 2000;
		m_enumeration_port		= 39999;
		m_gvcp_port_range_start	= 40000;
		m_gvcp_port_range_end	= 49999;
		m_streamPktSize			= 1500;
		m_streamPktDelay			= 0;
		m_bEnableAutoDiscovery  = TRUE;

		//======================================================================
		// Future - possible additions : 
		// - Add a function to register a callback to a global shared named 
		//		event so all processes can be notified if there are changes detected.
		// - Either start a thread to monitor all the comings and goings of cameras
		// 	or use a separate daemon process and shared memory. (Might require an
		//		atexit() function for automatic cleanup.
		//=======================================================================

		// Intialization is now done;
		m_APIInit = TRUE;
	}
	return status;
}

//
//! Shutdown the GigE Vision API.
//
/*!
	This function is used un-initialize the API.
	\return status
	\note None
*/
GEV_STATUS	GevApiUninitialize(void)
{
	GEV_STATUS status = GEVLIB_OK;
	int i = 0;
	if (m_APIInit)
	{
		// Indicate we are shutting down (to keep other threads away)
		m_APIInit = FALSE;

		// Go through the open camera handle list and close the cameras.
		if (m_openCameraList != NULL)
		{
			for (i = 0; i < m_numOpenCameras; i++)
			{
				GevCloseCamera(&m_openCameraList[i]);
			}
		}

		// Acquire the critical section.
		_EnterCriticalSection(&m_csUpdateLock);

		// Free the storage for camera lists.
		if (m_openCameraList != NULL)
		{
			free(m_openCameraList);
			m_openCameraList = NULL;
			m_numOpenCameras = 0;
		}

		m_maxDevices = GEV_DEFAULT_NUM_DEVICES;
		m_numDevices = 0;
		if (m_pDevice != NULL)
		{	
			free(m_pDevice);
			m_pDevice = NULL;
		}
		if (m_pNetIF != NULL)
		{
			free(m_pNetIF);
			m_pNetIF = NULL;
			m_numNetIF = 0;
		}

		// Free the port table.
		if (m_portTable != NULL)
		{
			free(m_portTable);
			m_portTable = NULL;
		}

		// Free the critical section for updating the library state.
		_LeaveCriticalSection( &m_csUpdateLock );
		_ReleaseCriticalSection( &m_csUpdateLock );

		// Stop communication with the socket API.
		_CloseSocketAPI ();		
		
	}
	return status;
}

//=============================================================================
// Camera Discovery functions
//
//! Count the total number of GigE Vision devices present.
//
/*!
	This function is used count all of the detectable GigE Vision devices 
	present in the system. The device discovery procedure is used to perform
	the detection.
	\param [in] pNumDevices Pointer to return the number of devices found..
	\return Number of devices present.
	\note None
*/
static GEV_STATUS _DetectAttachedDevices( PUINT32 pNumDevices )
{
	UINT32 numNetIF = 0;
	UINT32 i;
	UINT32 numFound = 0;
	UINT32 numCamera = 0;
	UINT32 numFree = 0;
	GEV_STATUS status = GEVLIB_OK;

	numNetIF = _GetMaxNetworkInterfaces();

	if (numNetIF != m_numNetIF)
	{
		// Re-scan the network interfaces.
		if (m_numNetIF < numNetIF)
		{
			PGEV_NETWORK_INTERFACE pNetIF = (PGEV_NETWORK_INTERFACE) malloc(numNetIF * sizeof(GEV_NETWORK_INTERFACE));
			if (m_pNetIF != NULL)
			{
				// Re-size the internal 
				_EnterCriticalSection(&m_csUpdateLock);
				free(m_pNetIF);
				m_pNetIF = pNetIF;
				m_numNetIF = numNetIF;
				_LeaveCriticalSection(&m_csUpdateLock);
			}
			else
			{
				m_pNetIF = pNetIF;
				m_numNetIF = numNetIF;
			}

			// Enumerate the network interfaces.
			status = GevEnumerateNetworkInterfaces(m_pNetIF, m_numNetIF, &numNetIF);		
		}
	}

	// Discover all the devices on each interface, count them, and load them into the internal camera list.
	numFree = m_maxDevices;
	for (i = 0; i < numNetIF; i++)
	{
		BOOL done = FALSE;

		// Fill out the device list for this network card.
		while( !done)
		{
			status = GevEnumerateGevDevices( &m_pNetIF[i], m_discoveryTimeout, &m_pDevice[numCamera],
										numFree, &numFound);
			if (numFound < numFree)
			{
				numCamera += numFound;
				numFree -= numFound;
				done = TRUE;
			}
			else
			{
				// Allocate more space for the device info structures.
				PGEV_DEVICE_INTERFACE pDevice = NULL;
				GEV_CAMERA_HANDLE *pHandles = NULL;
				UINT32 delta = 0;
				delta = (numFound > GEV_DEFAULT_NUM_DEVICES) ? numFound : GEV_DEFAULT_NUM_DEVICES;
	
				// Allocate extra space (delta) for new camera in the list. (within a locked critical section).
				_EnterCriticalSection(&m_csUpdateLock);
				pDevice = (PGEV_DEVICE_INTERFACE) malloc((m_maxDevices+delta) * sizeof(GEV_DEVICE_INTERFACE));
				memcpy(pDevice, m_pDevice, (m_maxDevices * sizeof(GEV_DEVICE_INTERFACE)));
				m_maxDevices += delta;
				free(m_pDevice);
				m_pDevice = pDevice;
				numFree += delta;
				
				// Keep the possible number of open handles at twice the maximum number of cameras in the list.
				pHandles = (GEV_CAMERA_HANDLE *) malloc(2*m_maxDevices * sizeof(GEV_CAMERA_HANDLE));
				memcpy(pHandles, m_openCameraList, m_numOpenCameras*sizeof(GEV_CAMERA_HANDLE));
				free(m_openCameraList);
				m_openCameraList = pHandles;

				_LeaveCriticalSection(&m_csUpdateLock);		
			}
		}
	}
	m_numDevices = numCamera;
	if (pNumDevices != NULL)
	{
		*pNumDevices = numCamera;
	}
	return status;
}


//! Count the total number of GigE Vision devices present.
//
/*!
	This function is used count all of the detectable GigE Vision devices 
	present in the system. The device discovery procedure is used to perform
	the detection.
	\param None
	\return Number of devices present.
	\note None
*/
int GevDeviceCount(void)
{
	UINT32 numCamera = 0;
	
	_DetectAttachedDevices( &numCamera );

	return numCamera;
}

//! Get a list containig information on all of the GigE Vision devices present.
//
/*!
	This function is used to fill a list of GEV_CAMERA_INFO data structures, one
	for each device present in the system. The list is "maxCameras" entries long.
	The number of actual devices in the system is returned in "numCameras" (unless
	"numCameras" is NULL).
	\param [in] camera  		Pointer to a list of GEV_CAMERA_INFO entries.
	\param [in] maxCameras 	Number of entries that the list can contain.
	\param [in] numCameras  Pointer to hold the number of devices actually present.
	\return Number of devices present.
	\note None
*/
GEV_STATUS GevGetCameraList( GEV_CAMERA_INFO *cameras, int maxCameras, int *numCameras)
{
	GEV_STATUS status = GEVLIB_OK;
	UINT32 num = 0;

	if (!m_APIInit)
	{
		// Initialize the API.
		GevApiInitialize();
	}

	// Fill up the camera list.
	if (m_bEnableAutoDiscovery)
	{
		status = _DetectAttachedDevices( &num );
	}

	// Copy only what there is room for into the passed in data structure.
	if (cameras != NULL)
	{
		int count = (maxCameras < (int)num) ? maxCameras : (int)num;

		if (count > 0)
		{
			memcpy(cameras, m_pDevice, (count * sizeof(GEV_CAMERA_INFO)));
		}
	}

	// Update the number of cameras found (can be larger than that returned in the information array.
	if (numCameras != NULL)
	{
		*numCameras = (int)num;
	}

	return status;
}

//! Force IP address
/*!
	This function is used to force the IP address of a device to a known value.
	It is part of the GiGE functions that a device must support. The device 
	is identified by MAC address.
	NOTE : A returned error may indicate that the command was silently discarded
          rather than being an actual error.
	\param [in] pNetIF	Network interface data structure (IP addresses).
	\param [in] macHi		Hi 16 bits of MAC address for device.
	\param [in] macLo		Low 32 bits of MAC address for device.
	\param [in] ip			IP address to force device to.
	\param [in] subnetmask	Number of network interfaces found.
	\return GEV status code
	\note None
*/
GEV_STATUS GevForceCameraIPAddress (UINT32 macHi, UINT32 macLo, UINT32 IPAddress, UINT32 subnetmask)
{
	GEV_STATUS status = GEVLIB_ERROR_API_NOT_INITIALIZED;
	UINT32 numNetIF = 0;
	if (m_APIInit)
	{
		status = GEVLIB_OK;
		
		// See if there is a change in the number of network interfaces.
		numNetIF = _GetMaxNetworkInterfaces();
		if (numNetIF != m_numNetIF)
		{
			// Re-scan the network interfaces.
			if (m_numNetIF < numNetIF)
			{
				PGEV_NETWORK_INTERFACE pNetIF = (PGEV_NETWORK_INTERFACE) malloc(numNetIF * sizeof(GEV_NETWORK_INTERFACE));
				if (m_pNetIF != NULL)
				{
					// Re-size the internal 
					_EnterCriticalSection(&m_csUpdateLock);
					free(m_pNetIF);
					m_pNetIF = pNetIF;
					m_numNetIF = numNetIF;
					_LeaveCriticalSection(&m_csUpdateLock);
				}
				else
				{
					m_pNetIF = pNetIF;
					m_numNetIF = numNetIF;
				}

				// Enumerate the network interfaces.
				status = GevEnumerateNetworkInterfaces(m_pNetIF, m_numNetIF, &numNetIF);		
			}
		}

		if (status == GEVLIB_OK)
		{
			UINT32 i = 0;

			for (i = 0; i < m_numNetIF; i++)
			{
				status = GevForceIP( &m_pNetIF[i], macHi, macLo, IPAddress, subnetmask);
				if (status == GEVLIB_OK)
				{
					break;
				}
			}
		}
	}
	return status;
}

//! Enumerate network interfaces
/*!
	This function is used to fill a list of network interfaces visible from the application.
	\param [in]  pIPAddr			Network interface data structure (IP addresses).
	\param [in]  maxInterfaces	Maximum number of interfaces for which there is storage in pIPAddr.
	\param [out] pNumIntefaces	Number of network interfaces found.
	\return GEV status code
	\note None
*/
GEV_STATUS GevEnumerateNetworkInterfaces(GEV_NETWORK_INTERFACE *pIPAddr, UINT32 maxInterfaces, PUINT32 pNumInterfaces )
{
	GEV_STATUS status = GEV_STATUS_ERROR;
	UINT32 i = 0;
	UINT32 numFound = 0;
	UINT16 macHigh;
	UINT32 macLow;

	if ((maxInterfaces == 0) || (pIPAddr == NULL) || (pNumInterfaces == NULL))
	{
		return status;
	}

	for (i = 0; i < maxInterfaces; i++)
	{
		pIPAddr[i].fIPv6 = FALSE; // Only do IPv4 for now.
		pIPAddr[i].ipAddr = 0;
		pIPAddr[i].ipAddrLow = 0;
		pIPAddr[i].ipAddrHigh = 0;
		pIPAddr[i].ifIndex = 0;
		if ( _GetMacAddress (i, &macHigh, &macLow, &pIPAddr[i].ipAddr, &pIPAddr[i].ifIndex) )
		{
			numFound++;
		}
	}
	*pNumInterfaces = numFound;
	status = GEV_STATUS_SUCCESS;
	return status;
}

//! Manually set the camera list containig information on the GigE Vision device of interest to the API.
/*!
	This function is used to fill the internal list of GEV_CAMERA_INFO data structures
   from the list entered. This allows an application to manually set up only the cameras
   it is interested in and skip the "automatic" detection step.

	Note : If you set the camera list manually (with at least one camera), all calls to 
		the GevGetCameraList function will return this manually set list. 
		No automatic detection will be performed.
		Automatic detection can be re-enabled by setting a zero length (NULL) camera list
		with this function.

	\param [in] camera  		Pointer to a list of GEV_CAMERA_INFO entries.
	\param [in] numCameras  The number of camera / device entries in the list.
	\return GEV_STATUS code.
	\note None
*/

GEV_STATUS GevSetCameraList( GEV_CAMERA_INFO *cameras, int numCameras)
{
	GEV_STATUS status = GEVLIB_OK;
	if (!m_APIInit)
	{
		// Initialize the API.
		GevApiInitialize();
	}

	if ((cameras != NULL) && (numCameras > 0))
	{

		// Go through the open camera handle list and close any open cameras.
		if (m_openCameraList != NULL)
		{
			int i = 0;
			for (i = 0; i < m_numOpenCameras; i++)
			{
				GevCloseCamera(&m_openCameraList[i]);
			}
		}
		m_bEnableAutoDiscovery = FALSE;

		if (numCameras > (int)m_maxDevices)
		{
			// Free up existing camera list memory
			// Allocate new camera list memory
			PGEV_DEVICE_INTERFACE pDevice = NULL;
			GEV_CAMERA_HANDLE *pHandles = NULL;

			_EnterCriticalSection(&m_csUpdateLock);
			pDevice = (PGEV_DEVICE_INTERFACE) malloc((numCameras) * sizeof(GEV_DEVICE_INTERFACE));
			m_maxDevices = numCameras;
			free(m_pDevice);
			m_pDevice = pDevice;
				
			// Keep the possible number of open handles at twice the maximum number of cameras in the list.
			pHandles = (GEV_CAMERA_HANDLE *) malloc(2*m_maxDevices * sizeof(GEV_CAMERA_HANDLE));
			free(m_openCameraList);
			m_openCameraList = pHandles;
		}

		_LeaveCriticalSection(&m_csUpdateLock);		

		// Copy the data.
		m_numDevices = numCameras;
		memcpy(m_pDevice, cameras, (numCameras * sizeof(GEV_CAMERA_INFO)));
	}
	else
	{
		// Re-enable the automatic detection mechanism.
		m_bEnableAutoDiscovery = TRUE;
	}
	return status;
}

//=============================================================================
// Library / camera configuration functions
//
//! Get Library Configration options
/*!
	This function is used to get the internal library options that control API behaviour.
	(Things like debug logging level, number of retries, comm timeout, and ip port settings.)
	\param [in] option  Pointer to library configuration options data structure.
	\return GEV status code
	\note None
*/
GEV_STATUS GevGetLibraryConfigOptions( GEVLIB_CONFIG_OPTIONS *options)
{
	if (options != NULL)
	{
		options->version = m_library_version;		// Read-only.
		options->logLevel = m_library_logLevel;
		options->numRetries = m_numRetries;
		options->command_timeout_ms = m_command_timeout_ms;
		options->discovery_timeout_ms = m_discoveryTimeout;
		options->enumeration_port = m_enumeration_port;
		options->gvcp_port_range_start = m_gvcp_port_range_start;
		options->gvcp_port_range_end = m_gvcp_port_range_end;
	}
	return GEVLIB_OK;
}

//! Set Library Configration options
/*!
	This function is used to set the internal library options that control API behaviour.
	(Things like debug logging level, number of retries, comm timeout, and ip port settings.)
	\param [in] option  Pointer to library configuration options data structure.
	\return GEV status code
	\note None
*/
GEV_STATUS GevSetLibraryConfigOptions( GEVLIB_CONFIG_OPTIONS *options)
{
	GEV_STATUS status = GEVLIB_ERROR_NULL_PTR;
	if (options != NULL)
	{
		if (options->gvcp_port_range_start < options->gvcp_port_range_end)
		{
			m_library_logLevel = options->logLevel;
			m_numRetries = options->numRetries;
			m_command_timeout_ms = options->command_timeout_ms;
			m_discoveryTimeout = options->discovery_timeout_ms;
			m_enumeration_port = options->enumeration_port;
			m_gvcp_port_range_start = options->gvcp_port_range_start;
			m_gvcp_port_range_end = options->gvcp_port_range_end;
			status = GEVLIB_OK;

			// Set the underlying library options as well.
			GevSetInterfaceOptions(options);
		}
		else
		{
			status = GEVLIB_ERROR_ARG_INVALID;
		}
	}
	return status;
}

//! Get Camera operation configration options
/*!
	This function is used to get configuration options to control the behaivour of a specific camera.
	(Things like debug the number of retries, various operation timeouts, and the streaming packet delay.)
	\param [in] handle  Handle to the camera.
	\param [in] option  Pointer to camera configuration options data structure.
	\return GEV status code
	\note None
*/
GEV_STATUS GevGetCameraInterfaceOptions( GEV_CAMERA_HANDLE handle, GEV_CAMERA_OPTIONS *options)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	if (handle != NULL)
	{
		GEV_CAMERA_OPTIONS *pCameraOptions = Gev_GetCameraOptionsFromHandle( handle );
	
		if ( pCameraOptions != NULL )
		{
			if (options != NULL)
			{	
				memcpy(options, pCameraOptions, sizeof(GEV_CAMERA_OPTIONS));
				status = GEVLIB_OK;
			}
			else
			{
				status = GEVLIB_ERROR_NULL_PTR;
			}
		}
	}
	return status;
}


//! Set Camera operation configration options
/*!
	This function is used to set configuration options to control the behaivour of a specific camera.
	(Things like debug the number of retries, various operation timeouts, and the streaming packet delay.)
	\param [in] handle  Handle to the camera.
	\param [in] option  Pointer to camera configuration options data structure.
	\return GEV status code
	\note None
*/
GEV_STATUS GevSetCameraInterfaceOptions( GEV_CAMERA_HANDLE handle, GEV_CAMERA_OPTIONS *options)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	if (handle != NULL)
	{
		GEV_CAMERA_OPTIONS *pCameraOptions = Gev_GetCameraOptionsFromHandle( handle );
	
		if ( pCameraOptions != NULL )
		{
			if (options != NULL)
			{
				UINT32 streamState = 0;
				// Make sure the streaming thread is not active.
				Gev_GetStreamState(handle, &streamState);
				if (streamState & THREAD_ACTIVE)
				{
					status = GEVLIB_ERROR_ACCESS_DENIED;
					return status;
				}
				
				// Check that the streaming thread CPU affinity is in range.
				if (options->streamThreadAffinity != -1)
				{
					int numcpus = sysconf(_SC_NPROCESSORS_CONF);
					if (options->streamThreadAffinity >= numcpus)
					{
						// The cpu affinity (index) is out of range. Make it the default case and continue.
						options->streamThreadAffinity = -1; 
					}
				}
				pCameraOptions->numRetries = options->numRetries;
				pCameraOptions->command_timeout_ms = options->command_timeout_ms;
				pCameraOptions->heartbeat_timeout_ms = options->heartbeat_timeout_ms;
				pCameraOptions->streamNumFramesBuffered = options->streamNumFramesBuffered;
				pCameraOptions->streamMaxPacketResends = 	options->streamMaxPacketResends;
				pCameraOptions->streamFrame_timeout_ms = 	options->streamFrame_timeout_ms;
				pCameraOptions->streamThreadAffinity = options->streamThreadAffinity;
				pCameraOptions->serverThreadAffinity = options->serverThreadAffinity;

				pCameraOptions->msgChannel_timeout_ms = options->msgChannel_timeout_ms;

				// Set the memory limit - (numFramesBuffered * frame size) will be clipped at this amount.
				// (It defaults to 64MB. Maybe keep it under 256MB or so - depends on system RAM).
				// (Shared by all cameras so it is more of an API level global!!!!)
				pCameraOptions->streamMemoryLimitMax = options->streamMemoryLimitMax;
				
				if (pCameraOptions->streamPktDelay != options->streamPktDelay)
				{
					// Update the streaming packet delay directly.
					status = Gev_SetStreamPacketDelay( handle, options->streamPktDelay);
				}
				if (pCameraOptions->streamPktSize != options->streamPktSize)
				{
					// Try to update the streaming packet size (MTU) setting.
					status = Gev_SetStreamPacketSize( handle, options->streamPktSize);
				}
				
				// Ping the internal threads to pick up any new settings.
				status = Gev_UpdateCameraOptions( handle );
			}
			else
			{
				status = GEVLIB_ERROR_NULL_PTR;
			}
		}
	}
	return status;
}

//! Get Camera Device Information structure.
/*!
	This function is used to get the GEV_DEVICED_INFO structure for the camera.
	\param [in] handle  Handle to the camera.
	\return Pointer to (internal) camera information structure.
	\note None
*/
GEV_CAMERA_INFO *GevGetCameraInfo( GEV_CAMERA_HANDLE handle)
{
	return Gev_GetCameraInfoFromHandle(handle);
}

//=============================================================================
// Camera Access functions
//
// 

//! Open a camera and return a handle.
//
/*!
	\param [in] device  		Camera info structure for camera/device to be opened.
	\param [in] mode  		Access mode for camera (GevMonitorMode, GevControlMode, or GevExclusiveMode) .
	\param [in] handle  		Handle for the opened camera (allocated internally).
	\return Status.
	\note None
*/
GEV_STATUS GevOpenCamera( GEV_CAMERA_INFO *device, GevAccessMode mode, GEV_CAMERA_HANDLE *handle)
{
	GEV_STATUS status = GEVLIB_ERROR_API_NOT_INITIALIZED;
	UINT16 ports[GEV_NUM_PORTS_PER_HANDLE] = {0};

	// Check if API has been initialized.
	if (m_APIInit)
	{
		if (handle != NULL)
		{
			// Allocate a GEV_CAMERA_HANDLE.
			*handle = Gev_AllocateCameraHandle();

			if (*handle != NULL)
			{
				// Create a connection (based on the access mode)
				switch(mode)
				{
					case GevMonitorMode:
							status = Gev_CreateReadOnlyConnection( *handle, device);
						break;

					case GevControlMode:
							status = _GetAvailablePorts(ports);
							if (status == GEVLIB_OK)
							{
								status = Gev_CreateControlConnection( *handle, device, ports);
							}
						break;

					case GevExclusiveMode:
					default:
							status = _GetAvailablePorts(ports);
							if (status == GEVLIB_OK)
							{
								status = Gev_CreateConnection( *handle, device, ports);
							}
						break;
				}

				if (status == GEVLIB_OK)
				{
					// Determine the type of camera and obtain is register structure.
					status = GevInitCameraRegisters( *handle);

					// Get ourselves an IPort object for GenApi accesses.
					CGevPort *camPort = new CGevPort( *handle);
					
					// Wrap it into itself so we can get it back.
					GevSetCameraPortObject( *handle, (void *)camPort);
					
					// Add the handle to the internal camera handle list.
					_EnterCriticalSection(&m_csUpdateLock);
					m_openCameraList[m_numOpenCameras++] = *handle;
					_LeaveCriticalSection(&m_csUpdateLock);
				}
				else
				{
					*handle = NULL;
				}

			}
		}
		else
		{
			status = GEVLIB_ERROR_INVALID_HANDLE;		
		}
	}
	return status;
}

//! Open a camera (by IP address) and return a handle.
//
/*!
	\param [in] ip_address	IP Address for camera to be opened (it must be in the detected Camera list).
	\param [in] mode  		Access mode for camera (GevMonitorMode, GevControlMode, or GevExclusiveMode) .
	\param [in] handle  		Handle for the opened camera (allocated internally).
	\return Status.
	\note None
*/
GEV_STATUS GevOpenCameraByAddress( unsigned long ip_address, GevAccessMode mode, GEV_CAMERA_HANDLE *handle)
{
	GEV_STATUS status = GEVLIB_ERROR_API_NOT_INITIALIZED;
	UINT32 i;

	if (m_APIInit)
	{
		// Search the detected camera list for one with this ip address.
		if (m_numDevices == 0)
		{
			// Fill up the camera list in case more have been added.
			status = _DetectAttachedDevices( NULL );
		}
		for (i = 0; i < m_numDevices; i++)
		{
			if (m_pDevice[i].ipAddr == ip_address)
			{
				// Found one - open it.
				status = GevOpenCamera( &m_pDevice[i], mode, handle);
				return status;
			}
		}
		status = GEVLIB_ERROR_DEVICE_NOT_FOUND;
	}
	return status;
}

//! Open a camera (by user name) and return a handle.
//
/*!
	\param [in] name		"User" name for camera to be opened (it must be in the detected Camera list).
	\param [in] mode  	Access mode for camera (GevMonitorMode, GevControlMode, or GevExclusiveMode) .
	\param [in] handle  	Handle for the opened camera (allocated internally).
	\return Status.
	\note None
*/
GEV_STATUS GevOpenCameraByName( char *name, GevAccessMode mode, GEV_CAMERA_HANDLE *handle)
{
	GEV_STATUS status = GEVLIB_ERROR_API_NOT_INITIALIZED;
	UINT32 i;
	int len = 0;

	if (name != NULL)
	{
		len = strlen(name);
	}
	if (m_APIInit)
	{
		// Search the detected camera list for one with this name.
		if (m_numDevices == 0)
		{
			// Fill up the camera list in case more have been added.
			status = _DetectAttachedDevices( NULL );
		}
		for (i = 0; i < m_numDevices; i++)
		{
			int len2 = strlen(m_pDevice[i].username);

			len2 = (len <= len2) ? len : len2;
			if (!strncmp(m_pDevice[i].username, name, len2))
			{
				// Found one - open it.
				status = GevOpenCamera( &m_pDevice[i], mode, handle);
				return status;
			}	
		}
		status = GEVLIB_ERROR_DEVICE_NOT_FOUND;
	}
	return status;
}

//! Open a camera (by serial number string) and return a handle.
//
/*!
	\param [in] sn			Serial number string for camera to be opened (it must be in the detected Camera list).
	\param [in] mode  	Access mode for camera (GevMonitorMode, GevControlMode, or GevExclusiveMode) .
	\param [in] handle  	Handle for the opened camera (allocated internally).
	\return Status.
	\note None
*/
GEV_STATUS GevOpenCameraBySN( char *sn, GevAccessMode mode, GEV_CAMERA_HANDLE *handle)
{
	GEV_STATUS status = GEVLIB_ERROR_API_NOT_INITIALIZED;
	UINT32 i;
	int len = 0;

	if (sn != NULL)
	{
		len = strlen(sn);
	}
	if (m_APIInit)
	{
		// Search the detected camera list for one with this name.
		if (m_numDevices == 0)
		{
			// Fill up the camera list in case more have been added.
			status = _DetectAttachedDevices( NULL );
		}
		for (i = 0; i < m_numDevices; i++)
		{
			int len2 = strlen(m_pDevice[i].username);

			len2 = (len <= len2) ? len : len2;
			if (!strncmp(m_pDevice[i].serial, sn, len2))
			{
				// Found one - open it.
				status = GevOpenCamera( &m_pDevice[i], mode, handle);
				return status;
			}
		}
		status = GEVLIB_ERROR_DEVICE_NOT_FOUND;
	}
	return status;
}
	
//! Close an open camera handle.
//
/*!
	\param [in] handle  		Handle to the camera to be closed.
	\return Status.
	\note None
*/
GEV_STATUS GevCloseCamera(GEV_CAMERA_HANDLE *pHandle)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;
	UINT16 ports[GEV_NUM_PORTS_PER_HANDLE] = {0};
	int i = 0;
	int index = -1;

	if (*pHandle != NULL)
	{
		// Find the handle in the open camera list and remove it.
		_EnterCriticalSection(&m_csUpdateLock);
		for (i = 0; i < m_numOpenCameras; i++)
		{
			if (m_openCameraList[i] == *pHandle)
			{
				index = i;
				m_openCameraList[i] = NULL;
				
				// Extract the camera information from the handle.
				status = Gev_GetPortInfoFromHandle( *pHandle, ports);

				// Remove the featureNodeMap (if present)
				GenApi::CNodeMapRef* camFeatures = static_cast<GenApi::CNodeMapRef*>(GevGetFeatureNodeMapObject(*pHandle));
				if (camFeatures)
				{
					camFeatures->_Destroy();
					// !! Did not allocate this so do not delete it !!  delete camFeatures;
				}
				
				// Free the IPort (if present)
				CGevPort* camPort = static_cast<CGevPort*>(GevGetCameraPortObject(*pHandle));
				if (camPort)
				{
					delete camPort;
				}
				
				// Close the device.
				Gev_CloseCameraHandle(*pHandle); 
				Gev_FreeCameraHandle(*pHandle);

				// Release ports to API.
				status = _ReleasePorts( ports);						
				break;
			}
		}
		if (index >= 0)
		{
			m_numOpenCameras -= 1;
			for (i = index; i < m_numOpenCameras; i++)
			{
				m_openCameraList[i] = m_openCameraList[i+1];
			}
		}
		_LeaveCriticalSection(&m_csUpdateLock);

		// Clean up the handle and free it.
		*pHandle = NULL;
	}
	return status;
}


//! Connect GenICam features to the device handle.
//
// This function connects a GenApi::CNodeMapRef object with the device port associated 
// with the camera handle. The CNodeMapRef object is passed in as a void pointer. 
// This internal function is able to indicate if the CNodeMapRef object is to be
// treated an opaque external pointer or as one that is managed by the API 
// (i.e was allocated by the API and is to be deleted by the API).
//
/*!
	\param [in] handle  			Handle to the camera to be closed.
	\param [in] featureNodeMap Pointer to the featureNodeMap class (as a void pointer).
	\param [in] featureNodeMap Pointer to the featureNodeMap class (as a void pointer).
	\return Status.
	\note None
*/
static GEV_STATUS _Internal_GevConnectFeatures(  GEV_CAMERA_HANDLE handle,  void *featureNodeMap, int apiManagedNodeMap)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;
	
	if (handle != NULL)
	{
		status = GEVLIB_ERROR_NULL_PTR;
		if ( featureNodeMap != NULL )
		{
			status = GEVLIB_OK;
			
			// Check if there is already a Feature Node Map object in use in the handle 
			// and if it is the same one (by address) as the one being connected.
			if ( (uint64_t)featureNodeMap != (uint64_t)GevGetFeatureNodeMapObject(handle) )
			{
				// Get the Feature Node Map object from the handle.
				GenApi::CNodeMapRef *camera = static_cast<GenApi::CNodeMapRef*>(GevGetFeatureNodeMapObject(handle));
				if (camera)
				{
					// Destroy this Feature Node Map so the new one can override it.
					try {
						camera->_Destroy();
						if ( GevIsNodeMapManaged(handle) )
						{
							delete camera;
						}
					}
					CATCH_GENAPI_ERROR(status);
				
					status = GevSetFeatureNodeMapObject(handle, NULL, apiManagedNodeMap);
				}
					
				// No Feature Node Map connected - connect one.
				// Get the camera port object back from the handle.
				CGevPort *camPort = static_cast<CGevPort*>(GevGetCameraPortObject(handle));			
				if (camPort)
				{
					// Associate the node map with the camera port object.
					camera = static_cast<GenApi::CNodeMapRef*>(featureNodeMap);
					if (camera)
					{
						status = GEVLIB_OK;
						// Attempt the connection.
						try {
							camera->_Connect( camPort, "Device" );
						}
						CATCH_GENAPI_ERROR(status);
						
						if (status == GEVLIB_OK)
						{
							status = GevSetFeatureNodeMapObject(handle, featureNodeMap, apiManagedNodeMap);
						}
					}
				}
			}
		}
	}
	return status;
}
//! Connect GenICam features to the device handle.
//
// This function connects a GenApi::CNodeMapRef object with the device port associated 
// with the camera handle. The CNodeMapRef object is passed in as a void pointer. 
//
/*!
	\param [in] handle  			Handle to the camera to be closed.
	\param [in] featureNodeMap Pointer to the featureNodeMap class (as a void pointer).
	\return Status.
	\note None
*/
GEV_STATUS GevConnectFeatures(  GEV_CAMERA_HANDLE handle,  void *featureNodeMap)
{
	return _Internal_GevConnectFeatures(handle, featureNodeMap, 0);
}

//! Retrieve GenICam features from the device handle.
//
// This function returns, as a void pointer, a pointer to a GenApi::CNodeMapRef object 
// that was previously associated with the camera handle by a call to GevConnectFeatures. 
// This allows the pointer to be retrieved from the API for use in cases where only the 
// camera handle is available 
//
/*!
	\param [in] handle  			Handle to the camera to be closed.
	\param [in] featureNodeMap Pointer to the featureNodeMap class (as a void pointer).
	\return Status.
	\note None
*/
void * GevGetFeatureNodeMap(  GEV_CAMERA_HANDLE handle)
{
	return GevGetFeatureNodeMapObject(handle);
}

//! Initialize GenICam XML features from the device.
//
// This function retrieves the GenICam XML file from the camera and uses it to initialize 
// internal access to the GenICam GenApi via an internal GenApi::CNodeMapRef object 
// connected to the camera. Optionally, the XML file read from the camera is stored to 
// disk based on the value of the input flag. 
// If this flag is false, the XML file is not stored to disk.
// If this flag is true, the XML file is stored to disk. The location (path) to the stored 
//	XML files will be relative to the GIGEV_XML_DOWNLOAD environment variable. The path will 
// be $GIGEV_XML_DOWNLOAD/xml/download. 
// If that location is not writable by the application, the XML file will be stored in the 
// “current” directory that the executable is running in.
//
/*!
	\param [in] handle  			Handle to the camera.
	\param [in] updateXMLFile  T/F flag to control updating the XML file on disk.
	\return Status.
	\note None
*/
GEV_STATUS GevInitGenICamXMLFeatures( GEV_CAMERA_HANDLE handle, BOOL updateXMLFile)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;
	
	if (handle != NULL)
	{
		// Allocate a NodeMap.
		GenApi::CNodeMapRef *Camera = new GenApi::CNodeMapRef();	// The GenICam XML-based feature node map.			
		if (Camera)
		{
			char xmlFileName[MAX_PATH] = {0};
			
			if ( updateXMLFile )
			{
				// Get the XML file onto disk and use it to create the CNodeMap object.
					
				status = Gev_RetrieveXMLFile( handle, xmlFileName, sizeof(xmlFileName), FALSE );
				if ( status == GEVLIB_OK)
				{
					try {
						Camera->_LoadXMLFromFile( xmlFileName );
					}
					CATCH_GENAPI_ERROR(status);
				}
			}
			else
			{
				// Create CNodeMap object from Camera XML File data 
				// Get the XML data itself (into an array) and use it to make the CNodeMap object.
				int xmlDataSize = 0;
				int zipData = 0;
            char *pXmlData;
               
            // Query the XML size.
				Gev_RetrieveXMLData( handle, 0, NULL, &xmlDataSize, NULL);
				pXmlData = (char *)malloc( ((xmlDataSize + 3) & (~3)) + 1);
					
				// Get the XML data.
				Gev_RetrieveXMLData( handle, (xmlDataSize + 3) & (~3), pXmlData, &xmlDataSize, &zipData);
				pXmlData[xmlDataSize ] = 0;
					
				// Use it to make the CNodeMap object
				try {
					if ( zipData )
					{
						GenICam::gcstring xmlStr( pXmlData );
						Camera->_LoadXMLFromString(xmlStr);
					}
					else
					{
						Camera->_LoadXMLFromZIPData((void *)pXmlData, xmlDataSize);
					}
				}
				CATCH_GENAPI_ERROR(status);
				free(pXmlData);
			}
			
			if (status == GEVLIB_OK)
			{
				// Connect the features in the node map to the camera handle.
				status = _Internal_GevConnectFeatures( handle, static_cast<void *>(Camera), 1);
				if ( status == 0 )
				{
					// Store the xml file name in the handle. 
					status = GevSetXMLFileName( handle,  xmlFileName);
				}		
			}
		}
	}
	return status;
}

//! Initialize GenICam XML features from a file.
//
// This function uses the GenICam XML file, identified by name, to initialize internal 
// access to the GenICam GenApi via an internal GenApi::CNodeMapRef object connected 
// to the camera.
// (The file must be an uncompressed XML file).
//
/*!
	\param [in] handle  			Handle to the camera.
	\param [in] xmlFileName    The full path name of the XML file that is to be used.
	\return Status.
	\note None
*/
GEV_STATUS GevInitGenICamXMLFeatures_FromFile( GEV_CAMERA_HANDLE handle, char *xmlFileName)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;
	
	if (handle != NULL)
	{
		// Allocate a NodeMap.
		GenApi::CNodeMapRef *Camera = new GenApi::CNodeMapRef();	// The GenICam XML-based feature node map.			
		if (Camera)
		{
			try {
				Camera->_LoadXMLFromFile( xmlFileName );
			}
			CATCH_GENAPI_ERROR(status);
			
			if (status == GEVLIB_OK)
			{
				// Connect the features in the node map to the camera handle.
				status = _Internal_GevConnectFeatures( handle, static_cast<void *>(Camera), 1);
				if ( status == 0 )
				{
					// Store the xml file name in the handle. 
					status = GevSetXMLFileName( handle,  xmlFileName);
				}	
			}						
		}
	}
	return status;
}

//! Initialize GenICam XML features from a data array.
//
// This function uses the GenICam XML data string, contained in the xmlDataBuffer, 
// to initialize internal access to the GenICam GenApi via an internal 
// GenApi::CNodeMapRef object connected to the camera . 
// The source of the XML data string is not specified - it can be read from a file 
// or it can be compiled in to the program as string data.
//
/*!
	\param [in] handle  			Handle to the camera.
	\param [in] size           The number of bytes in the XML data array (including the terminating 0).
	\param [in] pXmlDataBuffer Pointer to a data array containing the XML data.
	\return Status.
	\note None
*/
GEV_STATUS GevInitGenICamXMLFeatures_FromData( GEV_CAMERA_HANDLE handle, int size, void *pXmlData)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;
	
	if (handle != NULL)
	{
		// Allocate a NodeMap.
		GenApi::CNodeMapRef *Camera = new GenApi::CNodeMapRef();	// The GenICam XML-based feature node map.			
		if (Camera)
		{		
			if (pXmlData != NULL)
			{
			
				try {
					char *ptr = (char *)pXmlData;
					ptr[size] = 0;  // Force a zero at the end.
					// Create CNodeMap object from Camera XML data 
					// Get the XML data itself (into an array) and use it to make the CNodeMap object.
					GenICam::gcstring xmlStr( ptr );
					Camera->_LoadXMLFromString(xmlStr);			
				}
				CATCH_GENAPI_ERROR(status);

			if (status == GEVLIB_OK)
			{
				// Connect the features in the node map to the camera handle.
				status = _Internal_GevConnectFeatures( handle, static_cast<void *>(Camera), 1);
				if ( status == 0 )
				{
					// Store the xml file name in the handle (as a NULL)
					status = GevSetXMLFileName( handle,  NULL);
				}	
			}						

			}
			
		}
	}
	return status;
}

//! Return the name of the GenICam XML file used to create the feature tree for the camera.
//
// This function returns the full path name of the XML file that was used to create the 
// GenApi::CNodeMapRef object containing the feature tree for the camera.
//
// Note: If the XML data used was from a string/data buffer, or was from the camera 
// but not stored on disk, then the returned file name will be blank (0 length).
//
/*!
	\param [in]  handle  			Handle to the camera.
	\param [out] xmlFileName    The full path name of the XML file for the feature tree.
	\return Status.
	\note None
*/
GEV_STATUS GevGetGenICamXML_FileName( GEV_CAMERA_HANDLE handle, int size, char *xmlFileName)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;
	
	if (handle != NULL)
	{
		status = GEVLIB_ERROR_NULL_PTR;
		if ( xmlFileName != NULL )
		{
			status = GevGetXMLFileName( handle, size, xmlFileName);
		}
	}
	return status;
}



//===========================================================================
// 
// Static helper functions access to features.
//

static GEV_STATUS _Gev_GenICam_VerifyDesiredAccess( GenApi::CNodePtr node, bool readable, bool writable)
{
	GEV_STATUS status = GEVLIB_OK;
	if (IsImplemented(node))
	{
		if (IsAvailable(node))
		{
			if (readable)
			{
				if (!IsReadable(node))
				{
					status = GEVLIB_ERROR_ACCESS_DENIED;
				}
			}
			if (status == 0)
			{
				if (writable)
				{
					if (!IsWritable(node))
					{
						status = GEVLIB_ERROR_ACCESS_DENIED;
					}
				}
			}
		}
		else
		{
			// Currently not available.
			status = GEVLIB_ERROR_NOT_AVAILABLE;
		}
	}
	else
	{
		status = GEVLIB_ERROR_NOT_IMPLEMENTED;
	}
	return status;
}

//====================================================
// API calls for feature access
//
//! Set the value of a GenICam feature using its string representation.
//
// This function writes the value of the feature indicated by the
// feature_name parameter, using its representation as a string. 
//
/*!
	\param [in]  handle  		Handle to the camera to be closed.
	\param [in]  feature_name 	Feature name (string)
	\param [in]  value_string      Pointer to the storage to hold the value of the feature as a string.
	\return Status.
	\note None
*/

GEV_STATUS GevSetFeatureValueAsString( GEV_CAMERA_HANDLE handle, const char *feature_name, const char *value_string)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	if (handle != NULL)
	{
		status = GEVLIB_ERROR_ARG_INVALID;
		if ( (feature_name != NULL) && (value_string != NULL) )
		{
			GenApi::CNodeMapRef *Camera = static_cast<GenApi::CNodeMapRef*>(GevGetFeatureNodeMap(handle));
			if ( Camera )
			{
				// Node map is not NULL - give it a shot.
				// Get the node (feature) by name and filter its suitability
				GenApi::CNodePtr node = Camera->_GetNode(feature_name);
				if ( node )
				{					
					// Node map is not NULL - give it a shot - filter its suitability
					try {
						status = _Gev_GenICam_VerifyDesiredAccess( node, false, true);
						if (status == 0)
						{
							// Handle the node by type.
							switch(node->GetPrincipalInterfaceType())
							{
								// The ICommand pattern.
								case GenApi::intfICommand:
									{
										// Get us a CICommand type to use.
										CCommandPtr cmdNode(node);  
										
										// Execute the ICommand. (don't care about the string value).
										status = 0;
										cmdNode->Execute();
									}
									break;

								// The IValue pattern for integer, bool, float, string, and enumeration 
								// can all be handled the same way - as a CValNode....
								case GenApi::intfIInteger:
								case GenApi::intfIBoolean:
								case GenApi::intfIFloat:
								case GenApi::intfIString:
								case GenApi::intfIEnumeration:
									{
										// Get us an IValue type to use.
										status = 0;
										CValuePtr valNode(node);  
										GenICam::gcstring str = value_string;
										valNode->FromString(str); 
									}
									break;
								default:
									status = GEVLIB_ERROR_ARG_INVALID;
									break;
							}
						}
					}
					CATCH_GENAPI_ERROR(status);
				}
				else
				{
					// Not initialized (node map is NULL).
					status = GEVLIB_ERROR_NULL_PTR;
				}
			}
			else
			{
				// Not initialized (node map is NULL).
				status = GEVLIB_ERROR_NULL_PTR;
			}
		}
	}
	return status;
}

//! Retrieve the value of a GenICam feature using its string representation.
//
// This function returns, as a string, the value of the feature indicated by the
// feature_name parameter. The type of the feature is also returned as an integer.
// The string to be returned must fit inside the storage passed in that is indicated
// in the value_string_size parameter.
//
/*!
	\param [in]  handle  			 Handle to the camera to be closed.
	\param [in]  feature_name 		 Feature name (string)
	\param [out] feature_type 		 Pointer to hold the GenApi feature type for the feature as an integer.
	\param [in]  value_string_size The size of the storage available for the feature value as a string (value_string).
	\param [in]  value_string      Pointer to the storage to hold the value of the feature as a string.
	\return Status.
	\note The feature type corresponds to the GenApi::EInterfaceType definition and is provided as
	an integer here in order to be useful in C application programs. The values are: 
	 	GENAPI_UNUSED_TYPE	 = 1  intfIBase/intfIValue/intfICategory are not accessible from C 
	 	GENAPI_INTEGER_TYPE	 = 2   corresponds to GenApi::EInterfaceType intfIInteger   
	 	GENAPI_BOOLEAN_TYPE	 = 3   corresponds to GenApi::EInterfaceType intfIBoolean   
	 	GENAPI_COMMAND_TYPE	 = 4   corresponds to GenApi::EInterfaceType intfICommand   
	 	GENAPI_FLOAT_TYPE		 = 5   corresponds to GenApi::EInterfaceType intfIFloat     
	 	GENAPI_STRING_TYPE	 = 6   corresponds to GenApi::EInterfaceType intfIString    
	 	GENAPI_REGISTER_TYPE	 = 7   corresponds to GenApi::EInterfaceType intfRegister   
	 	GENAPI_ENUM_TYPE		 = 9   corresponds to GenApi::EInterfaceType intfIEnum       
	 	GENAPI_ENUMENTRY_TYPE = 10  corresponds to GenApi::EInterfaceType intfIEnumEntry 

*/

GEV_STATUS GevGetFeatureValueAsString( GEV_CAMERA_HANDLE handle, const char *feature_name, int *feature_type, int value_string_size, char *value_string)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	if (handle != NULL)
	{
		status = GEVLIB_ERROR_ARG_INVALID;
		if ( (feature_name != NULL) && (value_string != NULL) && (feature_type != NULL) && (value_string_size > 0) )
		{
			GenApi::CNodeMapRef *Camera = static_cast<GenApi::CNodeMapRef*>(GevGetFeatureNodeMap(handle));
			if ( Camera )
			{
				// Node map is not NULL - give it a shot.
				// Get the node (feature) by name and filter its suitability
				GenApi::CNodePtr node = Camera->_GetNode(feature_name);
				if ( node )
				{
					// Node map is not NULL - give it a shot - filter its suitability
					GenApi::EInterfaceType nodeType = node->GetPrincipalInterfaceType();
					try {
						status = _Gev_GenICam_VerifyDesiredAccess( node, true, false);
						if ((status == 0) || ( (status != 0) && (nodeType == GenApi::intfICommand)))
						{
							status = 0;
							
							// Handle the node by type.
							switch(nodeType)
							{
								// The ICommand pattern.
								case GenApi::intfICommand:
									{
										*feature_type = GENAPI_COMMAND_TYPE;
										// Get us a CICommand type to use.
										CCommandPtr cmdNode(node);  
										
										// Check if the command has completed
										if ( value_string_size > (int)sizeof("false"))
										{
											if ( cmdNode->IsDone())
											{
												strncpy(value_string, "true", value_string_size);
											}
											else
											{
												strncpy(value_string, "false", value_string_size);
											}
										}
										else
										{
											status = GEVLIB_ERROR_ARG_INVALID;
										}
									}
									break;

								// The IValue pattern for integer, bool, float, string, and enumeration 
								// can all be handled the same way - as a CValNode....
								case GenApi::intfIInteger:
								case GenApi::intfIBoolean:
								case GenApi::intfIFloat:
								case GenApi::intfIString:
								case GenApi::intfIEnumeration:
								case GenApi::intfIEnumEntry:
									{
										// Get us an IValue type to use.
										status = 0;
										CValuePtr valNode(node);  
										GenICam::gcstring str = valNode->ToString(str);  

										size_t len = strlen(str);
										if (value_string_size >= (int)len)
										{
											strncpy(value_string, str, len);
										}
										if ( nodeType == GenApi::intfIInteger ) *feature_type = GENAPI_INTEGER_TYPE;
										if ( nodeType == GenApi::intfIBoolean ) *feature_type = GENAPI_BOOLEAN_TYPE;
										if ( nodeType == GenApi::intfIFloat ) *feature_type = GENAPI_FLOAT_TYPE;
										if ( nodeType == GenApi::intfIString ) *feature_type = GENAPI_STRING_TYPE;
										if ( nodeType == GenApi::intfIEnumeration ) *feature_type = GENAPI_ENUM_TYPE;
										if ( nodeType == GenApi::intfIEnumEntry ) *feature_type = GENAPI_INTEGER_TYPE;
								}
									break;
								default:
										*feature_type = GENAPI_UNUSED_TYPE;
									status = GEVLIB_ERROR_ARG_INVALID;
									break;
							}
						}
					}
					CATCH_GENAPI_ERROR(status);
				}
				else
				{
					// Not initialized (node map is NULL).
					status = GEVLIB_ERROR_NULL_PTR;
				}
			}
			else
			{
				// Not initialized (node map is NULL).
				status = GEVLIB_ERROR_NULL_PTR;
			}
		}
	}
	return status;
}

//! Set the value of a GenICam feature using its binary representation.
//
// This function writes the value of the feature indicated by the
// feature_name parameter, using its binary representation. 
//
/*!
	\param [in]  handle  		Handle to the camera to be closed.
	\param [in]  feature_name 	Feature name (string)
	\param [in]  value_size 	The size of the storage available for the feature value.
	\param [in]  value  			Pointer to the storage holding the value of the feature.
	\return Status.
	\note None
*/

GEV_STATUS GevSetFeatureValue( GEV_CAMERA_HANDLE handle, const char *feature_name, int value_size, void *value)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	if (handle != NULL)
	{
		status = GEVLIB_ERROR_ARG_INVALID;
		if ( (feature_name != NULL) && (value != NULL) && (value_size != 0) )
		{
			GenApi::CNodeMapRef *Camera = static_cast<GenApi::CNodeMapRef*>(GevGetFeatureNodeMap(handle));
			if ( Camera )
			{
				// Node map is not NULL - give it a shot.
				// Get the node (feature) by name and filter its suitability
				GenApi::CNodePtr node = Camera->_GetNode(feature_name);
				if ( node )
				{
					// Node map is not NULL - give it a shot - filter its suitability
					try {
						status = _Gev_GenICam_VerifyDesiredAccess( node, false, true);
						if (status == 0)
						{
							// Handle the node by type.
							switch(node->GetPrincipalInterfaceType())
							{
								// The ICommand pattern.
								case GenApi::intfICommand:
									{
										// Get us a CICommand type to use.
										CCommandPtr cmdNode(node);  
										
										// Execute the ICommand.
										status = 0;
										cmdNode->Execute();
										//??????? Is there an automatic polling we can do here ???										
									}
									break;

								// The IValue pattern for integer.
								case GenApi::intfIInteger:
									{
										UINT64 intValue = 0;

										// Get us a CInteger type to use.
										CIntegerPtr intNode(node);  

										// Set up the value to write (always a UINT64).
										// Input is either a 32-bit or 64-bit integer.
										if (value_size < (int)sizeof(UINT32) )
										{
											status = GEVLIB_ERROR_ARG_INVALID;
										}
										else if (value_size == sizeof(UINT32))
										{
											intValue = (UINT64) (*(UINT32 *)value);
										}
										else
										{
											intValue = *(UINT64 *)value;
										}
										
										if (status == 0)
										{
											intNode->SetValue(intValue);  
											status = 0;
										}
									}
									break;

								// The IValue pattern for boolean.
								case GenApi::intfIBoolean:
									{
										bool boolValue = false;

										// Get us a CBoolean type to use.
										CBooleanPtr boolNode(node);  

										// Set up the value to write (always a bool).
										// Input can be anything from UINT8 to UINT64.
										if (value_size < (int)sizeof(unsigned char) )
										{
											status = GEVLIB_ERROR_ARG_INVALID;
										}
										if (value_size == sizeof(unsigned char) )
										{
											boolValue = (bool) *(unsigned char *)value;
										}
										else if (value_size == sizeof(short))
										{
											boolValue = (bool) *(unsigned short *)value;
										}
										else if (value_size == sizeof(int))
										{
											boolValue = (bool) *(unsigned int *)value;
										}
										else if (value_size == sizeof(UINT64))
										{
											boolValue = (bool) *(UINT64 *)value;
										}
										else
										{
											boolValue = *(bool *)value;
										}
										
										if (status == 0)
										{
											boolNode->SetValue(boolValue);  
											status = 0;
										}
									}
									break;
					  
								// The IValue pattern for float.
								case GenApi::intfIFloat:
									{
										double dValue = 0.0;

										// Get us a CFloat type to use.
										CFloatPtr floatNode(node);  

										// Set up the value to write (always a double).
										// Input is either a float or a double.
										if (value_size < (int)sizeof(float) )
										{
											status = GEVLIB_ERROR_ARG_INVALID;
										}
										else if (value_size == sizeof(float))
										{
											dValue = (double) (*(float *)value);
										}
										else
										{
											dValue = *(double *)value;
										}
										
										if (status == 0)
										{
											floatNode->SetValue(dValue);  
											status = 0;
										}


									}
									break;
									
								// The IValue pattern for string.
								case GenApi::intfIString:
									{
										// Get us a CString type to use.
										CStringPtr stringNode(node);  

										// Check the size of the value input 
										if ( value_size <= stringNode->GetMaxLength() )
										{
											GenICam::gcstring stringValue( (const char *)value);
											stringNode->SetValue(stringValue);  
										}
										else
										{
											status = GEVLIB_ERROR_NO_SPACE; 
										}
									}
									break;
								
								// The IValue pattern for enumeration 
								case GenApi::intfIEnumeration:
									{
										UINT64 enumValue = 0;

										// Get us a CEnumeration type to use.
										CEnumerationPtr enumNode(node);  

										// Just write it I guess (as a UINT64) and let it handle the checking.
										// Set up the value to write (always a UINT64).
										// Input is either a 32-bit or 64-bit integer.
										if (value_size < (int)sizeof(UINT32) )
										{
											status = GEVLIB_ERROR_ARG_INVALID;
										}
										else if (value_size == sizeof(UINT32))
										{
											enumValue = (UINT64) (*(UINT32 *)value);
										}
										else
										{
											enumValue = *(UINT64 *)value;
										}
										
										if (status == 0)
										{
											enumNode->SetIntValue(enumValue);  
											status = 0;
										}
									}
									break;
									
								// The IValue pattern for register.
								case GenApi::intfIRegister:
									{
										// Get us a CRegister type to use.
										CRegisterPtr regNode(node);  

										// Check the size of the value input a
										if ( value_size <= regNode->GetLength() )
										{
											regNode->Set( (uint8_t *)value, (int64_t)value_size);
										}
										else
										{
											status = GEVLIB_ERROR_NO_SPACE; 
										}
									}
									break;
								default:
									status = GEVLIB_ERROR_ARG_INVALID;
									break;
							}
						}
					}
					CATCH_GENAPI_ERROR(status);
				}

			}
			else
			{
				// Not initialized (node map is NULL or node not found).
				status = GEVLIB_ERROR_NULL_PTR;
			}
		}
	}
	return status;
}

//! Get the value of a GenICam feature as binary data.
//
// This function reads the value of the feature indicated by the
// feature_name parameter, using its binary representation. 
//
/*!
	\param [in]  handle  		Handle to the camera to be closed.
	\param [in]  feature_name 	Feature name (string)
	\param [out] feature_type 	Pointer to hold the GenApi feature type for the feature as an integer.
	\param [in]  value_size 	The size of the storage available for the feature value.
	\param [in]  value_string  Pointer to the storage to hold the returned value of the feature.
	\return Status.
	\note The feature type corresponds to the GenApi::EInterfaceType definition and is provided as
	an integer here in order to be useful in C application programs. The values are: 
	 	GENAPI_UNUSED_TYPE	 = 1   intfIBase/intfIValue/intfICategory are not accessible from C 
	 	GENAPI_INTEGER_TYPE	 = 2   corresponds to GenApi::EInterfaceType intfIInteger   
	 	GENAPI_BOOLEAN_TYPE	 = 3   corresponds to GenApi::EInterfaceType intfIBoolean   
	 	GENAPI_COMMAND_TYPE	 = 4   corresponds to GenApi::EInterfaceType intfICommand   
	 	GENAPI_FLOAT_TYPE		 = 5   corresponds to GenApi::EInterfaceType intfIFloat     
	 	GENAPI_STRING_TYPE	 = 6   corresponds to GenApi::EInterfaceType intfIString    
	 	GENAPI_REGISTER_TYPE	 = 7   corresponds to GenApi::EInterfaceType intfRegister   
	 	GENAPI_ENUM_TYPE		 = 9   corresponds to GenApi::EInterfaceType intfIEnum       
	 	GENAPI_ENUMENTRY_TYPE = 10  corresponds to GenApi::EInterfaceType intfIEnumEntry 
*/

GEV_STATUS GevGetFeatureValue( GEV_CAMERA_HANDLE handle, const char *feature_name, int *feature_type, int value_size, void *value)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	if (handle != NULL)
	{
		status = GEVLIB_ERROR_ARG_INVALID;
		if ( (feature_name != NULL) && (value != NULL) && (value_size != 0) && (feature_type != NULL))
		{
			GenApi::CNodeMapRef *Camera = static_cast<GenApi::CNodeMapRef*>(GevGetFeatureNodeMap(handle));
			if ( Camera )
			{
				// Node map is not NULL - give it a shot.
				// Get the node (feature) by name and filter its suitability
				GenApi::CNodePtr node = Camera->_GetNode(feature_name);
				if ( node )
				{
					// Node map is not NULL - give it a shot - filter its suitability
					GenApi::EInterfaceType nodeType = node->GetPrincipalInterfaceType();
					try {
						status = _Gev_GenICam_VerifyDesiredAccess( node, true, false);
						if ((status == 0) || ( (status != 0) && (nodeType == GenApi::intfICommand)))
						{
							status = 0;
							// Handle the node by type.
							switch(nodeType)
							{
								// The ICommand pattern.
								case GenApi::intfICommand:
									{
										*feature_type = GENAPI_COMMAND_TYPE;
										// Get us a CCommandPtr type to use.
										CCommandPtr cmdNode(node);  
										
										status = 0;
										bool done = cmdNode->IsDone();  
										if (value_size < (int)sizeof(unsigned char) )
										{
											status = GEVLIB_ERROR_ARG_INVALID;
										}
										if (value_size == sizeof(unsigned char) )
										{
											unsigned char *ptr = (unsigned char *)value;
											*ptr = (unsigned char)done;
										}
										else if (value_size == sizeof(short))
										{
											short *ptr = (short *)value;
											*ptr = (short)done;
										}
										else if (value_size == sizeof(int))
										{
											int *ptr = (int *)value;
											*ptr = (int)done;
										}
										else if (value_size == sizeof(UINT64))
										{
											UINT64 *ptr = (UINT64 *)value;
											*ptr = (UINT64)done;
										}
										else
										{
											// Arg is too big for data (where would we put it)?
											status = GEVLIB_ERROR_ARG_INVALID; 
										}						
									}
									break;

								// The IValue pattern for integer.
								case GenApi::intfIInteger:
									{
										*feature_type = GENAPI_INTEGER_TYPE;
										// Get us a CInteger type to use.
										CIntegerPtr intNode(node);  

										// Determine how this is to be reported back (as a UINT32 or as a UINT64)
										// Output is either a 32-bit or 64-bit integer.
										status = 0;
										UINT64 val = intNode->GetValue();  
										if (value_size < (int)sizeof(UINT32) )
										{
											status = GEVLIB_ERROR_ARG_INVALID;
										}
										else if (value_size == sizeof(UINT32))
										{
											UINT32 *ptr = (UINT32 *)value;
											*ptr = (UINT32) val;
										}
										else if (value_size == sizeof(UINT64))
										{
											UINT64 *ptr = (UINT64 *)value;
											*ptr = (UINT64)val;
										}
										else
										{
											// Arg is too big for data (where would we put it)?
											status = GEVLIB_ERROR_ARG_INVALID; 
										}						
									}
									break;

								// The IValue pattern for boolean.
								case GenApi::intfIBoolean:
									{
										*feature_type = GENAPI_BOOLEAN_TYPE;
										// Get us a CBoolean type to use.
										CBooleanPtr boolNode(node);  

										// Output can be anything from UINT8 to UINT64.
										status = 0;
										bool val = boolNode->GetValue(); 
										if (value_size < (int)sizeof(unsigned char) )
										{
											status = GEVLIB_ERROR_ARG_INVALID;
										}
										if (value_size == sizeof(unsigned char) )
										{
											unsigned char *ptr = (unsigned char *)value;
											*ptr = (unsigned char)val;
										}
										else if (value_size == sizeof(short))
										{
											short *ptr = (short *)value;
											*ptr = (short)val;
										}
										else if (value_size == sizeof(int))
										{
											int *ptr = (int *)value;
											*ptr = (int)val;
										}
										else if (value_size == sizeof(UINT64))
										{
											UINT64 *ptr = (UINT64 *)value;
											*ptr = (UINT64)val;
										}
										else
										{
											// Arg is too big for data (where would we put it)?
											status = GEVLIB_ERROR_ARG_INVALID; 
										}						
									}
									break;
					  
								// The IValue pattern for float.
								case GenApi::intfIFloat:
									{
										*feature_type = GENAPI_FLOAT_TYPE;
										// Get us a CFloat type to use.
										CFloatPtr floatNode(node);  

										// Output is either a float or a double.
										status = 0;
										double val = floatNode->GetValue(); 
										if (value_size < (int)sizeof(float) )
										{
											status = GEVLIB_ERROR_ARG_INVALID;
										}
										else if (value_size == sizeof(float))
										{
											float *ptr = (float *)value;
											*ptr = (float) val;
										}
										else if (value_size == sizeof(double))
										{
											double *ptr = (double *)value;
											*ptr = val;
										}
										else 
										{
											// Arg is too big for data (where would we put it)?
											status = GEVLIB_ERROR_ARG_INVALID;  
										}						
									}
									break;
									
								// The IValue pattern for string.
								case GenApi::intfIString:
									{
										*feature_type = GENAPI_STRING_TYPE;
										// Get us a CString type to use.
										CStringPtr stringNode(node);  

										// Get the string
										GenICam::gcstring str = stringNode->GetValue();

										// Check the size of the value input 
										size_t len = strlen(str);
										if ( value_size > (int)len )
										{
											strncpy((char *)value, (const char *)str, len);
											status = 0;
										}
										else
										{
											// Arg is not big enough for data 
											status = GEVLIB_ERROR_ARG_INVALID; 
										}
									}
									break;
								
								// The IValue pattern for enumeration 
								case GenApi::intfIEnumeration:
									{
										*feature_type = GENAPI_ENUM_TYPE;
										// Get us a CEnumeration type to use.
										CEnumerationPtr enumNode(node);  

										// Output is either a 32-bit or 64-bit integer.
										UINT64 val = enumNode->GetIntValue(); 
										if (value_size < (int)sizeof(UINT32) )
										{
											status = GEVLIB_ERROR_ARG_INVALID;
										}
										else if (value_size == sizeof(UINT32))
										{
											UINT32 *ptr = (UINT32 *)value;
											*ptr = (UINT32)val;
										}
										else if (value_size == sizeof(UINT64))
										{
											UINT64 *ptr = (UINT64 *)value;
											*ptr = val;
										}
										else
										{
											// Arg is too big for data (where would we put it)?
											status = GEVLIB_ERROR_ARG_INVALID; 
										}
									}
									break;
									
								// The IValue pattern for an enumEntry (to get integer from name) 
								case GenApi::intfIEnumEntry:
									{
										*feature_type = GENAPI_ENUMENTRY_TYPE;
										// Get us a CEnumeration type to use.
										CEnumEntryPtr entryNode(node);  

										// Output is either a 32-bit or 64-bit integer. 
										// (EnumEntry can also be a double)
										UINT64 val = (UINT64)entryNode->GetValue(); 
										if (value_size < (int)sizeof(UINT32) )
										{
											status = GEVLIB_ERROR_ARG_INVALID;
										}
										else if (value_size == sizeof(UINT32))
										{
											UINT32 *ptr = (UINT32 *)value;
											*ptr = (UINT32)val;
										}
										else if (value_size == sizeof(UINT64))
										{
											UINT64 *ptr = (UINT64 *)value;
											*ptr = val;
										}
										else
										{
											// Arg is too big for data (where would we put it)?
											status = GEVLIB_ERROR_ARG_INVALID; 
										}
									}
									break;
									
								// The IValue pattern for register.
								case GenApi::intfIRegister:
									{
										*feature_type = GENAPI_REGISTER_TYPE;
										// Get us a CRegister type to use.
										CRegisterPtr regNode(node);  

										// Check the size of the value input a
										if ( value_size <= regNode->GetLength() )
										{
											regNode->Get( (uint8_t *)value, (int64_t)value_size);
										}
										else
										{
											// Data is too big for arg. 
											status = GEVLIB_ERROR_ARG_INVALID; 
										}
									}
									break;
								default:
									*feature_type = GENAPI_UNUSED_TYPE;
									status = GEVLIB_ERROR_ARG_INVALID;
									break;
							}
						}
					}
					CATCH_GENAPI_ERROR(status);
				}
			}
			else
			{
				// Not initialized (node map is NULL).
				status = GEVLIB_ERROR_NULL_PTR;
			}
		}
	}
	return status;

}

//=============================================================================
// Camera image acquisition
//
//
//		Get Image parameters
/*! 
	This function obtains the image parameter from the current camera settings.
	\param [in] handle		Handle to the camera.
	\param [in] width			ptr to hold width setting returned from the camera.
	\param [in] height		ptr to hold height setting returned from the camera.
	\param [in] x_offset		ptr to hold top left corner pixel (x) setting returned from the camera.
	\param [in] y_offset		ptr to hold top left corner line (y) setting returned from the camera.
	\param [in] format		ptr to hold image format setting returned from the camera.
	\return Gev error code or CORHW error code..
	\note Errors include attempting to initialize the transfer on a 
	\note	connection that is not set up for streaming.
*/
GEV_STATUS GevGetImageParameters(GEV_CAMERA_HANDLE handle,PUINT32 width, PUINT32 height, PUINT32 x_offset, PUINT32 y_offset, PUINT32 format)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	if (handle != NULL)
	{
		status = GEVLIB_ERROR_NOT_AVAILABLE;
		// Check for the GenICam XML NodeMap.
		GenApi::CNodeMapRef *Camera = static_cast<GenApi::CNodeMapRef*>(GevGetFeatureNodeMap(handle));
		if (Camera)
		{
			status = GEVLIB_OK;
			// Use the GenICam XML Features to get the image parameters.
			GenApi::CIntegerPtr ptrIntNode;
			if (width != NULL)
			{
				try 
				{
					ptrIntNode = Camera->_GetNode("Width");
					*width = (UINT32) ptrIntNode->GetValue();  // Width is a mandatory feature.
				}
				CATCH_GENAPI_ERROR(status);
			}
			if ((height != NULL) && (status == GEVLIB_OK))
			{
				try 
				{
					ptrIntNode = Camera->_GetNode("Height");
					*height = (UINT32) ptrIntNode->GetValue();  // Height is a mandatory feature.		
				}
				CATCH_GENAPI_ERROR(status);
			}
			if ((format != NULL) && (status == GEVLIB_OK))
			{
				try 
				{
					GenApi::CEnumerationPtr ptrEnumNode = Camera->_GetNode("PixelFormat") ;
					*format = (UINT32)ptrEnumNode->GetIntValue();  // PixelFormat is a mandatory feature.	
				}
				CATCH_GENAPI_ERROR(status);
			}
			if ((x_offset != NULL) && (status == GEVLIB_OK))
			{
				*x_offset = 0;
				try 
				{
					ptrIntNode = Camera->_GetNode("OffsetX");
					*x_offset = (UINT32) ptrIntNode->GetValue();		
				}
				CATCH_GENAPI_ERROR(status);
				status = 0; // Not a mandatory feature - so - we don't really care.
			}
			if ((y_offset != NULL) && (status == GEVLIB_OK))
			{
				*y_offset = 0;
				try 
				{
					ptrIntNode = Camera->_GetNode("OffsetY");
					*y_offset = (UINT32) ptrIntNode->GetValue();		
				}
				CATCH_GENAPI_ERROR(status);
				status = 0; // Not a mandatory feature - so - we don't really care.
			}
		}
		else
		{
			// Use the static GEV_REGISTER contents to get the image parameters.
		
			DALSA_GENICAM_GIGE_REGS *registers = Gev_GetGenICamRegistersFromHandle(handle);

			if (registers != NULL)
			{
				// The camera is open so we do have access to the registers.
				// Fill in the parameters.
		
				if (width != NULL)
				{
					status = GevRegisterReadInt(handle, &registers->Width, 0, width);
				}
				if ((height != NULL) && (status == GEVLIB_OK))
				{
					status = GevRegisterReadInt(handle, &registers->Height, 0, height);
				}
				if ((format != NULL) && (status == GEVLIB_OK))
				{
					status = GevRegisterReadInt(handle, &registers->PixelFormat, 0, format);
				}
				if ((x_offset != NULL) && (status == GEVLIB_OK))
				{
					status = GevRegisterReadInt(handle, &registers->OffsetX, 0, x_offset);
				}
				if ((y_offset != NULL) && (status == GEVLIB_OK))
				{
					status = GevRegisterReadInt(handle, &registers->OffsetY, 0, y_offset);
				}
			}
		}
	}
	return status;
}

//
//		Set Image parameters
/*! 
	This function sets camera imaging ROI paramters with the input values.
	\param [in] handle		Handle to the camera.
	\param [in] width			width setting for camera ROI.
	\param [in] height		height setting for camera ROI.
	\param [in] x_offset		top left corner pixel (x) setting for camera ROI.
	\param [in] y_offset		top left corner line (y) setting for camera ROI.
	\param [in] format		image format setting for camera data.
	\return Gev error code or CORHW error code..
	\note Errors include attempting to initialize the transfer on a 
	\note	connection that is not set up for streaming.
*/
GEV_STATUS GevSetImageParameters(GEV_CAMERA_HANDLE handle,UINT32 width, UINT32 height, UINT32 x_offset, UINT32 y_offset, UINT32 format)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	if (handle != NULL)
	{
		status = GEVLIB_ERROR_NOT_AVAILABLE;
		// Check for the GenICam XML NodeMap.
		GenApi::CNodeMapRef *Camera = static_cast<GenApi::CNodeMapRef*>(GevGetFeatureNodeMap(handle));
		if (Camera)
		{
			status = GEVLIB_OK;
			
			// Use the GenICam XML Features to get the image parameters.
			GenApi::CIntegerPtr ptrIntNode;
			status = GEVLIB_OK;	
			if (status == GEVLIB_OK)
			{
				try 
				{
					ptrIntNode = Camera->_GetNode("Width");
					ptrIntNode->SetValue(width);  	// Width is a mandatory feature.
				}
				CATCH_GENAPI_ERROR(status);
			}

			if (status == GEVLIB_OK)
			{
				try 
				{
					ptrIntNode = Camera->_GetNode("Height");
					ptrIntNode->SetValue(height);  // Height is a mandatory feature.		
				}
				CATCH_GENAPI_ERROR(status);
			}
			if (status == GEVLIB_OK)
			{
				try 
				{
					GenApi::CEnumerationPtr ptrEnumNode = Camera->_GetNode("PixelFormat") ;
					ptrEnumNode->SetIntValue( (int64_t)format);	// PixelFormat is a mandatory feature.	
				}
				CATCH_GENAPI_ERROR(status);
			}
			if (status == GEVLIB_OK)
			{
				try 
				{
					ptrIntNode = Camera->_GetNode("OffsetX");
					ptrIntNode->SetValue(x_offset);		
				}
				CATCH_GENAPI_ERROR(status);
				status = GEVLIB_OK; // Not a mandatory feature - so - we don't really care.
			}
			if (status == GEVLIB_OK)
			{
				try 
				{
					ptrIntNode = Camera->_GetNode("OffsetY");
					ptrIntNode->SetValue(y_offset);		
				}
				CATCH_GENAPI_ERROR(status);
				status = GEVLIB_OK; // Not a mandatory feature - so - we don't really care.
			}
		}
		else
		{
			// Use the static GEV_REGISTER contents to get the image parameters.			
			DALSA_GENICAM_GIGE_REGS *registers = Gev_GetGenICamRegistersFromHandle(handle);

			if (registers != NULL)
			{
				// The camera is open so we do have access to the registers.
				// Fill in the parameters.
				status = GEVLIB_OK;	
				if (status == GEVLIB_OK)
				{
					status = GevRegisterWriteInt(handle, &registers->Width, 0, width);
				}
				if (status == GEVLIB_OK)
				{
					status = GevRegisterWriteInt(handle, &registers->Height, 0, height);
				}
				if (status == GEVLIB_OK)
				{
					status = GevRegisterWriteInt(handle, &registers->PixelFormat, 0, format);
				}
				if (status == GEVLIB_OK)
				{
					status = GevRegisterWriteInt(handle, &registers->OffsetX, 0, x_offset);
				}
				if (status == GEVLIB_OK)
				{
					status = GevRegisterWriteInt(handle, &registers->OffsetY, 0, y_offset);
				}
			}
		}
	}
	return status;
}



//=============================================================================
// Transport layer functions.
//

//! TL Lock
/*! 
	This function sets the transport parameters locked (TLParamsLocked) feature. 
	TLParamsLocked must be set after AcquisitionStart to present features from 
	being modified during acquisition.
	
	\param [in] handle	 Camera handle.
	\return True (locked) / False (not locked - error).
	\note None
*/
static bool _GenTLLock( GEV_CAMERA_HANDLE handle )
{
	// Check for the GenICam XML NodeMap.
	GenApi::CNodeMapRef *Camera = static_cast<GenApi::CNodeMapRef*>(GevGetFeatureNodeMap(handle));
	if (Camera)
	{
		GEV_STATUS status = GEVLIB_OK;
		try {
			// Transport Layer : Requires TLParamsLocked to be 1 after Start (A MANDATORY GenICam:SFNC feature).
			GenApi::CIntegerPtr ptrIntNode = Camera->_GetNode("TLParamsLocked");
			ptrIntNode->SetValue(1);
		}
		CATCH_GENAPI_ERROR(status);
		return (status == GEVLIB_OK);
	}
	else
	{
		// No nodemap - so no TLParamsLocked - so its OK.
		return true;
	}
}

//! TL Unlock
/*! 
	This function clears the transport parameters locked (TLParamsLocked) feature. 
	TLParamsLocked must be set after AcquisitionStart to present features from 
	being modified during acquisition.
	
	\param [in] handle	 Camera handle.
	\return True (locked) / False (not locked - error).
	\note None
*/
static bool _GenTLUnlock( GEV_CAMERA_HANDLE handle )
{
	// Check for the GenICam XML NodeMap.
	GenApi::CNodeMapRef *Camera = static_cast<GenApi::CNodeMapRef*>(GevGetFeatureNodeMap(handle));
	if (Camera)
	{
		GEV_STATUS status = GEVLIB_OK;
		try {
			// Transport Layer : Requires TLParamsLocked to be 0 after stop/abort (A MANDATORY GenICam:SFNC feature).
			GenApi::CIntegerPtr ptrIntNode = Camera->_GetNode("TLParamsLocked");
			ptrIntNode->SetValue(0);
		}
		CATCH_GENAPI_ERROR(status);
		return (status == GEVLIB_OK);
	}
	else
	{
		// No nodemap - so no TLParamsLocked - so its OK.
		return true;
	}
}

//! Snap Transfer Complete
/*! 
	This function is called internally when a snap based transfer has completed
	after the "SnapCount" frames have been delivered.
	It is used as an automatic way to unlock the TlParamsLocked feature from 
	the lower level GevLib code.

	\param [in] handle	 Camera handle.
	\return None
	\note None
*/
void Gev_Stream_TransferComplete( GEV_CAMERA_HANDLE handle )
{
	_GenTLUnlock(handle);
}
//! Start transfer
/*! 
	This function starts the streaming transfer.
	\param [in] handle	 Camera handle.
	\param [in] numFrames Number of frames to be acquired (-1 for continuous).
	\return Gev error code or CORHW error code..
	\note Errors include attempting to initialize the transfer on a 
	\note	connection that is not set up for streaming.
*/
GEV_STATUS Gev_Stream_StartTransfer( GEV_CAMERA_HANDLE handle, UINT32 numFrames)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;
	if (handle != NULL)
	{
		UINT32 state = 0;
		status = Gev_GetStreamState( handle, &state);
		
		if ( (status == GEVLIB_OK) && (state & THREAD_ACTIVE) )
		{
			// Set the snap count.
			Gev_SetStreamSnapCount(handle, numFrames);
			
			// Set the stream state to active.
			state |= STREAM_STATE_ACQUIRING;
			Gev_SetStreamState( handle, state);
					
			// Check for the GenICam XML NodeMap.
			GenApi::CNodeMapRef *Camera = static_cast<GenApi::CNodeMapRef*>(GevGetFeatureNodeMap(handle));
			if (Camera)
			{
				int num = (int)numFrames;
				if (num == -1) num = 1;
				
				// Dynamic GenICam XML feature accesses available.
				
				try
				{
					GenApi::CEnumerationPtr ptrEnumMode = Camera->_GetNode("AcquisitionMode");	// Mandatory feature
					
					if (numFrames == (UINT32)-1)
					{
						ptrEnumMode->FromString("Continuous");  // The only mandatory setting for this mandatory feature
					}
					else if (numFrames > 1)
					{
						// Vendor specific - Dalsa DFNC uses this one.
						ptrEnumMode->FromString("MultiFrame");  
					}
					else
					{
						// Vendor specific - Dalsa DFNC uses this one.
						ptrEnumMode->FromString("SingleFrame");  
					}
				}
				CATCH_GENAPI_ERROR(status);

				if (status == GEVLIB_OK)
				{	
					if (num != -1)
					{			
						try
						{
							// This is Dalsa DFNC feature (maybe others use it also - it is not mandatory).
							GenApi::CIntegerPtr ptrIntNode = Camera->_GetNode("AcquisitionFrameCount");
							ptrIntNode->SetValue( (int64_t)num);
							status = GEVLIB_OK;
						}
						CATCH_GENAPI_ERROR(status);
						if (status != GEVLIB_OK) status = GEVLIB_OK;
					}	
					
					// Transport Layer : Requires TLParamsLocked to be 1 after start (A MANDATORY GenICam:SFNC feature).
					_GenTLLock(handle);
				
					try
					{
						GenApi::CCommandPtr ptrCmdStart = Camera->_GetNode("AcquisitionStart");		// Mandatory feature
						ptrCmdStart->Execute();
						status = GEVLIB_OK;		
					}
					CATCH_GENAPI_ERROR(status);
				}
			}
			else
			{
				// Static GEV_REGISTER accesses.
				DALSA_GENICAM_GIGE_REGS *regs = Gev_GetGenICamRegistersFromHandle(handle);

				// Note : Stream thread will not be ACTIVE if internal fStreamingAvailable is FALSE 
				if ( (regs != NULL) )
				{
					// Call the GVCP comand to start acquiring images.
					if (numFrames == (UINT32)(-1))
					{
						// Continuous grab mode.
						Gev_SetStreamSnapCount( handle, 1);
						status = GevRegisterWriteInt( handle, &regs->AcquisitionFrameCount, 0, 1);
						if (status == GEVLIB_OK)
							status = GevRegisterWriteInt( handle, &regs->AcquisitionMode, 0, 0);   // enum Continuous = 0 for continuous - check XML
						if (status == GEVLIB_OK)
							status = GevRegisterWriteInt( handle, &regs->AcquisitionStart, 0, 1);
					}
					else if (numFrames == 1)
					{
						// Snap mode
						Gev_SetStreamSnapCount( handle, numFrames);
						status = GevRegisterWriteInt( handle, &regs->AcquisitionFrameCount, 0, numFrames);
						if (status == GEVLIB_OK)
							status = GevRegisterWriteInt( handle, &regs->AcquisitionMode, 0, 1);   // enum SingleFrame = 1 for single snap - check XML
						if (status == GEVLIB_OK)
							status = GevRegisterWriteInt( handle, &regs->AcquisitionStart, 0, 1);
					}
					else
					{
						// Multi-fram snap mode.
						Gev_SetStreamSnapCount( handle, numFrames);
						status = GevRegisterWriteInt( handle, &regs->AcquisitionFrameCount, 0, numFrames);
						if (status == GEVLIB_OK)
							status = GevRegisterWriteInt( handle, &regs->AcquisitionMode, 0, 2);   // enum MultiFrame = 2 for snap N - check XML
						if (status == GEVLIB_OK)
							status = GevRegisterWriteInt( handle, &regs->AcquisitionStart, 0, 1);
					}
				}
			}
			if (status != GEVLIB_OK)
			{
				// Failed !!! Set the stream state to inactive.
				state &= ~STREAM_STATE_ACQUIRING;
				Gev_SetStreamState( handle, state);
			}
		}
	}
	return status;
}

//! Stop transfer
/*! 
	This function stops the streaming transfer.
	\param [in] handle	Camera handle.
	\return Gev error code or CORHW error code..
	\note Errors include attempting to initialize the transfer on a 
	\note	connection that is not set up for streaming.
*/

GEV_STATUS Gev_Stream_StopTransfer( GEV_CAMERA_HANDLE handle)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;
	if (handle != NULL)
	{
		UINT32 state = 0;
		status = Gev_GetStreamState( handle, &state);

		if ( (status == GEVLIB_OK) && (state & THREAD_ACTIVE) )
		{
			// Check for the GenICam XML NodeMap.
			GenApi::CNodeMapRef *Camera = static_cast<GenApi::CNodeMapRef*>(GevGetFeatureNodeMap(handle));
			if (Camera)
			{
				// Dynamic GenICam XML feature accesses available.
				try
				{
					// Send the stop command  (AcquisitionStop is a mandatory feature.
					GenApi::CCommandPtr cmdPtr = Camera->_GetNode("AcquisitionStop"); 
					cmdPtr->Execute();
					
					// Poll to until done (or 1 second).
					bool done = cmdPtr->IsDone();
					int timeout = 20;   // 1 second timeout.
					while (!done && (timeout-- > 0))
					{
						Sleep(50); // 50 ms sleep.
						done = cmdPtr->IsDone();
					}
					status = GEVLIB_OK;
				}
				CATCH_GENAPI_ERROR(status);
			}
			else
			{
				// Static GEV_REGISTER accesses.
				GEV_REGISTER *reg = NULL;
				status = GevGetRegisterPtrByName( handle, (char *)"AcquisitionStop", &reg);

				// Note : Stream thread will not be ACTIVE if internal fStreamingAvailable is FALSE 
				if ( status == GEVLIB_OK)
				{
					// Call the GVCP comand to abort acquiring images.
					// (This can be vendor specific !!!)
					status = GevRegisterWriteInt( handle, reg, 0, 1);
				}
			}
			if (status == GEVLIB_OK )
			{
				// Set the stream state to inactive.
				state &= ~STREAM_STATE_ACQUIRING;
				Gev_SetStreamState( handle, state);
				
				// Transport Layer : Requires TLParamsLocked to be 0 after stop (A MANDATORY GenICam:SFNC feature).
				_GenTLUnlock(handle);
			}
		}
	}
	return status;
}



//! Abort transfer
/*! 
	This function aborts the streaming transfer.
	\param [in] handle	Camera handle.
	\return Gev error code or CORHW error code..
	\note Errors include attempting to initialize the transfer on a 
	\note	connection that is not set up for streaming.
*/


GEV_STATUS Gev_Stream_AbortTransfer( GEV_CAMERA_HANDLE handle)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;
	if (handle != NULL)
	{
		UINT32 state = 0;
		status = Gev_GetStreamState( handle, &state);

		if ( (status == GEVLIB_OK) && (state & THREAD_ACTIVE) )
		{
			// Check for the GenICam XML NodeMap.
			GenApi::CNodeMapRef *Camera = static_cast<GenApi::CNodeMapRef*>(GevGetFeatureNodeMap(handle));
			if (Camera)
			{
				// Dynamic GenICam XML feature accesses available.
				try
				{
					// Send the stop command  (AcquisitionAbort is in Dalsa DFNC - not mandatory).
					GenApi::CCommandPtr cmdPtr = Camera->_GetNode("AcquisitionAbort"); 
					cmdPtr->Execute();
					
					// Poll to until done (or 1 second).
					bool done = cmdPtr->IsDone();
					int timeout = 20;   // 1 second timeout.
					while (!done && (timeout-- > 0))
					{
						Sleep(50); // 50 ms sleep.
						done = cmdPtr->IsDone();
					}
					status = GEVLIB_OK;
				}
				CATCH_GENAPI_ERROR(status);
				
				if (status != GEVLIB_OK)
				{
					// Error - try the hard coded register table (in case it was manually programmed).
					GEV_REGISTER *reg = NULL;
					status = GevGetRegisterPtrByName( handle, (char *)"AcquisitionAbort", &reg);
					if (status == GEVLIB_OK)
					{
						status = GevRegisterWriteInt( handle, reg, 0, 1);
					}		
				}
			}
			else
			{
				// Static GEV_REGISTER accesses.
				GEV_REGISTER *reg = NULL;
				status = GevGetRegisterPtrByName( handle, (char *)"AcquisitionAbort", &reg);

				// Note : Stream thread will not be ACTIVE if internal fStreamingAvailable is FALSE 
				if ( status == GEVLIB_OK)
				{
					// Call the GVCP comand to abort acquiring images.
					// (This can be vendor specific !!!)
					status = GevRegisterWriteInt( handle, reg, 0, 1);
				}
			}
			if (status == GEVLIB_OK )
			{
				// Set the stream state to inactive.
				state |= STREAM_STATE_STOPPING;
				state &= ~STREAM_STATE_ACQUIRING;
				Gev_SetStreamState( handle, state);
				
				// Transport Layer : Requires TLParamsLocked to be 0 after abort (A MANDATORY GenICam:SFNC feature).
				_GenTLUnlock(handle);
			}
		}
	}
	return status;
}



#ifdef __cplusplus
}
#endif
