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


// VSocket.h

// RFB V3.0

// The VSocket class provides simple socket functionality,
// independent of platform.  Hurrah.

class VSocket;

#if (!defined(_ATT_VSOCKET_DEFINED))
#define _ATT_VSOCKET_DEFINED

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <MSTcpIP.h>
#include "VTypes.h"

////////////////////////////
// Socket implementation

// Create one or more VSocketSystem objects per application
class VSocketSystem
{
public:
	VSocketSystem();
	~VSocketSystem();
	bool Initialised() {return m_status;};
private:
	bool m_status;
};

// The main socket class
class VSocket
{
public:
	// Constructor/Destructor
	VSocket(int sock = -1);
	virtual ~VSocket();

	////////////////////////////
	// Socket implementation

	// Create
	//        Create a socket and attach it to this VSocket object
	bool Create();

	// Shutdown
	//        Shutdown the currently attached socket
	bool Shutdown();

	// Close
	//        Close the currently attached socket
	bool Close();

	bool SetBlock(bool bBlock = true);

	bool Select(unsigned int secs);
	// Bind
	//        Bind the attached socket to the specified port
	//		If localOnly is true then the socket is bound only
	//        to the loopback adapter.
	bool Bind(const VCard port, const bool localOnly=false);

	// Connect
	//        Make a stream socket connection to the specified port
	//        on the named machine.
	bool Connect(const char* address, const VCard port,int outime = 25);

	// IsConnected
	//        Test a stream socket connection is still connected to
	//        the server.
	bool IsConnected();

	// Listen
	//        Set the attached socket to listen for connections
	bool Listen();

	// Accept
	//        If the attached socket is set to listen then this
	//        call blocks waiting for an incoming connection, then
	//        returns a new socket object for the new connection
	VSocket Accept();

	// GetPeerName
	//        If the socket is connected then this returns the name
	//        of the machine to which it is connected.
	//        This string MUST be copied before the next socket call...
	VString GetPeerName();

	// GetSockName
	//		If the socket exists then the name of the local machine
	//		is returned.  This string MUST be copied before the next
	//		socket call!
	VString GetSockName();

	// Resolve
	//        Uses the Winsock API to resolve the supplied DNS name to
	//        an IP address and returns it as an Int32
	static VCard32 Resolve(const VString name);

	// SetTimeout
	//        Sets the socket timeout on reads and writes.
	bool SetTimeout(VCard32 secs);

	// SetKeepAlive
	//		  Sets the socket keepalive
	bool SetKeepAlive(int nIdleTime = 30, int nInterval = 1);

	// I/O routines

	// Send and Read return the number of bytes sent or recieved.
	VInt Send(const char *buff, const VCard bufflen);
	VInt Read(char *buff, const VCard bufflen);

	// SendExact and ReadExact attempt to send and recieve exactly
	// the specified number of bytes.
	bool SendExact(const char *buff, const VCard bufflen);
	bool ReadExact(char *buff, const VCard bufflen);

	////////////////////////////
	// Internal structures
protected:
	// The internal socket id
	int sock;
	static VSocketSystem sys;
};

#endif // _ATT_VSOCKET_DEFINED
