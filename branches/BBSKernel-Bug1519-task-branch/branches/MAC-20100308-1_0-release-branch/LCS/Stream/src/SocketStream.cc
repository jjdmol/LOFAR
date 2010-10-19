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

// A class which autodestructs a struct addrinfo result list
class AddrInfoHolder {
  public:
    AddrInfoHolder( struct addrinfo *info ): info(info) {}
    ~AddrInfoHolder() { freeaddrinfo( info ); }
  private:
    struct addrinfo *info;
};

SocketStream::SocketStream(const char *hostname, short port, Protocol protocol, Mode mode)
{
  int retval;
  struct addrinfo hints;
  struct addrinfo *result;
  
  // use getaddrinfo, because gethostbyname is not thread safe
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET; // IPv4
  hints.ai_flags = AI_NUMERICSERV; // we only use numeric port numbers, not strings like "smtp"

  if (protocol == TCP) {
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
  } else {
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
  }

  char portStr[16];
  snprintf( portStr, sizeof portStr, "%hd", port );

  if ((retval = getaddrinfo(hostname, portStr, &hints, &result)) != 0 )
    throw SystemCallException("getaddrinfo", retval, THROW_ARGS);

  AddrInfoHolder infoHolder( result ); // make sure result will be freed

  // result is a linked list of resolved addresses, we only use the first

  if ((fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) < 0)
    throw SystemCallException("socket", errno, THROW_ARGS);

  if (mode == Client) {
    while (connect(fd, result->ai_addr, result->ai_addrlen) < 0)
      if (errno == ECONNREFUSED) {
	if (sleep(1) > 0) {
          // interrupted by a signal handler -- abort to allow this thread to
          // be forced to continue after receiving a SIGINT, as with any other
          // system call in this constructor 
 	  throw SystemCallException("sleep", errno, THROW_ARGS);
        }
      } else
	throw SystemCallException("connect", errno, THROW_ARGS);
  } else {
    int on = 1;

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on) < 0)
      throw SystemCallException("setsockopt(SO_REUSEADDR)", errno, THROW_ARGS);

    if (bind(fd, result->ai_addr, result->ai_addrlen) < 0)
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
