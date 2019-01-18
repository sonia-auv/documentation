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

/*! \file gev_packet_mmap.c
\brief Helper functions to receive packets using the AF_PACKET 
       socket API (with RX_RING and MMAP enabled).

*/


//====================================================================
// INCLUDE FILES
//====================================================================
#define GEVLIB_DEFS	0
#include "gevapi.h"

#include "linux/if_ether.h"
#include "linux/ip.h"
#include "linux/udp.h"


// Set up definitions for what exists on the system being compiled on.
// Ideally, we are looking for PF_PACKET support and support for mmap of a receive
// ring buffer (the RX_RING functionality with CONFIG_PACKET_MMAP set in the kernel).
//
#ifdef PF_PACKET

	#include <linux/if_packet.h>

	#ifdef PACKET_HOST
		#define HAVE_PF_PACKET_SOCKETS
		#ifdef PACKET_AUXDATA
			#define HAVE_PACKET_AUXDATA
		#endif
	#endif

	#ifdef TPACKET_HDRLEN
		#define HAVE_PACKET_RING
		#ifdef TPACKET2_HDRLEN
			#define HAVE_TPACKET2
		#else
			#define TPACKET_V1	0
		#endif
	#endif
#endif

/* Keep the VLAN tag concept (for now) */
#define VLAN_TAG_LEN	4

union thdr
{
	struct tpacket_hdr	*h1;
	struct tpacket2_hdr	*h2;
	void						*raw;
};


#ifdef HAVE_PACKET_RING
#define RING_GET_FRAME(h) (((union thdr **)h->buffer)[h->offset])
#endif

BOOL _FilterPacket( GevPktFilter_t *pFilter, void *pPacket, void **pDataOut, int *numBytes);

//====================================================================
// PACKET INTERFACE FUNCTIONS
//====================================================================

static void Compute_RxRing_BlockSize(int packet_size, unsigned int *block_size, unsigned int *packets_per_block)
{
	// Compute the minimum block size that will handle the packet size.
	// Constraints/caveats are :
	// 	a) Memory block must have page size granularity.
	//		b) The kernel enforces a maximum block size that we don't know.
	// 		(so we may need to reduce this if it fails.)
	//	 	c) Gev cameras uses only 15 bits for maximum packet size (0x8000) and
	// 	   DALSA uses 9144 bytes max (so 3 pages (of 4K) maximum).
	//
	*block_size = getpagesize();
	while (*block_size < packet_size) 
		*block_size <<= 1;

	*packets_per_block = *block_size/packet_size;
}



//GevRxPkt_t *pRxInfo;
//! Destroy the mmap'd RX_RING usign the socket interface.
/*!
	This function is used to disable the RX_RING mechanism and unmap the ring 
   buffer(s) for the socket that was originally opened with PF_PACKET, RX_RING, 
	and MMAP capabilities enabled.

	\param [out]     pSocket          Pointer to socket for stream channel
	\param [in/out]  pPktInfo         Pointer to GevRxPkt_t structure for managing recv on PF_PACKET socket.
	\return GEV status code
	\note None
*/
static void GevDestroy_RxRing(SOCKET *pSocket, GevRxPkt_t *pPktInfo)
{
	struct tpacket_req req;

	if ( (pSocket != NULL) && (pPktInfo != NULL) )
	{
		// Destroy the RxRing
		memset(&req, 0, sizeof(req));
		setsockopt(*pSocket, SOL_PACKET, PACKET_RX_RING, (void *)&req, sizeof(req));

		// Unmap RxRing (if mapped)
		if (pPktInfo->mmap_ptr) 
		{
			// Get the ring size from the creation info.
			unsigned packets_per_block, block_size;
			Compute_RxRing_BlockSize(pPktInfo->buffer_size, &block_size, &packets_per_block);

			// Unmap all the ring memory mmap'd in earlier.
			munmap(pPktInfo->mmap_ptr, block_size * pPktInfo->ring_size / packets_per_block);
			pPktInfo->mmap_ptr = NULL;
		}
	}
}

static GEV_STATUS GevPreparePacketSocket(SOCKET *pSocket, GevRxPkt_t *pPktInfo )
{
	GEV_STATUS status = GEVLIB_STATUS_ERROR;
#ifdef HAVE_TPACKET2
	socklen_t len;
	int val;
#endif

	pPktInfo->tp_version = TPACKET_V1;
	pPktInfo->tp_hdrlen = sizeof(struct tpacket_hdr);

#ifdef HAVE_TPACKET2
	/* Probe whether kernel supports TPACKET_V2 */
	val = TPACKET_V2;
	len = sizeof(val);
	if (getsockopt( *pSocket, SOL_PACKET, PACKET_HDRLEN, &val, &len) < 0) 
	{
		/* Failed setting packet HDRLEN */
		if (errno == ENOPROTOOPT)
		{
			/* Option not present - means TPACKET_V2 not available (but TPACKET_V1 was - so return OK to use that version. */
			return GEVLIB_SUCCESS;
		}
		else
		{
			GevPrint( GEV_LOG_ERROR, __FILE__, __LINE__, "GevPreparePacketSocket : Failed to get TPACKET_V2 header len on socket %d: %d-%s",
						 *pSocket, errno, strerror(errno));
			return status;
		}
	}
	pPktInfo->tp_hdrlen = val;

	val = TPACKET_V2;
	if (setsockopt(*pSocket, SOL_PACKET, PACKET_VERSION, &val, sizeof(val)) < 0) 
	{
		GevPrint( GEV_LOG_ERROR, __FILE__, __LINE__, "GevPreparePacketSocket : Failed activate TPACKET_V2 on socket %d: %d-%s",
			 *pSocket, errno, strerror(errno));
		return status;
	}
	pPktInfo->tp_version = TPACKET_V2;

	/* Reserve space for VLAN tag reconstruction (are these required ???) */
	val = VLAN_TAG_LEN;
	if (setsockopt(*pSocket, SOL_PACKET, PACKET_RESERVE, &val, sizeof(val)) < 0) 
	{
		GevPrint( GEV_LOG_ERROR, __FILE__, __LINE__, "GevPreparePacketSocket : Failed to set up reserve on socket %d: %d-%s",
			 *pSocket, errno, strerror(errno));
		return status;
	}

#endif /* HAVE_TPACKET2 */
	return GEVLIB_SUCCESS;
}

static GEV_STATUS GevCreateMmapRxRing(SOCKET *pSocket, GevRxPkt_t *pPktInfo )
{
	unsigned i, j, ringsize, frames_per_block;
	int done = FALSE;
	struct tpacket_req req;

	/* Note that with large snapshot (say 64K) only a few frames 
	 * will be available in the ring even with pretty large ring size
	 * (and a lot of memory will be unused). 
	 * The snap len should be carefully chosen to achive best
	 * performance 
	 *
	 * Here, I am using the mtu_size setting as the snap length since
	 * GVSP activity is all multiples of  packet size that gets set 
	 * (ideally) to the mtu size of the NIC interface.
	 * NOTE : I needed to add an amount to align to a higher value 
	 * 	(it was truncating 20 bytes so I added 32).
	*/
	req.tp_frame_size = TPACKET_ALIGN(pPktInfo->mtu_size + (2*TPACKET_ALIGNMENT) + 
						TPACKET_ALIGN(pPktInfo->tp_hdrlen) +
					  	sizeof(struct sockaddr_ll));
	req.tp_frame_nr = pPktInfo->kernel_bufsize/req.tp_frame_size;

	Compute_RxRing_BlockSize(req.tp_frame_size, &req.tp_block_size, &frames_per_block);
	req.tp_block_nr = req.tp_frame_nr / frames_per_block;

	/* req.tp_frame_nr is requested to match frames_per_block*req.tp_block_nr */
	req.tp_frame_nr = req.tp_block_nr * frames_per_block;

	/* Have the kernel create RxRing mechanism. */
	while(!done)
	{
		if (setsockopt(*pSocket, SOL_PACKET, PACKET_RX_RING, (void *) &req, sizeof(req))) 
		{
			/* RxRing creation failed */
			if ((errno == ENOMEM) && (req.tp_block_nr > 1)) 
			{
				/* Reduce the size of the RxRing (so we can try again) */
				req.tp_frame_nr >>= 1;
				req.tp_block_nr = req.tp_frame_nr/frames_per_block;
			}
			else
			{
				/* Memory allocation failure or other error */
				GevPrint( GEV_LOG_ERROR, __FILE__, __LINE__, "GevCreateMmapRing : Failed to create RxRing on "
						"packet socket %d: %d-%s", *pSocket, errno, strerror(errno));
				return GEVLIB_ERROR_INSUFFICIENT_MEMORY;
			}
		}
		else
		{
			done = TRUE;
		}
	}

	/* Mmap the RxRing structure */
	ringsize = req.tp_block_nr * req.tp_block_size;
	pPktInfo->mmap_ptr = mmap(0, ringsize, PROT_READ| PROT_WRITE, MAP_SHARED, *pSocket, 0);
	if (pPktInfo->mmap_ptr == MAP_FAILED) 
	{
		GevPrint( GEV_LOG_ERROR, __FILE__, __LINE__, "GevCreateMmapRing : Failed to mmap RxRing: %d-%s",
			errno, strerror(errno));

		/* Clean-up the allocated ring on error*/
		GevDestroy_RxRing(pSocket, pPktInfo);
		return GEVLIB_ERROR_RESOURCE_ACCESS;
	}

	/* Allocate a ring for each frame header pointer */
	pPktInfo->ring_size = req.tp_frame_nr;
	pPktInfo->buffer = malloc(pPktInfo->ring_size * sizeof(union thdr *));
	if (!pPktInfo->buffer) 
	{
		GevDestroy_RxRing(pSocket, pPktInfo);
		return GEVLIB_ERROR_INSUFFICIENT_MEMORY;
	}

	/* fill the header ring with proper frame ptr*/
	pPktInfo->offset = 0;
	for (i=0; i<req.tp_block_nr; ++i) 
	{
		PUINT8 ptr = (PUINT8)pPktInfo->mmap_ptr;
		void *base = &ptr[i*req.tp_block_size];
		for (j=0; j<frames_per_block; ++j, ++pPktInfo->offset) 
		{
			RING_GET_FRAME(pPktInfo) = base;
			base += req.tp_frame_size;
		}
	}

	pPktInfo->buffer_size = req.tp_frame_size;
	pPktInfo->offset = 0;
	pPktInfo->use_packet_interface = TRUE;
	pPktInfo->use_mmap_interface = TRUE;
	return GEVLIB_SUCCESS;
}

//static inline union thdr * GevGetRxRingFrame( GevRxPkt_t *pPktInfo, int status) 
static union thdr * GevGetRxRingFrame( GevRxPkt_t *pPktInfo, int status) 
{
	union thdr h;

	h.raw = RING_GET_FRAME(pPktInfo);
	switch (pPktInfo->tp_version) 
	{
		default:
		case TPACKET_V1:
			{
				if (status != (h.h1->tp_status ? TP_STATUS_USER : 	TP_STATUS_KERNEL))
					return NULL;
			}
			break;
	#ifdef HAVE_TPACKET2
		case TPACKET_V2:
			{
				if (status != (h.h2->tp_status ? TP_STATUS_USER : TP_STATUS_KERNEL))
					return NULL;
			}
			break;
	#endif
	}
	return h.raw;
}


					
//! Clean up the mmap'd RX_RING socket interface.
/*!
	This function is used to shut down and close a socket, used with the streaming camera 
	interface, that was originally opened with PF_PACKET, RX_RING, and MMAP capabilities
	enabled.

	\param [out]     pSocket          Pointer to socket for stream channel
	\param [in/out]  pPktInfo         Pointer to GevRxPkt_t structure for managing recv on PF_PACKET socket.
	\return GEV status code
	\note None
*/
void GevCloseRxRingMmap( SOCKET *pSocket, GevRxPkt_t *pPktInfo)
{
	GevDestroy_RxRing(pSocket, pPktInfo);
}

//! Set up a high performance socket for the streaming interface to use.
/*!
	This function is used to open a socket for use with the streaming camera interface.
	An attempt is made to also use the PF_PACKET interface with an RX_RING socket option
   as a high performance channel in addition to the standard UDP socket connection.

	\param [out]     pSocket          Pointer to socket for stream channel
	\param [in/out]  pPktInfo         Pointer to GevRxPkt_t structure for managing recv on PF_PACKET socket.
	\return GEV status code
	\note None
*/
GEV_STATUS GevSetupRxRingMmap( SOCKET *pSocket, GevRxPkt_t *pPktInfo )
{
	GEV_STATUS status = GEVLIB_STATUS_ERROR;

	if (pPktInfo != NULL)
	{
#ifdef HAVE_PACKET_RING
		if (pPktInfo->kernel_bufsize == 0) 
		{
			// If not specified : use 4M for the kernel ring buffer 
			pPktInfo->kernel_bufsize = 4*1024*1024;
		}
		status = GevPreparePacketSocket(pSocket, pPktInfo ); 
		if (status == GEVLIB_SUCCESS)
		{
			status = GevCreateMmapRxRing(pSocket, pPktInfo );
		}
		return status;
#endif /* HAVE_PACKET_RING */
	}
	return status;
}


#ifdef HAVE_PACKET_RING

static void SwitchToNextFramePacket(GevRxPkt_t *pPktInfo, union thdr *h)
{
	if (pPktInfo != NULL)
	{
		// Switch to the next packet.
		switch (pPktInfo->tp_version) 
		{
			case TPACKET_V1:
					h->h1->tp_status = TP_STATUS_KERNEL;
					break;
		#ifdef HAVE_TPACKET2
				case TPACKET_V2:
					h->h2->tp_status = TP_STATUS_KERNEL;
					break;
		#endif
		}

		if (++pPktInfo->offset >= pPktInfo->ring_size) 
		{
			pPktInfo->offset = 0;
		}
	}
}

//?????DEBUG?????
static void dumpdata( void *ptr)
{
	int i;
	UINT32 *lptr = (UINT32 *)ptr;

	for (i = 0; i < 64; i+=8)
	{
		printf("%02d : 0x%08x  0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n", i, lptr[i+0], lptr[i+1], lptr[i+2], lptr[i+3], lptr[i+4], lptr[i+5], lptr[i+6], lptr[i+7]); 
	}
}
//?????DEBUG???????


GEV_STATUS GevReadMmapPacket( GevRxPkt_t *pPktInfo, int *numBytes, void *dataBuffer, struct timeval *pTimeout )
{
	GEV_STATUS status = GEVLIB_ERROR_NULL_PTR;
	int timeout = 0;
	union thdr h;
	BOOL keepPacket = FALSE;

	if (pTimeout != NULL)
	{
		timeout = (pTimeout->tv_usec / 1000) + (pTimeout->tv_sec * 1000);
	}

	if ( (pPktInfo == NULL) || (numBytes == NULL) || (dataBuffer == NULL))
	{
		return status;
	}
	if (!pPktInfo->use_packet_interface  || (pPktInfo->mmap_ptr == NULL))
	{
		// Packet interface is not enabled (why did this get called if that is the case?)
		status = GEVLIB_ERROR_SOFTWARE;
		return status;
	}

	// Try to get a packet frame.
	// We will get it and copy it one packet at a time (applying our filter to it).
	h.raw = GevGetRxRingFrame(pPktInfo, TP_STATUS_USER);
	
	if (h.raw == NULL)
	{
		// Unable to get a frame. Poll for one (with timeout).
		struct pollfd pollit;
		pollit.fd = pPktInfo->packet_socket;
		pollit.events = POLLIN;
		status = poll(&pollit, 1, timeout); 
		if ( status <= 0)
		{
			// Timeout - or other error
			status = GEV_STATUS_NO_MSG;
			*numBytes = 0;
			return status;
		}
		
		// Data is now ready.
		h.raw = GevGetRxRingFrame(pPktInfo, TP_STATUS_USER);
	}
		
	if (h.raw == NULL)
	{
		// This should not happen because we waited for it !
		status = GEV_STATUS_NO_MSG;
		*numBytes = 0;
		return status;
	}
	else
	{
		struct sockaddr_ll *sll;
		unsigned char *ptr;
		unsigned int tp_len;
		unsigned int tp_mac;
		unsigned int tp_snaplen;

		// Get information on the received packet.
		switch (pPktInfo->tp_version) 
		{
			case TPACKET_V1:
				tp_len	  = h.h1->tp_len;
				tp_mac	  = h.h1->tp_mac;
				tp_snaplen = h.h1->tp_snaplen;
				break;
	#ifdef HAVE_TPACKET2
			case TPACKET_V2:
				tp_len	  = h.h2->tp_len;
				tp_mac	  = h.h2->tp_mac;
				tp_snaplen = h.h2->tp_snaplen;
				break;
	#endif
			default:
				status = GEVLIB_ERROR_DATA_INVALID_HEADER; /* Bad packet type */
				SwitchToNextFramePacket(pPktInfo, &h); 
				return status;
		}

		/* Check for (potential) buffer overflow */
		if (pPktInfo->buffer_size < (tp_snaplen + tp_mac)) 
		{
			status = GEVLIB_ERROR_DATA_OVERRUN; /* Packet overflow = RxRing is corrupted */
			SwitchToNextFramePacket(pPktInfo, &h); 
			return status;
		}

		// Received a candidate packet - perform the filtering 
		sll = (void *)h.raw + TPACKET_ALIGN( pPktInfo->tp_hdrlen);

		if ( (sll->sll_ifindex == pPktInfo->netIf_index) &&
			  (sll->sll_pkttype == PACKET_HOST) )
		{
			// This packet is on the correct interface and in the correct direction.
			// It is a candidate for content filtering!!
			// Perform the content filtering operation.
			if (tp_len > 0)
			{
//dumpdata(ptr);
				int nB = 0;
				void *pktData = NULL;

				ptr = (unsigned char*)h.raw + tp_mac;

				keepPacket = _FilterPacket( &pPktInfo->filter, ptr, &pktData, &nB);	
				if (keepPacket)
				{
					// Copy the received (and filtered) packet in the data buffer
					memcpy( dataBuffer, pktData, nB);
					*numBytes = nB;

					// Switch the ring buffer to the next packet.
					SwitchToNextFramePacket(pPktInfo, &h); 
					status = GEV_STATUS_SUCCESS;
					return status;
				}
			}
		}

		// Receive data is filtered out. If we did read some data it would already have been returned.
		status = GEV_STATUS_NO_MSG;
		*numBytes = 0;

		// Switch to the next packet.
		SwitchToNextFramePacket(pPktInfo, &h); 
	}
	return status;
}


//! Receive a data from the mmap'd packet ring buffer. 
/*!
	This function is used to receive data from the mmap'd packet ring buffer
	used by the RxRing interface to the PF_PACKET protocol. No filtering is
	done by this function.

	\param [in]  pPktInfo  	Pointer to packet socket interface info structure.
	\param [in]  numBytes  	Pointer to receive the number of bytes available.
	\param [in]  dataBuffer Pointer to receive the data available (MTUsize bytes).
	\param [in]  pTimeout  	Pointer to a timeval structure for timeout handling.
	\return GEV status code
	\note None
*/
GEV_STATUS GevRecvMmapPacket( GevRxPkt_t *pPktInfo, int *numBytes, void *dataBuffer, struct timeval *pTimeout )
{
	GEV_STATUS status = GEVLIB_ERROR_NULL_PTR;
	int timeout = 0;
	union thdr h;

	if (pTimeout != NULL)
	{
		timeout = (pTimeout->tv_usec / 1000) + (pTimeout->tv_sec * 1000);
	}

	if ( (pPktInfo == NULL) || (numBytes == NULL) || (dataBuffer == NULL))
	{
		return status;
	}
	if (!pPktInfo->use_packet_interface  || (pPktInfo->mmap_ptr == NULL))
	{
		// Packet interface is not enabled (why did this get called if that is the case?)
		status = GEVLIB_ERROR_SOFTWARE;
		return status;
	}

	// Try to get a packet frame.
	// We will get it and copy it one packet at a time (applying our filter to it).
	h.raw = GevGetRxRingFrame(pPktInfo, TP_STATUS_USER);
	
	if (h.raw == NULL)
	{
		// Unable to get a frame. Poll for one (with timeout).
		struct pollfd pollit;
		pollit.fd = pPktInfo->packet_socket;
		pollit.events = POLLIN;
		status = poll(&pollit, 1, timeout); 
		if ( status <= 0)
		{
			// Timeout - or other error
			status = GEV_STATUS_NO_MSG;
			*numBytes = 0;
			return status;
		}
		
		// Data is now ready.
		h.raw = GevGetRxRingFrame(pPktInfo, TP_STATUS_USER);
	}
		
	if (h.raw == NULL)
	{
		// This should not happen because we waited for it !
		status = GEV_STATUS_NO_MSG;
		*numBytes = 0;
		return status;
	}
	else
	{
		struct sockaddr_ll *sll;
		unsigned char *ptr;
		unsigned int tp_len;
		unsigned int tp_mac;
		unsigned int tp_snaplen;

		// Get information on the received packet.
		switch (pPktInfo->tp_version) 
		{
			case TPACKET_V1:
				tp_len	  = h.h1->tp_len;
				tp_mac	  = h.h1->tp_mac;
				tp_snaplen = h.h1->tp_snaplen;
				break;
	#ifdef HAVE_TPACKET2
			case TPACKET_V2:
				tp_len	  = h.h2->tp_len;
				tp_mac	  = h.h2->tp_mac;
				tp_snaplen = h.h2->tp_snaplen;
				break;
	#endif
			default:
				status = GEVLIB_ERROR_DATA_INVALID_HEADER; /* Bad packet type */
				SwitchToNextFramePacket(pPktInfo, &h); 
				return status;
		}

		/* Check for (potential) buffer overflow */
		if (pPktInfo->buffer_size < (tp_snaplen + tp_mac)) 
		{
			status = GEVLIB_ERROR_DATA_OVERRUN; /* Packet overflow = RxRing is corrupted */
			SwitchToNextFramePacket(pPktInfo, &h); 
			return status;
		}

		// Received a candidate packet - perform the filtering 
		sll = (void *)h.raw + TPACKET_ALIGN( pPktInfo->tp_hdrlen);

		if ( (sll->sll_ifindex == pPktInfo->netIf_index) &&
			  (sll->sll_pkttype == PACKET_HOST) )
		{
			// This packet is on the correct interface and in the correct direction.
			// It is a candidate for content filtering!!
			if (tp_len > 0)
			{
//dumpdata(ptr);
				ptr = (unsigned char*)h.raw + tp_mac;

				// Copy the received (and filtered) packet in the data buffer
				// All packets are mtu_size.
				memcpy( dataBuffer, ptr, pPktInfo->mtu_size);
				*numBytes = pPktInfo->mtu_size;

				// Switch the ring buffer to the next packet.
				SwitchToNextFramePacket(pPktInfo, &h); 
				status = GEV_STATUS_SUCCESS;
				return status;
			}
		}

		// Receive data is filtered out. If we did read some data it would already have been returned.
		status = GEV_STATUS_NO_MSG;
		*numBytes = 0;

		// Switch to the next packet.
		SwitchToNextFramePacket(pPktInfo, &h); 
	}
	return status;
}




#endif /* HAVE_PACKET_RING */









