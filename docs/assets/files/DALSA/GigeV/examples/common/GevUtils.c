/*
  ---------------------------------------------
  GigE-V Framework Utility Functions
  -----------------------------------------------
*/

#include "SapX11Util.h"
#include "gevapi.h"

//=============================================================================
// Translation of pixel format information between GigE-Vision formats and
// both the X11_Display_Utils implementation and the basic RGB888  (no alpha 
//	channel) and RGB8888 (with alpha_channel) formats used by the GdkPixbuf.
//
// X11_Display_Utils (currently) only supports the following formats:
//	CORX11_DATA_FORMAT_MONO      - Any Monochrome (unpacked) format (8, 10, 12, 14, 16 bits).
//	CORX11_DATA_FORMAT_RGB888    - RGB (packed) 8 bits per channel in 24 bits.
//	CORX11_DATA_FORMAT_RGB8888   - RGB (unpacked) 8 bits per channel in 32 bits.
//	CORX11_DATA_FORMAT_RGB5551   - RGB (packed) "hicolor" mode (5:5:5:1)
//	CORX11_DATA_FORMAT_RGB565    - RGB (packed) "hicolor" mode (5:6:5)
//	CORX11_DATA_FORMAT_RGB101010 - RGB (packed) 10 bits per channel
// CORX11_DATA_FORMAT_YUV411		- composite RGB -> one RGB pixel is 12 bits (all packed together) -> RGB/Mono conversion available.
// CORX11_DATA_FORMAT_YUV422		- composite RGB -> one RGB pixel is 16 bits (all packed together) -> RGB/Mono conversion available.
// CORX11_DATA_FORMAT_YUV444		- composite RGB -> one RGB pixel is 24 bits (all packed together) -> RGB/Mono conversion available.

// GEV API supports formats in the enumGevPixelFormat type :
//
// MONO formats
//
// fmtMono8          (supported as CORX11_DATA_FORMAT_MONO with depth 8)
// fmtMono8Signed    (supported as CORX11_DATA_FORMAT_MONO with depth 8 - might look funny)
// fmtMono10         (supported as CORX11_DATA_FORMAT_MONO with depth 10)
// fmtMono10Packed   (8 pixels packed into 10 bytes -> Conversion to MONO available)
// fmtMono12         (supported as CORX11_DATA_FORMAT_MONO with depth 12)
// fmtMono12Packed   (4 12-bit pixels packed into 6 bytes -> Conversion to MONO available)
// fmtMono14         (supported as CORX11_DATA_FORMAT_MONO with depth 14)
// fmtMono16         (supported as CORX11_DATA_FORMAT_MONO with depth 16)
//
// Bayer filter formats (color conversion not supported -> treated as monochrome)
//
// fMtBayerGR8       (becomes MONO8)
// fMtBayerRG8       (becomes MONO8)
// fMtBayerGB8       (becomes MONO8)
// fMtBayerBG8       (becomes MONO8)
// fMtBayerGR10      (becomes MONO10)
// fMtBayerRG10      (becomes MONO10)
// fMtBayerGB10      (becomes MONO10)
// fMtBayerBG10      (becomes MONO10)
// fMtBayerGR12      (becomes MONO12)
// fMtBayerRG12	   (becomes MONO12)
// fMtBayerGB12      (becomes MONO12)
// fMtBayerBG12      (becomes MONO12)
//
// RGB Packed formats
//
// fmtRGB8Packed     (RGB unpacking conversion available -> converts to CORX11_DATA_FORMAT_RGB8888)
// fmtBGR8Packed     (RGB unpacking conversion available -> converts to CORX11_DATA_FORMAT_RGB8888)
// fmtRGBA8Packed    (supported as CORX11_DATA_FORMAT_RGB8888)
// fmtBGRA8Packed    (conversion available -> converts to CORX11_DATA_FORMAT_RGB8888)
// fmtRGB10Packed    (RGB unpacking conversion available -> converts to CORX11_DATA_FORMAT_RGB8888)
// fmtBGR10Packed    (RGB unpacking conversion available -> converts to CORX11_DATA_FORMAT_RGB8888s)
// fmtRGB12Packed    (RGB unpacking conversion available -> converts to CORX11_DATA_FORMAT_RGB8888)
// fmtBGR12Packed    (RGB unpacking conversion available -> converts to CORX11_DATA_FORMAT_RGB8888)
// fmtRGB10V1Packed  (RGB unpackign conversion available -> converts to CORX11_DATA_FORMAT_RGB8888)
// fmtRGB10V2Packed  (RGB unpackign conversion available -> converts to CORX11_DATA_FORMAT_RGB8888)
//
// YUV Packed formats (not supported - conversion routines supplied).
//
// fmtYUV411packed   (conversion required for X11 display -> RGB/Mono conversion available.)
// fmtYUV422packed   (conversion required for X11 display -> RGB/Mono conversion available.)
// fmtYUV444packed   (conversion required for X11 display -> RGB/Mono conversion available.)
//
// Planar formats : (Not supported yet - Each image plane a separate (sequential) image region.)
//
// fmtRGB8Planar		(No Support)
// fmtRGB10Planar		(No Support)
// fmtRGB12Planar		(No Support)
// fmtRGB16Planar		(No Support)
//
//=============================================================================
//
int IsGevPixelTypeX11Displayable(int format)
{
	int displayable = FALSE;
	switch(format)
	{
		case fmtYUV411packed:
		case fmtYUV422packed:
		case fmtYUV444packed:
      case fmtMono10Packed:
      case fmtMono12Packed:
      case fmtRGB10V1Packed:
      case fmtRGB10V2Packed:
      case fmtRGB8Planar:
      case fmtRGB10Planar:
      case fmtRGB12Planar:
      case fmtRGB16Planar:
			displayable = FALSE;
			break;
		default:
			displayable = TRUE;
			break;
	}
	return displayable;
}


int Convert_GevFormat_To_X11( int GevDataFormat)
{
   int format = CORX11_DATA_FORMAT_DEFAULT;
   
   if ( GevIsPixelTypeRGB(GevDataFormat) )
   {
		// Color format - see if we can handle it directly (without conversion).
		// (Note: there are many (many) formats supported by the GigE standard that 
		//  would require conversion before display).
      switch (GevDataFormat)
      {
 			case fmtRGB8Packed:
			case fmtBGR8Packed:
         case fmtYUV444packed:
            format = CORX11_DATA_FORMAT_RGB888;
            break; 
			case fmtRGBA8Packed:
			case fmtBGRA8Packed:
            format = CORX11_DATA_FORMAT_RGB8888;
            break; 
         case fmtRGB10Packed:
         case fmtBGR10Packed:
         case fmtRGB10V1Packed:
         case fmtRGB10V2Packed:
            format = CORX11_DATA_FORMAT_RGB101010;
            break; 
         case fmtYUV411packed:
					format = CORX11_DATA_FORMAT_MONO;  // 12 bit YUV pixel - conversion required.
					break;
         case fmtYUV422packed:
            format = CORX11_DATA_FORMAT_RGB5551;
            break; 
         default:
            format = CORX11_DATA_FORMAT_DEFAULT;
            break;
      }
   }
   else if ( GevIsPixelTypeMono(GevDataFormat) )
   {
      // Its monochrome (only 10/12 bit packed is not handled yet)
      switch (GevDataFormat)
      {
         case fmtMono10Packed:
         case fmtMono12Packed:
            format = CORX11_DATA_FORMAT_DEFAULT;
            break; 
         default:
            format = CORX11_DATA_FORMAT_MONO;
            break;
      }
   }
	else
	{
		// Not supported - just handle it as default (copy to display).
		format = CORX11_DATA_FORMAT_DEFAULT;
	}
   return format;
}

int Convert_GevFormat_To_Sapera( int GevDataFormat)
{
   int format = CORX11_DATA_FORMAT_DEFAULT;
   
   if ( GevIsPixelTypeRGB(GevDataFormat) )
   {
		// Color format - see if we can handle it directly (without conversion).
		// (Note: there are many (many) formats supported by the GigE standard that 
		//  would require conversion before display).
      switch (GevDataFormat)
      {
 			case fmtRGB8Packed:
			case fmtBGR8Packed:
            format = CORDATA_FORMAT_RGB888;
            break; 
			case fmtRGBA8Packed:
			case fmtBGRA8Packed:
            format = CORDATA_FORMAT_RGB8888;
            break; 
         case fmtRGB10Packed:
         case fmtBGR10Packed:
         case fmtRGB10V1Packed:
         case fmtRGB10V2Packed:
            format = CORDATA_FORMAT_RGB101010;
            break; 
			case fmtYUV411packed:
				format = CORDATA_FORMAT_Y411;
				break;
			case fmtYUV422packed:
				format = CORDATA_FORMAT_YUY2;
				break;
			case fmtYUV444packed:
				format = CORDATA_FORMAT_YUYV;
				break;
         default:
            format = CORDATA_FORMAT_UINT8;
            break;
      }
   }
   else if ( GevIsPixelTypeMono(GevDataFormat) )
   {
      // Its monochrome 
      switch (GevDataFormat)
      {
			case fmtMono8Signed:
					format = CORDATA_FORMAT_INT8;
				break;
			case fmtMono10:
			case fmtMono10Packed:
			case fmtMono12:
			case fmtMono12Packed:
			case fmtMono14:
			case fmtMono16:
			case fMtBayerGR10:
			case fMtBayerRG10:
			case fMtBayerGB10:
			case fMtBayerBG10:
			case fMtBayerGR12:
			case fMtBayerRG12:
			case fMtBayerGB12:
			case fMtBayerBG12:
					format = CORDATA_FORMAT_MONO16;
				break;			
         default:
            format = CORDATA_FORMAT_UINT8;
            break;
      }
   }
	else
	{
		// Not supported - just handle it as mono / default (copy to display).
		format = CORDATA_FORMAT_MONO8;
	}
   return format;
}

static void Convert_YUV411_To_Mono(int pixelCount, void *in, int outDepth, void *out)
{
	int i;
	if ((in != NULL) && (out != NULL))
	{
		switch (outDepth)
		{
			case 16:
			case 12:
			case 10:
				{
					unsigned char *pIn = (unsigned char *)in;
					unsigned short *pOut = (unsigned short *)out;
					int lshift = outDepth - 8;
					for (i = 0; i < pixelCount; i+= 4)
					{
						pIn++;				// Skip U
						*pOut++ = ((unsigned short)*pIn++) << lshift;	// Store Y0
						*pOut++ = ((unsigned short)*pIn++) << lshift; 	// Store Y1
						pIn++;				// Skip V
						*pOut++ = ((unsigned short)*pIn++) << lshift;	// Store Y2
						*pOut++ = ((unsigned short)*pIn++) << lshift;	// Store Y3
					}
				}
				break;
			case 8:
			default:
				{
					unsigned char *pIn = (unsigned char *)in;
					unsigned char *pOut = (unsigned char *)out;
					for (i = 0; i < pixelCount; i+= 4)
					{
						pIn++;				// Skip U
						*pOut++ = *pIn++;	// Store Y0
						*pOut++ = *pIn++; // Store Y1
						pIn++;				// Skip V
						*pOut++ = *pIn++;	// Store Y2
						*pOut++ = *pIn++; // Store Y3
					}
				}
				break;
		}
	}
}

static void Convert_YUV422_To_Mono(int pixelCount, void *in, int outDepth, void *out)
{
	int i;
	if ((in != NULL) && (out != NULL))
	{
		switch (outDepth)
		{
			case 16:
			case 12:
			case 10:
				{
					unsigned char *pIn = (unsigned char *)in;
					unsigned short *pOut = (unsigned short *)out;
					int lshift = outDepth - 8;
					for (i = 0; i < pixelCount; i+= 4)
					{
						pIn++;				// Skip U
						*pOut++ = ((unsigned short)*pIn++) << lshift;	// Store Y0
						pIn++;				// Skip V
						*pOut++ = ((unsigned short)*pIn++) << lshift; 	// Store Y1
					}
				}
				break;
			case 8:
			default:
				{
					unsigned char *pIn = (unsigned char *)in;
					unsigned char *pOut = (unsigned char *)out;
					for (i = 0; i < pixelCount; i+= 4)
					{
						pIn++;				// Skip U0
						*pOut++ = *pIn++;	// Store Y0
						pIn++;				// Skip V0
						*pOut++ = *pIn++; // Store Y1
					}
				}
				break;
		}
	}
}

static void Convert_YUV444_To_Mono(int pixelCount, void *in, int outDepth, void *out)
{
	int i;
	if ((in != NULL) && (out != NULL))
	{
		switch (outDepth)
		{
			case 16:
			case 12:
			case 10:
				{
					unsigned char *pIn = (unsigned char *)in;
					unsigned short *pOut = (unsigned short *)out;
					int lshift = outDepth - 8;
					for (i = 0; i < pixelCount; i+= 3)
					{
						pIn++;				// Skip U
						*pOut++ = ((unsigned short)*pIn++) << lshift;	// Store Y
						pIn++;				// Skip V
					}
				}
				break;
			case 8:
			default:
				{
					unsigned char *pIn = (unsigned char *)in;
					unsigned char *pOut = (unsigned char *)out;
					for (i = 0; i < pixelCount; i+= 3)
					{
						pIn++;				// Skip U
						*pOut++ = *pIn++;	// Store Y
						pIn++;				// Skip V
					}
				}
				break;
		}
	}
}

#define YUV_SCALE_FACTOR	15

static void Convert_YUV411_To_RGB8x(int alpha_channel, int pixelCount, void *in, int outDepth, void *out)
{
	unsigned char *pIn = (unsigned char *)in;
	unsigned char *pOut = (unsigned char *)out;
	int i;
	uint32_t iPixel;
	int32_t	lCValue;
	int32_t 	Y[4];	
	int32_t 	V0;	
	int32_t 	U0;
	int32_t  Const16384 = (1 << YUV_SCALE_FACTOR);
	
	if ( (pIn != NULL) && (pOut != NULL))
	{
		for (i = 0; i < pixelCount; i+=4)
		{
			U0 = (int32_t) *pIn++ - 128;
			Y[0] = (int32_t) *pIn++;
			Y[1] = (int32_t) *pIn++;
			V0 = (int32_t) *pIn++ - 128;
			Y[2] = (int32_t) *pIn++;
			Y[3] = (int32_t) *pIn++;

			for (iPixel = 0; iPixel < 4; i++)
			{
				// Blue conversion.
				lCValue = (Y[iPixel]*Const16384 + 29147*U0) >> YUV_SCALE_FACTOR;
				*pOut++ = (lCValue < 0) ? 0 : ( (lCValue > 255) ? 255 : (unsigned char)lCValue);

				// Green conversion.
				lCValue = (Y[iPixel]*Const16384 - 5661*U0 -11746*V0) >> YUV_SCALE_FACTOR;
				*pOut++ = (lCValue < 0) ? 0 : ( (lCValue > 255) ? 255 : (unsigned char)lCValue);

				// Red conversion.
				lCValue = (Y[iPixel]*Const16384 + 23060*V0) >> YUV_SCALE_FACTOR;
				*pOut++ = (lCValue < 0) ? 0 : ( (lCValue > 255) ? 255 : (unsigned char)lCValue);
		
				// Pad (alpha channel)
				if (alpha_channel)
				{
					*pOut++ = 0xff;
				}
			}
		}
	}
}


static void Convert_YUV411_To_RGB888(int pixelCount, void *in, int inDepth, void *out)
{
	Convert_YUV411_To_RGB8x(FALSE, pixelCount, in, inDepth, out);
}
static void Convert_YUV411_To_RGB8888(int pixelCount, void *in, int inDepth, void *out)
{
	Convert_YUV411_To_RGB8x(TRUE, pixelCount, in, inDepth, out);
}


static void Convert_YUV422_To_RGB8x(int alpha_channel, int pixelCount, void *in, int outDepth, void *out)
{
	unsigned char *pIn = (unsigned char *)in;
	unsigned char *pOut = (unsigned char *)out;
	int i;
	int32_t	lCValue;
	int32_t 	Y0;	
	int32_t 	Y1;	
	int32_t 	V0;	
	int32_t 	U0;
	int32_t  Const16384 = (1 << YUV_SCALE_FACTOR);
	
	if ( (pIn != NULL) && (pOut != NULL))
	{
		for (i = 0; i < pixelCount; i+=2)
		{
			U0 = (int32_t) *pIn++ - 128;
			Y0 = (int32_t) *pIn++;
			V0 = (int32_t) *pIn++ - 128;
			Y1 = (int32_t) *pIn++;

			// Blue conversion.
			lCValue = (Y0*Const16384 + 29147*U0) >> YUV_SCALE_FACTOR;
			*pOut++ = (lCValue < 0) ? 0 : ( (lCValue > 255) ? 255 : (unsigned char)lCValue);

			// Green conversion.
			lCValue = (Y0*Const16384 - 5661*U0 -11746*V0) >> YUV_SCALE_FACTOR;
			*pOut++ = (lCValue < 0) ? 0 : ( (lCValue > 255) ? 255 : (unsigned char)lCValue);

			// Red conversion.
			lCValue = (Y0*Const16384 + 23060*V0) >> YUV_SCALE_FACTOR;
			*pOut++ = (lCValue < 0) ? 0 : ( (lCValue > 255) ? 255 : (unsigned char)lCValue);
		
			// Pad (alpha channel)
			pOut++;

			// Blue conversion.
			lCValue = (Y1*Const16384 + 29147*U0) >> YUV_SCALE_FACTOR;
			*pOut++ = (lCValue < 0) ? 0 : ( (lCValue > 255) ? 255 : (unsigned char)lCValue);

			// Green conversion.
			lCValue = (Y1*Const16384 - 5661*U0 -11746*V0) >> YUV_SCALE_FACTOR;
			*pOut++ = (lCValue < 0) ? 0 : ( (lCValue > 255) ? 255 : (unsigned char)lCValue);

			// Red conversion.
			lCValue = (Y1*Const16384 + 23060*V0) >> YUV_SCALE_FACTOR;
			*pOut++ = (lCValue < 0) ? 0 : ( (lCValue > 255) ? 255 : (unsigned char)lCValue);

			// Pad (alpha channel)
			if (alpha_channel)
			{
				*pOut++ = 0xff;
			}
		}
	}
}

static void Convert_YUV422_To_RGB888(int pixelCount, void *in, int inDepth, void *out)
{
	Convert_YUV422_To_RGB8x(FALSE, pixelCount, in, inDepth, out);
}
static void Convert_YUV422_To_RGB8888(int pixelCount, void *in, int inDepth, void *out)
{
	Convert_YUV422_To_RGB8x(TRUE, pixelCount, in, inDepth, out);
}


static void Convert_YUV444_To_RGB8x(int alpha_channel, int pixelCount, void *in, int outDepth, void *out)
{
	unsigned char *pIn = (unsigned char *)in;
	unsigned char *pOut = (unsigned char *)out;
	int i;
	int32_t	lCValue;
	int32_t 	Y0;	
	int32_t 	V0;	
	int32_t 	U0;
	int32_t  Const16384 = (1 << YUV_SCALE_FACTOR);
	
	if ( (pIn != NULL) && (pOut != NULL))
	{
		for (i = 0; i < pixelCount; i++)
		{
			Y0 = (int32_t) *pIn++;
			V0 = (int32_t) *pIn++ - 128;
			U0 = (int32_t) *pIn++ - 128;

			// Blue conversion.
			lCValue = (Y0*Const16384 + 29147*U0) >> YUV_SCALE_FACTOR;
			*pOut++ = (lCValue < 0) ? 0 : ( (lCValue > 255) ? 255 : (unsigned char)lCValue);

			// Green conversion.
			lCValue = (Y0*Const16384 - 5661*U0 -11746*V0) >> YUV_SCALE_FACTOR;
			*pOut++ = (lCValue < 0) ? 0 : ( (lCValue > 255) ? 255 : (unsigned char)lCValue);

			// Red conversion.
			lCValue = (Y0*Const16384 + 23060*V0) >> YUV_SCALE_FACTOR;
			*pOut++ = (lCValue < 0) ? 0 : ( (lCValue > 255) ? 255 : (unsigned char)lCValue);
		
			// Pad (alpha channel)
			if (alpha_channel)
			{
				*pOut++ = 0xff;
			}
		}
	}
}

static void Convert_YUV444_To_RGB888(int pixelCount, void *in, int inDepth, void *out)
{
	Convert_YUV444_To_RGB8x(FALSE, pixelCount, in, inDepth, out);
}
static void Convert_YUV444_To_RGB8888(int pixelCount, void *in, int inDepth, void *out)
{
	Convert_YUV444_To_RGB8x(TRUE, pixelCount, in, inDepth, out);
}


static void Convert_RGBPacked_To_RGB8x(int alpha_channel, int pixelCount, void *in, int inDepth, void *out)
{
	unsigned char *pOut = (unsigned char *)out;
	int i;
	
	if ( (in != NULL) && (out != NULL))
	{
		switch(inDepth)
		{
			case 10:
			case 12:
				{
					int shift = inDepth - 8;
					unsigned short *pIn = (unsigned short *)in;
					for (i = 0; i < pixelCount; i+=3)
					{
						*pOut++ = (unsigned char) ((*pIn++) >> shift); // R
						*pOut++ = (unsigned char) ((*pIn++) >> shift); // G
						*pOut++ = (unsigned char) ((*pIn++) >> shift); // B
						if (alpha_channel)
						{
							*pOut++ = 0xff;
						}
					}
				}
				break;
			case 8:
			default:
				{
					unsigned char *pIn = (unsigned char *)in;
					for (i = 0; i < pixelCount; i+=3)
					{
						*pOut++ = (*pIn++); // R
						*pOut++ = (*pIn++); // G
						*pOut++ = (*pIn++); // B
						if (alpha_channel)
						{
							*pOut++ = 0xff;
						}
					}
				}
				break;
		}
	}
}

static void Convert_RGBPacked_To_RGB888(int pixelCount, void *in, int inDepth, void *out)
{
	Convert_RGBPacked_To_RGB8x(FALSE, pixelCount, in, inDepth, out);
}
static void Convert_RGBPacked_To_RGB8888(int pixelCount, void *in, int inDepth, void *out)
{
	Convert_RGBPacked_To_RGB8x(TRUE, pixelCount, in, inDepth, out);
}




static void Convert_BGRPacked_To_RGB8x(int alpha_channel, int pixelCount, void *in, int inDepth, void *out)
{
	unsigned char *pOut = (unsigned char *)out;
	int i;
	unsigned char red;
	unsigned char green;
	unsigned char blue;
	
	if ( (in != NULL) && (out != NULL))
	{
		switch(inDepth)
		{
			case 10:
			case 12:
				{
					int shift = inDepth - 8;
					unsigned short *pIn = (unsigned short *)in;
					for (i = 0; i < pixelCount; i+=3)
					{
						blue  = (unsigned char) ((*pIn++) >> shift); // B
						green = (unsigned char) ((*pIn++) >> shift); // G
						red   = (unsigned char) ((*pIn++) >> shift); // R
						*pOut++ = red;   // R
						*pOut++ = green; // G
						*pOut++ = blue;  // B
						if (alpha_channel)
						{
							*pOut++ = 0xff;
						}
					}
				}
				break;
			case 8:
			default:
				{
					unsigned char *pIn = (unsigned char *)in;
					for (i = 0; i < pixelCount; i+=3)
					{
						blue  = *pIn++; // B
						green = *pIn++; // G
						red   = *pIn++; // R
						*pOut++ = red;   // R
						*pOut++ = green; // G
						*pOut++ = blue;  // B
						if (alpha_channel)
						{
							*pOut++ = 0xff;
						}
					}
				}
				break;
		}
	}
}

static void Convert_BGRPacked_To_RGB888(int pixelCount, void *in, int inDepth, void *out)
{
	Convert_BGRPacked_To_RGB8x(FALSE, pixelCount, in, inDepth, out);
}
static void Convert_BGRPacked_To_RGB8888(int pixelCount, void *in, int inDepth, void *out)
{
	Convert_BGRPacked_To_RGB8x(TRUE, pixelCount, in, inDepth, out);
}



static void Convert_MonoPacked_To_Mono(int pixelCount, void *in, int inDepth, int outDepth, void *out)
{
	// 10 or 12 bit in (packed) -> 8, 10, or 12 bit out byte aligned).
	unsigned char *pIn = (unsigned char *)in;
	unsigned short input[4];
	int i, j;
	int shift = 0;
	int remainder = 0;
	
	if ( (in != NULL) && (out != NULL))
	{
		shift = inDepth - outDepth;
		remainder = pixelCount % 2;
		switch(outDepth)
		{
			case 10:
			case 12:
				{
					unsigned short *pOut = (unsigned short *)out;
					switch(inDepth)
					{
						case 12:
							{
								// 3 input bytes for 2 output pixels.
								for (i = 0; i < pixelCount; i+= 2)
								{
									for (j = 0; j < 3; j++)
									{
										input[j] = (unsigned short)*pIn++;
									}
									*pOut++ = ( (input[1] && 0x0F) | (input[0] << 4) ) >> shift;
									*pOut++ = (((input[1] && 0xF0) >> 4) | (input[2] << 4) ) >> shift;
								}
								// Handle the remainder.
								if (remainder != 0)
								{
									// There is one more output pixel to be extracted.
									*pOut = ( (pIn[1] && 0x0F) | (pIn[0] << 4) ) >> shift;
								}
							}
							break;
						default:
						case 10:
							{
								// 3 input bytes for 2 output pixels.
								for (i = 0; i < pixelCount; i+= 2)
								{
									for (j = 0; j < 3; j++)
									{
										input[j] = (unsigned short)*pIn++;
									}
									*pOut++ = ( (input[1] && 0x03) | (input[0] << 2) ) >> shift;
									*pOut++ = (((input[1] && 0x30) >> 4) | (input[2] << 2) ) >> shift;
								}						
								// Handle the remainder.
								if (remainder != 0)
								{
									// There is one more output pixel to be extracted.
									*pOut = ( (pIn[1] && 0x03) | (pIn[0] << 2) ) >> shift;
								}
							}
							break;
					}
				}
				break;
			case 8:
			default:
				{
					unsigned char *pOut = (unsigned char *)out;
					switch (inDepth)
					{
						case 12:
							// 3 input bytes for 2 output pixels.
							for (i = 0; i < pixelCount; i+= 2)
							{
								for (j = 0; j < 3; j++)
								{
									input[j] = (unsigned short)*pIn++;
								}							
								*pOut++ = (unsigned char)( (input[1] && 0x0F) | (input[0] << 4) ) >> shift;
								*pOut++ = (unsigned char)(((input[1] && 0xF0) >> 4) | (input[2] << 4 ) ) >> shift;
							}
							// Handle the remainder.
							if (remainder != 0)
							{
								// There is one more output pixel to be extracted.
								*pOut = (unsigned char)( (pIn[1] && 0x0F) | (pIn[0] << 4) ) >> shift;
							}
							break;
						default:
						case 10:
							// 3 input bytes for 2 output pixels.
							for (i = 0; i < pixelCount; i+= 2)
							{
								for (j = 0; j < 3; j++)
								{
									input[j] = (unsigned short)*pIn++;
								}							
								*pOut++ = (unsigned char)( (input[1] && 0x03) | (input[0] << 2) ) >> shift;
								*pOut++ = (unsigned char)(((input[1] && 0x30) >> 4) | (input[2] << 2 ) ) >> shift;
							}							
							// Handle the remainder.
							if (remainder != 0)
							{
								// There is one more output pixel to be extracted.
								*pOut = (unsigned char)( (pIn[1] && 0x03) | (pIn[0] << 2) ) >> shift;
							}
							break;
					}
				}
				break;
		}
	}
}

// Expand a mono8 value into and RGB triple with optional alpha_channel.
static inline void _expand_mono_to_rgb( unsigned char value, unsigned char *rgb_data, int alpha_channel)
{
	if ( rgb_data != NULL)
	{
		*rgb_data++ = value;
		*rgb_data++ = value;
		*rgb_data++ = value;
		if (alpha_channel)
		{
			*rgb_data++ = 0xFF;
		}
	}
}

static void Convert_MonoPacked_To_RGB8x(int alpha_channel, int pixelCount, void *in, int inDepth, void *out)
{
	// 10 or 12 bit in (packed) -> truncate to 8bpp and output as RGB888.
	unsigned short *pIn = (unsigned short *)in;
	unsigned short input[5];
	int i, j;
	int shift = 0;
	int remainder = 0;
	
	if ( (in != NULL) && (out != NULL))
	{
		unsigned char *pOut = (unsigned char *)out;
		unsigned short val = 0;
		shift = inDepth - 8;
		remainder = pixelCount % 2;
		switch (inDepth)
		{
			case 12:
				// 3 input bytes for 2 output pixels.
				for (i = 0; i < pixelCount; i+= 2)
				{
					for (j = 0; j < 3; j++)
					{
						input[j] = (unsigned short)*pIn++;
					}
					val = ( (input[1] && 0x0F) | (input[0] << 4) ) >> shift;
					_expand_mono_to_rgb( (unsigned char) val, pOut, alpha_channel);

					val = (((input[1] && 0xF0) >> 4) | (input[2] << 4) ) >> shift;
					_expand_mono_to_rgb( (unsigned char) val, pOut, alpha_channel);
				}
				// Handle the remainder.
				if (remainder != 0)
				{
					// There is one more output pixel to be extracted.
					val = ( (pIn[1] && 0x0F) | (pIn[0] << 4) ) >> shift;
					_expand_mono_to_rgb( (unsigned char) val, pOut, alpha_channel);
				}
				break;
			default:
			case 10:
				// 3 input bytes for 2 output pixels.
				for (i = 0; i < pixelCount; i+= 2)
				{
					for (j = 0; j < 3; j++)
					{
						input[j] = (unsigned short)*pIn++;
					}
					val = ( (input[1] && 0x03) | (input[0] << 2) ) >> shift;
					_expand_mono_to_rgb( (unsigned char) val, pOut, alpha_channel);

					val = (((input[1] && 0x30) >> 4) | (input[2] << 2) ) >> shift;
					_expand_mono_to_rgb( (unsigned char) val, pOut, alpha_channel);
				}
				// Handle the remainder.
				if (remainder != 0)
				{
					// There is one more output pixel to be extracted.
					val = ( (pIn[1] && 0x03) | (pIn[0] << 2) ) >> shift;
					_expand_mono_to_rgb( (unsigned char) val, pOut, alpha_channel);
				}
				break;
		}
	}
}

static void Convert_MonoPacked_To_RGB888(int pixelCount, void *in, int inDepth, void *out)
{
	Convert_MonoPacked_To_RGB8x(FALSE, pixelCount, in, inDepth, out);
}
static void Convert_MonoPacked_To_RGB8888(int pixelCount, void *in, int inDepth, void *out)
{
	Convert_MonoPacked_To_RGB8x(TRUE, pixelCount, in, inDepth, out);
}


static void Convert_Mono_To_RGB8x(int alpha_channel, int pixelCount, void *in, int inDepth, void *out)
{
	// 8, 10, 12, 14, 16 bit in -> truncate to 8bpp and output as RGB888.
	unsigned char *pOut = (unsigned char *)out;
	int shift = inDepth - 8;
	int i;
	
	if ( (in != NULL) && (out != NULL))
	{
		if (inDepth == 8)
		{
			unsigned char *pIn = (unsigned char *)in;
			for (i = 0; i < pixelCount; i+= 4)
			{
				*pOut++ = *pIn;
				*pOut++ = *pIn;
				*pOut++ = *pIn++;
				if (alpha_channel)
				{
					*pOut++ = 0xff;
				}
			}
		}
		else
		{
			unsigned short *pIn = (unsigned short *)in;
			unsigned short val = 0;
			for (i = 0; i < pixelCount; i+= 4)
			{
				val = (*pIn++ >> shift) & 0xff;
				*pOut++ = (unsigned char)val;
				*pOut++ = (unsigned char)val;
				*pOut++ = (unsigned char)val;
				if (alpha_channel)
				{
					*pOut++ = 0xff;
				}
			}
		}
	}
}

static void Convert_Mono_To_RGB888(int pixelCount, void *in, int inDepth, void *out)
{
	Convert_Mono_To_RGB8x(FALSE, pixelCount, in, inDepth, out);
}

static void Convert_Mono_To_RGB8888(int pixelCount, void *in, int inDepth, void *out)
{
	Convert_Mono_To_RGB8x(TRUE, pixelCount, in, inDepth, out);
}


static void Convert_RGB10V1Packed_To_RGB8x(int alpha_channel, int pixelCount, void *in, void *out)
{
	unsigned char *pOut = (unsigned char *)out;
	int i;
	
	if ( (in != NULL) && (out != NULL))
	{
		unsigned char *pIn = (unsigned char *)in;
		for (i = 0; i < pixelCount; i+=3)
		{
			pIn++;				// Skip 2 LSBs for each color.
			*pOut++ = *pIn++; // R
			*pOut++ = *pIn++; // G
			*pOut++ = *pIn++; // B
			if (alpha_channel)
			{
				*pOut++ = 0xff;
			}
		}
	}
}

static void Convert_RGB10V1Packed_To_RGB888(int pixelCount, void *in, void *out)
{
	Convert_RGB10V1Packed_To_RGB8x(FALSE, pixelCount, in, out);
}
static void Convert_RGB10V1Packed_To_RGB8888(int pixelCount, void *in, void *out)
{
	Convert_RGB10V1Packed_To_RGB8x(TRUE, pixelCount, in, out);
}


static void Convert_RGB10V2Packed_To_RGB8x(int alpha_channel, int pixelCount, void *in, void *out)
{
	unsigned char *pOut = (unsigned char *)out;
	int i;
	
	if ( (in != NULL) && (out != NULL))
	{
		unsigned char *pIn = (unsigned char *)in;
		unsigned char data[4];
		for (i = 0; i < pixelCount; i+=3)
		{
			for (i = 0; i < 4; i++)
			{
				data[i] = *pIn++;
			}
			*pOut++ = (data[0] >> 2) | ((data[1] & 0x03) << 6); // R
			*pOut++ = (data[1] >> 4) | ((data[2] & 0x0F) << 4); // G
			*pOut++ = (data[2] >> 6) | ((data[3] & 0x3F) << 2); // B
			if (alpha_channel)
			{
				*pOut++ = 0xff;
			}
		}
	}
}
static void Convert_RGB10V2Packed_To_RGB888(int pixelCount, void *in, void *out)
{
	Convert_RGB10V2Packed_To_RGB8x( FALSE, pixelCount, in, out);	
}

static void Convert_RGB10V2Packed_To_RGB8888(int pixelCount, void *in, void *out)
{
	Convert_RGB10V2Packed_To_RGB8x( TRUE, pixelCount, in, out);	
}


void ConvertGevImageToX11Format( int w, int h, int gev_depth, int gev_format, void *gev_input_data, 
											int x11_depth, int x11_format, void *x11_output_data)
{
	int numPixels = 0;

	if ((gev_input_data != NULL) && (x11_output_data != NULL))
	{
		// Only allow Mono and RGB8888 as X11 formats.
		switch (x11_format)
		{
			case CORX11_DATA_FORMAT_MONO:
				{
					switch(gev_format)
					{
						case fmtMono10Packed:
						case fmtMono12Packed:
							numPixels =  w*h;
							Convert_MonoPacked_To_Mono(numPixels, gev_input_data, gev_depth, x11_depth, x11_output_data);
							break;
						case fmtYUV411packed:
							numPixels =  w*h;
							Convert_YUV411_To_Mono(numPixels, gev_input_data, x11_depth, x11_output_data);
							break;
						case fmtYUV422packed:
							numPixels = w*h*2;
							Convert_YUV422_To_Mono(numPixels, gev_input_data, x11_depth, x11_output_data);
							break;
						case fmtYUV444packed:
							numPixels = w*h*3;
							Convert_YUV444_To_Mono(numPixels, gev_input_data, x11_depth, x11_output_data);
							break;
						default:
							// Zero the output buffer until these are supported.
							memset(x11_output_data, 0, numPixels*((x11_depth + 7)/8));
							break;
					}
				}
				break;
			case CORX11_DATA_FORMAT_RGB8888:
				{
					numPixels = w*h;
					switch(gev_format)
					{
						case fmtYUV411packed:
							Convert_YUV411_To_RGB8888(numPixels, gev_input_data, x11_depth, x11_output_data);
							break;
						case fmtYUV422packed:
							Convert_YUV422_To_RGB8888(numPixels, gev_input_data, x11_depth, x11_output_data);
							break;
						case fmtYUV444packed:
							Convert_YUV444_To_RGB8888(numPixels, gev_input_data, x11_depth, x11_output_data);
							break;
		 			case fmtRGB8Packed:
		 			case fmtRGB10Packed:
		 			case fmtRGB12Packed:
							Convert_RGBPacked_To_RGB8888(numPixels, gev_input_data, gev_depth, x11_output_data);
							break;
					case fmtBGR8Packed:
					case fmtBGR10Packed:
					case fmtBGR12Packed:
							Convert_BGRPacked_To_RGB8888(numPixels, gev_input_data, gev_depth, x11_output_data);
							break;
					case fmtRGB10V1Packed:
							Convert_RGB10V1Packed_To_RGB8888(numPixels, gev_input_data, x11_output_data);
							break;
					case fmtRGB10V2Packed:
							Convert_RGB10V2Packed_To_RGB8888(numPixels, gev_input_data, x11_output_data);
							break;
						default:
							// Zero the output buffer until these are supported.
							memset(x11_output_data, 0, numPixels*((x11_depth + 7)/8));
							break;
					}
				}
				break;
			default:
				break;
		}
	}
}

void ConvertGevImageToRGB8888Format( int w, int h, int gev_depth, int gev_format, void *gev_input_data, void *rgb_output_data)
{
	int numPixels = 0;
	int outdepth = 32;

	if ((gev_input_data != NULL) && (rgb_output_data != NULL))
	{
		// RGB8888 as output format.
		numPixels = w*h;
		switch(gev_format)
		{
			case fMtBayerGR8:		/* 8-bit Bayer    */
			case fMtBayerRG8:		/* 8-bit Bayer    */
			case fMtBayerGB8:		/* 8-bit Bayer    */
			case fMtBayerBG8:		/* 8-bit Bayer    */
			case fMtBayerGR10:	/* 10-bit Bayer   */
			case fMtBayerRG10:	/* 10-bit Bayer   */
			case fMtBayerGB10:	/* 10-bit Bayer   */
			case fMtBayerBG10:	/* 10-bit Bayer   */
			case fMtBayerGR12:	/* 12-bit Bayer   */
			case fMtBayerRG12:	/* 12-bit Bayer   */
			case fMtBayerGB12:	/* 12-bit Bayer   */
			case fMtBayerBG12:	/* 12-bit Bayer   */
				/* For Now : treat the bayer formats as monochrome - add bayer decoding later */
				Convert_Mono_To_RGB8888(numPixels, gev_input_data, gev_depth, rgb_output_data);
				break;
			case fmtMono8:
			case fmtMono8Signed:
			case fmtMono10:
			case fmtMono12:
			case fmtMono14:
			case fmtMono16:
				Convert_Mono_To_RGB8888(numPixels, gev_input_data, gev_depth, rgb_output_data);
				break;
			case fmtMono10Packed:
			case fmtMono12Packed:
				Convert_MonoPacked_To_RGB8888(numPixels, gev_input_data, gev_depth, rgb_output_data);
				break;
			case fmtYUV411packed:
				Convert_YUV411_To_RGB8888(numPixels, gev_input_data, outdepth, rgb_output_data);
				break;
			case fmtYUV422packed:
				Convert_YUV422_To_RGB8888(numPixels, gev_input_data, outdepth, rgb_output_data);
				break;
			case fmtYUV444packed:
				Convert_YUV444_To_RGB8888(numPixels, gev_input_data, outdepth, rgb_output_data);
				break;
			case fmtRGB8Packed:
			case fmtRGB10Packed:
			case fmtRGB12Packed:
				Convert_RGBPacked_To_RGB8888(numPixels, gev_input_data, outdepth, rgb_output_data);
				break;
			case fmtBGR8Packed:
			case fmtBGR10Packed:
			case fmtBGR12Packed:
				Convert_BGRPacked_To_RGB8888(numPixels, gev_input_data, outdepth, rgb_output_data);
				break;
		case fmtRGB10V1Packed:
				Convert_RGB10V1Packed_To_RGB8888(numPixels, gev_input_data, rgb_output_data);
				break;
		case fmtRGB10V2Packed:
				Convert_RGB10V2Packed_To_RGB8888(numPixels, gev_input_data, rgb_output_data);
				break;
			default:
				// Zero the output buffer until these are supported.
				memset(rgb_output_data, 0, numPixels*((outdepth + 7)/8));
				break;
		}
	}
}

void ConvertGevImageToRGB888Format( int w, int h, int gev_depth, int gev_format, void *gev_input_data, void *rgb_output_data)
{
	int numPixels = 0;
	int outdepth = 24;

	if ((gev_input_data != NULL) && (rgb_output_data != NULL))
	{
		// RGB888 as output format.
		numPixels = w*h;
		switch(gev_format)
		{
			case fMtBayerGR8:		/* 8-bit Bayer    */
			case fMtBayerRG8:		/* 8-bit Bayer    */
			case fMtBayerGB8:		/* 8-bit Bayer    */
			case fMtBayerBG8:		/* 8-bit Bayer    */
			case fMtBayerGR10:	/* 10-bit Bayer   */
			case fMtBayerRG10:	/* 10-bit Bayer   */
			case fMtBayerGB10:	/* 10-bit Bayer   */
			case fMtBayerBG10:	/* 10-bit Bayer   */
			case fMtBayerGR12:	/* 12-bit Bayer   */
			case fMtBayerRG12:	/* 12-bit Bayer   */
			case fMtBayerGB12:	/* 12-bit Bayer   */
			case fMtBayerBG12:	/* 12-bit Bayer   */
				/* For Now : treat the bayer formats as monochrome - add bayer decoding later */
				Convert_Mono_To_RGB888(numPixels, gev_input_data, gev_depth, rgb_output_data);
				break;
			case fmtMono8:
			case fmtMono8Signed:
			case fmtMono10:
			case fmtMono12:
			case fmtMono14:
			case fmtMono16:
				Convert_Mono_To_RGB888(numPixels, gev_input_data, gev_depth, rgb_output_data);
				break;
			case fmtMono10Packed:
			case fmtMono12Packed:
				Convert_MonoPacked_To_RGB888(numPixels, gev_input_data, gev_depth, rgb_output_data);
				break;
			case fmtYUV411packed:
				Convert_YUV411_To_RGB888(numPixels, gev_input_data, outdepth, rgb_output_data);
				break;
			case fmtYUV422packed:
				Convert_YUV422_To_RGB888(numPixels, gev_input_data, outdepth, rgb_output_data);
				break;
			case fmtYUV444packed:
				Convert_YUV444_To_RGB888(numPixels, gev_input_data, outdepth, rgb_output_data);
				break;
			case fmtRGB8Packed:
			case fmtRGB10Packed:
			case fmtRGB12Packed:
				Convert_RGBPacked_To_RGB888(numPixels, gev_input_data, outdepth, rgb_output_data);
				break;
			case fmtBGR8Packed:
			case fmtBGR10Packed:
			case fmtBGR12Packed:
				Convert_BGRPacked_To_RGB888(numPixels, gev_input_data, outdepth, rgb_output_data);
				break;
		case fmtRGB10V1Packed:
				Convert_RGB10V1Packed_To_RGB888(numPixels, gev_input_data, rgb_output_data);
				break;
		case fmtRGB10V2Packed:
				Convert_RGB10V2Packed_To_RGB888(numPixels, gev_input_data, rgb_output_data);
				break;
			default:
				// Zero the output buffer until these are supported.
				memset(rgb_output_data, 0, numPixels*((outdepth + 7)/8));
				break;
		}
	}
}





