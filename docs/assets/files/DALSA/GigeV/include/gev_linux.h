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
	-Neither the name of Teledyne DALSA nor the names of its contributors 
	may be used to endorse or promote products derived 
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

/*! \file gev_linux.h
\brief Linux-specific information

  This header file contains some definitions associated to Linux.

*/


#ifndef _GEV_LINUX_H_
#define _GEV_LINUX_H_		//!< used to avoid multiple inclusion

//====================================================================
// INCLUDE FILES
//====================================================================

#ifndef __cplusplus
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>

#include <poll.h>

#include <sched.h>
//#include <linux/tcp.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <sys/user.h>
#include <sys/mman.h>
#include <asm/byteorder.h>

#include <stdarg.h>
#include <string.h>

// Fix up re-definitions shared between "cordef.h" and Linux.
#undef INT8_MAX
#undef INT8_MIN
#undef UINT8_MAX
#undef INT16_MAX
#undef INT16_MIN
#undef UINT16_MAX
#undef INT32_MAX
#undef INT32_MIN
#undef UINT32_MAX

// Win32 API for Linux definitions.
#include "corenv.h"
#include "cordef.h"
#include "corposix.h" 

//====================================================================
// CONSTANTS
//====================================================================

typedef UINT16				GEV_STATUS;
typedef INT16				GEVLIB_STATUS;

typedef unsigned SOCKET, *PSOCKET;
//typedef unsigned char *LPCTSTR;

#define _stdcall

#define closesocket close 

#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR -1

// Windows basic types
#define _EVENT 				HANDLE				//!< _EVENT definition for Linux
#define _THREAD 				HANDLE				//!< _THREAD definition for Linux
#define _MUTEX 				HANDLE				//!< _MUTEX definition for Linux
#define _CRITICAL_SECTION 	CRITICAL_SECTION	//!< _CRITICAL_SECTION definition for Linux

// Thread priorities
#define _THREAD_PRIORITY_REAL_TIME	THREAD_PRIORITY_TIME_CRITICAL
#define _THREAD_PRIORITY_HIGH			THREAD_PRIORITY_HIGHEST
#define _THREAD_PRIORITY_NORMAL		THREAD_PRIORITY_NORMAL
#define _THREAD_PRIORITY_LOW			THREAD_PRIORITY_BELOW_NORMAL

//=============================================================================
// Packet Interface definitions. 
// Information to filter UDP packets when using the PF_PACKET interface.
// All IP DGRAM packets on this interface (for the local host) will be read.
// We only need those matching UDP with a specific src/dst and a single 
//	signature match (dump the rest).
//
typedef struct _GevPktFilter_t
{
	UINT32 	srcAddress;
	UINT32 	dstAddress;
	UINT16 	srcPort;
	UINT16 	dstPort;
	int 		check_signature;
	int 		signature_offset;
	int 		signature_size;
	int		signature_mask;
	int		signature_value;
} GevPktFilter_t, GEVPKT_FILTER, *PGEVPKT_FILTER;

typedef struct _GevRxPkt_t
{
	GevPktFilter_t filter;
	SOCKET			packet_socket;		// Socket for PF_PACKET network access to GVSP.
	SOCKET			gvsp_socket; 		// Socket for regular network access to GVSP (PF_PACKET not available).
	BOOL 	         use_packet_interface;
	BOOL 	         use_mmap_interface;
	int 	         netIf_index;
	int 	         buffer_size;	// Size of allocate "buffer".
	int 	         offset;			// Offset for size alignment
	int				mtu_size;		// Maximum packet size from NIC.
	int				gvsp_size;		// Maximum packet size for GVSP.
	void	         *buffer;
	void	         *mmap_ptr;	// NIC driver ring buffer pointer (mmap'd)
	int	         ring_size;	// Ring buffer size (object count).
	int				tp_version; // version of tpacket_hdr struct for mmaped ring.
	int				tp_hdrlen;  // header length of tpacket_hdr struct for mmaped ring.
	int				kernel_bufsize;	// Size of kernel buffer for mmapd ring (bytes). 
} GevRxPkt_t, GEVPKT_RX, *PGEVPKT_RX;

#define SLL_HDR_LEN 16
#if defined(__arm__)
	#ifndef PAGE_SIZE
		#define PAGE_SIZE 4096
	#endif
#endif

// Information about a packet.
typedef enum
{
	gvspNone    = 0,
	gvspLeader  = 1,
	gvspTrailer = 2,
	gvspPayload = 3
} GVSP_PACKET_TYPE;

typedef struct _GVSPPKT_INFO
{
	BOOL					valid;
	UINT16				status;
	GVSP_PACKET_TYPE	type;
	USHORT				dstPort;
	USHORT				block_id;
	UINT32				packet_id;
} GVSPPKT_INFO, *PGVSPPKT_INFO;


//====================================================================
// Prototypes (OS-specific code).
//====================================================================

GEV_STATUS GevSendMessage (UINT8 *pDatagram, int size, SOCKET *pSocket, struct sockaddr_in *pDest, BOOL showData);
GEV_STATUS GevBroadcastMessage (UINT8 *pDatagram, int size, SOCKET *pSocket, struct sockaddr_in *pDest, BOOL showData);
GEV_STATUS GevReceiveMessage (UINT8 *pDatagram, int maxSize, SOCKET *pSocket, struct timeval *pTimeout, UINT32 *pAddr, UINT16 *pPort, BOOL showData);
GEV_STATUS GevReceiveMessage_OnSocket (UINT8 *pDatagram, int maxSize, SOCKET *pSocket, int *numBytes, struct timeval *pTimeout);


#endif // _GEV_LINUX_H_

