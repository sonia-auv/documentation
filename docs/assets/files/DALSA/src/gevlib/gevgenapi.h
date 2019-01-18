/****************************************************************************** 
Copyright (c) 2008-2015, Teledyne DALSA Inc.
All rights reserved.

File: gevgenapi.h
	Public API for GenICam access to GigEVision C library.

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

/*! \file gevgenapi.h
\brief GEV GENAPI definitions.

*/

#ifndef _GEVGENAPI_H_
#define _GEVGENAPI_H_			

#ifdef USE_EMBEDDED_GENAPI
	// Use an embedded GenApi interface (stripped down implementation)
	USE_EMBEDDED_GENAPI is not implemented yet - do not use !
#else
	// Use the standard GenApi interface (all the GenICam standard components).
	#include "GenApi/GenApi.h"
#endif

#include "gevapi.h"

class CGevPort : public GenApi::IPort
{
public:
	virtual GenApi::EAccessMode GetAccessMode() const;

	// GVCP ReadReg / ReadMem from camera via handle.
	virtual void Read(void *pBuffer, int64_t Address, int64_t Length);

	// GVCP WriteReg / WriteMem from camera via handle.
	virtual void Write(const void *pBuffer, int64_t Address, int64_t Length);

	//! Constructor
	CGevPort(GEV_CAMERA_HANDLE handle) : m_handle(handle)
	{}

	//! Destructor
	virtual ~CGevPort() {}
	

protected:
	GEV_CAMERA_HANDLE m_handle ;

};

#endif
