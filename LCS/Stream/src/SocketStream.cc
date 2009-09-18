//# SocketStream.cc: 
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

#include <lofar_config.h>

#include <Stream/SocketStream.h>
#include <Stream/SystemCallException.h>

#include <cstring>
#include <cstdio>

#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


namespace LOFAR {

SocketStream::SocketStream(const char *hostname, short port, Protocol protocol, Mode mode)
{
  if (protocol == TCP)
    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  else
    fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if (fd < 0)
    throw SystemCallException("socket", errno, THROW_ARGS);

  struct sockaddr_in sa;
  struct hostent     *host;

  if ((host = gethostbyname(hostname)) == 0)
    throw SystemCallException("gethostbyname", errno, THROW_ARGS);

  memset(&sa, 0, sizeof sa);
  sa.sin_family = AF_INET;
  sa.sin_port   = htons(port);
  memcpy(&sa.sin_addr, host->h_addr, host->h_length);

  if (mode == Client) {
    while (connect(fd, (struct sockaddr *) &sa, sizeof sa) < 0)
      if (errno == ECONNREFUSED)
	sleep(1);
      else
	throw SystemCallException("connect", errno, THROW_ARGS);
  } else {
    int on = 1;

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on) < 0)
      throw SystemCallException("setsockopt(SO_REUSEADDR)", errno, THROW_ARGS);

    if (bind(fd, (struct sockaddr *) &sa, sizeof sa) < 0)
      throw SystemCallException("bind", errno, THROW_ARGS);

    if (protocol == TCP) {
      if (listen(fd, 5) < 0)
	throw SystemCallException("listen", errno, THROW_ARGS);

      int sk;

      if ((sk = accept(fd, 0, 0)) < 0)
	throw SystemCallException("accept", errno, THROW_ARGS);

      if (close(fd) < 0)
	throw SystemCallException("close", errno, THROW_ARGS);

      fd = sk;
    }
  }
}


SocketStream::~SocketStream()
{
}


void SocketStream::setReadBufferSize(size_t size)
{
  if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof size) < 0)
    perror("setsockopt failed");}
} // namespace LOFAR
