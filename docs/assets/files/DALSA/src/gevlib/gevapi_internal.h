/****************************************************************************** 
Copyright (c) 2008-2015, Teledyne DALSA Inc.
All rights reserved.

File: gevapi_internal.h
	Public API for underlying GigEVision C library.

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

/*! \file gevapi_internal.h
\brief GEV API internal definitions.

*/

#ifndef _GEVAPI_INTERNAL_H_
#define _GEVAPI_INTERNAL_H_			

#ifdef __cplusplus
extern "C" {
#endif

//====================================================================
// INCLUDES
//====================================================================
#include "gevoslib.h"		   //!< OS-specific definitions
#include "dynaqueue.h"

#define GEVAPI_DEFS
//====================================================================
// CONSTANTS
//====================================================================

#define THREAD_INACTIVE 0
#define THREAD_ACTIVE	1
#define THREAD_EXIT		2

#define STREAM_STATE_IDLE			0x1000
#define STREAM_STATE_ACQUIRING	0x2000
#define STREAM_STATE_STOPPING		0x4000

#define GEV_STATE_INACTIVE			0
#define GEV_STATE_OPENNING			1
#define GEV_STATE_INIT				2
#define GEV_STATE_DISCONNECTED	3
#define GEV_STATE_RECONNECTING	4
#define GEV_STATE_CLOSING			5


//=========================================================================
// Utility functions 
//=========================================================================
UINT32 GevGetLogLevel(void);
void PrintMAC (UINT32 macHigh, UINT32 macLow);
void PrintIP (UINT32 ip);

//=============================================================================
// Shared definitions (GevApi and GevLib)
//=============================================================================
#define GEVMEM_MAXBLOCKSIZE	536
#define GEVWRITEMEM_MAXDATASIZE	(GEVMEM_MAXBLOCKSIZE - 4)
#define GEVREADMEM_MAXDATASIZE	GEVMEM_MAXBLOCKSIZE

//=============================================================================
//=============================================================================
// Definitions for GEV API - which uses the low-level GevLib
//=============================================================================
//===================================================================
// Functions from the internal GEVLIB library.
// 
// Camera connection support.
#define GEV_NUM_PORTS_PER_HANDLE	3
GEV_CAMERA_HANDLE Gev_AllocateCameraHandle();
void Gev_FreeCameraHandle( GEV_CAMERA_HANDLE handle );
GEV_STATUS Gev_CloseCameraHandle( GEV_CAMERA_HANDLE handle );
GEV_STATUS Gev_CreateReadOnlyConnection( GEV_CAMERA_HANDLE handle, GEV_DEVICE_INTERFACE *pDevice);
GEV_STATUS Gev_CreateControlConnection( GEV_CAMERA_HANDLE handle, GEV_DEVICE_INTERFACE *pDevice, PUINT16 ports );
GEV_STATUS Gev_CreateConnection( GEV_CAMERA_HANDLE handle, GEV_DEVICE_INTERFACE *pDevice, PUINT16 ports );

// Camera register/feature access support.
GEV_STATUS Gev_WriteReg (GEV_CAMERA_HANDLE handle, UINT32 offset, UINT32 value);
GEV_STATUS Gev_WriteReg_NoAck (GEV_CAMERA_HANDLE handle, UINT32 offset, UINT32 value);
GEV_STATUS Gev_ReadReg (GEV_CAMERA_HANDLE handle, UINT32 offset, UINT32 *pValue);

GEV_STATUS Gev_ReadMem (GEV_CAMERA_HANDLE handle, UINT32 offset, char *pData, UINT32 numBytesToRead);
GEV_STATUS Gev_WriteMem (GEV_CAMERA_HANDLE handle, UINT32 offset, char *pData, UINT32 numBytesToWrite);
GEV_STATUS Gev_WriteMem_NoAck (GEV_CAMERA_HANDLE handle, UINT32 offset, char *pData, UINT32 numBytesToWrite);

// Async camera event support.
GEV_STATUS Gev_Event_RegisterEventHandler(GEV_CAMERA_HANDLE handle, UINT32 EventID, GEVEVENT_CBFUNCTION func, void *context);
GEV_STATUS Gev_Event_RegisterAppEvent( GEV_CAMERA_HANDLE handle, UINT32 EventID, _EVENT appEvent);
GEV_STATUS Gev_Event_UnregisterEvent( GEV_CAMERA_HANDLE handle, UINT32 EventID);

// Miscellaneous Gev functions.
GEV_STATUS GevForceIP (GEV_NETWORK_INTERFACE *pNetIF, UINT32 macHi, UINT32 macLo, UINT32 ip, UINT32 subnetmask);

// Streaming support.
GEV_STATUS Gev_Stream_InitTransfer( GEV_CAMERA_HANDLE handle, UINT32 height, UINT32 width, UINT32 depth, 
                                        UINT32 type, GevBufferCyclingMode mode, UINT32 numBuffers, UINT8 **bufAddress);
GEV_STATUS Gev_Stream_InitBasicTransfer( GEV_CAMERA_HANDLE handle, UINT32 height, UINT32 width, UINT32 depth, 
                                        UINT32 type, UINT32 numBuffers, UINT8 **bufAddress);
GEV_STATUS Gev_Stream_FreeTransfer( GEV_CAMERA_HANDLE handle);
GEV_STATUS Gev_Stream_StartTransfer( GEV_CAMERA_HANDLE handle, UINT32 numFrames);
GEV_STATUS Gev_Stream_StopTransfer( GEV_CAMERA_HANDLE handle);
GEV_STATUS Gev_Stream_AbortTransfer( GEV_CAMERA_HANDLE handle);

PUINT8 Gev_Stream_GetImage( GEV_CAMERA_HANDLE handle);
PGEVBUF_HEADER Gev_Stream_GetImageHeader( GEV_CAMERA_HANDLE handle);
PUINT8 Gev_Stream_GetNextImage( GEV_CAMERA_HANDLE handle, UINT32 timeout);
PGEVBUF_HEADER Gev_Stream_GetNextImageHeader( GEV_CAMERA_HANDLE handle, UINT32 timeout);

GEV_BUFFER_LIST * GevGetBufferListFromHandle( GEV_CAMERA_HANDLE handle );
// +Coming soon
GEV_STATUS GevGetImageByIndex( GEV_CAMERA_HANDLE handle, int bufIndex, GEV_BUFFER_OBJECT **pImage);
// -Coming soon


// Miscellaneous support functions.
BOOL	Gev_IsSupportedCamera( GEV_CAMERA_HANDLE handle );
BOOL	IsDALSASupportedCamera( GEV_CAMERA_HANDLE handle );
GEV_STATUS Gev_SetCameraType( GEV_CAMERA_HANDLE handle, cameraType type, BOOL fSupportedDalsaCamera);
GEV_STATUS	Gev_UpdateCameraOptions( GEV_CAMERA_HANDLE handle );

GEV_STATUS  Gev_GetPortInfoFromHandle( GEV_CAMERA_HANDLE handle, PUINT16 ports);

GEV_STATUS GevGetInterfaceOptions( GEVLIB_CONFIG_OPTIONS *options);
GEV_STATUS GevSetInterfaceOptions( GEVLIB_CONFIG_OPTIONS *options);

GEV_CAMERA_INFO * Gev_GetCameraInfoFromHandle( GEV_CAMERA_HANDLE handle );
GEV_CAMERA_OPTIONS *	Gev_GetCameraOptionsFromHandle( GEV_CAMERA_HANDLE handle );
DALSA_GENICAM_GIGE_REGS * Gev_GetGenICamRegistersFromHandle( GEV_CAMERA_HANDLE handle );
GEV_STATUS GevSetCameraRegList( GEV_CAMERA_HANDLE handle, int numRegs, GEV_REGISTER *regList);
GEV_STATUS GevGetCameraRegList( GEV_CAMERA_HANDLE handle, int *numRegs, GEV_REGISTER **regList);

GEV_STATUS Gev_SetStreamPacketDelay( GEV_CAMERA_HANDLE handle, INT32 delay);
GEV_STATUS Gev_SetStreamState( GEV_CAMERA_HANDLE handle, UINT32 state);
GEV_STATUS Gev_GetStreamState( GEV_CAMERA_HANDLE handle, PUINT32 state);
GEV_STATUS Gev_SetStreamSnapCount( GEV_CAMERA_HANDLE handle, INT32 count);
GEV_STATUS Gev_SetStreamPacketSize( GEV_CAMERA_HANDLE handle, UINT32 size);

void Gev_Stream_TransferComplete( GEV_CAMERA_HANDLE handle );

// Some XML utility functions.
GEV_STATUS _GetXMLFileUrlLocations( GEV_CAMERA_HANDLE handle, PUINT32 firstUrl, PUINT32 secondUrl, PUINT32 urlLength);
GEV_STATUS Gev_RetrieveXMLInfo( GEV_CAMERA_HANDLE handle, char *file_name, int size, int *address, int *length );


// GenApi object handling support functions.
BOOL	GevCameraIsOpen( GEV_CAMERA_HANDLE handle );
BOOL  GevCameraIsWritable( GEV_CAMERA_HANDLE handle );
GEV_STATUS GevSetFeatureNodeMapObject(  GEV_CAMERA_HANDLE handle,  void *featureNodeMap, int apiManagedNodeMap );
void *GevGetFeatureNodeMapObject(  GEV_CAMERA_HANDLE handle);
GEV_STATUS GevSetCameraPortObject(  GEV_CAMERA_HANDLE handle,  void *camPort);
void *GevGetCameraPortObject(  GEV_CAMERA_HANDLE handle);
GEV_STATUS GevSetXMLFileName(  GEV_CAMERA_HANDLE handle,  char *fileName);
GEV_STATUS GevGetXMLFileName(  GEV_CAMERA_HANDLE handle, int size, char *xmlFileName);
BOOL GevIsStringAddress( int Address );
BOOL GevIsNodeMapManaged(  GEV_CAMERA_HANDLE handle );


//====================================================================
// Low-level GEV API
//====================================================================
GEV_STATUS GevOpenStreamingChannelSocket( SOCKET *pSocket, PGEV_NETWORK_INTERFACE pNetIf, UINT16 streamPort, GevRxPkt_t *pPktInfo );
GEV_STATUS GevOpenPacketSocket( SOCKET *pSocket, int interfaceIndex, GevRxPkt_t *pPktInfo );
GEV_STATUS GevCloseStreamingChannelSocket( SOCKET *pSocket, GevRxPkt_t *pPktInfo );


//====================================================================
// Customizable portion of low-level GEV API
//====================================================================
GEV_STATUS GevReadPacketSocket( UINT8 *pDatagram, int *numBytes, struct timeval *pTimeout, GevRxPkt_t *pPktInfo );
GEV_STATUS GevRecvPacketSocket( UINT8 *pDatagram, int *numBytes, struct timeval *pTimeout, GevRxPkt_t *pPktInfo );

#ifdef __cplusplus
}
#endif



#endif
