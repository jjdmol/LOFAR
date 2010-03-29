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
#include <Stream/FileDescriptorBasedStream.h>

#include <time.h>


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

  	    SocketStream(const char *hostname, short port, Protocol, Mode, time_t timeout = 0);
    virtual ~SocketStream();

    void setReadBufferSize(size_t size);
};

} // namespace LOFAR

#endif
