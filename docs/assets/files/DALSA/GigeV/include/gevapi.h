/****************************************************************************** 
Copyright (c) 2008-2015, Teledyne DALSA Inc.
All rights reserved.

File: gevapi.h
	Public API for GigEVision C library.

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

/*! \file gevapi.h
\brief GEV API definitions.

*/

#ifndef _GEVAPI_H_
#define _GEVAPI_H_
			
// Environment variable for expected GenICam version.
// This gets updated with the targetted GenICam version supported.
#define GENICAM_TARGET_ROOT_VERSION 	"GENICAM_ROOT_V3_0"
#define GIGEV_XML_DOWNLOAD  "GIGEV_XML_DOWNLOAD"

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

#define GEV_LOG_LEVEL_OFF			0
#define GEV_LOG_LEVEL_NORMAL		1
#define GEV_LOG_LEVEL_ERRORS		1
#define GEV_LOG_LEVEL_WARNINGS	2
#define GEV_LOG_LEVEL_DEBUG	3
#define GEV_LOG_LEVEL_TRACE	4

#define GEV_LOG_FATAL	0
#define GEV_LOG_ERROR	1
#define GEV_LOG_WARNING 2
#define GEV_LOG_INFO		3
#define GEV_LOG_TRACE	4

//=========================================================================
// GevLib error codes. 
//=========================================================================
#define GEVLIB_OK					   	         0
#define GEVLIB_SUCCESS					   	   GEVLIB_OK
#define GEVLIB_STATUS_SUCCESS			   	   GEVLIB_OK
#define GEVLIB_STATUS_ERROR			   	   -1

//standard api errors
#define GEVLIB_ERROR_GENERIC                 -1    // Generic Error. A catch-all for unexpected behaviour.
#define GEVLIB_ERROR_NULL_PTR                -2    // NULL pointer passed to function or the result of a cast operation
#define GEVLIB_ERROR_ARG_INVALID			   	-3		// Passed argument to a function is not valid            												   
#define GEVLIB_ERROR_INVALID_HANDLE	         -4		// Invalid Handle
#define GEVLIB_ERROR_NOT_SUPPORTED           -5    // This version of hardware/fpga does not support this feature
#define GEVLIB_ERROR_TIME_OUT                -6    // Timed out waiting for a resource
#define GEVLIB_ERROR_NOT_IMPLEMENTED         -10   // Function / feature is not implemented.
#define GEVLIB_ERROR_NO_CAMERA			      -11   // The action can't be execute because the camera is not connected.
#define GEVLIB_ERROR_INVALID_PIXEL_FORMAT    -12   // Pixel Format is invalid (not supported or not recognized)
#define GEVLIB_ERROR_PARAMETER_INVALID       -13   // Passed Parameter (could be inside a data structure) is invalid/out of range.
#define GEVLIB_ERROR_SOFTWARE                -14   // software error, unexpected result
#define GEVLIB_ERROR_API_NOT_INITIALIZED     -15   // API has not been initialized
#define GEVLIB_ERROR_DEVICE_NOT_FOUND	      -16   // Device/camera specified was not found.
#define GEVLIB_ERROR_ACCESS_DENIED		      -17   // API will not access the device/camera/feature in the specified manner.
#define GEVLIB_ERROR_NOT_AVAILABLE		      -18   // Feature / function is not available for access (but is implemented).
#define GEVLIB_ERROR_NO_SPACE    		      -19   // The data being written to a feature is too large for the feature to store.

// Resource errors.
#define GEVLIB_ERROR_SYSTEM_RESOURCE          -2001 // Error creating a system resource
#define GEVLIB_ERROR_INSUFFICIENT_MEMORY	    -2002 // error allocating memory
#define GEVLIB_ERROR_INSUFFICIENT_BANDWIDTH   -2003 // Not enough bandwidth to perform operation and/or acquisition
#define GEVLIB_ERROR_RESOURCE_NOT_ALLOCATED   -2004 // Resource is not currently allocated
#define GEVLIB_ERROR_RESOURCE_IN_USE          -2005 // resource is currently being used.
#define GEVLIB_ERROR_RESOURCE_NOT_ENABLED     -2006 // The resource(feature) is not enabled
#define GEVLIB_ERROR_RESOURCE_NOT_INITIALIZED -2007 // Resource has not been intialized.
#define GEVLIB_ERROR_RESOURCE_CORRUPTED       -2008 // Resource has been corrupted
#define GEVLIB_ERROR_RESOURCE_MISSING         -2009 // A resource (ie.DLL) needed could not located
#define GEVLIB_ERROR_RESOURCE_LACK            -2010 // Lack of resource to perform a request.
#define GEVLIB_ERROR_RESOURCE_ACCESS          -2011 // Unable to correctly access the resource.
#define GEVLIB_ERROR_RESOURCE_INVALID         -2012 // A specified resource does not exist.
#define GEVLIB_ERROR_RESOURCE_LOCK            -2013 // resource is currently lock
#define GEVLIB_ERROR_INSUFFICIENT_PRIVILEGE   -2014 // Need administrator privilege.
#define GEVLIB_ERROR_RESOURCE_WRITE_PROTECTED -2015 // No data can be written to the resource
#define GEVLIB_ERROR_RESOURCE_INCOHERENCY     -2016 // The required resources are not valid together

// Data errors
#define GEVLIB_ERROR_DATA_NO_MESSAGES	      -5001	// no more messages (in fifo, queue, input stream etc)
#define GEVLIB_ERROR_DATA_OVERFLOW           -5002 // data could not be added to fifo, queue, stream etc.
#define GEVLIB_ERROR_DATA_CHECKSUM           -5003 // checksum validation fail
#define GEVLIB_ERROR_DATA_NOT_AVAILABLE		-5004 // data requested isn't available yet
#define GEVLIB_ERROR_DATA_OVERRUN				-5005 // data requested has been overrun by newer data
#define GEVLIB_ERROR_DATA_XFER_ABORT			-5006 // transfer of requested data did not finish
#define GEVLIB_ERROR_DATA_INVALID_HEADER     -5007 // data header is invalid.
#define GEVLIB_ERROR_DATA_ALIGNMENT          -5008 // data is not correctly align.

// Ethernet errors
#define GEVLIB_ERROR_CONNECTION_DROPPED          -11000
#define GEVLIB_ERROR_ANSWER_TIMEOUT              -11001
#define GEVLIB_ERROR_SOCKET_INVALID              -11002
#define GEVLIB_ERROR_PORT_NOT_AVAILABLE          -11003
#define GEVLIB_ERROR_INVALID_IP                  -11004
#define GEVLIB_ERROR_INVALID_CAMERA_OPERATION    -11005
#define GEVLIB_ERROR_INVALID_PACKET              -11006
#define GEVLIB_ERROR_INVALID_CONNECTION_ATTEMPT  -11007
#define GEVLIB_ERROR_PROTOCOL                    -11008
#define GEVLIB_ERROR_WINDOWS_SOCKET_INIT         -11009
#define GEVLIB_ERROR_WINDOWS_SOCKET_CLOSE        -11010
#define GEVLIB_ERROR_SOCKET_CREATE               -11011
#define GEVLIB_ERROR_SOCKET_RELEASE              -11012
#define GEVLIB_ERROR_SOCKET_DATA_SEND            -11013
#define GEVLIB_ERROR_SOCKET_DATA_READ            -11014
#define GEVLIB_ERROR_SOCKET_WAIT_ACKNOWLEDGE     -11015
#define GEVLIB_ERROR_INVALID_INTERNAL_COMMAND    -11016
#define GEVLIB_ERROR_INVALID_ACKNOWLEDGE         -11017
#define GEVLIB_ERROR_PREVIOUS_ACKNOWLEDGE        -11018
#define GEVLIB_ERROR_INVALID_MESSAGE             -11019
#define GEVLIB_ERROR_GIGE_ERROR                  -11020


//===================================================
// Device Level Status Codes (From low-level library)

#define GEV_STATUS_SUCCESS					0x0000		//!< Requested operation was completed successfully.
#define GEV_STATUS_NOT_IMPLEMENTED		0x8001		//!< The request isn't supported by the device.
#define GEV_STATUS_INVALID_PARAMETER	0x8002		//!< At least one parameter provided in the command is invalid (or out of range) for the device
#define GEV_STATUS_INVALID_ADDRESS		0x8003		//!< An attempt was made to access a non existent address space location.
#define GEV_STATUS_WRITE_PROTECT			0x8004		//!< The addressed register must not be written.
#define GEV_STATUS_BAD_ALIGNMENT			0x8005		//!< A badly aligned address offset or data size was specified.
#define GEV_STATUS_ACCESS_DENIED			0x8006		//!< An attempt was made to access an address location which is currently/momentary not accessible.
#define GEV_STATUS_BUSY						0x8007		//!< A required resource to service the request isn't currently available. The request may be retried.
#define GEV_STATUS_LOCAL_PROBLEM			0x8008		//!< A internal problem in the device implementation occurred while processing the request. 
#define GEV_STATUS_MSG_MISMATCH			0x8009		//!< Message mismatch (request and acknowledge don't match)
#define GEV_STATUS_INVALID_PROTOCOL		0x800A		//!< This version of the GVCP protocol is not supported
#define GEV_STATUS_NO_MSG					0x800B		//!< No message received, timeout.
#define GEV_STATUS_PACKET_UNAVAILABLE	0x800C		//!< The request packet is not available anymore.
#define GEV_STATUS_DATA_OVERRUN			0x800D		//!< Internal memory of device overrun (typically for image acquisition)
#define GEV_STATUS_INVALID_HEADER		0x800E		//!< The message header is not valid. Some of its fields do not match the specificiation.

#define GEV_STATUS_ERROR					0x8FFF		//!< Generic error. 

//=======================================
// Public Pixel Format Value Definitions
//=======================================

typedef enum
{
	fmtMono8         = 0x01080001,	/* 8 Bit Monochrome Unsigned    */
	fmtMono8Signed   = 0x01080002,	/* 8 Bit Monochrome Signed      */
	fmtMono10        = 0x01100003,	/* 10 Bit Monochrome Unsigned   */
	fmtMono10Packed  = 0x010C0004,	/* 10 Bit Monochrome Packed     */
	fmtMono12        = 0x01100005,	/* 12 Bit Monochrome Unsigned   */
	fmtMono12Packed  = 0x010C0006,	/* 12 Bit Monochrome Packed      */
	fmtMono14        = 0x01100025,	/* 14 Bit Monochrome Unsigned   */
	fmtMono16        = 0x01100007,	/* 16 Bit Monochrome Unsigned   */
	fMtBayerGR8      = 0x01080008,	/* 8-bit Bayer                  */
	fMtBayerRG8      = 0x01080009,	/* 8-bit Bayer                  */
	fMtBayerGB8      = 0x0108000A,	/* 8-bit Bayer                  */
	fMtBayerBG8      = 0x0108000B,	/* 8-bit Bayer                  */
	fMtBayerGR10     = 0x0110000C,	/* 10-bit Bayer                 */
	fMtBayerRG10     = 0x0110000D,	/* 10-bit Bayer                 */
	fMtBayerGB10     = 0x0110000E,	/* 10-bit Bayer                 */
	fMtBayerBG10     = 0x0110000F,	/* 10-bit Bayer                 */
	fMtBayerGR12     = 0x01100010,	/* 12-bit Bayer                 */
	fMtBayerRG12     = 0x01100011,	/* 12-bit Bayer                 */
	fMtBayerGB12     = 0x01100012,	/* 12-bit Bayer                 */
	fMtBayerBG12     = 0x01100013,	/* 12-bit Bayer                 */
	fmtRGB8Packed    = 0x02180014,	/* 8 Bit RGB Unsigned in 24bits */
	fmtBGR8Packed    = 0x02180015,	/* 8 Bit BGR Unsigned in 24bits */
	fmtRGBA8Packed   = 0x02200016,	/* 8 Bit RGB Unsigned           */
	fmtBGRA8Packed   = 0x02200017,	/* 8 Bit BGR Unsigned           */
	fmtRGB10Packed   = 0x02300018,	/* 10 Bit RGB Unsigned          */
	fmtBGR10Packed   = 0x02300019,	/* 10 Bit BGR Unsigned          */
	fmtRGB12Packed   = 0x0230001A,	/* 12 Bit RGB Unsigned          */
	fmtBGR12Packed   = 0x0230001B,	/* 12 Bit BGR Unsigned          */
	fmtRGB10V1Packed = 0x0220001C,	/* 10 Bit RGB custom V1 (32bits)*/
	fmtRGB10V2Packed = 0x0220001D,	/* 10 Bit RGB custom V2 (32bits)*/
	fmtYUV411packed  = 0x020C001E,	/* YUV411 (composite color) */
	fmtYUV422packed  = 0x0210001F,	/* YUV422 (composite color) */
	fmtYUV444packed  = 0x02180020,	/* YUV444 (composite color) */
	fmtRGB8Planar    = 0x02180021,	/* RGB8 Planar buffers      */
	fmtRGB10Planar   = 0x02300022,	/* RGB10 Planar buffers     */
	fmtRGB12Planar   = 0x02300023,	/* RGB12 Planar buffers     */
	fmtRGB16Planar   = 0x02300024		/* RGB16 Planar buffers     */
} enumGevPixelFormat;

#define GEV_PIXFORMAT_ISMONO		0x01000000
#define GEV_PIXFORMAT_ISCOLOR		0x02000000
#define GEV_PIXFORMAT_ISCUSTOM	0x80000000

#define GEV_PIXEL_FORMAT_MONO				0x0001		//!< Monochrome - each pixel aligned on a byte boundary.
#define GEV_PIXEL_FORMAT_MONO_PACKED	0x0002		//!< Monochrome - pixels packed end to end in memory.	
#define GEV_PIXEL_FORMAT_RGB				0x0004		//!< RGB - each pixel color aligned on a byte boundary.
#define GEV_PIXEL_FORMAT_RGB_PACKED		0x0008		//!< RGB - pixel colors packed end to end in memory.
#define GEV_PIXEL_FORMAT_BAYER			0x0010		//!< RGB - Bayer filter output.	
#define GEV_PIXEL_FORMAT_YUV				0x0020		//!< RGB - Packed YUV.	
#define GEV_PIXEL_FORMAT_RGB_PLANAR		0x0040		//!< RGB - Planar (each color plane in its own memory region).

#define GEV_PIXEL_ORDER_NONE		0x0000				//!< A "don't care" or "not applicable" value.
#define GEV_PIXEL_ORDER_RGB		0x0001				//!< Pixels are RGB (or RG for Bayer).
#define GEV_PIXEL_ORDER_BGR		0x0002				//!< Pixels are BGR (or BG for Bayer).
#define GEV_PIXEL_ORDER_GRB		0x0004				//!< Pixels are GR for Bayer.
#define GEV_PIXEL_ORDER_GBR		0x0008				//!< Pixels are GB for Bayer.
#define GEV_PIXEL_ORDER_RGB10V1	0xF000				//!< Custom format #1 for 10-bit RGB.
#define GEV_PIXEL_ORDER_RGB10V2	0xE000				//!< Custom format #2 for 10-bit RGB.

BOOL GevIsPixelTypeMono( UINT32 pixelType);
BOOL GevIsPixelTypeRGB( UINT32 pixelType);
BOOL GevIsPixelTypeCustom( UINT32 pixelType);
BOOL GevIsPixelTypePacked( UINT32 pixelType);
UINT32 GevGetPixelSizeInBytes( UINT32 pixelType);
UINT32 GevGetPixelDepthInBits( UINT32 pixelType);
UINT32 GevGetRGBPixelOrder( UINT32 pixelType);
GEVLIB_STATUS GevTranslateRawPixelFormat( UINT32 rawFormat, PUINT32 translatedFormat, PUINT32 bitDepth, PUINT32 order);
const char *GevGetFormatString( UINT32 format);
//=======================================
// Public Access Mode Value Definitions
//=======================================
typedef enum
{
	GevMonitorMode = 0,
	GevControlMode = 2,
	GevExclusiveMode = 4
} GevAccessMode;


//====================================================================
// Public Data Structures
//====================================================================

typedef struct
{
	UINT32 version;
	UINT32 logLevel;
	UINT32 numRetries;
	UINT32 command_timeout_ms;
	UINT32 discovery_timeout_ms;
	UINT32 enumeration_port;
	UINT32 gvcp_port_range_start;
	UINT32 gvcp_port_range_end;
} GEVLIB_CONFIG_OPTIONS, *PGEVLIB_CONFIG_OPTIONS;

typedef struct
{
	UINT32 numRetries;
	UINT32 command_timeout_ms;
	UINT32 heartbeat_timeout_ms;
	UINT32 streamPktSize;				// GVSP max packet size ( less than or equal to MTU size).
	UINT32 streamPktDelay;				// Delay between packets (microseconds) - to tune packet pacing out of NIC.
	UINT32 streamNumFramesBuffered;	// # of frames to buffer (min 2)
	UINT32 streamMemoryLimitMax;		// Maximum amount of memory to use (puts an upper limit on the # of frames to buffer).
	UINT32 streamMaxPacketResends;	// Maximum number of packet resends to allow for a frame (defaults to 100).
	UINT32 streamFrame_timeout_ms;	// Frame timeout (msec) after leader received.
	INT32  streamThreadAffinity;		// CPU affinity for streaming thread (marshall/unpack/write to user buffer) - default handling is "-1" 
	INT32  serverThreadAffinity;		// CPU affinity for packet server thread (recv/dispatch) - default handling is "-1"
	UINT32 msgChannel_timeout_ms;
} GEV_CAMERA_OPTIONS, *PGEV_CAMERA_OPTIONS;

typedef struct
{
	BOOL fIPv6;				// GEV is only IPv4 for now.
	UINT32 ipAddr;
	UINT32 ipAddrLow;
	UINT32 ipAddrHigh;
	UINT32 ifIndex;		// Index of network interface (set by system - required for packet interface access).
} GEV_NETWORK_INTERFACE, *PGEV_NETWORK_INTERFACE;

#define MAX_GEVSTRING_LENGTH	64

typedef struct
{
	BOOL fIPv6;				// GEV is only IPv4 for now.
	UINT32 ipAddr;
	UINT32 ipAddrLow;
	UINT32 ipAddrHigh;
	UINT32 macLow;
	UINT32 macHigh;
	GEV_NETWORK_INTERFACE host;
	UINT32 mode;
	UINT32 capabilities;
	char   manufacturer[MAX_GEVSTRING_LENGTH+1];
	char   model[MAX_GEVSTRING_LENGTH+1];
	char   serial[MAX_GEVSTRING_LENGTH+1];
	char   version[MAX_GEVSTRING_LENGTH+1];
	char   username[MAX_GEVSTRING_LENGTH+1];
} GEV_DEVICE_INTERFACE, *PGEV_DEVICE_INTERFACE, GEV_CAMERA_INFO, *PGEV_CAMERA_INFO;

typedef void* GEV_CAMERA_HANDLE;

// Buffer object structure - returned 
typedef struct _tag_GEVBUF_ENTRY
{
	UINT32 state;			// Full/empty state for image buffer
	UINT32 status;			// Frame Status (success, error types) (see below - GEV_FRAME_STATUS_*)
	UINT32 timestamp_hi;
	UINT32 timestamp_lo;
	UINT32 recv_size;		// Received size for buffer (allows variable sized data).
	UINT32 id;				// Block id for image (starts at 1, wraps to 1 at 65535).
	UINT32 h;				// Received heigth (pixels) for this buffer
	UINT32 w;				// Received width (pixels) for ROI in this buffer
	UINT32 x_offset;		// Received x offset for origin of ROI in this buffer
	UINT32 y_offset;		// Received y offset for origin of ROI in this buffer
	UINT32 x_padding;		// Received x padding bytes (invalid data padding end of each line [horizontal invalid])
	UINT32 y_padding;		// Received y padding bytes (invalid data padding end of image [vertical invalid])
	UINT32 d;				// Received depth (bytes per pixel) for this buffer
	UINT32 format;			// Received format for image.
	PUINT8 address;
} GEVBUF_ENTRY, *PGEVBUF_ENTRY, GEVBUF_HEADER, *PGEVBUF_HEADER, GEV_BUFFER_OBJECT, *PGEV_BUFFER_OBJECT;

#define GEV_FRAME_STATUS_RECVD		0	// Frame is complete.
#define GEV_FRAME_STATUS_PENDING		1	// Frame is not ready.
#define GEV_FRAME_STATUS_TIMEOUT		2	// Frame was not ready before timeout condition met.
#define GEV_FRAME_STATUS_OVERFLOW	3  // Frame was not complete before the max number of frames to buffer queue was full.
#define GEV_FRAME_STATUS_BANDWIDTH	4	// Frame had too many resend operations due to insufficient bandwidth.
#define GEV_FRAME_STATUS_LOST			5	// Frame had resend operations that failed.

// Buffer cycling control definition
typedef enum
{
	Asynchronous = 0, 
	SynchronousNextEmpty = 1 
} GevBufferCyclingMode;

// Buffer list
typedef struct _GEVBUF_QUEUE
{
	UINT32 type;		// Image type expected for buffers in list (from camera)
	UINT32 height;		// Image height for allocated buffers in list.
	UINT32 width;		// Image width for allocated buffers in list.
	UINT32 depth;		// Image depth (bytes) for allocated buffers in list.
	UINT32 size;		// Image size allocated for buffers in list.
	UINT32 numBuffer;	// Number of buffers in the list.
	INT32  lastBuffer; // Temp for debugging
	UINT32 nextBuffer; // Temp for debugging
	UINT32 trashCount;	// Number of buffers that have been sent to trash (no available free buffer).
	GevBufferCyclingMode	cyclingMode; // Buffer queue operation mode (Asynchonous, SynchronousNextEmpty, SynchronousNextEmptyWithTrash)
	DQUEUE	*pEmptyBuffers;	// Queue of empty/unlocked GEV_BUFFER_OBJECT data structures.
	DQUEUE	*pFullBuffers;		// Queue of full/locked GEV_BUFFER_OBJECT data structures.
	GEVBUF_ENTRY *pCurBuf;		// Pointer to the current (active) buffer object (NULL if cycling is dumping to trash).
	GEVBUF_ENTRY buffer[1];		// List of buffers available for storage.
} GEV_BUFFER_LIST;

// Asycnhronous event information structure - returned in callback.
typedef struct
{
   UINT16 reserved;              // Not used.
   UINT16 eventNumber;           // Event number.
   UINT16 streamChannelIndex;    // Channel index associated with this event.
   UINT16 blockId;               // Block Id associated with this event.
   UINT32 timeStampHigh;         // Most 32 significant bit of the timestamp.
   UINT32 timeStampLow;          // Least 32 significant bit of the timestamp.
} EVENT_MSG, *PEVENT_MSG;

typedef void (*GEVEVENT_CBFUNCTION) (PEVENT_MSG msg, PUINT8 data, UINT16 size, void *context);

//=========================================================================
// Utility functions and macros
//=========================================================================
UINT32 GevFormatCameraInfo( GEV_DEVICE_INTERFACE *pCamera, char *pBuf, UINT32 size);

#if 1
int GevPrint( int level, const char *file, unsigned int line, const char *fmt, ...);
#define LogPrint(lvl,args...) GevPrint((lvl), __FILE__, __LINE__, ##args)
#else
int GevPrint( int level, const char *fmt, ...);
#define LogPrint GevPrint
#endif

//======================================================================================
// GenApi feature access definitions.
// Exception handling catch macro (extensible - logging can also be modified / extended).
#define CATCH_GENAPI_ERROR(statusToReturn) \
				catch (InvalidArgumentException &E) \
				{ \
					GevPrint(GEV_LOG_ERROR, E.GetSourceFileName(), E.GetSourceLine(), " GenApi: Invalid Argument Exception %s", E.what()); \
					statusToReturn = GEVLIB_ERROR_ARG_INVALID; \
				} \
				catch (PropertyException &E) \
				{ \
					GevPrint(GEV_LOG_ERROR, E.GetSourceFileName(), E.GetSourceLine(), " GenApi: Property Exception %s at line %d of %s\n", E.what()); \
					statusToReturn = GEVLIB_ERROR_SOFTWARE; \
				} \
				catch (LogicalErrorException &E) \
				{ \
					GevPrint(GEV_LOG_ERROR, E.GetSourceFileName(), E.GetSourceLine(), " GenApi: Logical Error Exception %s at line %d of %s\n", E.what()); \
					statusToReturn = GEVLIB_ERROR_SOFTWARE; \
				} \
				catch (OutOfRangeException &E) \
				{ \
					GevPrint(GEV_LOG_ERROR, E.GetSourceFileName(), E.GetSourceLine(), " GenApi: OutOfRange Exception %s at line %d of %s\n", E.what()); \
					statusToReturn = GEVLIB_ERROR_PARAMETER_INVALID; \
				} \
				catch (RuntimeException &E) \
				{ \
					GevPrint(GEV_LOG_ERROR, E.GetSourceFileName(), E.GetSourceLine(), " GenApi: Runtime Exception %s at line %d of %s\n", E.what()); \
					statusToReturn = GEVLIB_ERROR_GENERIC; \
				} \
				catch (AccessException &E) \
				{ \
					GevPrint(GEV_LOG_ERROR, E.GetSourceFileName(), E.GetSourceLine(), " GenApi: Access Exception %s at line %d of %s\n", E.what()); \
					statusToReturn = GEVLIB_ERROR_ACCESS_DENIED; \
				} \
				catch (TimeoutException &E) \
				{ \
					GevPrint(GEV_LOG_ERROR, E.GetSourceFileName(), E.GetSourceLine(), " GenApi: Timeout Exception %s at line %d of %s\n", E.what()); \
					statusToReturn = GEVLIB_ERROR_TIME_OUT ; \
				} \
				catch (DynamicCastException &E) \
				{ \
					GevPrint(GEV_LOG_ERROR, E.GetSourceFileName(), E.GetSourceLine(), " GenApi: DynamiceCast Exception %s at line %d of %s\n", E.what()); \
					statusToReturn = GEVLIB_ERROR_NULL_PTR; \
				} \
				catch (std::exception &E) \
				{ \
					GevPrint(GEV_LOG_ERROR, __FILE__, __LINE__, " StdLib: Exception %s\n", E.what()); \
					statusToReturn = GEVLIB_ERROR_SOFTWARE; \
				} \
				catch (...) \
				{ \
					GevPrint(GEV_LOG_ERROR, __FILE__, __LINE__, " GenApi: Unknown Error Exception (catchall)\n"); \
					statusToReturn = GEVLIB_ERROR_GENERIC; \
				} \

// Feature types returned (in C API).
#define GENAPI_UNUSED_TYPE		1
#define GENAPI_VALUE_TYPE		0
#define GENAPI_BASE_TYPE		1	
#define GENAPI_INTEGER_TYPE	2
#define GENAPI_BOOLEAN_TYPE	3
#define GENAPI_COMMAND_TYPE	4
#define GENAPI_FLOAT_TYPE		5
#define GENAPI_STRING_TYPE		6
#define GENAPI_REGISTER_TYPE	7
#define GENAPI_CATEGORY_TYPE	8
#define GENAPI_ENUM_TYPE		9
#define GENAPI_ENUMENTRY_TYPE	10

// Feature access mode (in C API)
#define	GENAPI_ACCESSMODE_NI		0	// Not implemented
#define	GENAPI_ACCESSMODE_NA		1	// Not available
#define	GENAPI_ACCESSMODE_WO		2	// Write-only
#define	GENAPI_ACCESSMODE_RO		3	// Read-only
#define	GENAPI_ACCESSMODE_RW		4	// Read-write
#define	GENAPI_ACCESSMODE_NONE	5	// Undefined

// Feature visibility (in C API)
#define	GENAPI_VISIBILITY_BEGINNER		0
#define	GENAPI_VISIBILITY_EXPERT		1
#define	GENAPI_VISIBILITY_GURU			2
#define	GENAPI_VISIBILITY_INVISIBLE	3
#define	GENAPI_VISIBILITY_UNDEFINED	99

// Feature increment type (in C API)
#define GENAPI_INCREMENT_NONE		0
#define GENAPI_INCREMENT_FIXED	1
#define GENAPI_INCREMENT_LIST		2



//====================================================================
// Public API
//====================================================================
// API Initialization
GEV_STATUS	GevApiInitialize(void);
GEV_STATUS	GevApiUninitialize(void);

//====================================================================
// API Configuratoin options
GEV_STATUS GevGetLibraryConfigOptions( GEVLIB_CONFIG_OPTIONS *options);
GEV_STATUS GevSetLibraryConfigOptions( GEVLIB_CONFIG_OPTIONS *options);

//=================================================================================================
// Camera automatic discovery
int GevDeviceCount(void);	// Get the number of Gev devices seen by the system.
GEV_STATUS GevGetCameraList( GEV_CAMERA_INFO *cameras, int maxCameras, int *numCameras); // Automatically detect and list cameras.

GEV_STATUS GevForceCameraIPAddress( UINT32 macHi, UINT32 macLo, UINT32 IPAddress, UINT32 subnetmask);
GEV_STATUS GevEnumerateNetworkInterfaces(GEV_NETWORK_INTERFACE *pIPAddr, UINT32 maxInterfaces, PUINT32 pNumInterfaces );

//=================================================================================================
// Utility function (external) for discovering camera devices.  
GEV_STATUS GevEnumerateGevDevices(GEV_NETWORK_INTERFACE *pIPAddr, UINT32 discoveryTimeout, GEV_DEVICE_INTERFACE *pDevice, UINT32 maxDevices, PUINT32 pNumDevices );

// Camera Manual discovery/setup 
GEV_STATUS GevSetCameraList( GEV_CAMERA_INFO *cameras, int numCameras); // Manually set camera list from data structure.

//=================================================================================================
// Gige Vision Camera Access
GEV_STATUS GevOpenCamera( GEV_CAMERA_INFO *device, GevAccessMode mode, GEV_CAMERA_HANDLE *handle);
GEV_STATUS GevOpenCameraByAddress( unsigned long ip_address, GevAccessMode mode, GEV_CAMERA_HANDLE *handle);
GEV_STATUS GevOpenCameraByName( char *name, GevAccessMode mode, GEV_CAMERA_HANDLE *handle);
GEV_STATUS GevOpenCameraBySN( char *sn, GevAccessMode mode, GEV_CAMERA_HANDLE *handle);

GEV_STATUS GevCloseCamera(GEV_CAMERA_HANDLE *handle);

GEV_CAMERA_INFO *GevGetCameraInfo( GEV_CAMERA_HANDLE handle);

GEV_STATUS GevGetCameraInterfaceOptions( GEV_CAMERA_HANDLE handle, GEV_CAMERA_OPTIONS *options);
GEV_STATUS GevSetCameraInterfaceOptions( GEV_CAMERA_HANDLE handle, GEV_CAMERA_OPTIONS *options);

//=================================================================================================
// Manual GigeVision access to GenICam XML File
GEV_STATUS Gev_RetrieveXMLData( GEV_CAMERA_HANDLE handle, int size, char *xml_data, int *num_read, int *data_is_compressed );
GEV_STATUS Gev_RetrieveXMLFile( GEV_CAMERA_HANDLE handle, char *file_name, int size, BOOL force_download );

//=================================================================================================
// GenICam XML Feature Node Map manual registration/access functions (for use in C++ code).
GEV_STATUS GevConnectFeatures(  GEV_CAMERA_HANDLE handle,  void *featureNodeMap);
void * GevGetFeatureNodeMap(  GEV_CAMERA_HANDLE handle);

//=================================================================================================
// GenICam XML Feature access functions (C language compatible).
GEV_STATUS GevGetGenICamXML_FileName( GEV_CAMERA_HANDLE handle, int size, char *xmlFileName);
GEV_STATUS GevInitGenICamXMLFeatures( GEV_CAMERA_HANDLE handle, BOOL updateXMLFile);
GEV_STATUS GevInitGenICamXMLFeatures_FromFile( GEV_CAMERA_HANDLE handle, char *xmlFileName);
GEV_STATUS GevInitGenICamXMLFeatures_FromData( GEV_CAMERA_HANDLE handle, int size, void *pXmlData);

GEV_STATUS GevGetFeatureValue( GEV_CAMERA_HANDLE handle, const char *feature_name, int *feature_type, int value_size, void *value);
GEV_STATUS GevSetFeatureValue( GEV_CAMERA_HANDLE handle, const char *feature_name, int value_size, void *value);
GEV_STATUS GevGetFeatureValueAsString( GEV_CAMERA_HANDLE handle, const char *feature_name, int *feature_type, int value_string_size, char *value_string);
GEV_STATUS GevSetFeatureValueAsString( GEV_CAMERA_HANDLE handle, const char *feature_name, const char *value_string);

//=================================================================================================
// Camera image acquisition
GEV_STATUS GevGetImageParameters(GEV_CAMERA_HANDLE handle,PUINT32 width, PUINT32 height, PUINT32 x_offset, PUINT32 y_offset, PUINT32 format);
GEV_STATUS GevSetImageParameters(GEV_CAMERA_HANDLE handle,UINT32 width, UINT32 height, UINT32 x_offset, UINT32 y_offset, UINT32 format);

GEV_STATUS GevInitImageTransfer( GEV_CAMERA_HANDLE handle, GevBufferCyclingMode mode, UINT32 numBuffers, UINT8 **bufAddress);
GEV_STATUS GevInitializeImageTransfer( GEV_CAMERA_HANDLE handle, UINT32 numBuffers, UINT8 **bufAddress);
GEV_STATUS GevFreeImageTransfer( GEV_CAMERA_HANDLE handle);
GEV_STATUS GevStartImageTransfer( GEV_CAMERA_HANDLE handle, UINT32 numFrames);
GEV_STATUS GevStopImageTransfer( GEV_CAMERA_HANDLE handle);
GEV_STATUS GevAbortImageTransfer( GEV_CAMERA_HANDLE handle);

GEV_STATUS GevQueryImageTransferStatus( GEV_CAMERA_HANDLE handle, PUINT32 pTotalBuffers, PUINT32 pNumUsed, PUINT32 pNumFree, PUINT32 pNumTrashed, GevBufferCyclingMode *pMode);
int GetPixelSizeInBytes (UINT32 pixelType);

// +Coming soon
GEV_STATUS GevResetImageTransfer( GEV_CAMERA_HANDLE handle );
// -Coming soon

GEV_STATUS GevGetNextImage( GEV_CAMERA_HANDLE handle, GEV_BUFFER_OBJECT **image_object_ptr, struct timeval *pTimeout);
GEV_STATUS GevGetImageBuffer( GEV_CAMERA_HANDLE handle, void **image_buffer);
GEV_STATUS GevGetImage( GEV_CAMERA_HANDLE handle, GEV_BUFFER_OBJECT **image_object);
GEV_STATUS GevWaitForNextImageBuffer( GEV_CAMERA_HANDLE handle, void **image_buffer, UINT32 timeout);
GEV_STATUS GevWaitForNextImage( GEV_CAMERA_HANDLE handle, GEV_BUFFER_OBJECT **image_object, UINT32 timeout);

GEV_STATUS GevReleaseImage( GEV_CAMERA_HANDLE handle, GEV_BUFFER_OBJECT *image_object_ptr);
GEV_STATUS GevReleaseImageBuffer( GEV_CAMERA_HANDLE handle, void *image_buffer_ptr);

//=================================================================================================
// Camera event handling
GEV_STATUS GevRegisterEventCallback(GEV_CAMERA_HANDLE handle,  UINT32 EventID, GEVEVENT_CBFUNCTION func, void *context);
GEV_STATUS GevRegisterApplicationEvent(GEV_CAMERA_HANDLE handle,  UINT32 EventID, _EVENT appEvent);
GEV_STATUS GevUnregisterEvent(GEV_CAMERA_HANDLE handle,  UINT32 EventID);



//=================================================================================================
// GEV_REGISTER definitions 
// (static camera register definitions for tables in cameraregdata.c)
//
//
//====================================================================
// GEV API Camera identification information (internal)
//====================================================================
typedef enum
{
	cameraUnknown     = 0,
	cameraGenieMono   = 1,
	cameraGenieColor  = 2,
	cameraGenieHM     = 3,
	cameraDracoBased  = 4,
	cameraSpyder3SG114K = 5,
	cameraSpyder3SG111K = 6,
	cameraSpyder3SG144K = 7,
	cameraSpyder3SG324K = 8,
	cameraSpyder3SG344K = 9,
	cameraGenieTS = 10,
	cameraXMLBased = 11,
} cameraType;

//=============================================================================
// Camera basic "feature" access. 
// (Complex features use the full feature access methods (not implemented yet)).
//=============================================================================

#define FEATURE_NAME_MAX_SIZE 64
#define NOREF_ADDR 				0	// No reference address - feature has no backing register.

typedef void* PGENICAM_FEATURE;	// Place holder until feature methods are complete.

typedef enum
{
	RO = 1,
	WO = 2,
	RW = 3
} RegAccess;

typedef enum
{
	stringReg,
	floatReg,		/* Big-endian register - float 			*/
	integerReg,		/* Big-endian register - integer 		*/
	bitReg,			/* Big-endian register - bit 				*/
	fixedVal,		/* Reg has one value - access is a trigger    */
	intVal,			/* Locally stored value - no backing register */
	floatVal,
	dataArea,		/* Application must handle endian-ness of a data area */
	floatRegLE,		/* Little-endian register - float 		*/
	integerRegLE,	/* Little-endian register - integer 	*/
	bitRegLE		/* Little-endian register - bit 			*/
} RegType;

typedef struct
{
	char   name[FEATURE_NAME_MAX_SIZE];	
	UINT32 value;
} ENUM_ENTRY, *PENUM_ENTRY;

typedef union
{
		UINT32			bitIndex;	// Bit number if a Bit
		UINT32			intValue;
		float				floatValue;
} GENIREG_VALUE;

//typedef UINT32 BOOL32;

typedef struct
{
	char					featureName[FEATURE_NAME_MAX_SIZE];   // String name of feature for this register.
	UINT32				address;		 // Address for accessing feature in camera (NOREF_ADDR if not in camera).
	RegAccess			accessMode;	 // RO, WO, RW access allowed.
	BOOL32				available;	 // True if feature is available (in camera or not) - False is not available.
	RegType				type;			 // String, Float, Integer, Enum, Bit
	UINT32				regSize;		 // Size of storage for register (or register set / area).
	UINT32				regStride;   // Increment between register items accessed via selector
	UINT32				minSelector; // Minimum value for selector (corresponds to base address).
	UINT32				maxSelector; // Maximum value for selector.
	GENIREG_VALUE		value;       // Current value (storage for features not backed by a register).
	GENIREG_VALUE		minValue;    // Minimum allowable value.
	GENIREG_VALUE		maxValue;    // Maximum allowable value.
	UINT32				readMask;	 // AND Mask for read (integers only)
	UINT32				writeMask;	 // AND Mask for write (integers only)
	PGENICAM_FEATURE	feature;		 // Pointer to feature in feature table (future).
	char					selectorName[FEATURE_NAME_MAX_SIZE];  // String name of selector for feature.
	char					indexName[FEATURE_NAME_MAX_SIZE];     // String name of index (second selector)for feature.
} GEV_REGISTER, *PGEV_REGISTER;



//=======================================================================================
// Registers for GigE Cameras (includes DALSA-specific registers and GeniCam standard registers).
// Only actual registers are here - not generic "features".
//=======================================================================================

typedef struct
{
	GEV_REGISTER DeviceVendorName;
	GEV_REGISTER DeviceModelName; 
	GEV_REGISTER DeviceVersion;
	GEV_REGISTER DeviceFirmwareVersion;
	GEV_REGISTER DeviceID;
	GEV_REGISTER DeviceUserID;
	GEV_REGISTER DeviceScanType;
	GEV_REGISTER DeviceMaxThroughput; 
	GEV_REGISTER DeviceRegistersStreamingStart; 
	GEV_REGISTER DeviceRegistersStreamingEnd; 
	GEV_REGISTER DeviceRegistersCheck; 
	GEV_REGISTER DeviceRegistersValid;

	GEV_REGISTER SensorWidth;	
	GEV_REGISTER SensorHeight;	
	GEV_REGISTER WidthMax;	
	GEV_REGISTER HeightMax;	 
	GEV_REGISTER Width;	 
	GEV_REGISTER Height;	 
	GEV_REGISTER OffsetX;	
	GEV_REGISTER OffsetY; 
	GEV_REGISTER LinePitch;	 
	GEV_REGISTER BinningHorizontal;	 
	GEV_REGISTER BinningVertical;	
	GEV_REGISTER DecimationHorizontal;
	GEV_REGISTER DecimationVertical;
	GEV_REGISTER ReverseX;	 
	GEV_REGISTER ReverseY;	 
	GEV_REGISTER PixelColorFilter;	 
	GEV_REGISTER PixelCoding;	 
	GEV_REGISTER PixelSize;	 
	GEV_REGISTER PixelFormat;	 
	GEV_REGISTER PixelDynamicRangeMin;	
	GEV_REGISTER PixelDynamicRangeMax;	
	GEV_REGISTER TestImageSelector;
	
	GEV_REGISTER AcquisitionMode;	
	GEV_REGISTER AcquisitionStart;	
	GEV_REGISTER AcquisitionStop;	 
	GEV_REGISTER AcquisitionAbort;	
	GEV_REGISTER AcquisitionArm;
	GEV_REGISTER AcquisitionFrameCount;	
	GEV_REGISTER AcquisitionFrameRateMax;
	GEV_REGISTER AcquisitionFrameRateMin;
	GEV_REGISTER AcquisitionFrameRateRaw;	
	GEV_REGISTER AcquisitionFrameRateAbs;	 
	GEV_REGISTER AcquisitionLineRateRaw;
	GEV_REGISTER AcquisitionLineRateAbs;	 
	GEV_REGISTER AcquisitionStatusSelector;
	GEV_REGISTER AcquisitionStatus;	 
	GEV_REGISTER TriggerSelector;	
	GEV_REGISTER TriggerMode;	
	GEV_REGISTER TriggerSoftware;	
	GEV_REGISTER TriggerSource;	
	GEV_REGISTER TriggerActivation;	 
	GEV_REGISTER TriggerOverlap;
	GEV_REGISTER TriggerDelayAbs;	
	GEV_REGISTER TriggerDelayRaw;	
	GEV_REGISTER TriggerDivider;
	GEV_REGISTER TriggerMultiplier;
	GEV_REGISTER ExposureMode;	
	GEV_REGISTER ExposureAlignment;	// (*) in Genie but not in standard	
	GEV_REGISTER ExposureDelay;		// (*) in Genie but not in standard	
	GEV_REGISTER ExposureTimeRaw;	 
	GEV_REGISTER ExposureTimeAbs;
	GEV_REGISTER ExposureAuto;	
	GEV_REGISTER ExposureTimeMin;	  // (*) in Genie but not in standard	
	GEV_REGISTER ExposureTimeMax;	  // (*) in Genie but not in standard		

	GEV_REGISTER LineSelector;
	GEV_REGISTER LineMode;
	GEV_REGISTER LineInverter;
	GEV_REGISTER LineStatus;
	GEV_REGISTER LineStatusAll;
	GEV_REGISTER LineSource;
	GEV_REGISTER OutputLineEventSource;			// Genie version of "LineSource" 
	GEV_REGISTER LineFormat;
	GEV_REGISTER UserOutputValue;
	GEV_REGISTER OutputLineValue;					// Genie Version of "UserOutputValue"
	GEV_REGISTER UserOutputSelector;
	GEV_REGISTER UserOutputValueAll;
	GEV_REGISTER UserOutputValueAllMask;

	GEV_REGISTER InputLinePolarity;				// (*) in Genie but not in standard	
	GEV_REGISTER InputLineDebouncingPeriod;	// (*) in Genie but not in standard	
	GEV_REGISTER OutputLinePulsePolarity;		// (*) in Genie but not in standard	
	GEV_REGISTER OutputLineMode;					// (*) in Genie but not in standard	
	GEV_REGISTER OutputLinePulseDelay;			// (*) in Genie but not in standard		 
	GEV_REGISTER OutputLinePulseDuration;		// (*) in Genie but not in standard	

	GEV_REGISTER CounterSelector;	
	GEV_REGISTER CounterEventSource;	
	GEV_REGISTER CounterLineSource;			// (*) in Genie but not in standard	
	GEV_REGISTER CounterReset;
	GEV_REGISTER CounterValue;					
	GEV_REGISTER CounterValueAtReset;		
	GEV_REGISTER CounterDuration;				
	GEV_REGISTER CounterStatus;				
	GEV_REGISTER CounterTriggerSource;		
	GEV_REGISTER CounterTriggerActivation;	
	GEV_REGISTER TimerSelector;				
	GEV_REGISTER TimerDurationAbs;			
	GEV_REGISTER TimerDurationRaw;			
	GEV_REGISTER TimerDelayAbs;				
	GEV_REGISTER TimerDelayRaw;				
	GEV_REGISTER TimerValueAbs;				
	GEV_REGISTER TimerValueRaw;				
	GEV_REGISTER TimerStatus;					
	GEV_REGISTER TimerTriggerSource;			
	GEV_REGISTER TimerTriggerActivation;	

	GEV_REGISTER EventSelector; 
	GEV_REGISTER EventNotification; 

	GEV_REGISTER GainSelector;	
	GEV_REGISTER GainRaw;	 
	GEV_REGISTER GainAbs;					
	GEV_REGISTER GainAuto;					
	GEV_REGISTER GainAutoBalance;			
	GEV_REGISTER BlackLevelSelector;	 
	GEV_REGISTER BlackLevelRaw;	 
	GEV_REGISTER BlackLevelAbs;			
	GEV_REGISTER BlackLevelAuto;			
	GEV_REGISTER BlackLevelAutoBalance;	
	GEV_REGISTER WhiteClipSelector;		
	GEV_REGISTER WhiteClipRaw;				
	GEV_REGISTER WhiteClipAbs;				
	GEV_REGISTER BalanceRatioSelector;	
	GEV_REGISTER BalanceRatioAbs;			
	GEV_REGISTER BalanceWhiteAuto;		
	GEV_REGISTER Gamma;						

	GEV_REGISTER LUTSelector;
	GEV_REGISTER LUTEnable;
	GEV_REGISTER LUTIndex;
	GEV_REGISTER LUTValue;
	GEV_REGISTER LUTValueAll;			

	GEV_REGISTER UserSetDefaultSelector; 
	GEV_REGISTER UserSetSelector;
	GEV_REGISTER UserSetLoad; 
	GEV_REGISTER UserSetSave; 

	//===================================================
	// Gev transport layer registers.
	//
	GEV_REGISTER PayloadSize; 
/*==================================================
	GEV_REGISTER GevVersionMajor; 
	GEV_REGISTER GevVersionMinor; 
	GEV_REGISTER GevDeviceModeIsBigEndian; 
	GEV_REGISTER GevDeviceModeCharacterSet; 
	GEV_REGISTER GevInterfaceSelector; 
	GEV_REGISTER GevMACAddress; 
==================================================*/
	GEV_REGISTER GevSupportedIPConfigurationLLA; 
	GEV_REGISTER GevSupportedIPConfigurationDHCP; 
	GEV_REGISTER GevSupportedIPConfigurationPersistentIP; 


	GEV_REGISTER GevCurrentIPConfigurationLLA;				// (**)not in Genie
	GEV_REGISTER GevCurrentIPConfigurationDHCP;				// (**)not in Genie 
	GEV_REGISTER GevCurrentIPConfigurationPersistentIP;	// (**)not in Genie 

	GEV_REGISTER GevCurrentIPConfiguration; 
	GEV_REGISTER GevCurrentIPAddress; 
	GEV_REGISTER GevCurrentSubnetMask; 
	GEV_REGISTER GevCurrentDefaultGateway; 
	GEV_REGISTER GevPersistentIPAddress; 
	GEV_REGISTER GevPersistentSubnetMask; 
	GEV_REGISTER GevPersistentDefaultGateway; 
	GEV_REGISTER GevFirstURL; 
	GEV_REGISTER GevSecondURL; 
	GEV_REGISTER GevNumberOfInterfaces;
/*================================================== 
	GEV_REGISTER GevMessageChannelCount; 
	GEV_REGISTER GevStreamChannelCount;
	GEV_REGISTER GevSupportedOptionalCommandsUserDefinedName; 
	GEV_REGISTER GevSupportedOptionalCommandsSerialNumber; 
	GEV_REGISTER GevSupportedOptionalCommandsEVENTDATA; 
	GEV_REGISTER GevSupportedOptionalCommandsEVENT; 
	GEV_REGISTER GevSupportedOptionalCommandsPACKETRESEND; 
	GEV_REGISTER GevSupportedOptionalCommandsWRITEMEM; 
	GEV_REGISTER GevSupportedOptionalCommandsConcatenation; 
	GEV_REGISTER GevHeartbeatTimeout; 
	GEV_REGISTER GevTimestampTickFrequency; 
	GEV_REGISTER GevTimestampControlLatch; 
	GEV_REGISTER GevTimestampControlReset; 
	GEV_REGISTER GevTimestampValue;

	GEV_REGISTER GevCCP;	
	GEV_REGISTER GevMCPHostPort;
	GEV_REGISTER GevMCDA;		
	GEV_REGISTER GevMCTT;	
	GEV_REGISTER GevMCRC;	

	GEV_REGISTER GevStreamChannelSelector; 
	GEV_REGISTER GevSCPInterfaceIndex;

	GEV_REGISTER GevSCPHostPort;	
	GEV_REGISTER GevSCPFireTestPacket;
	GEV_REGISTER GevSCPDoNotFragment;
	GEV_REGISTER GevSCPSBigEndian;	

	GEV_REGISTER GevSCPSPacketSize;
	GEV_REGISTER GevSCPD; 

	GEV_REGISTER GevSCDA;			
====================================*/

	GEV_REGISTER GevLinkSpeed;	
	GEV_REGISTER GevIPConfigurationStatus;

	//============================================
	//	Chunk data support (not in Genie)
	//
	GEV_REGISTER ChunkModeActive;			
	GEV_REGISTER ChunkSelector;			
	GEV_REGISTER ChunkEnable;				
	GEV_REGISTER ChunkOffsetX;				
	GEV_REGISTER ChunkOffsetY;				
	GEV_REGISTER ChunkWidth;				
	GEV_REGISTER ChunkHeight;				
	GEV_REGISTER ChunkPixelFormat;		
	GEV_REGISTER ChunkDynamicRangeMax;
	GEV_REGISTER ChunkDynamicRangeMin;	
	GEV_REGISTER ChunkTimestamp;			
	GEV_REGISTER ChunkLineStatusAll;		
	GEV_REGISTER ChunkCounterSelector;
	GEV_REGISTER ChunkCounter;				
	GEV_REGISTER ChunkTimerSelector;		
	GEV_REGISTER ChunkTimer;				

	//============================================
	// File Access support (not in Genie)
	//
	GEV_REGISTER FileSelector;				
	GEV_REGISTER FileOperationSelector;	
	GEV_REGISTER FileOperationExecute;	
	GEV_REGISTER FileOpenModeSelector;	
	GEV_REGISTER FileAccessOffset;		
	GEV_REGISTER FileAccessLength;		
	GEV_REGISTER FileAccessBuffer;		
	GEV_REGISTER FileOperationStatus;	
	GEV_REGISTER FileOperationResult;	
	GEV_REGISTER FileSize;

} DALSA_GENICAM_GIGE_REGS;


//=================================================================================================
// GEV_REGISTER API : Camera register access. (Standard features implemented as simple static register structures)
// (Extensible / camera-device specific).
//
// Obtaining camera register structures.

GEV_STATUS GevGetCameraRegisters( GEV_CAMERA_HANDLE handle, DALSA_GENICAM_GIGE_REGS *camera_registers, int size);
GEV_STATUS GevSetCameraRegInfo( GEV_CAMERA_HANDLE handle, cameraType type, BOOL fSupportedDalsaCamera, 
							DALSA_GENICAM_GIGE_REGS *camera_registers, int size);
GEV_STATUS GevInitCameraRegisters( GEV_CAMERA_HANDLE handle);

GEV_STATUS GevGetNumberOfRegisters(GEV_CAMERA_HANDLE handle, UINT32 *pNumReg);
GEV_STATUS GevGetRegisterNameByIndex(GEV_CAMERA_HANDLE handle, UINT32 index, int size, char *name);
GEV_STATUS GevGetRegisterByName(GEV_CAMERA_HANDLE handle, char *name, GEV_REGISTER *pReg);
GEV_STATUS GevGetRegisterPtrByName(GEV_CAMERA_HANDLE handle, char *name, GEV_REGISTER **pReg);
GEV_STATUS GevGetRegisterByIndex(GEV_CAMERA_HANDLE handle, UINT32 index, GEV_REGISTER *pReg);
GEV_STATUS GevGetRegisterPtrByIndex(GEV_CAMERA_HANDLE handle, UINT32 index, GEV_REGISTER **pReg);

GEV_STATUS GevReadRegisterByName( GEV_CAMERA_HANDLE handle, char *name, int selector, UINT32 size, void *value);
GEV_STATUS GevWriteRegisterByName( GEV_CAMERA_HANDLE handle, char *name, int selector, UINT32 size, void *value);

GEV_STATUS GevRegisterRead(GEV_CAMERA_HANDLE handle, GEV_REGISTER *pReg, int selector, UINT32 size, void *data);
GEV_STATUS GevRegisterWrite(GEV_CAMERA_HANDLE handle, GEV_REGISTER *pReg, int selector, UINT32 size, void *data);
GEV_STATUS GevRegisterWriteNoWait(GEV_CAMERA_HANDLE handle, GEV_REGISTER *pReg, int selector, UINT32 size, void *data);

GEV_STATUS GevRegisterWriteArray(GEV_CAMERA_HANDLE handle, GEV_REGISTER *pReg, int selector, UINT32 array_offset, UINT32 num_entries, void *data);
GEV_STATUS GevRegisterReadArray(GEV_CAMERA_HANDLE handle, GEV_REGISTER *pReg, int selector, UINT32 array_offset, UINT32 num_entries, void *data);

GEV_STATUS GevRegisterWriteInt(GEV_CAMERA_HANDLE handle, GEV_REGISTER *pReg, int selector, UINT32 value);
GEV_STATUS GevRegisterReadInt(GEV_CAMERA_HANDLE handle, GEV_REGISTER *pReg, int selector, UINT32 *value);
GEV_STATUS GevRegisterWriteFloat(GEV_CAMERA_HANDLE handle, GEV_REGISTER *pReg, int selector, float value);
GEV_STATUS GevRegisterReadFloat(GEV_CAMERA_HANDLE handle, GEV_REGISTER *pReg, int selector, float *value);

#ifdef __cplusplus
}
#endif



#endif
