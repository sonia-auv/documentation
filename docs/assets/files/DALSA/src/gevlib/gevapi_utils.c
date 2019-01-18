/****************************************************************************** 
Copyright (c) 2008-2015, Teledyne DALSA Inc.
All rights reserved.

File : gevapi_utils.c
	Public API / Utility functions for GenApi/GevApi on top of GEV C library.

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
/*! \file gevapi_utils.c
\brief GEV API definitions.

*/

#include "gevapi.h"
#include "gevapi_internal.h"
//#include "gevlib.h"

#define NUM_RETRIES 4

//=============================================================================
// Camera image format helper functions
//
//
//		IsPixelTypeMono
/*! 
	This function returns TRUE if the input raw (GigEVision) image format 
	is monochrome.

	\param [in] pixelType	A GigEVision pixel data format.
	\return TRUE for mono / FALSE otherwise.
	\note Bayer formats are considered monochrome and need conversion to color.
*/
BOOL GevIsPixelTypeMono (UINT32 pixelType)
{
	return ((pixelType &  GEV_PIXFORMAT_ISMONO) != 0) ? TRUE : FALSE;
}

//
//		IsPixelTypeRGB
/*! 
	This function returns TRUE if the input raw (GigEVision) image format 
	is color.

	\param [in] pixelType	A GigEVision pixel data format.
	\return TRUE for color / FALSE otherwise.
	\note Bayer formats are considered monochrome and need conversion to color.
*/
BOOL GevIsPixelTypeRGB (UINT32 pixelType)
{
	return ((pixelType &  GEV_PIXFORMAT_ISCOLOR) != 0) ? TRUE : FALSE;
}

//
//		IsPixelTypeCustom
/*! 
	This function returns TRUE if the input raw (GigEVision) image format 
	is a custom type (vendor specific).

	\param [in] pixelType	A GigEVision pixel data format.
	\return TRUE for custom / FALSE otherwise.
	\note None
*/
BOOL GevIsPixelTypeCustom (UINT32 pixelType)
{
	return ((pixelType &  GEV_PIXFORMAT_ISCUSTOM) != 0) ? TRUE : FALSE;
}

//
//		IsPixelTypePacked
/*! 
	This function returns TRUE if the input raw (GigEVision) image format 
	is packed.

	\param [in] pixelType	A GigEVision pixel data format.
	\return TRUE for packed data / FALSE otherwise.
	\note None.
*/
BOOL GevIsPixelTypePacked (UINT32 pixelType)
{
	BOOL packed;
	switch (pixelType)
	{
		case fmtYUV411packed:
		case fmtYUV422packed:
		case fmtYUV444packed:
		case fmtMono10Packed:
		case fmtRGB10Packed:
		case fmtBGR10Packed:
		case fmtRGB10V1Packed:
		case fmtRGB10V2Packed:
		case fmtMono12Packed:
		case fmtRGB12Packed:
		case fmtBGR12Packed:
			packed = TRUE;
			break;
		default:
			packed = FALSE;
			break;
	}
	return packed;
}

//
//		GetPixelSizeInBytes
/*! 
	This function returns the number of bytes taken up by a single pixel 
	for the input raw (GigEVision) image format. 

	\param [in] pixelType	A GigEVision pixel data format.
	\return Pixel size (in bytes).
	\note None
*/
UINT32 GevGetPixelSizeInBytes (UINT32 pixelType)
{
	return (UINT32)GetPixelSizeInBytes(pixelType);
}

//
//		GetPixelDepthInBits
/*! 
	This function returns the number of bits taken up by a single color
	channel in a pixel for the input raw (GigEVision) image format. 
	(It is intended for simplifying display and LUT functions).

	\param [in] pixelType	A GigEVision pixel data format.
	\return Pixel depth (in bits).
	\note Note : YUV composite color pixel formats need to be converted to an RGB equivalent.
                They will be treated as 8 bit monochrome since each of the Y/U/V 
                components are packed as separate 8 bit values.
*/
UINT32 GevGetPixelDepthInBits (UINT32 pixelType)
{
	UINT32 pixelDepth = 8;

	switch (pixelType)
	{
		case fmtYUV411packed:
			pixelDepth = 4;
			break;
		case fmtYUV422packed:
			pixelDepth = 5;
			break;
		case fmtYUV444packed:
			pixelDepth = 8;
			break;
		case fmtMono10:
		case fmtMono10Packed:
		case fMtBayerGR10:
		case fMtBayerRG10:
		case fMtBayerGB10:
		case fMtBayerBG10:
		case fmtRGB10Packed:
		case fmtBGR10Packed:
		case fmtRGB10V1Packed:
		case fmtRGB10V2Packed:
		case fmtRGB10Planar:
			pixelDepth = 10;
			break;
		case fmtMono12:
		case fmtMono12Packed:
		case fMtBayerGR12:
		case fMtBayerRG12:
		case fMtBayerGB12:
		case fMtBayerBG12:
		case fmtRGB12Packed:
		case fmtBGR12Packed:
		case fmtRGB12Planar:
			pixelDepth = 12;
			break;
		case fmtMono14:
			pixelDepth = 14;
			break;
		case fmtMono16:
		case fmtRGB16Planar:
			pixelDepth = 16;
			break;
		default:
			break;
	}
	return pixelDepth;
}

//
//		GetRGBPixelOrder
/*! 
	This function returns the order in which the Red/Green/Blue color channels
	are present in a pixel. 
	(It is intended for simplifying display and LUT functions).

	\param [in] pixelType	A GigEVision pixel data format.
	\return Pixel depth (in bits).
	\note Note : YUV composite color pixel formats need to be converted to an RGB equivalent.
                They will be treated as 8 bit monochrome since each of the Y/U/V 
                components are packed as separate 8 bit values.
                Bayer formats are also considered to be monochrome.
*/
UINT32 GevGetRGBPixelOrder( UINT32 pixelType)
{
	UINT32 pixelOrder = GEV_PIXEL_ORDER_NONE;
	if ( GevIsPixelTypeRGB(pixelType) )
	{
		// RGB pixel format - determine the RGB channel order.
		// (Note : Bayer formats are all monochrome).
		switch (pixelType)
		{
			case fmtBGR8Packed:
			case fmtBGRA8Packed:
			case fmtBGR10Packed:
			case fmtBGR12Packed:
				pixelOrder = GEV_PIXEL_ORDER_BGR;
				break;
			case fmtRGB10V1Packed:
				pixelOrder = GEV_PIXEL_ORDER_RGB10V1;
				break;
			case fmtRGB10V2Packed:
				pixelOrder = GEV_PIXEL_ORDER_RGB10V2;
				break;
			default:
				pixelOrder = GEV_PIXEL_ORDER_RGB;
				break;
		}
	}
	return pixelOrder;
}

//
//		Get Format String
/*! 
	This function obtains a pointer to a useful string to describe the input format.
	(It is a helper for GUI applications)
	The string is static so should not be freed or changed.

	\param [in] format		image format setting (returned from a camera).
	\return Pointer to an internal static string.
*/

static const char *_mono_format_strings[] = {	"Mono8", "Mono10","Mono12", "Mono14", "Mono16", 
													"Mono8","Mono10Packed",  "Mono12Packed", "Mono14Packed", "Mono16" };

static const char *_color_format_strings[] = {	"Bayer8", "Bayer10", "Bayer12", "Bayer16",
														"RGB8", "RGB10", "RGB12", "RGB16",
														"BGR8", "BGR10", "BGR12", "BGR16",
														"RGBPlanar8", "RGB10Planar", "RGB12Planar", "RGB16Planar",
														"RGBA8", "BGRA8", "Custom", "Custom",
														"YUV411", "YUV421", "YUV422", "Custom" };

static const char *_custom_format_strings[] = {"Custom", "RGB10V1Packed", "RGB10V2Packed"};



const char *GevGetFormatString( UINT32 format)
{
	UINT32 depth;
	UINT32 order;
	int offset = 0;
	UINT32 type = 0;

	if ( GevTranslateRawPixelFormat( format, &type, &depth, &order) == 0)
	{
		switch( type )
		{
			case GEV_PIXEL_FORMAT_MONO:
				offset = ((depth - 8)/2);
				return _mono_format_strings[offset];
				break;
			case GEV_PIXEL_FORMAT_MONO_PACKED:
				offset = 5 + ((depth - 8)/2);
				return _mono_format_strings[offset];
				break;
			case GEV_PIXEL_FORMAT_RGB:
				if (order == GEV_PIXEL_ORDER_RGB10V1)
				{
					return _custom_format_strings[1];
				}
				else if (order == GEV_PIXEL_ORDER_RGB10V2)
				{
					return _custom_format_strings[2];
				}
				else
				{
					offset = 20 + (order-1);
					return _color_format_strings[offset];
				}
				break;
			case GEV_PIXEL_FORMAT_RGB_PACKED:
				order = 4*(order-1) + ((depth - 8)/2);
				return _color_format_strings[offset];
				break;
			case GEV_PIXEL_FORMAT_BAYER:
				offset = ((depth - 8) / 2);
				return _color_format_strings[offset];
				break;

			case GEV_PIXEL_FORMAT_YUV:
				offset = 20 + (depth/12);
				return _color_format_strings[offset];
				break;
			case GEV_PIXEL_FORMAT_RGB_PLANAR:
				offset = 16 + ((depth - 8) / 2);
				return _color_format_strings[offset];
				break;
			default:
				break;
		}
	}
	return _custom_format_strings[0];
}


//! Initialize raw streaming transfer 
/*! 
	This function initializes a streaming transfer to the list of buffers indicated.
	Control over the buffer cycling mode is provided. The data transferred is the
	raw data from the camera. 
	
	Note : Certain packed data formats are un-displayable. 
		(i.e. fmtMono10Packed / fmtMono12Packed)

	\param [in] handle		Handle to the camera.
	\param [in] mode			Buffer cycling mode (Either Asynchronous or SynchronousNextEmpty).
	\param [in] numBuffers	Number of buffers addresses in array.
	\param [in] bufAddress	Array of buffer addresses (already allocated).
	\return Gev error code or CORHW error code..
	\note Errors include attempting to initialize the transfer on a 
	\note	connection that is not set up for streaming.
*/
GEV_STATUS GevInitRawImageTransfer( GEV_CAMERA_HANDLE handle, GevBufferCyclingMode mode, 
													UINT32 numBuffers, UINT8 **bufAddress)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	if (handle != NULL)
	{
		UINT32 height, width, x, y, format;
	
		// Get the current image size settings from the camera.
		// (Note : the previously allocated images must be at least the same size as the current camera
		//         image settings).
	
		status = GevGetImageParameters( handle, &width, &height, &x, &y, &format);
		if (status == GEVLIB_OK)
		{
			UINT32 depth = GetPixelSizeInBytes(format);
	
			status = Gev_Stream_InitTransfer( handle, height, width, depth, format, mode, numBuffers, bufAddress); 
	
			// Make sure the transfer settings are consistent.
			if (status == GEVLIB_OK)
			{
				UINT32 value = 1;
				// Mainly for DALSA cameras (DFNC)
				status = GevWriteRegisterByName( handle, (char *)"DeviceRegistersCheck", 0, sizeof(UINT32), &value);
				if (status = GEVLIB_OK)
				{
					int timeout = NUM_RETRIES;
					value = 0;
					while ((value == 0) && (timeout-- > 0))
					{
						GevReadRegisterByName(handle, (char *)"DeviceRegistersValid", 0, sizeof(UINT32), &value);
					}
				}
			}
		}
	}
	return status;
}

//! Initialize streaming transfer.
/*! 
	This function initializes a streaming transfer to the list of buffers indicated.
	Control over the buffer cycling mode is provided. Packed input data from
	the camera will be unpacked to a usable/displayable data type.
	
	Note :  The currently supported unpacking conversions are :
			Source Type		 --->	Converted Type
			fmtMono10Packed --->	fmtMono10
			fmtMono12Packed --->	fmtMono12
			
	\param [in] handle		Handle to the camera.
	\param [in] mode			Buffer cycling mode (Either Asynchronous or SynchronousNextEmpty).
	\param [in] numBuffers	Number of buffers addresses in array.
	\param [in] bufAddress	Array of buffer addresses (already allocated).
	\return Gev error code or CORHW error code..
	\note Errors include attempting to initialize the transfer on a 
	\note	connection that is not set up for streaming.
*/
GEV_STATUS GevInitImageTransfer( GEV_CAMERA_HANDLE handle, GevBufferCyclingMode mode, 
													UINT32 numBuffers, UINT8 **bufAddress)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	if (handle != NULL)
	{
		UINT32 height, width, x, y, camera_format, image_format;
	
		// Get the current image size settings from the camera.
		// (Note : the previously allocated images must be at least the same size as the current camera
		//         image settings).
	
		status = GevGetImageParameters( handle, &width, &height, &x, &y, &camera_format);
		if (status == GEVLIB_OK)
		{
			UINT32 depth = GetPixelSizeInBytes(camera_format);
			
			// Get the corresponding unpacked data format from the packed one.
			switch(camera_format)
			{
				case fmtMono10Packed:
					image_format = fmtMono10;
					break;
			case fmtMono12Packed:
					image_format = fmtMono12;
					break;
				default:
					image_format = camera_format;
					break;
			}

			status = Gev_Stream_InitTransfer( handle, height, width, depth, image_format, mode, numBuffers, bufAddress); 

			// Make sure the transfer settings are consistent.
			if (status == GEVLIB_OK)
			{
				UINT32 value = 1;
				// Mainly for DALSA cameras (DFNC)
				status = GevWriteRegisterByName( handle, (char *)"DeviceRegistersCheck", 0, sizeof(UINT32), &value);
				if (status = GEVLIB_OK)
				{
					int timeout = NUM_RETRIES;
					value = 0;
					while ((value == 0) && (timeout-- > 0))
					{
						GevReadRegisterByName(handle, (char *)"DeviceRegistersValid", 0, sizeof(UINT32), &value);
					}
				}
			}
		}	
	}
	return status;
}

//! Initialize an asynchronous streaming transfer (legacy)
/*! 
	This function initializes an asynchronous streaming transfer to the list 
	of buffers indicated. This is a legacy function call replaced by the
	newer GevInitImageTransfer call which supports setting the cycling mode
	for the transfer.
	\param [in] handle		Handle to the camera.
	\param [in] numBuffers	Number of buffers addresses in array.
	\param [in] bufAddress	Array of buffer addresses (already allocated).
	\return Gev error code or CORHW error code..
	\note Errors include attempting to initialize the transfer on a 
	\note	connection that is not set up for streaming.
*/
GEV_STATUS GevInitializeImageTransfer( GEV_CAMERA_HANDLE handle, UINT32 numBuffers, UINT8 **bufAddress)
{
	return GevInitImageTransfer( handle, Asynchronous, numBuffers, bufAddress);
}


//! Free a streaming transfer
/*! 
	This function initializes a streaming transfer to the list of buffers indicated.
	\param [in] handle		Handle to the camera.
	\return Gev error code or CORHW error code..
	\note Errors include attempting to initialize the transfer on a 
	\note	connection that is not set up for streaming.
*/
GEV_STATUS GevFreeImageTransfer( GEV_CAMERA_HANDLE handle)
{
	return Gev_Stream_FreeTransfer( handle);
}


//! Start transfer
/*! 
	This function starts the streaming transfer.
	\param [in] handle		Handle to the camera.
	\param [in] numFrames   Number of frames to be acquired (-1 for continuous).
	\return Gev error code or CORHW error code..
	\note Errors include attempting to initialize the transfer on a 
	\note	connection that is not set up for streaming.
*/
GEV_STATUS GevStartImageTransfer( GEV_CAMERA_HANDLE handle, UINT32 numFrames)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;
	int timeout = 10;

	if (handle != NULL)
	{
		if (Gev_IsSupportedCamera(handle))
		{
			status = Gev_Stream_StartTransfer( handle, numFrames);
			if (status == GEV_STATUS_BUSY)
			{
				do
				{
					Sleep(100);
					status = Gev_Stream_StartTransfer( handle, numFrames);
				} while ((status == GEV_STATUS_BUSY) && (timeout-- > 0));
			}
		}
		else
		{
			// Add support for generic feature access for all devices.
			status = GEVLIB_ERROR_NOT_SUPPORTED;
		}
	}
	return status;
}

//! Stop transfer
/*! 
	This function stops the streaming transfer.
	\param [in] handle		Handle to the camera.
	\return Gev error code or CORHW error code..
	\note Errors include attempting to initialize the transfer on a 
	\note	connection that is not set up for streaming.
*/
GEV_STATUS GevStopImageTransfer( GEV_CAMERA_HANDLE handle)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	if (handle != NULL)
	{
		if (Gev_IsSupportedCamera(handle))
		{
			status = Gev_Stream_StopTransfer( handle);
		}
		else
		{
			// Add support for generic feature access for all devices.
			status = GEVLIB_ERROR_NOT_SUPPORTED;
		}
	}
	return status;
}

//! Abort transfer
/*! 
	This function stops the streaming transfer.
	\param [in] handle		Handle to the camera.
	\param [in] numFrames   Number of frames to be acquired (-1 for continuous).
	\return Gev error code or CORHW error code..
	\note Errors include attempting to initialize the transfer on a 
	\note	connection that is not set up for streaming.
*/
GEV_STATUS GevAbortImageTransfer( GEV_CAMERA_HANDLE handle)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	if (handle != NULL)
	{
		if (Gev_IsSupportedCamera(handle))
		{
			status = Gev_Stream_AbortTransfer( handle );
		}
		else
		{
			// Add support for generic feature access for all devices.
			status = GEVLIB_ERROR_NOT_SUPPORTED;
		}
	}
	return status;
}



//! Query Image Transfer status
/*! 
	This function returns information on the buffer status in the transfer.
	The total number of buffers, the number of filled buffers available, the number
	of empty buffers available, and the number of buffers that have been "trashed"
	is returned. In addition, the transfers cycle mode is returned.
	\param [in] handle			Handle to the camera.
	\param [in] pTotalBuffers  Pointer to receive the total number of buffers in the transfer list.
	\param [in] pNumUsed			Pointer to receive the number of filled buffers ready to be obtained from the transfer list.
	\param [in] pNumFree  		Pointer to receive the number of empty (free) buffers that are available to be filled.
	\param [in] pNumTrashed		Pointer to receive the total number of buffers that were "trashed" (dropped when there are no more empty buffers).
	\param [in] pMode			   Pointer to receive the cycling mode (Asynchronous = 0, SynchronousNextEmpty = 1).
	\return GEVLIB_ERROR_INVALID_HANDLE is the handle is invalid.
	\note If any of the pointers to contain the returned values are NULL, no information is returned for that value.
*/
GEV_STATUS GevQueryImageTransferStatus( GEV_CAMERA_HANDLE handle, PUINT32 pTotalBuffers, PUINT32 pNumUsed, PUINT32 pNumFree, PUINT32 pNumTrashed, GevBufferCyclingMode *pMode)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	if (handle != NULL)
	{
		// Find out the queue information from the Empty/Free queue.
		GEV_BUFFER_LIST *pBufList= GevGetBufferListFromHandle(handle);
			
		if (pBufList != NULL)
		{
			UINT32 length = 0;
			UINT32 length2 = 0;
			UINT32 numUsed = 0;
			UINT32 numFree = 0;
			DQueueQuery( pBufList->pFullBuffers, &length, &numUsed, NULL);
			DQueueQuery( pBufList->pEmptyBuffers, &length2, &numFree, NULL);

			if ( pNumUsed != NULL)
			{
				*pNumUsed = numUsed;
			}
			if ( pNumFree != NULL)
			{
				*pNumFree = numFree;
			}
			if (pTotalBuffers != NULL)
			{
				*pTotalBuffers = length;
			}
			if (pNumTrashed != NULL)
			{
				*pNumTrashed = pBufList->trashCount;			
			}
			if (pMode != NULL)
			{
				*pMode = pBufList->cyclingMode;			
			}
			status = GEVLIB_OK;
		}

	}
	return status;
}


//! Get image object - wait for completion
/*! 
	This function waits for the next image object to be acquired and returns its pointer.
	If no buffer has been acquired before the timeout period expires, a NULL pointer is returned.
	\param [in] handle				Handle to the camera.
	\param [in] image_object_ptr  Pointer to receive the image object pointer.
	\param [in] pTimeout			   Pointer to struct timeval for period to wait for a frame to be acquired.
	\return Gev error code or CORHW error code..
	\note Errors include attempting to initialize the transfer on a 
	\note	connection that is not set up for streaming.
*/
GEV_STATUS GevGetNextImage( GEV_CAMERA_HANDLE handle, GEV_BUFFER_OBJECT **image_object_ptr, struct timeval *pTimeout)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	if (handle != NULL)
	{
		status = GEVLIB_ERROR_NULL_PTR;
		if (image_object_ptr != NULL)
		{
			GEV_BUFFER_LIST *pBufList= GevGetBufferListFromHandle(handle);
			
			if (pBufList != NULL)
			{
				*image_object_ptr = (GEV_BUFFER_OBJECT *)DQueuePendEx( pBufList->pFullBuffers, pTimeout);
				if (*image_object_ptr == NULL)
				{
					status = GEVLIB_ERROR_TIME_OUT;
				}
				else
				{
					status = GEVLIB_OK;
				}
			}
		}
	}
	return status;
}


//! Get image buffer
/*! 
	This function returns the pointer to the most recently acquired image buffer data.
	If no buffer has been acquired, a NULL pointer is returned with a timeout condition.
	\param [in] handle				Handle to the camera.
	\param [in] image_buffer_ptr  Pointer to receive the image buffer data pointer.
	\return Gev error code or CORHW error code..
	\note Errors include attempting to initialize the transfer on a 
	\note	connection that is not set up for streaming.
*/
GEV_STATUS GevGetImageBuffer( GEV_CAMERA_HANDLE handle, void **image_buffer_ptr)
{
	GEV_STATUS status = GEVLIB_ERROR_NULL_PTR;

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	if (image_buffer_ptr != NULL)
	{
		GEV_BUFFER_OBJECT *image_object_ptr = NULL;
		status = GevGetNextImage( handle, &image_object_ptr, &tv);
		if (status == GEVLIB_OK)
		{
			if (image_object_ptr != NULL)
			{
				image_buffer_ptr = (void **)image_object_ptr->address;
			}
			else
			{
				image_buffer_ptr = NULL;
			}
		}
	}
	return status;
}

//! Get image object
/*! 
	This function returns the pointer to the most recently acquired image object
	without waiting. If there is no image object waiting, a NULL pointer is returned.
	\param [in] handle				Handle to the camera.
	\param [in] image_object_ptr  Pointer to receive the image object pointer.
	\return Gev error code or CORHW error code..
	\note Errors include attempting to initialize the transfer on a 
	\note	connection that is not set up for streaming.
*/
GEV_STATUS GevGetImage( GEV_CAMERA_HANDLE handle, GEV_BUFFER_OBJECT **image_object_ptr)
{
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	return GevGetNextImage( handle, image_object_ptr, &tv);
}

//! Get next image buffer
/*! 
	This function waits for the next image to be acquired and returns the pointer to the image data.
	If no buffer has been acquired before the timeout period expires, a NULL pointer is returned.
	\param [in] handle				Handle to the camera.
	\param [in] image_buffer_ptr  Pointer to receive the image buffer data pointer.
	\param [in] timeout			   Timeout period (in msec) to wait for the next frame.
	\return Gev error code or CORHW error code..
	\note Errors include attempting to initialize the transfer on a 
	\note	connection that is not set up for streaming.
*/
GEV_STATUS GevWaitForNextImageBuffer( GEV_CAMERA_HANDLE handle, void **image_buffer_ptr, UINT32 timeout)
{
	GEV_STATUS status = GEVLIB_ERROR_NULL_PTR;
	GEV_BUFFER_OBJECT *image_object_ptr = NULL;
	struct timeval tv;

	tv.tv_sec = timeout/1000;
	tv.tv_usec = (timeout - (tv.tv_sec * 1000)) * 1000;
	
	if (image_buffer_ptr != NULL)
	{
		if (timeout == (UINT32)INFINITE)
		{
			status = GevGetNextImage(handle, &image_object_ptr, NULL);
		}
		else
		{
			status = GevGetNextImage(handle, &image_object_ptr, &tv);
		}

		if (status == GEVLIB_OK)
		{
			*image_buffer_ptr = (void *)image_object_ptr->address;
		}
	}
	return status;
}


//! Get next image object
/*! 
	This function waits for the next image object to be acquired and returns its pointer.
	If no buffer has been acquired before the timeout period expires, a NULL pointer is returned.
	\param [in] handle				Handle to the camera.
	\param [in] image_object_ptr  Pointer to receive the image object pointer.
	\param [in] timeout			   Timeout perdio (in msec) to wait for the next frame.
	\return Gev error code or CORHW error code..
	\note Errors include attempting to initialize the transfer on a 
	\note	connection that is not set up for streaming.
*/
GEV_STATUS GevWaitForNextImage( GEV_CAMERA_HANDLE handle, GEV_BUFFER_OBJECT **image_object_ptr, UINT32 timeout)
{
	struct timeval tv;
	tv.tv_sec = timeout/1000;
	tv.tv_usec = (timeout - (tv.tv_sec * 1000)) * 1000;
	
	if (timeout == (UINT32)INFINITE)
	{
		return GevGetNextImage(handle, image_object_ptr, NULL);
	}
	else
	{
		return GevGetNextImage(handle, image_object_ptr, &tv);
	}
}



//! Release Image 
/*! 
	This function releases an image object back to the acquisition process so it can be re-used.
	\param [in] handle    			Camera handle for streaming acquisition process.
	\param [in] image_object_ptr 	Pointer to the image object to be returned for re-use.
	\return GEV_STATUS information. 
	\note An eror is returned if the transfer is not set up, the image object is not
			a member of the original set of images used by the transfer.
*/
GEV_STATUS GevReleaseImage( GEV_CAMERA_HANDLE handle, GEV_BUFFER_OBJECT *image_object_ptr)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	// Search the buffer list
	if (handle != NULL)
	{
		GEV_BUFFER_OBJECT *img = NULL;
		GEV_BUFFER_LIST *pBufList= GevGetBufferListFromHandle(handle);
	
		if ((pBufList != NULL) && (pBufList->cyclingMode == SynchronousNextEmpty))
		{
			// Verify that the image object pointer is part of the buffer list.
			UINT32 i = 0;
			status =  GEVLIB_ERROR_PARAMETER_INVALID;
			for (i = 0; i < pBufList->numBuffer; i++)
			{
				img = (GEV_BUFFER_OBJECT *)&pBufList->buffer[i];
				if (img == image_object_ptr)
				{
					status = DQueuePost( pBufList->pEmptyBuffers, image_object_ptr);
					break;
				}
			}
		}
		else
		{
			status = GEVLIB_ERROR_ARG_INVALID;
		}
	}
	return status;

}


//! Release Image Buffer
/*! 
	This function releases an image object back to the acquisition process so it can be re-used. 
	The image object is identified by the pointer to its image data buffer.
	\param [in] handle    			Camera handle for streaming acquisition process.
	\param [in] image_buffer_ptr 	Pointer to the image data buffer for tyh eimage object to be returned for re-use.
	\return GEV_STATUS information. 
	\note An eror is returned if the transfer is not set up, the image object is not
			a member of the original set of images used by the transfer, or the image data buffer
			is not associated with one of the image objects in the transfer.
*/
GEV_STATUS GevReleaseImageBuffer( GEV_CAMERA_HANDLE handle, void *image_buffer_ptr)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	// Search the buffer list
	if (handle != NULL)
	{
		GEV_BUFFER_OBJECT *img = NULL;
		GEV_BUFFER_LIST *pBufList= GevGetBufferListFromHandle(handle);
	
		if ((pBufList != NULL) && (pBufList->cyclingMode == SynchronousNextEmpty))
		{
			UINT32 i = 0;
			status =  GEVLIB_ERROR_PARAMETER_INVALID;
			for (i = 0; i < pBufList->numBuffer; i++)
			{
				img = (GEV_BUFFER_OBJECT *)&pBufList->buffer[i];
				if (img->address == image_buffer_ptr)
				{
					status = DQueuePost( pBufList->pEmptyBuffers, img);
					break;
				}
			}
		}
		else
		{
			status = GEVLIB_ERROR_ARG_INVALID;
		}
	}
	return status;
}

//=============================================================================
//
// Camera Asynchronous Event (EVENTCMD and EVENTDATACMD) handling.
//
//
//! Register Event Handler
/*! 
	This function registers an event handler/callback function for a camera level event.
	\param [in] handle	Camera handle.
	\param [in] EventID	Event ID for event to be handled.
	\param [in] function	Function to be called when event occurs.
	\param [in] context	Context to be passed to event handler function.	
	\return GEV status code
	\note None
*/
GEV_STATUS GevRegisterEventCallback(GEV_CAMERA_HANDLE handle,  UINT32 EventID, GEVEVENT_CBFUNCTION func, void *context)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	if (handle != NULL)
	{
		status = Gev_Event_RegisterEventHandler( handle, EventID, func, context);
	}
	return status;
}

//! Register Application Event
/*! 
	This function registers an application event to be signalled when 
	a particular camera level event occurs.
	\param [in] handle	Camera handle.
	\param [in] EventID	Event ID for event to be signalled.
	\param [in] appEvent	Host/App level event to be signalled when camera event occurs.
	\return GEV status code
	\note None
*/
GEV_STATUS GevRegisterApplicationEvent(GEV_CAMERA_HANDLE handle,  UINT32 EventID, _EVENT appEvent)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	if (handle != NULL)
	{
		status = Gev_Event_RegisterAppEvent( handle, EventID, appEvent);
	}
	return status;
}

//! Unregister Event
/*! 
	This function removes all event handling for a particular camera level event ID.
	\param [in] handle	Camera handle.
	\param [in] EventID	Event ID for event unregistered.
	\return GEV status code
	\note None
*/
GEV_STATUS GevUnregisterEvent(GEV_CAMERA_HANDLE handle,  UINT32 EventID)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	if (handle != NULL)
	{
		status = Gev_Event_UnregisterEvent( handle, EventID);
	}
	return status;
}


//=============================================================================
//
// Camera register access. (Standard features are implemented as simple registers)
//
// 
//
//! Get camera register structure.
/*! 
	This function obtains the camera register structure for the camera accessed through 'handle'.
	\param [in] handle				Camera handle.
	\param [in] camera_registers	Ptr to camera register structure.
	\param [in] size					Size (bytes) of the structure pointed to by 'camera_registers'.
	\return GEV status code
	\note None
*/
GEV_STATUS GevGetCameraRegisters( GEV_CAMERA_HANDLE handle, DALSA_GENICAM_GIGE_REGS *camera_registers, int size)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	if (handle != NULL)
	{
		DALSA_GENICAM_GIGE_REGS *pInternalRegs = Gev_GetGenICamRegistersFromHandle(handle);

		if (pInternalRegs != NULL)
		{
			int copy_size = (size < (int)sizeof(DALSA_GENICAM_GIGE_REGS)) ? size : sizeof(DALSA_GENICAM_GIGE_REGS);

			if (camera_registers != NULL)
			{
				memcpy(camera_registers, pInternalRegs, copy_size);
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


//
//! Set camera register structure.
/*! 
	This function sets the camera register structure for the camera accessed through 'handle' as
	well as the camera type and if it is a supported DALSA camera.
	\param [in] handle						Camera handle.
	\param [in] type							Type of camera (a see "cameraType" definition).
	\param [in] fSupportedDalsaCamera	T/F if camera is defined as supported.
	\param [in] camera_registers			Ptr to camera register structure.
	\param [in] size							Size (bytes) of the structure pointed to by 'camera_registers'.
	\return GEV status code
	\note None
*/
GEV_STATUS GevSetCameraRegInfo( GEV_CAMERA_HANDLE handle, cameraType type, BOOL fSupportedDalsaCamera, 
							DALSA_GENICAM_GIGE_REGS *camera_registers, int size)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	if (handle != NULL)
	{
		DALSA_GENICAM_GIGE_REGS *pInternalRegs = Gev_GetGenICamRegistersFromHandle(handle);

		if (pInternalRegs != NULL)
		{
			int copy_size = (size < (int)sizeof(DALSA_GENICAM_GIGE_REGS)) ? size : sizeof(DALSA_GENICAM_GIGE_REGS);

			if (camera_registers != NULL)
			{
				memcpy( pInternalRegs, camera_registers, copy_size);
				status = Gev_SetCameraType( handle, type, fSupportedDalsaCamera);
			}
			else
			{
				status = GEVLIB_ERROR_NULL_PTR;
			}
		}
	}
	return status;
}

//=============================================================================
//
// Camera "feature" access using static register definitions. 
// Extensible via GEV_REGISTER definitions - assumes camera register 
// addresses are constant and do not vary based on camera state. This is
// not always a valid assumption.
// 
static GEV_STATUS GevRegisterCheckParameters( BOOL writeAccess, GEV_REGISTER *pReg, int selector, UINT32 size, void *value)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	if ((pReg != NULL) && (value != NULL))
	{
		status = GEVLIB_OK;
		if (pReg->available)
		{
			// Check selector range.
			if ( (selector >= (int)pReg->minSelector) && (selector <= (int)pReg->maxSelector))
			{
				UINT32 offset = pReg->address + (selector - pReg->minSelector) * pReg->regStride;

				if ( (pReg->type == dataArea) && (size + (offset - pReg->address)) > pReg->regSize)
				{
					status = GEVLIB_ERROR_SOFTWARE;	// Something wrong with the register data.(address out of range).
				}
				else
				{
					if (writeAccess)
					{
						// Want to write to register - make sure it is not RO (read-only).
						if (pReg->accessMode == RO)
						{
							status = GEVLIB_ERROR_NOT_IMPLEMENTED; // Write access to RO is not implemented.
						}
					}
					else
					{
						// Want read access to register - make sure it is not WO (write-only).
						if (pReg->accessMode == WO)
						{
							status = GEVLIB_ERROR_NOT_IMPLEMENTED; // Read access to WO is not implemented.
						}
					}
				}
			}
			else
			{
				status = GEVLIB_ERROR_PARAMETER_INVALID;  // Selector out of range.
			}
		}
		else
		{
			status = GEVLIB_ERROR_RESOURCE_NOT_ENABLED;
		}
	}
	return status;
}

//! Get a Camera register structure
/*!
	This function is used to find and return a GEV_REGISTER structure from the 
	camera using the name of the GEV_REGISTER.
	\param [in] handle	Camera handle. 
	\param [in] name		Ptr to the name of a GEV_REGISTER structure stored in the handle.
	\param [in] pReg		Ptr to storage for the GEV_REGISTER structure describing the camera register/feature to be accessed.
	\return GEV status code
	\note None
*/

GEV_STATUS GevGetRegisterByName(GEV_CAMERA_HANDLE handle, char *name, GEV_REGISTER *pReg)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	if (handle != NULL)
	{
		status = GEVLIB_ERROR_NULL_PTR;
		if ( (name != NULL) && (pReg != NULL) )
		{
			DALSA_GENICAM_GIGE_REGS *pStandardRegs = Gev_GetGenICamRegistersFromHandle(handle);
			GEV_REGISTER *regList = NULL;
			int num = 0;
			int i = 0;
			
			// Look up the name in the table of registers stored in the handle.
			// Check the non-standard registers first. 
			status = GevGetCameraRegList( handle, &num, &regList);
			if ((status == GEVLIB_OK) && (num > 0) )
			{
				status = GEVLIB_ERROR_RESOURCE_LACK;
				for (i = 0; i < num; i++)
				{
					if ( !strncmp( name, regList[i].featureName, FEATURE_NAME_MAX_SIZE) )
					{
						// Found it. 
						memcpy( pReg, &regList[i], sizeof(GEV_REGISTER));
						status = GEVLIB_OK;
						return status;
					}
				}
			}
			
			// Not found. Check the standard registers.
			num = sizeof(DALSA_GENICAM_GIGE_REGS) / sizeof(GEV_REGISTER);
			regList = (GEV_REGISTER *) pStandardRegs;
			if (regList != NULL)
			{
				status = GEVLIB_ERROR_RESOURCE_LACK;
				for (i = 0; i < num; i++)
				{
					if ( !strncmp( name, regList[i].featureName, FEATURE_NAME_MAX_SIZE) )
					{
						// Found it. 
						memcpy( pReg, &regList[i], sizeof(GEV_REGISTER));
						status = GEVLIB_OK;
						return status;
					}
				}
			}			
		}
	}
	return status;
}

//! Get a pointer to a Camera register structure
/*!
	This function is used to find and return the pointer to a GEV_REGISTER 
	structure from the camera using the name of the GEV_REGISTER.
	\param [in] handle	Camera handle. 
	\param [in] name		Ptr to the name of a GEV_REGISTER structure stored in the handle.
	\param [in] pReg		Ptr to contain the ptr to the GEV_REGISTER structure sought. (NULL if not found).
	\return GEV status code
	\note None
*/

GEV_STATUS GevGetRegisterPtrByName(GEV_CAMERA_HANDLE handle, char *name, GEV_REGISTER **pReg)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	if (handle != NULL)
	{
		status = GEVLIB_ERROR_NULL_PTR;
		if ( (name != NULL) && (pReg != NULL) )
		{
			DALSA_GENICAM_GIGE_REGS *pStandardRegs = Gev_GetGenICamRegistersFromHandle(handle);
			GEV_REGISTER *regList = NULL;
			int num = 0;
			int i = 0;
			
			// Assume failure.
			*pReg = NULL;
			
			// Look up the name in the table of registers stored in the handle.
			// Check the non-standard registers first. 
			status = GevGetCameraRegList( handle, &num, &regList);
			if ((status == GEVLIB_OK) && (num > 0) )
			{
				status = GEVLIB_ERROR_RESOURCE_LACK;
				for (i = 0; i < num; i++)
				{
					if ( !strncmp( name, regList[i].featureName, FEATURE_NAME_MAX_SIZE) )
					{
						// Found it. 
						*pReg = &regList[i];
						status = GEVLIB_OK;
						return status;
					}
				}
			}
			
			// Not found. Check the standard registers.
			num = sizeof(DALSA_GENICAM_GIGE_REGS) / sizeof(GEV_REGISTER);
			regList = (GEV_REGISTER *) pStandardRegs;
			if (regList != NULL)
			{
				status = GEVLIB_ERROR_RESOURCE_LACK;
				for (i = 0; i < num; i++)
				{
					if ( !strncmp( name, regList[i].featureName, FEATURE_NAME_MAX_SIZE) )
					{
						// Found it. 
						*pReg = &regList[i];
						status = GEVLIB_OK;
						return status;
					}
				}
			}			
		}
	}
	return status;
}

//! Get the number of Camera register entries configured for the camera.
/*!
	This function returns the number of value GEV_REGISTER structures defined 
	in the camera handle.
	\param [in] handle	Camera handle. 
	\param [in] num		Ptr to storage to return the number of valid GEV_REGISTER strucutre in.
	\return GEV status code
	\note None
*/

GEV_STATUS GevGetNumberOfRegisters(GEV_CAMERA_HANDLE handle, UINT32 *pNumReg)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	if (handle != NULL)
	{
		status = GEVLIB_ERROR_NULL_PTR;
		if ( (pNumReg != NULL) )
		{
			DALSA_GENICAM_GIGE_REGS *pStandardRegs = Gev_GetGenICamRegistersFromHandle(handle);
			GEV_REGISTER *regList = NULL;
			int num = 0;
			int i = 0;
			
			// Assume none found.
			*pNumReg = 0;
			status = GEVLIB_OK;
			
			// Look up the register in the table of registers stored in the handle.
			// Check the non-standard registers first. 
			status = GevGetCameraRegList( handle, &num, &regList);
			if ((status == GEVLIB_OK) && (num > 0) )
			{
				for (i = 0; i < num; i++)
				{
					if (regList[i].available)
					{
						*pNumReg += 1;
					}
				}
			}
			
			// Not found. Check the standard registers.
			num = sizeof(DALSA_GENICAM_GIGE_REGS) / sizeof(GEV_REGISTER);
			regList = (GEV_REGISTER *) pStandardRegs;
			if (regList != NULL)
			{
				for (i = 0; i < num; i++)
				{
					if (regList[i].available)
					{
						*pNumReg += 1;
					}
				}
			}			
		}
	}
	return status;
}

//! Get the name of a Camera register by index.
/*!
	This function returns the name of a GEV_REGISTER structure defined 
	in the camera handle based on the input index.
	\param [in] handle	Camera handle. 
	\param [in] index		Ptr to storage to return the number of valid GEV_REGISTER strucutre in.
	\param [in] size		Size (in bytes) of the storage for the register name passed in the 'name' argument.
	\param [in] name		Ptr to storage to return the name of the register.
	\return GEV status code
	\note None
*/

GEV_STATUS GevGetRegisterNameByIndex(GEV_CAMERA_HANDLE handle, UINT32 index, int size, char *name)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	if (handle != NULL)
	{
		status = GEVLIB_ERROR_NULL_PTR;
		if ( (name != NULL) )
		{
			DALSA_GENICAM_GIGE_REGS *pStandardRegs = Gev_GetGenICamRegistersFromHandle(handle);
			GEV_REGISTER *regList = NULL;
			int num = 0;
			int i = 0;
			UINT32 found = 0;
			int len = MIN(size, FEATURE_NAME_MAX_SIZE);
			

			// Look up the register in the table of registers stored in the handle.
			// Check the non-standard registers first. 
			status = GevGetCameraRegList( handle, &num, &regList);
			if ((status == GEVLIB_OK) && (num > 0) )
			{
				status = GEVLIB_ERROR_RESOURCE_LACK;
				for (i = 0; i < num; i++)
				{
					if (regList[i].available)
					{
						if (found == index)
						{
								strncpy( name, regList[i].featureName, len);
								status = GEVLIB_OK;
								return status;
						}
						found++;
					}
				}
			}
			
			// Not found. Check the standard registers.
			num = sizeof(DALSA_GENICAM_GIGE_REGS) / sizeof(GEV_REGISTER);
			regList = (GEV_REGISTER *) pStandardRegs;
			if (regList != NULL)
			{
				for (i = 0; i < num; i++)
				{
					if (regList[i].available)
					{
						if (found == index)
						{
							strncpy( name, regList[i].featureName, len);
							status = GEVLIB_OK;
							return status;
						}
						found++;
					}
				}
			}			
		}
	}
	return status;
}

//! Get a Camera register by index.
/*!
	This function is used to find and return a GEV_REGISTER structure from the 
	camera handle using the input index.
	\param [in] handle	Camera handle. 
	\param [in] index		Ptr to storage to return the number of valid GEV_REGISTER strucutre in.
	\param [in] pReg		Ptr to storage for the GEV_REGISTER structure describing the camera register/feature to be accessed.
	\return GEV status code
	\note None
*/
GEV_STATUS GevGetRegisterByIndex(GEV_CAMERA_HANDLE handle, UINT32 index, GEV_REGISTER *pReg)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	if (handle != NULL)
	{
		status = GEVLIB_ERROR_NULL_PTR;
		if ( (pReg != NULL) )
		{
			DALSA_GENICAM_GIGE_REGS *pStandardRegs = Gev_GetGenICamRegistersFromHandle(handle);
			GEV_REGISTER *regList = NULL;
			int num = 0;
			int i = 0;
			UINT32 found = 0;
			
			// Look up the register in the table of registers stored in the handle.
			// Check the non-standard registers first. 
			status = GevGetCameraRegList( handle, &num, &regList);
			if ((status == GEVLIB_OK) && (num > 0) )
			{
				status = GEVLIB_ERROR_RESOURCE_LACK;
				for (i = 0; i < num; i++)
				{
					if (regList[i].available)
					{
						if (found == index)
						{
							// Found it. 
							memcpy( pReg, &regList[i], sizeof(GEV_REGISTER));
							status = GEVLIB_OK;
							return status;
						}
						found++;
					}
				}
			}
			
			// Not found. Check the standard registers.
			num = sizeof(DALSA_GENICAM_GIGE_REGS) / sizeof(GEV_REGISTER);
			regList = (GEV_REGISTER *) pStandardRegs;
			if (regList != NULL)
			{
				for (i = 0; i < num; i++)
				{
					if (regList[i].available)
					{
						if (found == index)
						{
							// Found it. 
							memcpy( pReg, &regList[i], sizeof(GEV_REGISTER));
							status = GEVLIB_OK;
							return status;
						}
						found++;
					}
				}
			}			
		}
	}
	return status;
}

//! Get a pointer to a Camera register by index.
/*!
	This function is used to find and return a pointer to a GEV_REGISTER structure from the 
	camera handle using the input index.
	\param [in] handle	Camera handle. 
	\param [in] index		Ptr to storage to return the number of valid GEV_REGISTER strucutre in.
	\param [in] pReg		Ptr to a ptr for the GEV_REGISTER structure describing the camera register/feature to be accessed.
	\return GEV status code
	\note None
*/

GEV_STATUS GevGetRegisterPtrByIndex(GEV_CAMERA_HANDLE handle, UINT32 index, GEV_REGISTER **pReg)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;

	if (handle != NULL)
	{
		status = GEVLIB_ERROR_NULL_PTR;
		if ( (pReg != NULL) )
		{
			DALSA_GENICAM_GIGE_REGS *pStandardRegs = Gev_GetGenICamRegistersFromHandle(handle);
			GEV_REGISTER *regList = NULL;
			int num = 0;
			int i = 0;
			UINT32 found = 0;
			
			status = GEVLIB_OK;
			*pReg = NULL;

			// Look up the register in the table of registers stored in the handle.
			// Check the non-standard registers first. 
			status = GevGetCameraRegList( handle, &num, &regList);
			if ((status == GEVLIB_OK) && (num > 0) )
			{
				status = GEVLIB_ERROR_RESOURCE_LACK;
				for (i = 0; i < num; i++)
				{
					if (regList[i].available)
					{
						if (found == index)
						{
							// Found it. 
							*pReg = &regList[i];
							status = GEVLIB_OK;
							return status;
						}
						found++;
					}
				}
			}
			
			// Not found. Check the standard registers.
			num = sizeof(DALSA_GENICAM_GIGE_REGS) / sizeof(GEV_REGISTER);
			regList = (GEV_REGISTER *) pStandardRegs;
			if (regList != NULL)
			{
				for (i = 0; i < num; i++)
				{
					if (regList[i].available)
					{
						if (found == index)
						{
							// Found it. 
							*pReg = &regList[i];
							status = GEVLIB_OK;
							return status;
						}
						found++;
					}
				}
			}			
		}
	}
	return status;
}

//! Write a Camera register
/*!
	This function is used to write into a camera register
	\param [in] handle	Camera handle. 
	\param [in] pReg		Ptr to the GEV_REGISTER structure describing the camera register/feature to be accessed.
	\param [in] selector	Index used to select different registers within the set available for this feature.
	\param [in] size		Size of data to be written.
	\param [in] data		Data to be written.
	\param [in] bAsync	True if call is to be asynchronous (i.e. no Ack back from device).
	\return GEV status code
	\note None
*/
static GEV_STATUS _GevRegisterWriteEx(GEV_CAMERA_HANDLE handle, GEV_REGISTER *pReg, int selector, UINT32 size, void *data, BOOL bAsync)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;
	BOOL32 little_endian = FALSE;

	if (handle != NULL)
	{
		status = GevRegisterCheckParameters( TRUE, pReg, selector, size, data);
		if ( status == GEVLIB_OK)
		{
			if ((pReg->type == integerRegLE) || (pReg->type == bitRegLE) || (pReg->type == floatRegLE))
			{
				little_endian = TRUE;
			}
			if (pReg->address == NOREF_ADDR)
			{
				// No reference address in camera - store the value in the register structure.
				switch(pReg->type)
				{
					case floatRegLE:
					case floatReg:
					case floatVal:
						{
							float *value = (float *)data;
							pReg->value.floatValue = *value;
						}
						break;
					case integerRegLE:
					case integerReg:
					case intVal:
					case fixedVal:
						{
							UINT32 *value = (UINT32 *)data;
							pReg->value.intValue = *value & pReg->writeMask;
						}
						break;
					case bitRegLE:
					case bitReg:
						{
							UINT32 *value = (UINT32 *)data;
							pReg->value.bitIndex = *value & pReg->writeMask;
						}
						break;
					default:
							status = GEVLIB_ERROR_SOFTWARE;	// Something wrong with the register data.(NOREF_ADDRESS of wrong type).
						break;
				}
			}
			else
			{
				UINT32 offset = pReg->address + (selector - pReg->minSelector) * pReg->regStride;

				if ( status == GEVLIB_OK)
				{
					switch(pReg->type)
					{
						case floatRegLE:
						case floatReg:
							{
								UINT32 *valuePtr = (UINT32 *)data;
								UINT32 *value = (UINT32 *)data;
								if (little_endian)
								{
									// Convert value for LittleEndian feature interpretation.
									// (Low level Gev_WriteReg converts to network order (BigEndian))
									// Note: The IEEE754 float data order may be universal but sometimes
									// the BE/LE switch is done.  
									*value = _Convert_to_LEFeature_Order(*valuePtr);
								}
								// Write the actual data.
								if (bAsync)
								{
									status = Gev_WriteReg_NoAck( handle, offset, *value);
								}
								else
								{
									status = Gev_WriteReg( handle, offset, *value);
								}
							}
							break;
						case integerRegLE:
						case integerReg:
						case bitRegLE:
						case bitReg:
							{
								UINT32 value = *(UINT32 *)data;
								UINT32 masked_value = value & pReg->writeMask;
								if (little_endian)
								{
									// Convert value for LittleEndian feature interpretation.
									// (Low level Gev_WriteReg converts to network order (BigEndian))
									masked_value = _Convert_to_LEFeature_Order(masked_value);
								}
								// Write the actual data.
								if (bAsync)
								{
									status = Gev_WriteReg_NoAck( handle, offset, masked_value);
								}
								else
								{
									status = Gev_WriteReg( handle, offset, masked_value);
								}
							}
							break;
						case stringReg:
						case dataArea:
							{
								if (bAsync)
								{
									status = Gev_WriteMem_NoAck( handle, offset, (char *)data, size);
								}
								else
								{
									status = Gev_WriteMem( handle, offset, (char *)data, size);
								}
							}
							break;
						default:
								status = GEVLIB_ERROR_SOFTWARE;	// Something wrong with the register data.
							break;
					}
				}
			}
		}
	}
	return status;
}

//! Write a Camera register
/*!
	This function is used to write into a camera register 
	\param [in] handle	Camera handle. 
	\param [in] pReg		Ptr to the GEV_REGISTER structure describing the camera register/feature to be accessed.
	\param [in] selector	Index used to select different registers within the set available for this feature.
	\param [in] size		Size of data to be written.
	\param [in] data		Data to be written.
	\return GEV status code returned from camera
	\note None
*/
GEV_STATUS GevRegisterWrite(GEV_CAMERA_HANDLE handle, GEV_REGISTER *pReg, int selector, UINT32 size, void *data)
{
	return _GevRegisterWriteEx(handle, pReg, selector, size, data, FALSE);
}


//! Write a Camera register without waiting for status from the camera (asynchronous)
/*!
	This function is used to write into a camera register and does not wait for an
   acknowledgement that the write was successful. This makes it much faster at the
   expense of not knowing if the write actually worked.
	\param [in] handle	Camera handle. 
	\param [in] pReg		Ptr to the GEV_REGISTER structure describing the camera register/feature to be accessed.
	\param [in] selector	Index used to select different registers within the set available for this feature.
	\param [in] size		Size of data to be written.
	\param [in] data		Data to be written.
	\return GEV status code returned from camera
	\note None
*/
GEV_STATUS GevRegisterWriteNoWait(GEV_CAMERA_HANDLE handle, GEV_REGISTER *pReg, int selector, UINT32 size, void *data)
{
	return _GevRegisterWriteEx(handle, pReg, selector, size, data, TRUE);
}


//! Read a Camera register
/*!
	This function is used to read from a camera register
	\param [in] handle	Camera handle. 
	\param [in] pReg		Ptr to the GEV_REGISTER structure describing the camera register/feature to be accessed.
	\param [in] selector	Index used to select different registers within the set available for this feature.
	\param [in] size		Size of storage for data read from camera.
	\param [in] data		Ptr to contain data read from the camera.
	\return GEV status code
	\note None
*/
GEV_STATUS GevRegisterRead(GEV_CAMERA_HANDLE handle, GEV_REGISTER *pReg, int selector, UINT32 size, void *data)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;
	BOOL32 little_endian = FALSE;

	if (handle != NULL)
	{
		status = GevRegisterCheckParameters( FALSE, pReg, selector, size, data);
		if ( status == GEVLIB_OK)
		{
			if ((pReg->type == integerRegLE) || (pReg->type == bitRegLE) || (pReg->type == floatRegLE))
			{
				little_endian = TRUE;
			}
			if (pReg->address == NOREF_ADDR)
			{
				// No reference address in camera - use the value stored in the register structure.
				switch(pReg->type)
				{
					case floatRegLE:
					case floatReg:
					case floatVal:
						{
							float *value = (float *)data;
							*value = pReg->value.floatValue;
						}
						break;
					case integerRegLE:
					case integerReg:
					case intVal:
					case fixedVal:
						{
							UINT32 *value = (UINT32 *)data;
							*value = pReg->value.intValue;
						}
						break;
					case bitRegLE:
					case bitReg:
						{
							UINT32 *value = (UINT32 *)data;
							*value = pReg->value.bitIndex;
						}
						break;
					default:
							status = GEVLIB_ERROR_SOFTWARE;	// Something wrong with the register data.(NOREF_ADDRESS of wrong type).
						break;
				}
			}
			else
			{
				UINT32 offset = pReg->address + (selector - pReg->minSelector) * pReg->regStride;

				// Read the actual data.
				if ( status == GEVLIB_OK)
				{
					switch(pReg->type)
					{
						case floatRegLE:
						case floatReg:
							{
								UINT32 *value = (UINT32 *)data;

								status = Gev_ReadReg( handle, offset, value);
								if (little_endian)
								{
									// Convert value for LittleEndian intepretation.
									// (Low level Gev_ReadReg converts from network order (BigEndian))
									*value = _Convert_from_LEFeature_Order(*value);
								}
							}
							break;
						case integerRegLE:
						case bitRegLE:
						case integerReg:
						case bitReg:
							{
								UINT32 *valueIn = (UINT32 *)data;
								status = Gev_ReadReg( handle, offset, valueIn);
								if (little_endian)
								{
									// Convert value for LittleEndian intepretation.
									// (Low level Gev_ReadReg converts from network order (BigEndian))
									*valueIn = _Convert_from_LEFeature_Order(*valueIn);
								}
								*valueIn = *valueIn & pReg->readMask;
							}
							break;
						case stringReg:
						case dataArea:
							{
								status = Gev_ReadMem( handle, offset, (char *)data, size);
							}
							break;
						default:
								status = GEVLIB_ERROR_SOFTWARE;	// Something wrong with the register data.
							break;
					}
				}
			}
		}	
		else
		{
			status = GEVLIB_ERROR_ARG_INVALID;
		}
	}
	return status;
}

//! Read a Camera register by name
/*!
	This function is used to read from a camera register identified by name.
	\param [in] handle	Camera handle. 
	\param [in] name		Ptr to the name of a GEV_REGISTER structure stored in the handle.
	\param [in] selector	Index used to select different registers within the set available for this feature.
	\param [in] size		Size of storage for data read from camera.
	\param [in] data		Ptr to contain data read from the camera.
	\return GEV status code
	\note None
*/
GEV_STATUS GevReadRegisterByName( GEV_CAMERA_HANDLE handle, char *name, int selector, UINT32 size, void *value)
{
	GEV_STATUS status = -1;
	if ( (name != NULL) || (value != NULL))
	{
		GEV_REGISTER *reg = NULL;
		status = GevGetRegisterPtrByName( handle, name, &reg);
		if (status == 0)
		{
			status = GevRegisterRead( handle, reg, selector, size, value);
		}		
	}
	return status;
}

//! Write a Camera register by name.
/*!
	This function is used to write into a camera register identified by name.
	\param [in] handle	Camera handle. 
	\param [in] name		Ptr to the name of a GEV_REGISTER structure stored in the handle.
	\param [in] selector	Index used to select different registers within the set available for this feature.
	\param [in] size		Size of data to be written.
	\param [in] data		Data to be written.
	\return GEV status code returned from camera
	\note None
*/
GEV_STATUS GevWriteRegisterByName( GEV_CAMERA_HANDLE handle, char *name, int selector, UINT32 size, void *value)
{
	GEV_STATUS status = -1;
	if ( (name != NULL) || (value != NULL))
	{
		GEV_REGISTER *reg = NULL;
		status = GevGetRegisterPtrByName( handle, name, &reg);
		if (status == 0)
		{
			status = GevRegisterWrite( handle, reg, selector, size, value);
		}		
	}
	return status;
}

//! Write a Camera register array
/*!
	This function is used to write into a camera register array. 
	It only writes to registers that are defined as an array.
	(Note: All entries in the array are 32 bits in size).

	\param [in] handle        Camera handle. 
	\param [in] pReg          Ptr to the GEV_REGISTER structure describing the camera register/feature to be accessed.
	\param [in] selector	     Index used to select different arrays of registers within the set available for this feature.
	\param [in] array_offset  Index into the array for data to be written to.
	\param [in] num_entries   Number of entries to be written to the array (each entry is 4 bytes).
	\param [in] data          Data to be written.
	\return GEV status code
	\note None

*/
GEV_STATUS GevRegisterWriteArray(GEV_CAMERA_HANDLE handle, GEV_REGISTER *pReg, int selector, UINT32 array_offset, UINT32 num_entries, void *data)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;
	UINT32 size = num_entries * sizeof(UINT32);

	if (handle != NULL)
	{
		status = GevRegisterCheckParameters( TRUE, pReg, selector, size, data);
		if ( status == GEVLIB_OK)
		{
			if (pReg->address == NOREF_ADDR)
			{
				// No reference address in camera - this is invalid for an array.
				status = GEVLIB_ERROR_SOFTWARE;	// Something wrong with the register array definition data.(NOREF_ADDRESS or wrong type).
			}
			else
			{
				UINT32 offset = 4*array_offset + pReg->address + (selector - pReg->minSelector) * pReg->regStride;

				if ( status == GEVLIB_OK)
				{
					switch(pReg->type)
					{
						case stringReg:
						case dataArea:
							{
								int i = 0;
								int remainder = size;
								char *ptr = (char *)data;
								while (remainder > GEVWRITEMEM_MAXDATASIZE)
								{
									status = Gev_WriteMem( handle, (offset+i), (char *)&ptr[i], GEVWRITEMEM_MAXDATASIZE);
									remainder -= GEVWRITEMEM_MAXDATASIZE;
									i += GEVWRITEMEM_MAXDATASIZE;
									if (status != 0)
									{
										break;
									}
								}
								if (status == 0)
								{
									status = Gev_WriteMem( handle, (offset+i), (char *)&ptr[i], remainder);
								}
							}
							break;
						default:
								status = GEVLIB_ERROR_SOFTWARE;	// Something wrong with the register data.
							break;
					}
				}
			}
		}
	}
	return status;
}

//! Read a Camera register array
/*!
	This function is used to read from a camera register array. 
	It only reads from registers that are defined as an array.
	(Note: All entries in the array are 32 bits in size).

	\param [in] handle        Camera handle. 
	\param [in] pReg          Ptr to the GEV_REGISTER structure describing the camera register/feature to be accessed.
	\param [in] selector	     Index used to select different arrays of registers within the set available for this feature.
	\param [in] array_offset  Index into the array at which data is to be read from.
	\param [in] num_entries   Number of entries to be read from the array (each entry is 4 bytes).
	\param [in] data          Pointer to memory where array data is to be returned.
	\return GEV status code
	\note None
*/
GEV_STATUS GevRegisterReadArray(GEV_CAMERA_HANDLE handle, GEV_REGISTER *pReg, int selector, UINT32 array_offset, UINT32 num_entries, void *data)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;
	UINT32 size = num_entries * sizeof(UINT32);

	if (handle != NULL)
	{
		status = GevRegisterCheckParameters( FALSE, pReg, selector, size, data);
		if ( status == GEVLIB_OK)
		{
			if (pReg->address == NOREF_ADDR)
			{
				// No reference address in camera - this is invalid for an array.
				status = GEVLIB_ERROR_SOFTWARE;	// Something wrong with the register array definition data.(NOREF_ADDRESS or wrong type).
			}
			else
			{
				UINT32 offset = 4*array_offset + pReg->address + (selector - pReg->minSelector) * pReg->regStride;

				if ( status == GEVLIB_OK)
				{
					switch(pReg->type)
					{
						case stringReg:
						case dataArea:
							{
								int i = 0;
								int remainder = size;
								char *ptr = (char *)data;
								while (remainder > GEVREADMEM_MAXDATASIZE)
								{
									status = Gev_ReadMem( handle, (offset+i), (char *)&ptr[i], GEVREADMEM_MAXDATASIZE);
									remainder -= GEVREADMEM_MAXDATASIZE;
									i += GEVREADMEM_MAXDATASIZE;
									if (status != 0)
									{
										break;
									}
								}
								if (status == 0)
								{
									status = Gev_ReadMem( handle, (offset+i), (char *)&ptr[i], remainder);
								}
							}
							break;
						default:
								status = GEVLIB_ERROR_SOFTWARE;	// Something wrong with the register data.
							break;
					}
				}
			}
		}
	}
	return status;
}



//! Write an Integer Camera register
/*!
	This function is used to write into a camera register
	\param [in] handle	Camera handle. 
	\param [in] pReg		Ptr to the GEV_REGISTER structure describing the camera register/feature to be accessed.
	\param [in] selector	Index used to select different registers within the set available for this feature.
	\param [in] value		Value to be written.
	\return GEV status code
	\note None
*/
GEV_STATUS GevRegisterWriteInt(GEV_CAMERA_HANDLE handle, GEV_REGISTER *pReg, int selector, UINT32 value)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;
	BOOL32 little_endian = FALSE;

	if (handle != NULL)
	{
		status = GevRegisterCheckParameters( TRUE, pReg, selector, sizeof(UINT32), &value);
		if ( status == GEVLIB_OK)
		{
			if ((pReg->type == integerRegLE) || (pReg->type == bitRegLE) || (pReg->type == floatRegLE))
			{
				little_endian = TRUE;
			}
			if (pReg->address == NOREF_ADDR)
			{
				// No reference address in camera - store the value in the register structure.
				switch(pReg->type)
				{
					case floatRegLE:
					case floatReg:
					case floatVal:
							status = GEVLIB_ERROR_PARAMETER_INVALID;	// Not an integer!!!
						break;
					case integerRegLE:
					case integerReg:
					case intVal:
					case fixedVal:
							pReg->value.intValue = value & pReg->writeMask;
						break;
					case bitRegLE:
					case bitReg:
							pReg->value.bitIndex = value & pReg->writeMask;
						break;
					default:
							status = GEVLIB_ERROR_SOFTWARE;	// Something wrong with the register data.(NOREF_ADDRESS of wrong type).
						break;
				}
			}
			else
			{
				UINT32 offset = pReg->address + (selector - pReg->minSelector) * pReg->regStride;
				UINT32 data = 0;

				if ( status == GEVLIB_OK)
				{
					switch(pReg->type)
					{
						case floatRegLE:
						case floatReg:
							status = GEVLIB_ERROR_PARAMETER_INVALID;	// Not an integer!!!
							break;
						case integerRegLE:
						case bitRegLE:
						case integerReg:
						case bitReg:
						case fixedVal:
						case dataArea:
							{
								UINT32 masked_value = value & pReg->writeMask;
								if (little_endian)
								{
									// Convert value for LittleEndian feature interpretation.
									// (Low level Gev_WriteReg converts to network order (BigEndian))
									masked_value = _Convert_to_LEFeature_Order(masked_value);
								}
								data = masked_value;
							}
							break;
						default:
								status = GEVLIB_ERROR_SOFTWARE;	// Something wrong with the register data.
							break;
					}

					if (status == GEVLIB_OK)
					{
						// Write the actual data 
						status = Gev_WriteReg( handle, offset, data);
					}
				}
			}
		}
	}
	return status;
}

//! Read an Integer Camera register
/*!
	This function is used to read from a camera register
	\param [in] handle	Camera handle. 
	\param [in] pReg		Ptr to the GEV_REGISTER structure describing the camera register/feature to be accessed.
	\param [in] selector	Index used to select different registers within the set available for this feature.
	\param [in] value		Ptr to contain data read from the camera.
	\return GEV status code
	\note None
*/
GEV_STATUS GevRegisterReadInt(GEV_CAMERA_HANDLE handle, GEV_REGISTER *pReg, int selector, UINT32 *value)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;
	BOOL32 little_endian = FALSE;

	if (handle != NULL)
	{
		status = GevRegisterCheckParameters( FALSE, pReg, selector, sizeof(UINT32), value);
		if ( status == GEVLIB_OK)
		{
			if ((pReg->type == integerRegLE) || (pReg->type == bitRegLE) || (pReg->type == floatRegLE))
			{
				little_endian = TRUE;
			}
			if (pReg->address == NOREF_ADDR)
			{
				// No reference address in camera - use the value stored in the register structure.
				switch(pReg->type)
				{
					case floatRegLE:
					case floatReg:
					case floatVal:
							status = GEVLIB_ERROR_PARAMETER_INVALID;	// Not an integer!!!
						break;
					case integerRegLE:
					case integerReg:
					case intVal:
					case fixedVal:
							*value = pReg->value.intValue;
						break;
					case bitRegLE:
					case bitReg:
							*value = pReg->value.bitIndex;
						break;
					default:
							status = GEVLIB_ERROR_SOFTWARE;	// Something wrong with the register data.(NOREF_ADDRESS of wrong type).
						break;
				}
			}
			else
			{
				UINT32 offset = pReg->address + (selector - pReg->minSelector) * pReg->regStride;

				// Read the actual data.
				if ( status == GEVLIB_OK)
				{
					switch(pReg->type)
					{
						case floatRegLE:
						case floatReg:
							status = GEVLIB_ERROR_PARAMETER_INVALID;	// Not an integer!!!
							break;
						case integerRegLE:
						case integerReg:
						case fixedVal:
						case bitReg:
							{
								UINT32 data = 0;
								status = Gev_ReadReg( handle, offset, &data);
								if (little_endian)
								{
									// Convert value for LittleEndian intepretation.
									// (Low level Gev_ReadReg converts from network order (BigEndian))
									data = _Convert_from_LEFeature_Order(data);
								}
								*value = data & pReg->readMask;
							}
							break;
						default:
								status = GEVLIB_ERROR_SOFTWARE;	// Something wrong with the register data.
							break;
					}
				}
			}
		}	
		else
		{
			status = GEVLIB_ERROR_ARG_INVALID;
		}
	}
	return status;
}

//! Write a floating point Camera register
/*!
	This function is used to write into a camera register
	\param [in] handle	Camera handle. 
	\param [in] pReg		Ptr to the GEV_REGISTER structure describing the camera register/feature to be accessed.
	\param [in] selector	Index used to select different registers within the set available for this feature.
	\param [in] value		Value to be written.
	\return GEV status code
	\note None
*/
GEV_STATUS GevRegisterWriteFloat(GEV_CAMERA_HANDLE handle, GEV_REGISTER *pReg, int selector, float value)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;
	BOOL32 little_endian = FALSE;

	if (handle != NULL)
	{
		status = GevRegisterCheckParameters( TRUE, pReg, selector, sizeof(float), &value);
		if ( status == GEVLIB_OK)
		{
			if ((pReg->type == integerRegLE) || (pReg->type == bitRegLE) || (pReg->type == floatRegLE))
			{
				little_endian = TRUE;
			}
			if (pReg->address == NOREF_ADDR)
			{
				// No reference address in camera - store the value in the register structure.
				switch(pReg->type)
				{
					case floatRegLE:
					case floatReg:
					case floatVal:
							pReg->value.floatValue = value;
						break;
					case integerRegLE:
					case integerReg:
					case intVal:
					case fixedVal:
					case bitRegLE:
					case bitReg:
							status = GEVLIB_ERROR_PARAMETER_INVALID;	// Not a float!!!
						break;
					default:
							status = GEVLIB_ERROR_SOFTWARE;	// Something wrong with the register data.(NOREF_ADDRESS of wrong type).
						break;
				}
			}
			else
			{
				UINT32 offset = pReg->address + (selector - pReg->minSelector) * pReg->regStride;

				if ( status == GEVLIB_OK)
				{
					switch(pReg->type)
					{
						case floatRegLE:
						case floatReg:
							{
								UINT32 data = *(UINT32 *)(&value);
								if (little_endian)
								{
									// Convert value for LittleEndian feature interpretation.
									// (Low level Gev_WriteReg converts to network order (BigEndian))
									// Note: The IEEE754 float data order may be universal but sometimes
									// the BE/LE switch is done.  
									data = _Convert_to_LEFeature_Order(data);
								}
								// Write the actual data.
								status = Gev_WriteReg( handle, offset, data);
							}
							break;
						case integerRegLE:
						case bitRegLE:
						case integerReg:
						case bitReg:
							status = GEVLIB_ERROR_PARAMETER_INVALID;	// Not a float!!!
							break;
						default:
								status = GEVLIB_ERROR_SOFTWARE;	// Something wrong with the register data.
							break;
					}
				}
			}
		}
	}
	return status;
}

//! Read a floating point Camera register
/*!
	This function is used to read from a camera register
	\param [in] handle	Camera handle. 
	\param [in] pReg		Ptr to the GEV_REGISTER structure describing the camera register/feature to be accessed.
	\param [in] selector	Index used to select different registers within the set available for this feature.
	\param [in] value		Ptr to contain data read from the camera.
	\return GEV status code
	\note None
*/
GEV_STATUS GevRegisterReadFloat(GEV_CAMERA_HANDLE handle, GEV_REGISTER *pReg, int selector, float *value)
{
	GEV_STATUS status = GEVLIB_ERROR_INVALID_HANDLE;
	BOOL32 little_endian = FALSE;

	if (handle != NULL)
	{
		status = GevRegisterCheckParameters( FALSE, pReg, selector, sizeof(float), value);
		if ( status == GEVLIB_OK)
		{
			if ((pReg->type == integerRegLE) || (pReg->type == bitRegLE) || (pReg->type == floatRegLE))
			{
				little_endian = TRUE;
			}
			if (pReg->address == NOREF_ADDR)
			{
				// No reference address in camera - use the value stored in the register structure.
				switch(pReg->type)
				{
					case floatRegLE:
					case floatReg:
					case floatVal:
							*value = pReg->value.floatValue;
						break;
					case integerRegLE:
					case integerReg:
					case intVal:
					case fixedVal:
					case bitRegLE:
					case bitReg:
							status = GEVLIB_ERROR_PARAMETER_INVALID;	// Not a float!!!
						break;
					default:
							status = GEVLIB_ERROR_SOFTWARE;	// Something wrong with the register data.(NOREF_ADDRESS of wrong type).
						break;
				}
			}
			else
			{
				UINT32 offset = pReg->address + (selector - pReg->minSelector) * pReg->regStride;

				// Read the actual data.
				if ( status == GEVLIB_OK)
				{
					switch(pReg->type)
					{
						case floatRegLE:
						case floatReg:
							{
								float fval = 0.0;
								UINT32 *valOut = (UINT32 *)value;
								UINT32 *valIn = (UINT32 *)&fval;

								// (Yes - aliased pointers doing conversions - bad bad).
								status = Gev_ReadReg( handle, offset, (UINT32 *)&fval);
								if (little_endian)
								{
									// Convert value for LittleEndian intepretation.
									// (Low level Gev_ReadReg converts from network order (BigEndian))
									*valOut = _Convert_from_LEFeature_Order( *valIn);
								}
								else
								{
									*valOut = *valIn;
								}
								
							}
							break;
						case integerRegLE:
						case bitRegLE:
						case integerReg:
						case bitReg:
							status = GEVLIB_ERROR_PARAMETER_INVALID;	// Not a float!!!
							break;
						default:
								status = GEVLIB_ERROR_SOFTWARE;	// Something wrong with the register data.
							break;
					}
				}
			}
		}	
		else
		{
			status = GEVLIB_ERROR_ARG_INVALID;
		}
	}
	return status;
}

#ifdef __cplusplus
}
#endif
