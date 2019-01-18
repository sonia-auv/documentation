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

ioctlutils.c			 											

Description:
   Posix device ioctl handling compatibility layer with Win32.

Platform:
	-Generic Posix.

History:
   1.00 march 10, 2004, parhug

$Log: ioctlutils.c $
Revision 1.3  2004/11/10 09:20:07  parhug
Verified handle before use.
Revision 1.2  2004/10/06 16:56:50  parhug
Update for new handle model.
Revision 1.1  2004/08/19 12:26:08  parhug
Initial revision

*******************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cordef.h>
#include <GDrv_Common.h>

#include "linuxobjs.h"
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
BOOL DeviceIoControl( HANDLE handle, ULONG ioctlCode,
						void *inputBuffer, ULONG inputBufSize,
						void *outputBuffer, ULONG outputBufSize,
						ULONG *bytesTransferred, void *overlap)
{
	DEVIOCTL dioc;
	ULONG bytes;

	if (HANDLE_IS_VALID(handle))
	{
		// Fill in the I/O control structure.
		dioc.inBuffer = inputBuffer;
		dioc.inputBufSize = inputBufSize;
		dioc.outBuffer = outputBuffer;
		dioc.outputBufSize = outputBufSize;
		dioc.bytesTransferred = (ULONG *)&bytes;

		// Send the ioctl to the driver.
		if ( ioctl( HANDLE_REF(handle)->filedes, ioctlCode, &dioc) == 0)
		{
			if ( bytesTransferred != (ULONG *)NULL)
			{
				*bytesTransferred = bytes;
			}
			return TRUE;
		}
	}

	// Failed, no data moved.
	if ( bytesTransferred != (ULONG *)NULL)
	{
		*bytesTransferred = 0;
	}
	return FALSE;

}




