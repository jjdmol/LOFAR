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

#include <Common/LofarLogger.h>
#include <Stream/SocketStream.h>

#include <cstring>
#include <cstdio>

#include <errno.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>

#include <boost/lexical_cast.hpp>


namespace LOFAR {

// port range for unused ports search
const int MINPORT = 10000;
const int MAXPORT = 30000;

SocketStream::SocketStream(const char *hostname, uint16 _port, Protocol protocol, Mode mode, time_t timeout, const char *nfskey )
:
  protocol(protocol),
  mode(mode),
  hostname(hostname),
  port(_port),
  nfskey(nfskey),
  listen_sk(-1)
{  
  struct addrinfo hints;
  bool            autoPort = (port == 0);

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

  for(;;) {
    try {
      try {
        char portStr[16];
        int  retval;
        struct addrinfo *result;

        if (mode == Client && nfskey)
          port = boost::lexical_cast<uint16>(readkey(nfskey, timeout));

        if (mode == Server && autoPort && port == 0)
          port = MINPORT;

        snprintf(portStr, sizeof portStr, "%hu", port);

        if ((retval = getaddrinfo(hostname, portStr, &hints, &result)) != 0)
          throw SystemCallException("getaddrinfo", retval, THROW_ARGS);

        // make sure result will be freed
        struct D {
          ~D() {
            freeaddrinfo(result);
          }

          struct addrinfo *result;
        } onDestruct = { result };
        (void)onDestruct;

        // result is a linked list of resolved addresses, we only use the first

        if ((fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) < 0)
          throw SystemCallException("socket", errno, THROW_ARGS);

        if (mode == Client) {
          time_t latestTime = time(0) + timeout;

          while (connect(fd, result->ai_addr, result->ai_addrlen) < 0)
            if (errno == ECONNREFUSED) {
              if (timeout > 0 && time(0) >= latestTime)
                throw TimeOutException("client socket", THROW_ARGS);

              if (usleep(999999) > 0) {
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
            throw BindException("bind", errno, THROW_ARGS);

          if (protocol == TCP) {
            listen_sk = fd;
            fd	= -1;

            if (listen(listen_sk, 5) < 0)
              throw BindException("listen", errno, THROW_ARGS);

            accept( timeout );  
          }
        }

        // we have an fd! break out of the infinite loop
        break;
      } catch (...) {
        if (listen_sk >= 0) { close(listen_sk); listen_sk = -1; }
        if (fd >= 0)        { close(fd);        fd = -1; }

        throw;
      }
    } catch (BindException &e) {
      if (mode == Server && autoPort && ++port < MAXPORT)
        continue;
      else
        throw;
    }
  }
}


SocketStream::~SocketStream()
{
  if (listen_sk >= 0 && close(listen_sk) < 0)
    throw SystemCallException("close listen_sk", errno, THROW_ARGS);
}


void SocketStream::reaccept( time_t timeout )
{
  ASSERT( mode == Server );

  if (fd >= 0 && close(fd) < 0)
    throw SystemCallException("close", errno, THROW_ARGS);

  accept( timeout );  
}


void SocketStream::accept( time_t timeout )
{
  if (nfskey)
    writekey(nfskey, port);

  // make sure the key will be deleted
  struct D {
    ~D() {
      if (nfskey)
        deletekey(nfskey);
    }

    const char *nfskey;
  } onDestruct = { nfskey };
  (void)onDestruct;

  if (timeout > 0) {
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(listen_sk, &fds);

    struct timeval timeval;

    timeval.tv_sec  = timeout;
    timeval.tv_usec = 0;

    switch (select(listen_sk + 1, &fds, 0, 0, &timeval)) {
      case -1 : throw SystemCallException("select", errno, THROW_ARGS);

      case  0 : throw TimeOutException("server socket", THROW_ARGS);
    }
  }

  if ((fd = ::accept(listen_sk, 0, 0)) < 0)
    throw SystemCallException("accept", errno, THROW_ARGS);
}


void SocketStream::setReadBufferSize(size_t size)
{
  if (fd >= 0 && setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof size) < 0)
    perror("setsockopt failed");
}


std::string SocketStream::readkey(const char *nfskey, time_t &timeout)
{
  for(;;) {
    char portStr[16];
    ssize_t len;

    len = readlink(nfskey, portStr, sizeof portStr);

    if (len >= 0) {
      portStr[len] = 0;
      return std::string(portStr);
    }

    if (timeout == 0)
      throw TimeOutException("client socket", THROW_ARGS);

    if (usleep(999999) > 0) {
      // interrupted by a signal handler -- abort to allow this thread to
      // be forced to continue after receiving a SIGINT, as with any other
      // system call
      throw SystemCallException("sleep", errno, THROW_ARGS);
    }

    timeout--;
  }
}

void SocketStream::writekey(const char *nfskey, uint16 port)
{
  char portStr[16];

  snprintf(portStr, sizeof portStr, "%hu", port);

  // Symlinks can be atomically created over NFS
  if (symlink(portStr, nfskey) < 0)
    throw SystemCallException("symlink", errno, THROW_ARGS);
}

void SocketStream::deletekey(const char *nfskey)
{
  if (unlink(nfskey) < 0)
    throw SystemCallException("unlink", errno, THROW_ARGS);
}

} // namespace LOFAR
