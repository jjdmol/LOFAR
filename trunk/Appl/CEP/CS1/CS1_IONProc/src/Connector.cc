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
#include <Stream/FileStream.h>
#include <Stream/NullStream.h>
#include <Stream/SocketStream.h>
#include <Connector.h>

#include <boost/lexical_cast.hpp>

namespace LOFAR {
namespace CS1 {

Stream *Connector::getInputStream(const CS1_Parset *ps, const string &key)
{
  string streamType = ps->getString("OLAP.OLAP_Conn.station_Input_Transport");
#if 0
  if (streamType=="ETHERNET") {
    string interface = ps.getString(key + ".interface");
    string srcMac = ps.getString(key + ".sourceMac");
    string dstMac = ps.getString(key + ".destinationMac");
    // we are using UDP now, so the eth type is 0x0008
    theTH = new TH_Ethernet(interface, 
			    srcMac,
			    dstMac, 
			    0x0008,
			    16 * 1024 * 1024);
  } else
#endif      
  if (streamType == "FILE") {
    string file = ps->getString(key + ".inputFile");
    std::clog << "receive from FILE stream " << file << std::endl;
    return new FileStream(file.c_str());
  } else if (streamType == "NULL") {
    std::clog << "receive from NULL stream" << std::endl;
    return new NullStream;
  } else if (streamType == "TCP") {
    short port = boost::lexical_cast<short>(ps->inputPortnr(key));
    std::clog << "receive from TCP socket, port = " << port << std::endl;
    return new SocketStream("0.0.0.0", port, SocketStream::TCP, SocketStream::Server);
  } else if (streamType == "UDP") {
    short port = boost::lexical_cast<short>(ps->inputPortnr(key));
    std::clog << "receive from UDP socket, port = " << port << std::endl;
    return new SocketStream("0.0.0.0", port, SocketStream::UDP, SocketStream::Server);
  } else {
    ASSERTSTR(false, "Stream " << streamType << " unknown to Connector");
  }
}

} // namespace CS1
} // namespace LOFAR
