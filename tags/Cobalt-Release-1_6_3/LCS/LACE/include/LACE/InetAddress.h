//# InetAddress.h: Class for storing an (IPV4) internet address.
//#
//# Copyright (C) 2008
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_LACE_INET_ADDRESS_H
#define LOFAR_LACE_INET_ADDRESS_H

// \file InetAddress.h
// Class for storing an (IPV4) internet address.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#if defined(__APPLE__)
#include <netinet/in.h> 
#endif
#include <resolv.h>
#include <Common/LofarTypes.h>
#include <LACE/Address.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace LACE {

// \addtogroup LACE
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.
//# class ...;


// class_description
// ...
class InetAddress : public Address
{
public:
	InetAddress();
	virtual ~InetAddress();

	virtual const void*	getAddress()     const	{ return((void*) &itsTCPAddr); }
	virtual int			getAddressSize() const  { return(sizeof(itsTCPAddr));  }
	virtual string		deviceName()     const;

	// resolve the installe or given address.
	// returns 0 on success.
	int		set(uint16			portNr,
				const string&	hostname = "",
				const string&	protocol = "tcp");
	int		set(const string&	service, 
				const string&	hostname = "",
				const string&	protocol = "tcp");
	int		set(const sockaddr_in*	sockAddr,
				int					len);

	int		protocolNr() const 	{ return(itsProtocolNr); }
	int		portNr() 	 const 	{ return(itsPortNr); }

	// operators for comparison.
	bool		 operator==(const InetAddress&	that);
	bool		 operator!=(const InetAddress&	that);
	InetAddress& operator= (const InetAddress&	that);

protected:

private:
	//# --- Datamembers ---
	uint16					itsPortNr;		//# 
	string					itsHostname;	//# entered by user
	int						itsProtocolNr;	//# 

	struct sockaddr_in		itsTCPAddr;		// result structure after resolv.
};


//# ----- inline functions -----
inline bool InetAddress::operator==(const InetAddress&	that)
{
	return (Address::operator==(that) && 
			itsPortNr == that.itsPortNr && 
			itsHostname == that.itsHostname &&
			itsProtocolNr == that.itsProtocolNr);
}

inline bool InetAddress::operator!=(const InetAddress&	that)
{
	return (!InetAddress::operator==(that));
}


// @}
  } // namespace LACE
} // namespace LOFAR

#endif
