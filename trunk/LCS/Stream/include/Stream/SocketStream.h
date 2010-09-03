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
#include <Common/SystemCallException.h>
#include <Stream/FileDescriptorBasedStream.h>

#include <time.h>
#include <string>

namespace LOFAR {

class SocketStream : public FileDescriptorBasedStream
{
  public:
    EXCEPTION_CLASS(TimeOutException, LOFAR::Exception);
    SYSTEMCALLEXCEPTION_CLASS(BindException, LOFAR::SystemCallException);

    enum Protocol {
      TCP, UDP
    };

    enum Mode {
      Client, Server
    };

  	    SocketStream(const char *hostname, uint16_t port, Protocol, Mode, time_t timeout = 0, const char *nfskey = 0);
    virtual ~SocketStream();

    void    reaccept(time_t timeout = 0); // only for TCP server socket
    void    setReadBufferSize(size_t size);

  private:
    const char *hostname;
    uint16_t port;
    const Protocol protocol;
    const Mode mode;
    const char *nfskey;
    int     listen_sk;

    void accept(time_t timeout);

    static std::string readkey(const char *nfskey, time_t &timeout);
    static void writekey(const char *nfskey, uint16_t port);
    static void deletekey(const char *nfskey);
};

} // namespace LOFAR

#endif
