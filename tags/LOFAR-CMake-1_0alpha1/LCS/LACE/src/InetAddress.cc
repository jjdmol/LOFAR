//#  InetAddress.cc: Class for storing and reslving an IPv4 address.
//#
//#  Copyright (C) 2008
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//#include <Common/hexdump.h>

//# Includes
#include <netdb.h>
#include <arpa/inet.h>
#include <Common/LofarLogger.h>
#include <LACE/InetAddress.h>

namespace LOFAR {
  namespace LACE {

//
// InetAddress()
//
InetAddress::InetAddress() :
	Address(INET_TYPE),
	itsPortNr    (0),
	itsHostname  (""),
	itsProtocolNr(0)
{}


//
// ~InetAddress()
//
InetAddress::~InetAddress()
{}


//
// set(portNr, hostname)
//
// Resolve service and hostname
//
int InetAddress::set(uint16			portNr, 
					 const string&	hostname,
					 const string&	protocol)
{
	// copy setting to internals
	itsPortNr 	= portNr;
	itsHostname = hostname;
	setStatus(false);

	// portnumber must at least be valid.
	if (!itsPortNr) {
		LOG_ERROR("InetAddress:Portnumber 0 is not a valid portnumber.");
		return (ENODEV);
	}

	// Preinit part of TCP address structure
	memset (&itsTCPAddr, 0, sizeof(itsTCPAddr));
	itsTCPAddr.sin_family 	   = AF_INET;
	itsTCPAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	itsTCPAddr.sin_port 	   = htons(itsPortNr);

	// Make sure the hostname is filled.
	if (itsHostname.empty()) {
		char	namebuffer[MAXHOSTNAMELEN+1];
		if (gethostname(namebuffer, MAXHOSTNAMELEN) != 0) {
			LOG_ERROR_STR("InetAddress: Hostname '" << itsHostname << 
						  "' can not be resolved");
			return (ENODEV);
		}
		itsHostname = namebuffer;
	}

	// Try to resolve the hostname, is the hostname an IP address??
	uint32	IPbytes;
	if ((IPbytes = inet_addr(itsHostname.c_str())) != INADDR_NONE) {
		// found the IPv4 number
		memcpy ((char*) &itsTCPAddr.sin_addr.s_addr, 
				(char*) &IPbytes, sizeof(IPbytes));
	}
	else {
		// hostname is not an IPv4 address try to resolve it as a hostname
		struct hostent*       hostEnt;        // server host entry
		if (!(hostEnt = gethostbyname(itsHostname.c_str()))) {
			LOG_ERROR_STR("InetAddress: Hostname '" << itsHostname << 
						  "' can not be resolved");
			return (ENODEV);
		}

		// Check type
		if (hostEnt->h_addrtype != AF_INET) {
			LOG_ERROR_STR("InetAddress: Hostname '" << itsHostname << 
						  "' is not of the type AF_INET");
			return (ENODEV);
		}
		memcpy ((char*) &itsTCPAddr.sin_addr.s_addr, hostEnt->h_addr, sizeof(int32));
	}

	// Finally try to map protocol name to protocol number
	struct protoent*    protoEnt;
	if (!(protoEnt = getprotobyname(protocol.c_str()))) {
		LOG_ERROR_STR("InetAddress: Protocol '" << protocol << "' is invalid.");
		return (ENODEV);
	}
	itsProtocolNr = protoEnt->p_proto;
	
	setStatus(true);
	return (0);
}


//
// set(service, hostname)
//
// Resolve servicename like ssh, ftp, ...
//
int InetAddress::set(const string&	service, 
					 const string&	hostname,
					 const string&	protocol)
{
	itsPortNr 	= 0;
	itsHostname = hostname;
	setStatus(false);

	// resolve the service name to a portnumber
	struct servent*     servEnt;        // service info entry
	if ((servEnt = getservbyname(service.c_str(), protocol.c_str()))) {
		itsTCPAddr.sin_port = servEnt->s_port;	// network byte order.
		itsPortNr = ntohs(servEnt->s_port);		// human readable format.
	}

	// The rest of the resolving is implemented by the other 'set' function
	return(set(itsPortNr, itsHostname, protocol));
}

//
// set (sockaddr*, len)
//
int	InetAddress::set(const sockaddr_in*	sockAddr,
					 int					len)
{
	memcpy ((char*) &itsTCPAddr, (char*) sockAddr, len);
	itsPortNr   = ntohs(itsTCPAddr.sin_port);
	itsHostname = inet_ntoa(itsTCPAddr.sin_addr);
	LOG_DEBUG_STR("InetAddress.set(struct): " << itsPortNr << "@" << itsHostname);
	setStatus(true);
	return (0);
}

//
// deviceName()
//
string InetAddress::deviceName() const
{
	if (!isSet()) {
		return (formatString("Illegal InetAddres: %d@%s:%d", 
							 itsPortNr, itsHostname.c_str(), itsProtocolNr));
	}

	return (formatString("%d@%s:%d", itsPortNr, itsHostname.c_str(), itsProtocolNr));
}

//
// operator=
//
InetAddress& InetAddress::operator= (const InetAddress&	that) {
	if (this != &that) { 
		this->itsPortNr		= that.itsPortNr;
		this->itsHostname	= that.itsHostname;
		this->itsProtocolNr	= that.itsProtocolNr;
		this->itsTCPAddr	= that.itsTCPAddr;
	} 
	return (*this); 
}

  } // namespace LACE
} // namespace LOFAR
