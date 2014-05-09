//# SocketStream.h: 
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

#ifndef LOFAR_LCS_STREAM_SOCKET_STREAM_H
#define LOFAR_LCS_STREAM_SOCKET_STREAM_H

#include <Common/Exception.h>
#include <Common/LofarTypes.h>
#include <Common/SystemCallException.h>
#include <Stream/FileDescriptorBasedStream.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <string>
#include <vector>

namespace LOFAR {

class SocketStream : public FileDescriptorBasedStream
{
  public:
    EXCEPTION_CLASS(TimeOutException, LOFAR::Exception);

    enum Protocol {
      TCP, UDP
    };

    enum Mode {
      Client, Server
    };

  	    SocketStream(const std::string &hostname, uint16 _port, Protocol, Mode, time_t deadline = 0, const std::string &nfskey = "", bool doAccept = true);
    virtual ~SocketStream();

    FileDescriptorBasedStream *detach();

    void    reaccept(time_t deadline = 0); // only for TCP server socket
    void    setReadBufferSize(size_t size);

    const Protocol protocol;
    const Mode mode;

    template<typename T> size_t recvmmsg( std::vector<T> &buffers, bool oneIsEnough, struct timespec *timeout = NULL ); // only for UDP server socket

    // wrap ::recvfrom(), using MSG_PEEK if peek == true
    size_t recvfrom(void *buffer, size_t numBytes, struct ::sockaddr &src, bool peek = false);

    // wrap ::sendto(), using MSG_DONTWAIT if blocking == false
    size_t sendto(const void *buffer, size_t numBytes, const struct ::sockaddr &dest, bool blocking = true);

  private:
    const std::string hostname;
    uint16 port;
    const std::string nfskey;
    int listen_sk;

    void accept(time_t timeout);

    static void syncNFS();

    static std::string readkey(const std::string &nfskey, time_t deadline);
    static void writekey(const std::string &nfskey, uint16 port);
    static void deletekey(const std::string &nfskey);
};

} // namespace LOFAR

#include "SocketStream.tcc"

#endif
