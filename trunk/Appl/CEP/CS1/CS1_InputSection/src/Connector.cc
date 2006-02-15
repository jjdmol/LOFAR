//#  Connector.cc: one line description
//#
//#  Copyright (C) 2006
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

//# Includes
#include <Common/LofarLogger.h>
#include <CS1_InputSection/Connector.h>
#include <Transport/TH_Mem.h>
#include <Transport/TH_Ethernet.h>
#include <Transport/TH_MPI.h>
#include <Transport/TH_File.h>
#include <Transport/TH_Null.h>
#include <Transport/TH_Socket.h>
#include <Transport/TH_ShMem.h>

namespace LOFAR {
  namespace CS1_InputSection {

    Connector::Connector()
    {}

    Connector::~Connector()
    {}

    static TransportHolder* readTH(const ACC::APS::ParameterSet& ps, const string& key, const bool isReceiver) {
      TransportHolder* theTH = 0;
      string transportType = ps.getString(key + ".th");
      if (transportType=="ETHERNET") {
	string interface = ps.getString(key + ".interface");
	string srcMac = ps.getString(key + ".sourceMac");
	string dstMac = ps.getString(key + ".destinationMac");
	// we are using UDP now, so the eth type is 0x0008
	theTH = new TH_Ethernet(interface, 
				srcMac,
				dstMac, 
				0x0008,
				2097152);
      } else if (transportType=="FILE") {
	string file = ps.getString(key + ".inputFile");
	if (isReceiver) {
	  theTH = new TH_File(file, TH_File::Read);
	} else {
	  theTH = new TH_File(file, TH_File::Write);
	}	  
      } else if (transportType=="NULL") {
	theTH = new TH_Null();
      } else if (transportType=="SOCKET") {
	string service = ps.getString(key + ".port");
	if (!isReceiver) {
	  theTH = new TH_Socket(service, 
				true, 
				Socket::TCP, 
				5, 
				false);
	} else {
	  string host = ps.getString(key + ".host");
	  theTH = new TH_Socket(host,
				service,
				true,
				Socket::TCP,
				false);
	}
      } else {
	ASSERTSTR(false, "TransportHolder " << transportType << " unknown to Connector");
      }
      return theTH;
    }

    void Connector::connectSteps(Step* src, int srcDH, Step* dst, int dstDH) {
      // TransportHolders (best first)
      // TH_Mem -> use this one if on same node and same process
      // TH_ShMem -> use this one if on same host (cannot check for that right now)
      // TH_MPI -> use this one if we have MPI and are not on the same node
      // TH_Socket -> use this one in other cases (cannot use it because we need more info then)

      // Special TransportHolders:
      // TH_Null -> does not send any data
      // TH_Ethernet -> uses raw Ethernet
      // TH_File -> uses a file for in or output

      if (src->getNode() == dst->getNode()) {
	dst->connect(dstDH,
		    src,
		    srcDH,
		    1,
		    new TH_Mem(),
		    false);
#ifdef HAVE_MPI
      } else if (src->getNode() != dst->getNode()) {
	dst->connect(dstDH,
		    src,
		    srcDH,
		    1,
		    new TH_MPI(src->getNode(), dst->getNode()),
		    true);
#endif
      } else {
	ASSERTSTR(false, "Cannot find a suitable TransportHolder to connect with");
      }
    }


    // Remove lines or remove comments for copy constructor and assignment.
    ////Connector::Connector (const Connector& that)
    ////{}
    ////Connector& Connector::operator= (const Connector& that)
    ////{
    ////  if (this != &that) {
    ////    ... copy members ...
    ////  }
    ////  return *this;
    ////}

  } // namespace CS1_InputSection
} // namespace LOFAR
