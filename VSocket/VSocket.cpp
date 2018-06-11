//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
//
//  This file is part of the VNC system.
//
//  The VNC system is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// If the source code for the VNC system is not available from the place 
// whence you received this file, check http://www.uk.research.att.com/vnc or contact
// the authors on vnc@uk.research.att.com for information on obtaining it.


// VSocket.cpp

// The VSocket class provides a platform-independent socket abstraction
// with the simple functionality required for an RFB server.

class VSocket;

////////////////////////////////////////////////////////
// System includes
#include "StdAfx.h"

////////////////////////////////////////////////////////
// *** Lovely hacks to make Win32 work.  Hurrah!

#ifdef __WIN32__
#define EWOULDBLOCK WSAEWOULDBLOCK
#endif

////////////////////////////////////////////////////////
// Socket implementation

#include "VSocket.h"
#include "../public/base64.h"

// The socket timeout value (currently 5 seconds, for no reason...)
// *** THIS IS NOT CURRENTLY USED ANYWHERE
const VInt rfbMaxClientWait = 5000;

////////////////////////////
// Socket implementation initialisation

static WORD winsockVersion = 0;

VSocketSystem::VSocketSystem()
{
	// Initialise the socket subsystem
	// This is only provided for compatibility with Windows.

	// Initialise WinPoxySockets on Win32
	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(2, 0);
	if (WSAStartup(wVersionRequested, &wsaData) != 0)
	{
		m_status = false;
		return;
	}

	winsockVersion = wsaData.wVersion;


	// If successful, or if not required, then continue!
	m_status = true;
}

VSocketSystem::~VSocketSystem()
{
	if (m_status)
	{
		WSACleanup();
	}
}

////////////////////////////
VSocketSystem VSocket::sys;

VSocket::VSocket(int sock /* = -1 */)
{
	// Clear out the internal socket fields
	this->sock = sock;
}

////////////////////////////

VSocket::~VSocket()
{
	// Close the socket
	Close();
}

////////////////////////////

bool VSocket::Create()
{
	const int one = 1;

	// Check that the old socket was closed
	if (sock >= 0)
		Close();

	// Create the socket
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		return false;
	}

	// Set the socket options:
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one)))
	{
		return false;
	}
	if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof(one)))
	{
		return false;
	}

	return true;
}

////////////////////////////

bool VSocket::Close()
{
	if (sock >= 0)
	{
		shutdown(sock, SD_BOTH);
		closesocket(sock);
		sock = -1;
	}
	return true;
}

////////////////////////////


bool VSocket::SetBlock(bool bBlock)
{
	unsigned long ul;
	if(bBlock)
		ul = 0;
	else 
		ul = 1;

	int ret = ioctlsocket(sock, FIONBIO, &ul);
	return (0 == ret); //设置为非阻塞模式
}

bool VSocket::Select(unsigned int secs)
{
	timeval tm;
	fd_set set;
	tm.tv_sec = secs;
	tm.tv_usec = 0;
	FD_ZERO(&set);
	FD_SET((unsigned)sock, &set);
	bool ret = false;
	if( select(sock , NULL, &set, NULL, &tm) > 0)
	{
		int error = -1;
		int len = 4;
		getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
		if(error == 0)
		{
			ret = true;
		}
	}
	return ret;
}

bool VSocket::Shutdown()
{
	if (sock >= 0)
	{
		//	  log.Print(LL_SOCKINFO, VNCLOG("shutdown socket\n"));

		shutdown(sock, SD_BOTH);
	}
	return true;
}

////////////////////////////

bool VSocket::Bind(const VCard port, const bool localOnly)
{
	struct sockaddr_in addr;

	// Check that the socket is open!
	if (sock < 0)
		return false;

	// Set up the address to bind the socket to
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if (localOnly)
		addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	else
		addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// And do the binding
	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		return false;

	return true;
}


static bool GetActualAddress(const char* szIP, sockaddr_in& addr)
{
	addr.sin_addr.s_addr = inet_addr(szIP);
	// Was the string a valid IP address?
	if (addr.sin_addr.s_addr == -1)
	{
		// No, so get the actual IP address of the host name specified
		struct hostent *pHost;
		pHost = gethostbyname(szIP);
		if (pHost != NULL)
		{
			if (pHost->h_addr == NULL)
				return false;
			addr.sin_addr.s_addr = ((struct in_addr *)pHost->h_addr)->s_addr;
		}
		else
			return false;
	}
	return true;
}

////////////////////////////

bool VSocket::Connect(const char* address, const VCard port,int outime)
{
	// Check the socket
	if (sock < 0)
		return false;

	// Create an address structure and clear it
	struct sockaddr_in addr = {0};

	// Fill in the address if possible
	addr.sin_family = AF_INET;
	if (!GetActualAddress(address, addr))
	{
		return false;
	}

	// Set the port number in the correct format
	addr.sin_port = htons(port);

	//设置非阻塞模式
	if(!SetBlock(false))
	{
		return false;
	}

	// Actually connect the socket
	if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		if(!Select(outime))
		{
			SetBlock(true);
			return false;
		}

		//恢复阻塞模式
		bool RET = SetBlock(true);
		return RET;
	}

	//恢复阻塞模式
	bool RET = SetBlock(true);
	return RET;
}

////////////////////////////

bool VSocket::Listen()
{
	// Check socket
	if (sock < 0)
		return false;

	// Set it to listen
	if (listen(sock, 5) < 0)
		return false;

	return true;
}

////////////////////////////

VSocket VSocket::Accept()
{
	const int one = 1;

	int new_socket_id;

	// Check this socket
	if (sock < 0)
		return NULL;

	// Accept an incoming connection
	if ((new_socket_id = accept(sock, NULL, 0)) < 0)
		return -1;

	// Attempt to set the new socket's options
	setsockopt(new_socket_id, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof(one));

	return new_socket_id;
}

////////////////////////////

VString VSocket::GetPeerName()
{
	struct sockaddr_in	sockinfo;
	struct in_addr		address;
	int					sockinfosize = sizeof(sockinfo);
	VString				name;

	// Get the peer address for the client socket
	getpeername(sock, (struct sockaddr *)&sockinfo, &sockinfosize);
	memcpy(&address, &sockinfo.sin_addr, sizeof(address));

	name = inet_ntoa(address);
	if (name == NULL)
		return "<unavailable>";
	else
		return name;
}

////////////////////////////

VString VSocket::GetSockName()
{
	struct sockaddr_in	sockinfo;
	struct in_addr		address;
	int					sockinfosize = sizeof(sockinfo);
	VString				name;

	// Get the peer address for the client socket
	getsockname(sock, (struct sockaddr *)&sockinfo, &sockinfosize);
	memcpy(&address, &sockinfo.sin_addr, sizeof(address));

	name = inet_ntoa(address);
	if (name == NULL)
		return "<unavailable>";
	else
		return name;
}

////////////////////////////

VCard32 VSocket::Resolve(const VString address)
{
	VCard32 addr;

	// Try converting the address as IP
	addr = inet_addr(address);

	// Was it a valid IP address?
	if (addr == 0xffffffff)
	{
		// No, so get the actual IP address of the host name specified
		struct hostent *pHost;
		pHost = gethostbyname(address);
		if (pHost != NULL)
		{
			if (pHost->h_addr == NULL)
				return 0;
			addr = ((struct in_addr *)pHost->h_addr)->s_addr;
		}
		else
			return 0;
	}

	// Return the resolved IP address as an integer
	return addr;
}

////////////////////////////

bool VSocket::SetTimeout(VCard32 secs)
{
	if (LOBYTE(winsockVersion) < 2)
		return false;
	int timeout=secs *1000;
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
	{
		return false;
	}
	if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
	{
		return false;
	}
	return true;
}

////////////////////////////

VInt VSocket::Send(const char *buff, const VCard bufflen)
{
	return send(sock, buff, bufflen, 0);
}

////////////////////////////

bool VSocket::SendExact(const char *buff, const VCard bufflen)
{
	return Send(buff, bufflen) == (VInt)bufflen;
}

////////////////////////////

VInt VSocket::Read(char *buff, const VCard bufflen)
{
	return recv(sock, buff, bufflen, 0);
}

////////////////////////////

bool VSocket::ReadExact(char *buff, const VCard bufflen)
{
	int n;
	VCard currlen = bufflen;

	while (currlen > 0)
	{
		// Try to read some data in
		n = Read(buff, currlen);

		if (n > 0)
		{
			// Adjust the buffer position and size
			buff += n;
			currlen -= n;
		} else if (n == 0) {
			//			log.Print(LL_SOCKERR, VNCLOG("zero bytes read\n"));

			return false;
		} else {
			// 			if (errno != EWOULDBLOCK)
			// 			{
			// //				log.Print(LL_SOCKERR, VNCLOG("socket error %d\n"), errno);
			return false;
			// 			}
		}
	}

	return true;
}

bool VSocket::SetKeepAlive(int nIdleTime /*= 30*/, int nInterval /*= 1*/)
{
	DWORD dwRet = 0;
	tcp_keepalive keepalive = {TRUE, nIdleTime * 1000, nInterval * 1000};
	return WSAIoctl(sock, SIO_KEEPALIVE_VALS, &keepalive, sizeof(keepalive), NULL, 0, &dwRet, NULL, NULL) == 0;
}