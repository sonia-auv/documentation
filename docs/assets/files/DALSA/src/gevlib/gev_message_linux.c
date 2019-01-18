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

/*! \file gev_message_linux.c
\brief Helper functions to send/receive message using socket API

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

GEV_STATUS GevReadPacketSocket( UINT8 *pDatagram, int *numBytes, struct timeval *pTimeout, GevRxPkt_t *pPktInfo );;


//! Macro to get a BYTE from the datagram
#define GET_BYTE( p, o ) \
	( *( UINT8 * )( ( (UINT8 *)(p) ) + o ) )

//====================================================================
// MESSAGE FUNCTIONS
//====================================================================

//! Send a message using socket API
/*!
	This function is used to send a message to the specified destination
	\param [in] pDatagram Pointer to datagram message to send
	\param [in] size Size of message to send (in bytes)
	\param [in] pSocket Pointer to socket for stream channel
	\param [in] pDest Destination address of the message
	\param [in] showData Flag to indicate if we want to dump datagram content to console
	\return GEV status code
	\note None
*/
GEV_STATUS GevSendMessage (UINT8 *pDatagram, int size, SOCKET *pSocket, struct sockaddr_in *pDest, BOOL showData)
{
	int addrLen = sizeof(struct sockaddr_in);
	int cnt;		// number of bytes sent
	int offset;
	GEV_STATUS status = GEV_STATUS_SUCCESS;
	struct timeval timeout;
	struct timeval *pTimeout = &timeout;
	fd_set setWrite;
	int n = 0;
	// Add application socket to list
	FD_ZERO(&setWrite);
	FD_SET (*pSocket, &setWrite);
	
	if ((int)*pSocket > n)
	{
		n = (int)*pSocket;
	}
	
	timeout.tv_sec = 0;
	timeout.tv_usec = 500;
	
	// Perform select operation to block until a port is ready
	cnt = select ((n+1), NULL, &setWrite, NULL, pTimeout);
	while ((cnt == SOCKET_ERROR) && (errno == EINTR))
	{
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		cnt = select (n, &setWrite, NULL, NULL, pTimeout);
	}
	if (cnt == SOCKET_ERROR)
	{
		LogPrint( GEV_LOG_WARNING, "ERROR: SendMessage : select returned an error (error %d)\n\n\b", _GetSocketError ());
		status = GEV_STATUS_ERROR;
		return status;
	}

	// Send datagram
	cnt = sendto( *pSocket, (const char *)pDatagram, size, 0, (struct sockaddr *)pDest, addrLen);
	if (cnt < 0)
	{
		LogPrint( GEV_LOG_WARNING, "ERROR: Error in sendto (error %d)\n\n\b", _GetSocketError());
		status = GEV_STATUS_ERROR;
	}
	else
	{
		// Check if user wants to dump datagram content
		if (showData)
		{
			// print destination socket
			printf ("-> Sending a message to "); PrintIP (ntohl(pDest->sin_addr.s_addr)); printf (" port %d:\n", ntohs(pDest->sin_port));
			// print message content
			cnt = size  / sizeof (UINT32);
			offset = 0;
			while (cnt--)
			{
				printf ("\t0x%04X => %02X ", offset, GET_BYTE( pDatagram, offset));
				offset++;
				printf ("%02X ", GET_BYTE( pDatagram, offset++));
				printf ("%02X ", GET_BYTE( pDatagram, offset++));
				printf ("%02X\n", GET_BYTE( pDatagram, offset++));
//				offset += 4;
			}
		}
	}
	return status;
}


//! Broadcast a message using socket API
/*!
	This function is used to broadcast a message
	\param [in] pDatagram Pointer to datagram message to send
	\param [in] size Size of message to send (in bytes)
	\param [in] pSocket Pointer to socket for stream channel
	\param [in] pDest Destination address of the message
	\param [in] showData Flag to indicate if we want to dump datagram content to console
	\return GEV status code
	\note None
*/
GEV_STATUS GevBroadcastMessage (UINT8 *pDatagram, int size, SOCKET *pSocket, struct sockaddr_in *pDest, BOOL showData)
{
	GEV_STATUS status = GEV_STATUS_SUCCESS;
	int option;		// used to hold the broadcast option
	struct sockaddr_in destination;

	// Activate broadcast on this socket
	option = 1;
	setsockopt (*pSocket, SOL_SOCKET, SO_BROADCAST, (char *)&option, sizeof(option));
	//memcpy(&destination, pDest, sizeof(destination));
	destination = *pDest;
	destination.sin_addr.s_addr = INADDR_BROADCAST;	// enforce broadcast destination

	// Send the message using the regular function
	status = GevSendMessage (pDatagram, size, pSocket, &destination, showData);

	// Deactivate broadcast
	option = 0;
	setsockopt (*pSocket, SOL_SOCKET, SO_BROADCAST, (char *)&option, sizeof(option));

	return status;
}

//! Receive a generic message using the socket API
/*!
	This function is used to receive a message
	\param [in] pDatagram Pointer to datagram message to receive
	\param [in] maxSize Maximum size of message to receive (in bytes)
	\param [in] pSocket Pointer to socket for stream channel
	\param [in] pTimeout Timeout for select() operation
	\param [out] pAddr Pointer to IP address of the datagram
	\param [out] pPort Pointer to UDP port of the datagram
	\param [in] showData Flag to indicate if we want to dump datagram content to console
	\return GEV status code
	\note None
*/
GEV_STATUS GevReceiveMessage (UINT8 *pDatagram, int maxSize, SOCKET *pSocket, struct timeval *pTimeout, UINT32 *pAddr, UINT16 *pPort, BOOL showData)
{
	fd_set setRead;
	int n = 0;
	int cnt;			// number of instance
	int offset;
	struct sockaddr_in sa;
	socklen_t sa_len = sizeof(sa);
	GEV_STATUS status = GEV_STATUS_SUCCESS;

	// Add application socket to list
	FD_ZERO(&setRead);
	FD_SET (*pSocket, &setRead);
	
	if ((int)*pSocket > n)
	{
		n = (int)*pSocket;
	}

	// Perform select operation to block until a port is ready
	cnt = select ((n+1), &setRead, NULL, NULL, pTimeout);
	while ((cnt == SOCKET_ERROR) && (errno == EINTR))
	{
		cnt = select (n, &setRead, NULL, NULL, pTimeout);
	}

	if (cnt == SOCKET_ERROR)
	{
		LogPrint( GEV_LOG_WARNING, "ERROR: ReceiveMessage : select returned an error (error %d)\n\n\b", _GetSocketError ());
		status = GEV_STATUS_ERROR;
		return status;
	}

	// Check if at least one port has valid data
	if (cnt > 0)
	{
		// Check if this socket is ready
		if (FD_ISSET (*pSocket, &setRead))
		{
			// Read datagram
			memset(&sa, 0, sizeof(struct sockaddr_in));
			cnt = recvfrom (*pSocket, (char *)pDatagram, maxSize, 0, (struct sockaddr *)&sa, &sa_len);
			if (cnt > 0)
			{
				if (pAddr != NULL)	// only perform assignment if a valid buffer was provided
				{
					*pAddr = ntohl(sa.sin_addr.s_addr);
				}
				if (pPort != NULL)	// only perform assignment if a valid buffer was provided
				{
					*pPort = ntohs(sa.sin_port);
				}

				// Check if user wants to dump datagram content
				if (showData)
				{
					// print source socket
					printf ("-> Received a message from "); PrintIP (ntohl(sa.sin_addr.s_addr)); printf (" port %d:\n", ntohs(sa.sin_port));
					// print message content. Cannot use GVCP length field because this function is also
					// called by GVSP.
//					size = (UINT16)(GET_WORD( pDatagram, GVCP_LENGTH_OFFSET) + GVCP_HEADER_SIZE);
//					cnt = size / sizeof (UINT32);
					cnt /= sizeof(UINT32);
					offset = 0;
					while (cnt--)
					{
						printf ("\t0x%04X => %02X ", offset, GET_BYTE( pDatagram, offset));
						offset++;
						printf ("%02X ", GET_BYTE( pDatagram, offset++));
						printf ("%02X ", GET_BYTE( pDatagram, offset++));
						printf ("%02X\n", GET_BYTE( pDatagram, offset++));
						//printf ("\t0x%04X => 0x%08X\n", offset, htonl( GET_DWORD( pDatagram, offset)));
						//offset += 4;
					}
				}
				status = GEV_STATUS_SUCCESS;
			}
		}
	}
	else
	{
		// No message!
		status = GEV_STATUS_NO_MSG;
	}

	return status;
}


//! (Internal) Receive a message on a standard socket - block until data is available.
/*!
	This function is used to receive a message
	\param [in]  pDatagram Pointer to datagram message to receive
	\param [in]  maxSize Maximum size of message to receive (in bytes)
	\param [in]  pSocket Pointer to socket for stream channel
	\param [out] pNumBytes Pointer to contain the number of bytes read.
	\param [in]  pTimeout Timeout for select() operation
	\return GEV status code
	\note None
*/
GEV_STATUS GevReceiveMessage_OnSocket (UINT8 *pDatagram, int maxSize, SOCKET *pSocket, int *numBytes, struct timeval *pTimeout)
{
	int cnt;			// number of instance
	GEV_STATUS status = GEV_STATUS_SUCCESS;

	if (numBytes == NULL)
	{
		return GEV_STATUS_LOCAL_PROBLEM;
	}
	else
	{
		*numBytes = 0;
	}
		
	// Read datagram in a non_blocking way
	cnt = recv (*pSocket, (char *)pDatagram, maxSize, MSG_DONTWAIT);
	if (cnt > 0)
	{
		*numBytes = cnt;
	}
	else
	{
		// No data was received - perform the wait here for data.
		struct timeval timeout;
		fd_set setRead;
		int n = 0;

		// Add socket to list for select call (blocking wait for data).
		FD_ZERO(&setRead);
		FD_SET (*pSocket, &setRead);
	
		if ((int)*pSocket > n)
		{
			n = (int)*pSocket;
		}

		// Perform select operation to block until a port is ready
		timeout = *pTimeout;
		cnt = select ((n+1), &setRead, NULL, NULL, &timeout);
		while ((cnt == SOCKET_ERROR) && (errno == EINTR))
		{
			timeout = *pTimeout;
			cnt = select ((n+1), &setRead, NULL, NULL, &timeout);
		}
		if (cnt == SOCKET_ERROR)
		{
			LogPrint( GEV_LOG_WARNING, "ERROR: select returned an error (error %d)\n\n\b", _GetSocketError ());
		}

		// Check if at least one port has valid data
		*numBytes = 0;
		if (cnt > 0)
		{
			// Read the data now that it is present.
			cnt = recv (*pSocket, (char *)pDatagram, maxSize, MSG_DONTWAIT);
			if (cnt > 0)
			{
				*numBytes = cnt;
			}
		}
	}
	return status;
}


//! Receive a message on a socket (standard or packet) - block until data is available.
/*!
	This function is used to receive a message
	\param [in]  pDatagram Pointer to datagram message to receive
	\param [in]  maxSize   Maximum size of datagram message to receive (in bytes)
	\param [in]  pSocket   Pointer to socket for stream channel
	\param [in]  pTimeout Timeout for select() operation
	\param [out] pNumBytes Pointer to contain the number of bytes read.
	\param [out] pDataPtr  Pointer to contain pointer to data read.
	\param [in]  pPktInfo  Pointer to packet interface information.
	\return GEV status code
	\note None
*/
GEV_STATUS GevReceiveMessage_Block (UINT8 *pDatagram, int maxSize, SOCKET *pSocket, struct timeval *pTimeout, int *numBytes, void **pDataPtr, GevRxPkt_t *pPktInfo)
{
	GEV_STATUS status = GEVLIB_ERROR_NULL_PTR;

	if ((pDataPtr != NULL) && (numBytes != NULL))
	{
		// Check if we are using regular sockets or the packet socket interface (privileges required).
		if (pPktInfo == NULL)
		{
			// Not using the packet interface (for sure). Use normal interface.
			status = GevReceiveMessage_OnSocket( pDatagram, maxSize, pSocket, numBytes, pTimeout);
			*pDataPtr = (void *)pDatagram;
		}
		else
		{
			if (!pPktInfo->use_packet_interface)
			{
				// Not using the packet interface. Use normal interface.
				status = GevReceiveMessage_OnSocket( pDatagram, maxSize, pSocket, numBytes, pTimeout);
				*pDataPtr = (void *)pDatagram;
			}
			else
			{
				// Using the packet interface.
				//status = GevReadPacketSocket( pPktInfo, numBytes, pDataPtr, pTimeout );
				//status = GevReadPacketSocket( pPktInfo, numBytes, pDataPtr, pDatagram, pTimeout );
				status = GevReadPacketSocket( pDatagram, numBytes, pTimeout, pPktInfo );
				*pDataPtr = (void *)pDatagram;
			}
		}
	}

	return status;
}

