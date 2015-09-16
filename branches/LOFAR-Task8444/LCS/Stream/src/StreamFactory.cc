//# StreamFactory.cc: class/functions to construct streams
//# Copyright (C) 2008-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Stream/StreamFactory.h>

#include <vector>
#include <boost/lexical_cast.hpp>

#include <Common/StringUtil.h>
#include <Common/Exceptions.h>
#include <Stream/FileStream.h>
#include <Stream/NullStream.h>
#include <Stream/SocketStream.h>
#include <Stream/PortBroker.h>
#include <Stream/NamedPipeStream.h>

namespace LOFAR
{

  // Caller should wrap the returned pointer in some smart ptr type.
  Stream *createStream(const std::string &descriptor, bool asServer, time_t deadline, const std::string &bind_local_iface)
  {
    std::vector<std::string> split = StringUtil::split(descriptor, ':');

    // null:
    if (descriptor == "null:")
      return new NullStream;

    // udp:HOST:PORT
    else if (split.size() == 3 && split[0] == "udp")
      return new SocketStream(split[1].c_str(), boost::lexical_cast<unsigned short>(split[2]), SocketStream::UDP, asServer ? SocketStream::Server : SocketStream::Client, deadline, true, bind_local_iface);

    // tcp:HOST:PORT
    else if (split.size() == 3 && split[0] == "tcp")
      return new SocketStream(split[1].c_str(), boost::lexical_cast<unsigned short>(split[2]), SocketStream::TCP, asServer ? SocketStream::Server : SocketStream::Client, deadline, true, bind_local_iface);

    // tcpbroker:HOST:BROKERPORT:KEY
    else if (split.size() == 4 && split[0] == "tcpbroker")
      return asServer ? static_cast<Stream*>(new PortBroker::ServerStream(split[3])) : static_cast<Stream*>(new PortBroker::ClientStream(split[1], boost::lexical_cast<unsigned short>(split[2]), split[3], deadline, bind_local_iface));

    // file:PATH
    else if (split.size() > 1 && split[0] == "file") {
      // don't use split[1] to allow : in filenames
      const std::string filename = descriptor.substr(5);
      return asServer ? new FileStream(filename.c_str()) : new FileStream(filename.c_str(), 0666);
    }
   
    // pipe:PATH
    else if (split.size() == 2 && split[0] == "pipe")
      return new NamedPipeStream(split[1].c_str(), asServer);

    // HOST:PORT (udp)
    else if (split.size() == 2)
      return new SocketStream(split[0].c_str(), boost::lexical_cast<unsigned short>(split[1]), SocketStream::UDP, asServer ? SocketStream::Server : SocketStream::Client, deadline, true, bind_local_iface);

    // PATH (file)
    else if (split.size() == 1)
      return asServer ? new FileStream(split[0].c_str()) : new FileStream(split[0].c_str(), 0666);

    THROW(NotImplemented, std::string("createStream(): unrecognized descriptor: " + descriptor));
  }

} // namespace LOFAR

