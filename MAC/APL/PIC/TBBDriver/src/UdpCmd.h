//#  -*- mode: c++ -*-
//#
//#  UdpCmd.h: III
//#
//#  Copyright (C) 2002-2004
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

#ifndef UDPCMD_H_
#define UDPCMD_H_

#include <Common/LofarTypes.h>
#include <GCF/TM/GCF_Control.h>
#include <net/ethernet.h>

#include <APL/TBB_Protocol/TBB_Protocol.ph>
#include "TP_Protocol.ph"

#include "Command.h"
#include "DriverSettings.h"

namespace LOFAR {
	using namespace TBB_Protocol;
	namespace TBB {

class UdpCmd : public Command 
{
public:
	// Constructors for a UdpCmd object.
	UdpCmd();

	// Destructor for UdpCmd.
	virtual ~UdpCmd();
	
	virtual bool isValid(GCFEvent& event);
	
	virtual void saveTbbEvent(GCFEvent& event);
						
	virtual void sendTpEvent();

	virtual void saveTpAckEvent(GCFEvent& event);

	virtual void sendTbbAckEvent(GCFPortInterface* clientport);
	
private:
	//private methods
	
	// Convert a string containing a Ethernet MAC address
	// to an array of 6 bytes.
	void string2mac(const char* macstring, uint32 mac[2]);
	
	// Convert a string containing an IP address
	// to an array of 6 bytes.
	uint32 string2ip(const char* ipstring);
	
	// Setup an appropriate UDP/IP header
	void setup_udpip_header(uint32 boardnr, uint32 ip_hdr[6], uint32 udp_hdr[2]);
	
	// Compute the 16-bit 1-complements checksum for the IP header.
	uint16 compute_ip_checksum(void* addr, int count);

private:
	TbbSettings *TS;
	uint32 itsStatus[MAX_N_TBBOARDS];
	
	uint32	itsMode; // Transient or subbands
};

	} // end TBB namespace
} // end LOFAR namespace

#endif /* UDPCMD_H_ */
