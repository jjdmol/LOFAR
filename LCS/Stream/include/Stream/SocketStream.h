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
                 time_t deadline = 0, const std::string &nfskey = "", bool doAccept = true);
    virtual ~SocketStream();

    FileDescriptorBasedStream *detach();

    void    reaccept(time_t deadline = 0); // only for TCP server socket
    void    setReadBufferSize(size_t size);

    const Protocol protocol;
    const Mode mode;

    /*
     * Receive message(s). Note: only for UDP server socket!
     *   @buffers contains ptrs + sizes wrt the (max) nr of messages to receive
     *   @recvdMsgSizes will be populated with the received message sizes.
     *     Note: @recvdMsgSizes must be (at least) the size of buffers!
     * Returns the number of messages received if ok, or throws on syscall error
     */
    unsigned recvmmsg( std::vector<struct iovec> &buffers,
                       std::vector<unsigned> &recvdMsgSizes );

    // Allow individual recv()/send() calls to last for 'timeout' seconds before returning EWOULDBLOCK
    void setTimeout(double timeout);

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

EXCEPTION_CLASS(TimeOutException, LOFAR::Exception);

} // namespace LOFAR

#endif
