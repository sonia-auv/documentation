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

/*! \file gev_packet_linux.c
\brief Helper functions to receive packets using the AF_PACKET 
       socket API

  These functions are shared by the camera and the application.
  They provide a simple interface to send and receive message.
  This is true for the control, stream and message channels.

*/


//====================================================================
// INCLUDE FILES
//====================================================================
#define GEVLIB_DEFS	0
#include "gevapi.h"
#include "gevapi_internal.h"

#include "linux/if_ether.h"
#include "linux/ip.h"
#include "linux/udp.h"

//! Macro to get a BYTE from the datagram
#define GET_BYTE( p, o ) \
	( *( UINT8 * )( ( (UINT8 *)(p) ) + o ) )

#define GET_DWORD( p, o ) \
	( *( UINT32 * )( ( (UINT32 *)(p) ) + o ) )


#ifdef PF_PACKET
// Set up definitions for what exists on the system being compiled on.
// Ideally, we are looking for PF_PACKET support and support for mmap of a receive
// ring buffer (the RX_RING functionality with CONFIG_PACKET_MMAP set in the kernel).
	#include <linux/if_packet.h>
#endif

GEV_STATUS Gev_Stream_DecodeGVSPHeader( PUINT8 header, PUINT8 format, USHORT *block_id, UINT32 *packet_id, USHORT *packet_status, USHORT *payload_type);

GEV_STATUS GevRecvMmapPacket( GevRxPkt_t *pPktInfo, int *numBytes, void *dataBuffer, struct timeval *pTimeout );
GEV_STATUS GevSetupRxRingMmap( SOCKET *pSocket, GevRxPkt_t *pPktInfo );
void GevCloseRxRingMmap( SOCKET *pSocket, GevRxPkt_t *pPktInfo);

//====================================================================
// PACKET LAYER INTERFACE FUNCTIONS
//====================================================================

//! Open a socket for the streaming interface to use.
/*!
	This function is used to open a socket for use with the streaming camera interface.
	An attempt is made to also use the PF_PACKET interface with an RX_RING socket option
   as a high performance channel in addition to the standard UDP socket connection.

	\param [out]     pSocket          Pointer to socket for stream channel
	\param [in]      interfaceIndex   Index (sequential) for network interface.
	\param [in/out]  pPktInfo         Pointer to GevRxPkt_t structure for managing recv on PF_PACKET socket.
	\return GEV status code
	\note None
*/
GEV_STATUS GevOpenStreamingChannelSocket( SOCKET *pSocket, PGEV_NETWORK_INTERFACE pNetIf, UINT16 streamPort, GevRxPkt_t *pPktInfo )
{
	GEV_STATUS status = GEVLIB_ERROR_NULL_PTR;
	struct sockaddr_in sGVSPSource;

	if ( (pSocket != NULL) && (pNetIf != NULL) && (pPktInfo != NULL))
	{	
		status = GEV_STATUS_SUCCESS;
		pPktInfo->use_packet_interface = FALSE;

		// Create a UDP socket to receive image data
		*pSocket = socket (PF_INET, SOCK_DGRAM, 0);
		if (*pSocket == INVALID_SOCKET)
		{
			LogPrint( GEV_LOG_ERROR, "ERROR: Unable to create application streaming socket (error %d)\n\n\b", _GetSocketError ());
			status = GEV_STATUS_ERROR;
		}
		else
		{
			int flag = 1;
			setsockopt( *pSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag));
		}

		// Note : this may or may not help (doesn't hurt though).
		if (1)
		{
			// Disable Nagle's algorithm (has no real effect on UDP)
			int flag = 1;
			setsockopt( *pSocket, IPPROTO_UDP, TCP_NODELAY, (char *)&flag, sizeof(flag));
		}


		// Set source address for datagram and bind
		if (status == GEV_STATUS_SUCCESS)
		{
			sGVSPSource.sin_family = PF_INET;
			sGVSPSource.sin_port = htons((UINT16)streamPort);
			sGVSPSource.sin_addr.s_addr = htonl(pNetIf->ipAddr);
			if (bind (*pSocket, (struct sockaddr *) &sGVSPSource, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
			{
				LogPrint( GEV_LOG_ERROR, "ERROR: Unable to bind camera streaming socket (error %d)\n\n\b", _GetSocketError ());
				status = GEV_STATUS_ERROR;
			}
			
			pPktInfo->gvsp_socket = *pSocket;
		}


		//===========================
		// Set for Non-blocking I/O in Linux 
		// (not really important if PF_PACKET interface is also used).
		if (1)
		{
#ifndef O_DIRECT
		#define O_DIRECT	 040000
#endif
			// Set the socket to direct I/O and non-blocking
			int options = fcntl(*pSocket, F_GETFL);
			options |= (O_NONBLOCK | O_DIRECT);
			fcntl(*pSocket, F_SETFL, options);
		}
		//==========================

		// Attempt to open a packet socket to intercept the streaming packets before they hit the network stack.
		// Program needs privileges (UID 0 (sudo/root) or CAP_NET_RAW)
		if (1)
		{
			SOCKET soPacket;
			status = GevOpenPacketSocket( &soPacket, pNetIf->ifIndex, pPktInfo );

			if ((soPacket == INVALID_SOCKET) || (status != GEV_STATUS_SUCCESS))
			{
				closesocket(soPacket);
			}
			else
			{
				pPktInfo->packet_socket = soPacket;
				pPktInfo->use_packet_interface = TRUE;

				// Keep regular socket operation from affecting packet socket access 
				// (by loading the network stack downstream).
				//??????not necessssary??????? shutdown(*pSocket, SHUT_RDWR); 

			}
			status = GEV_STATUS_SUCCESS;
		}

	}
	return status;
}


//! Open a packet socket for the streaming interface to use.
/*!
	This function is used to open a socket for use with the streaming camera interface.
	Attempts are made to use the PF_PACKET interface with an RX_RING socket option. 
	(CONFIG_PACKET and CONFIG_PACKET_MMAP need to be set in the kernel for optimum
	 performance).

	\param [out]     pSocket          Pointer to socket for stream channel
	\param [in]      interfaceIndex   Index (sequential) for network interface.
	\param [in/out]  pPktInfo         Pointer to GevRxPkt_t structure for managing recv on PF_PACKET socket.
	\return GEV status code
	\note None
*/
GEV_STATUS GevOpenPacketSocket( SOCKET *pSocket, int interfaceIndex, GevRxPkt_t *pPktInfo )
{
	GEV_STATUS status = GEVLIB_ERROR_NULL_PTR;
	struct sockaddr_ll	sock_ll;


	if ((pSocket != NULL) && (pPktInfo != NULL))
	{
		pPktInfo->use_packet_interface = FALSE;
		pPktInfo->use_mmap_interface = FALSE;

		// Open a PF_PACKET socket (for IP Datagrams - since that is all we are interested in).
		// (Requires CAP_NET_RAW in order to work).
		//*pSocket = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
		*pSocket = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));
		if( *pSocket == -1)
		{
			LogPrint( GEV_LOG_INFO, "INFO: Unable to open streaming packet socket (error %d) - usually a permission issue\n", _GetSocketError());
			status = GEVLIB_ERROR_SOCKET_CREATE;
			return status;
		}
		else
		{
			int flag = 1;
			setsockopt( *pSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag));
		}

		// Bind the socket to an interface
		memset(&sock_ll, 0, sizeof(sock_ll));
		sock_ll.sll_family   = AF_PACKET;
		sock_ll.sll_ifindex  = interfaceIndex;
		sock_ll.sll_protocol	= htons(ETH_P_IP);

		if (bind(*pSocket, (struct sockaddr *) &sock_ll, sizeof(sock_ll)) == -1) 
		{
			// Unable to set up the PF_PACKET interface.
			closesocket(*pSocket);
			*pSocket = INVALID_SOCKET;
			status = GEVLIB_ERROR_INVALID_CONNECTION_ATTEMPT;
			return status;
		}

		// Attempt to set up mmap access to receive buffers using the RX_RING capability (if it is available)
		// Otherwise - just use the PF_PACKET socket access for lowest latency.
		// (Add mmap support later).	
		status = GevSetupRxRingMmap( pSocket, pPktInfo );
		if (status != GEV_STATUS_SUCCESS)
		{
			// mmap'd RX_RING setup failed.
			// Set up non RX_RING access for reading the packet socket.
			//
			// Set the socket buffer size and allocate a buffer to receive data into.
			pPktInfo->use_mmap_interface = FALSE;

			if (pPktInfo->buffer_size != 0) 
			{
				if (setsockopt(*pSocket, SOL_SOCKET, SO_RCVBUF, &pPktInfo->buffer_size, sizeof(pPktInfo->buffer_size)) == 0) 
				{
					// Allocate socket buffer for receiving.
					int size = PAGE_SIZE * (((pPktInfo->buffer_size + pPktInfo->offset) + PAGE_SIZE - 1) / PAGE_SIZE);
					pPktInfo->buffer	 = malloc(size);
					if (pPktInfo->buffer == NULL) 
					{
						// Failure - clean up
						status = GEVLIB_ERROR_INSUFFICIENT_MEMORY;
					}
					else
					{
						pPktInfo->use_packet_interface = TRUE;
						pPktInfo->netIf_index = interfaceIndex;
						status = GEVLIB_SUCCESS;
					}
				}
				else
				{
					// Failure - clean up
					status = GEVLIB_ERROR_INSUFFICIENT_MEMORY;
				}
			}
		}
		else
		{
			pPktInfo->netIf_index = interfaceIndex;
		}
	}
	return status;
}


//! Close an open socket used by the streaming interface.
/*!
	This function is used to close a socket for use with the streaming camera interface.
	If the PF_PACKET interface with (or without) an RX_RING socket option is used,
	this will be cleaned up as well.

	\param [out]     pSocket          Pointer to socket for stream channel
	\param [in]      interfaceIndex   Index (sequential) for network interface.
	\param [in/out]  pPktInfo         Pointer to GevRxPkt_t structure for managing recv on PF_PACKET socket.
	\return GEV status code
	\note None
*/
GEV_STATUS GevCloseStreamingChannelSocket( SOCKET *pSocket, GevRxPkt_t *pPktInfo )
{
	GEV_STATUS status = GEVLIB_ERROR_NULL_PTR;

	if (pSocket != NULL) 
	{
		// Release the regular UDP DGRAM socket
		if (*pSocket != INVALID_SOCKET)
		{
			closesocket (*pSocket);
			*pSocket = INVALID_SOCKET;
		}

		if (pPktInfo != NULL)
		{
			if (pPktInfo->use_packet_interface)
			{
				if (pPktInfo->use_mmap_interface)
				{
					// Clean up the mmap'd RX_RING.
					GevCloseRxRingMmap( pSocket, pPktInfo);
				}
				else
				{
					// Clean up the packet socket and associated storage..
					if (pPktInfo->packet_socket != INVALID_SOCKET)
					{
						closesocket( pPktInfo->packet_socket);
						pPktInfo->packet_socket = INVALID_SOCKET;
						
						if (pPktInfo->buffer != NULL)
						{
							free(pPktInfo->buffer); 
						}
					}
				}
			}
		}
	}
	return status;
}

//! Filter the packet
/*!
	This function is used to determine if a packet is one we are interested in.
	If accepts GVSP packets from a specific source that are intended for the interface
	received on. For accepted packets, it returns the type (format) of the packet
	and its intended destination port. 
	\param [in]		pFilter   	Pointer to (simple) filter data structure
	\param [in]    pPacket	 	Pointer to the input packet being filtered.
	\param [out]   pDataOut		Pointer to the filtered output data (headers stripped).
	\param [out]  	numBytes  	Pointer to receive valid number of bytes in the filtered packet.
	\param [out]  	pPktInfo  	Pointer to receive information about the packet (GEVPKT_INFO).
	\return BOOL 	TRUE for packet is valid / False for packet is invlid (drop it).
	\note None
*/
BOOL _FilterPacket( GevPktFilter_t *pFilter, void *pPacket, void **pDataOut, int *numBytes, PGVSPPKT_INFO pGVSPInfo)
{
	BOOL keepPacket = FALSE;

	if ((pFilter != NULL) && (pPacket != NULL) && (pDataOut != NULL) && (numBytes != NULL))
	{
		// Get the DGRAM packet as a UDP packet and check source / dest and signature against expected values.
		struct iphdr *ip_hdr = (struct iphdr *)pPacket;
		struct udphdr *udp_hdr = NULL;
		unsigned char *target_pkt = NULL;
		int udp_len = 0;

		*numBytes = 0;
		udp_hdr = (struct udphdr *)&ip_hdr[1];
		udp_len = ntohs(udp_hdr->len);
		target_pkt = (unsigned char *)&udp_hdr[1];
			
		// Filter source and destination addresses (Filter value of 0 is "don't care").
	
		if ( ((pFilter->srcAddress == 0) || ((pFilter->srcAddress != 0) && (pFilter->srcAddress == ntohl(ip_hdr->saddr)))) && \
			  ((pFilter->dstAddress == 0) || ((pFilter->dstAddress != 0) && (pFilter->dstAddress == ntohl(ip_hdr->daddr)))))
		{
			// Proper source / destination found. Check the source port. (Filter value of 0 is "don't care").
			if ( ((pFilter->srcPort == 0) || ((pFilter->srcPort != 0) && (pFilter->srcPort == ntohs(udp_hdr->source)))) && \
				  ((pFilter->dstPort == 0) || ((pFilter->dstPort != 0) && (pFilter->dstPort == ntohs(udp_hdr->dest)))))
			{
				// So far, this is a packet we want -  Decode the the packet info.
				if ( pGVSPInfo != NULL)
				{
					pGVSPInfo->dstPort = ntohs(udp_hdr->dest);							
					Gev_Stream_DecodeGVSPHeader(target_pkt, (PUINT8)&pGVSPInfo->type, &pGVSPInfo->block_id, &pGVSPInfo->packet_id, &pGVSPInfo->status, NULL);
					pGVSPInfo->valid = TRUE;
				}
				
				// Check the signature (if desired).
				keepPacket = TRUE;
				if (pFilter->check_signature)
				{
					unsigned char signature;
		
					signature = GET_DWORD( target_pkt, pFilter->signature_offset);
					signature = (signature & pFilter->signature_mask);
					if (signature != pFilter->signature_value)
					{
						// Not the packet we want.
						keepPacket = FALSE;
						if ( pGVSPInfo != NULL)
						{
							pGVSPInfo->valid = FALSE;
						}
					}
				}
			}
		}
		// Set up the number of bytes in the valid packet
		if (keepPacket)
		{
			*pDataOut = (void *)target_pkt;
			*numBytes = udp_len;
		}
	}
	return keepPacket;	
}


//! Read a data from the packet socket. Check that it is data we are looking
//! for, and return the number of bytes available and a pointer to the data.
/*!
	This function is used to read data from a packet socket. It checks the data 
	read using a simple filter to determine if it is data we are interested in.
	The number of bytes and a pointer to any desired data is returned.

	\param [in]  pPktInfo  Pointer to packet socket interface info structure.
	\param [in]  numBytes  Pointer to receive the number of bytes available.
	\param [in]  dataPtr   Pointer to receive the pointer to the data available.
	\return GEV status code
	\note None
*/

GEV_STATUS GevReadPacketSocket( UINT8 *pDatagram, int *numBytes, struct timeval *pTimeout, GevRxPkt_t *pPktInfo )
{
	GEV_STATUS status = GEVLIB_ERROR_NULL_PTR;
	GVSPPKT_INFO GVSPInfo = {0};
	BOOL keepPacket = FALSE;
	BOOL done = FALSE;

	while( !done )
	{
		status = GevRecvPacketSocket( pDatagram, numBytes, pTimeout, pPktInfo);
		if (status == GEV_STATUS_SUCCESS)
		{
			int nB = 0;
			void *pktData = NULL;
			keepPacket = _FilterPacket( &pPktInfo->filter, pDatagram, &pktData, &nB,  &GVSPInfo);	
			if (keepPacket)
			{
				// Return received (and filtered) packet.
				*numBytes = nB;
				memmove( pDatagram, pktData, *numBytes);
				status = GEV_STATUS_SUCCESS;
				done = TRUE;
			}			
		}
		//????LATER WHEN THIS IS IN A THREAD !!!!else if (status != GEV_STATUS_NO_MSG)
		{
			done = TRUE;
		}
	}
	return status;
}



//! Receive data from the packet socket. 
/*!
	This function is used to receive data from a packet socket. The number of bytes 
	received is returned and no filtering is performed.

	\param [in]  dataframe Pointer to receive the data availabe (assumed to be MTU sized).
	\param [in]  numBytes  Pointer to receive the number of bytes available.
	\param [in]  pTimeout  Pointer to a timeval structure for timeout handling.
	\param [in]  pPktInfo  Pointer to packet socket interface info structure.
	\return GEV status code
	\note None
*/
GEV_STATUS GevRecvPacketSocket( UINT8 *pDatagram, int *numBytes, struct timeval *pTimeout, GevRxPkt_t *pPktInfo )
{
	GEV_STATUS status = GEVLIB_ERROR_NULL_PTR;
	int timeout = 0;
	u_char *ptr;
	int       packet_len;

	if (pTimeout != NULL)
	{
		timeout = (pTimeout->tv_usec / 1000) + (pTimeout->tv_sec * 1000);
	}

	if ( (pPktInfo == NULL) || (numBytes == NULL) || (pDatagram == NULL))
	{
		return status;
	}
	if (!pPktInfo->use_packet_interface  || (pPktInfo->packet_socket == INVALID_SOCKET))
	{
		// Packet interface is not enabled (why did this get called if that is the case?)
		status = GEVLIB_ERROR_SOFTWARE;
		return status;
	}
	
	// Init.
	*numBytes = 0;

	if (pPktInfo->mmap_ptr == NULL)
	{
		struct pollfd pollit;

		// Socket buffers are not mmap'd - use standard reads to get data.
		ptr = pPktInfo->buffer + pPktInfo->offset;

		// Issue a no_wait reads to fill up buffer.
		packet_len = recv( pPktInfo->packet_socket, ptr, (pPktInfo->buffer_size - pPktInfo->offset), MSG_DONTWAIT);

		if (packet_len > 0)
		{
			*numBytes = packet_len;
			memcpy( pDatagram, (void *)pPktInfo->buffer, *numBytes);
			status = GEV_STATUS_SUCCESS;
			return status;
		}

		// Timeout (or error) - Wait for new data here.
		if (packet_len <= 0)
		{
			// Wait for data to arrive.
			pollit.fd = pPktInfo->packet_socket;
			pollit.events = POLLIN;
			if ( (status = poll(&pollit, 1, timeout)) != 1)
			{
				// Timeout.
				status = GEV_STATUS_NO_MSG;
				*numBytes = 0;
				return status;
			}

			// Data is present on the socket - read it.
			do
			{
				// Receive (read) a single packet from the kernel.
				// Handle EINTR (via retry) and ENETDOWN (possibly unplugged) since network 
				//	was (once) attached and may come back.
				packet_len = recv( pPktInfo->packet_socket, pDatagram, pPktInfo->mtu_size, MSG_DONTWAIT);
			} while (packet_len == -1 && (errno == EINTR || errno == ENETDOWN));
		}
		// Check if some other error occured.
		if (packet_len == -1) 
		{
			if (errno == EAGAIN)
			{
				status = GEV_STATUS_BUSY;
				return status;
			}
			else 
			{
				// Some other error.
				status = GEVLIB_ERROR_SOCKET_DATA_READ;
				return status;
			}
		}

		// We got data, return it.
		{
			*numBytes = packet_len;
			memcpy( pDatagram, (void *)pPktInfo->buffer, *numBytes);
			status = GEV_STATUS_SUCCESS;
			return status;
		}
	}
	else
	{
		// Socket buffers are mmap'd
		status = GevRecvMmapPacket( pPktInfo, numBytes, pDatagram, pTimeout );
	}
	return status;
}


