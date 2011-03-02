//#  Stream.cc: one line descriptor
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
//#  $Id: Stream.cc 16396 2010-09-27 12:12:24Z mol $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Interface/Exceptions.h>
#include <Interface/Stream.h>
#include <Common/StringUtil.h>

#include <Stream/FileStream.h>
#include <Stream/NullStream.h>
#include <Stream/SocketStream.h>
#include <Stream/NamedPipeStream.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <vector>

using boost::format;

namespace LOFAR {
namespace RTCP {

Stream *createStream(const std::string &descriptor, bool asServer)
{
  std::vector<std::string> split = StringUtil::split(descriptor, ':');

  if (descriptor == "null:")
    return new NullStream;
  else if (split.size() == 3 && split[0] == "udp")
    return new SocketStream(split[1].c_str(), boost::lexical_cast<short>(split[2]), SocketStream::UDP, asServer ? SocketStream::Server : SocketStream::Client, 30);
  else if (split.size() == 3 && split[0] == "tcp")
    return new SocketStream(split[1].c_str(), boost::lexical_cast<short>(split[2]), SocketStream::TCP, asServer ? SocketStream::Server : SocketStream::Client, 30);
  else if (split.size() == 3 && split[0] == "udpkey")
    return new SocketStream(split[1].c_str(), 0, SocketStream::UDP, asServer ? SocketStream::Server : SocketStream::Client, 30, split[2].c_str());
  else if (split.size() == 3 && split[0] == "tcpkey")
    return new SocketStream(split[1].c_str(), 0, SocketStream::TCP, asServer ? SocketStream::Server : SocketStream::Client, 30, split[2].c_str());
  else if (split.size() == 2 && split[0] == "file")
    return asServer ? new FileStream(split[1].c_str()) : new FileStream(split[1].c_str(), 0666);
  else if (split.size() == 2 && split[0] == "pipe")
    return new NamedPipeStream(split[1].c_str(), asServer);
  else if (split.size() == 2)
    return new SocketStream(split[0].c_str(), boost::lexical_cast<short>(split[1]), SocketStream::UDP, asServer ? SocketStream::Server : SocketStream::Client, 30);
  else if (split.size() == 1)
    return asServer ? new FileStream(split[0].c_str()) : new FileStream(split[0].c_str(), 0666);
  else
    THROW(InterfaceException, string("unrecognized connector format: \"" + descriptor + '"'));
}

std::string getStreamDescriptorBetweenIONandCN(const char *streamType, unsigned pset, unsigned core, unsigned numpsets, unsigned numcores, unsigned channel)
{
  std::string descriptor;

  if (strcmp(streamType, "NULL") == 0) {
    descriptor = "null:";
  } else if (strcmp(streamType, "TCP") == 0) {
    // DEPRICATED -- use TCPKEY instead
    usleep(10000 * core); // do not connect all at the same time

    unsigned port = 5000 + (channel * numpsets + pset) * numcores + core;
    descriptor = str(format("tcp:127.0.0.1:%u") % port);
  } else if (strcmp(streamType, "TCPKEY") == 0) {
    usleep(10000 * core); // do not connect all at the same time

    descriptor = str(format("tcpkey:127.0.0.1:ion-cn-%u-%u-%u") % pset % core % channel );
  } else if (strcmp(streamType, "PIPE") == 0) {
    descriptor = str(format("pipe:/tmp/ion-cn-%u-%u-%u") % pset % core % channel);
  } else {
    THROW(InterfaceException, "unknown Stream type between ION and CN");
  }

  LOG_DEBUG_STR("Creating stream " << descriptor << " for pset " << pset << " core " << core << " channel " << channel);

  return descriptor;
}


#ifndef HAVE_BGP_CN
std::string getStreamDescriptorBetweenIONandStorage(const Parset &parset, const string &host, const std::string &filename)
{
  std::string connectionType = parset.getString("OLAP.OLAP_Conn.IONProc_Storage_Transport");

  if (connectionType == "NULL") {
    return "null:";
  } else if (connectionType == "TCP") {
    return str(format("tcpkey:%s:ion-storage-obs-%s-file-%s") % host % parset.observationID() % filename);
  } else if (connectionType == "FILE") {
    return str(format("file:%s") % filename );
  } else {
    THROW(InterfaceException, "unsupported ION->Storage stream type: " << connectionType);
  }
}
#endif

} // namespace RTCP
} // namespace LOFAR
