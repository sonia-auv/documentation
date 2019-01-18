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

fileutils.c			 											

Description:
   Posix file/device handling compatibility layer with Win32.

Platform:
	-Generic Posix.

History:
   1.00 October 17, 2003, parhug

$Log: fileutils.c $
Revision 1.6  2006/03/07 09:46:34  PARHUG
Added CheckFilePath function.
Revision 1.5  2005/01/07 11:37:41  parhug
Added DeleteFile as a function.
Revision 1.4  2004/11/12 15:01:21  parhug
Fix "rwmode" usage in CheckFileExists (it was reversed).
Revision 1.3  2004/11/03 09:27:18  parhug
Add GetFileSize function.
Revision 1.2  2004/10/06 16:55:01  parhug
Update for new handle model.
Revision 1.1  2004/08/19 12:26:07  parhug
Initial revision

*******************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __CELL__
#include "linuxobjs.h"
#else
#include <cordef.h>
#include <linuxobjs.h>
#endif

/**========================================================================**/
/*
* #define constants.
*/

/**========================================================================**/
/*
* Function globals.
*/

/**========================================================================**/
/*
* Function prototypes.
*/


/**============================== CODE ====================================**/
/*============================================================================
*
*  Name        : CheckFilePath
*  Description : This function creates a path to a file in the application's
*              working directory.
*
*  Parameters  :  inFile      : the input filename
*                 outFile     : fully expanded path name for file.
*
*  Returns     : True.
*
*  Context/Restrictions:
*===========================================================================*/
BOOL CheckFilePath( const char *inFile, char *outFile)
{
   // Check if filename contains full path or just a file name.
   if (strstr(inFile, FILESYSTEM_DELIMITER) == NULL)
   {
      char *path;
      int len;

      // Get application's path (current working directory).
      path = getenv("PWD");

      // Append application's path to filename
      len = sprintf(outFile, "%s%s%s", path, FILESYSTEM_DELIMITER, inFile);
      outFile[len] = '\0';
   }
   else
   {
      sprintf(outFile, "%s", inFile);
   }

   return TRUE;
}

/*============================================================================
*
*  Name        : CheckFileExists
*  Description : This function checks for the existence of the specified file
*					in the specified mode. If "rwmode" is non-zero (TRUE), the file
*					is checked for Read_Write access. If it is FALSE, the file is
*					checked for Read_Only access.
*
*  Parameters  :	inFile		: the input filename
*						rwmode		: TRUE if file is required to have R/W access.
*
*  Returns     : T/F for file accessibility in desired mode.
*
*  Context/Restrictions:
*===========================================================================*/
BOOL CheckFileExists( const char *inFile, BOOL rwmode)
{
	FILE *fp;
	struct stat statBuf;

	if ( stat( inFile, &statBuf) == 0)
	{
		// Check if it is a regular file.
		if ( S_ISREG(statBuf.st_mode) )
		{
			// Its a regular file.
			if ( rwmode )
			{
				fp = fopen( inFile, "r+");
				if ( fp == NULL)
				{
					return FALSE;
				}
				fclose(fp);
			}
			else
			{
				fp = fopen( inFile, "r");
				if ( fp == NULL )
				{
					return FALSE;
				}
				fclose(fp);
			}
			return TRUE;
		}
		else
		{
			// It is not a regular file (link, FIFO, directory, or device of some kind).
			return FALSE;
		}
	}
	else
	{
		// Cannot stat file. It must not exist (or is outside our permission mask).
		return FALSE;
	}
}

#ifndef __CELL__
/*============================================================================
*
*  Name        : CheckDeviceExists
*  Description : This function checks for the existence of the specified
(					"special"-file (corresponding to a device) in the specified
*					mode. If "rwmode" is non-zero (TRUE), the file
*					is checked for Read_Write access. If it is FALSE, the file is
*					checked for Read_Only access.
*
*  Parameters  :	inFile		: the input filename
*						rwmode		: TRUE if file is required to have R/W access.
*
*  Returns     : Void
*
*  Context/Restrictions:
*===========================================================================*/
BOOL CheckDeviceExists( const char *inFile, BOOL rwmode)
{
	int filedes;
	struct stat statBuf;

	if ( stat( inFile, &statBuf) == 0)
	{
		// Check if it is a device "special" file.
		if ( S_ISCHR(statBuf.st_mode) || S_ISBLK(statBuf.st_mode) )
		{
			// Its a device "special" file.
			if ( rwmode )
			{
				filedes = open( inFile, (O_RDONLY|O_NOCTTY));
				if ( filedes < 0)
				{
					return FALSE;
				}
				close(filedes);
			}
			else
			{
				filedes = open( inFile, (O_RDWR|O_NOCTTY));
				if ( filedes < 0 )
				{
					return FALSE;
				}
				close(filedes);
			}
			return TRUE;
		}
		else
		{
			// It is not a regular file (link, FIFO, directory, or device of some kind).
			return FALSE;
		}
	}
	else
	{
		// Cannot stat file. It must not exist (or is outside our permission mask).
		return FALSE;
	}
}

BOOL CloseFileObject( PHANDLEOBJ handle)
{
	// Check that handle is valid
	if ( VALID_HANDLEOBJ( DeviceObject, handle) )
	{
		close( handle->filedes);
		handle->filedes = 0;
		handle->file_attributes = 0;
	}
	else if (VALID_HANDLEOBJ(FileObject,handle))
	{
		fclose( handle->fp);
		handle->fp = NULL;
		handle->file_attributes = 0;
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CloseFile( HANDLE handle)
{
	if (HANDLE_IS_VALID(handle))
	{
		return CloseFileObject( HANDLE_REF(handle));
	}
	else
	{
		return FALSE;
	}
}


BOOL ReadFile( HANDLE hFile, void *buffer, ULONG bytesToRead,
					ULONG *bytesRead, void *overlapped)
{
	if ( (buffer != NULL) && (bytesRead != NULL) )
	{
		if ( VALID_HANDLE(DeviceObject,hFile))
		{
			*bytesRead = (ULONG) read( HANDLE_REF(hFile)->filedes, buffer, (size_t)bytesToRead);
			return TRUE;
		}
		else if (VALID_HANDLE(FileObject,hFile))
		{
			*bytesRead = (ULONG) fread( buffer, sizeof(char), (size_t)bytesToRead, HANDLE_REF(hFile)->fp);
			return TRUE;
		}
		else
		{
			// Not a file or a device.
			*bytesRead = 0;
			return FALSE;
		}
	}
	else
	{
		// Not able to read.
		if ( bytesRead != NULL)
		{
			*bytesRead = 0;
		}
		return FALSE;
	}
}

BOOL WriteFile( HANDLE hFile, void *buffer, ULONG bytesToWrite,
						ULONG *bytesWritten, void *overlapped)
{
	if ( (buffer != NULL) && (bytesWritten != NULL) )
	{
		if ( VALID_HANDLE(DeviceObject,hFile))
		{
			*bytesWritten = (ULONG) write( HANDLE_REF(hFile)->filedes, buffer, (size_t)bytesToWrite);
			return TRUE;
		}
		else if (VALID_HANDLE(FileObject,hFile))
		{
			*bytesWritten = (ULONG) fwrite( buffer, sizeof(char), (size_t)bytesToWrite, HANDLE_REF(hFile)->fp);
			return TRUE;
		}
		else
		{
			// Not a file or a device.
			*bytesWritten = 0;
			return FALSE;
		}
	}
	else
	{
		// Not able to write.
		if ( bytesWritten != NULL)
		{
			*bytesWritten = 0;
		}
		return FALSE;
	}
}

BOOL IsEndOfFile( HANDLE hFile )
{
	if ( VALID_HANDLE(FileObject, hFile) )
	{
		return (BOOL) feof(HANDLE_REF(hFile)->fp);
	}
	else
	{
		return FALSE;
	}
}

BOOL FlushFileBuffers( HANDLE hFile )
{
	if ( VALID_HANDLE(FileObject, hFile) )
	{
		return (BOOL) fflush(HANDLE_REF(hFile)->fp);
	}
	else
	{
		return FALSE;
	}
}

ULONG SetFilePointer( HANDLE hFile, LONG offsetLow, LONG *offsetHigh, ULONG method)
{
	ULONG current = 0;
	int origin;

	if ( !VALID_HANDLE(FileObject, hFile) )
	{
		return -1;
	}

	// Convert the inputs to the familiar fseek parameters.
	switch (method)
	{
		case FILE_CURRENT:
			origin = SEEK_CUR;
			current = ftell( HANDLE_REF(hFile)->fp);
			break;
		case FILE_END:
			origin = SEEK_END;
			current = ftell( HANDLE_REF(hFile)->fp);
			break;
		case FILE_BEGIN:
		default:
			origin = SEEK_SET;
			break;
	}

	if ( !fseek( HANDLE_REF(hFile)->fp, offsetLow, origin) )
	{
		// Succeeded - update the file position for return.
		return (current + offsetLow);
	}
	else
	{
		return -1;
	}
}

ULONG GetFileSize( HANDLE hFile, ULONG *lpFileSizeHigh)
{
	int length;
	if (!VALID_HANDLE(FileObject,hFile))
	{
		return INVALID_FILE_SIZE;
	}

	length = fseek( HANDLE_REF(hFile)->fp, 0, SEEK_END);

	// Only handle 32-bit file sizes for now!!!
	if ( lpFileSizeHigh != NULL)
	{
		*lpFileSizeHigh = 0;
	}

	return (ULONG)length;

}

BOOL DeleteFile( const char* lpFileName )
{
	if ( !remove( lpFileName) )
	{
		return TRUE;
	}
	return FALSE;
}
#endif
