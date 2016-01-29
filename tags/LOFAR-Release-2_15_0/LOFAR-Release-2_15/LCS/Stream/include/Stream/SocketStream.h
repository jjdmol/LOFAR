//# SocketStream.h: 
//#
//# Copyright (C) 2008, 2015
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

#include <ctime>
#include <sys/uio.h> // struct iovec
#include <string>
#include <vector>

#include <Common/LofarTypes.h>
#include <Common/Exception.h>
#include <Stream/FileDescriptorBasedStream.h>

namespace LOFAR {

class SocketStream : public FileDescriptorBasedStream
{
  public:
    enum Protocol {
      TCP, UDP
    };

    enum Mode {
      Client, Server
    };

    SocketStream(const std::string &hostname, uint16 _port, Protocol, Mode,
                 time_t deadline = 0, bool doAccept = true, const std::string &bind_local_iface = "");
    virtual ~SocketStream();

    FileDescriptorBasedStream *detach();

    void    reaccept(time_t deadline = 0); // only for TCP server socket
    void    setReadBufferSize(size_t size);

    const Protocol protocol;
    const Mode mode;

    /*
     * Receive message(s). Note: only for UDP server socket!
     *   @bufBase is large enough to store all to be received messages
     *   @maxMsgSize indicates the max size of _each_ (i.e. 1) message
     *   @recvdMsgSizes is passed in with a size indicating the max number of
     *     messages to receive. Actually received sizes will be written therein.
     * Returns the number of messages received if ok, or throws on syscall error
     */
    unsigned recvmmsg( void *bufBase, unsigned maxMsgSize,
                       std::vector<unsigned> &recvdMsgSizes ) const;

    // Allow individual recv()/send() calls to last for 'timeout' seconds before returning EWOULDBLOCK
    void setTimeout(double timeout);

    int getPort() const { return port; }

  private:
    const std::string hostname;
    uint16 port;
    int listen_sk;

    void accept(time_t timeout);
};

EXCEPTION_CLASS(TimeOutException, LOFAR::Exception);

} // namespace LOFAR

#endif
