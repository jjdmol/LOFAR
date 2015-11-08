//# SocketStream.cc: 
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

#include <lofar_config.h>

#include <Stream/SocketStream.h>

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <cerrno>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netdb.h>
#include <dirent.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include <Common/SystemCallException.h>
#include <Common/Thread/Cancellation.h>
#include <Common/LofarLogger.h>

#include "NetFuncs.h"

//# AI_NUMERICSERV is not defined on OS-X
#ifndef AI_NUMERICSERV
# define AI_NUMERICSERV 0
#endif

using boost::format;


namespace LOFAR {

SocketStream::SocketStream(const std::string &hostname, uint16 _port, Protocol protocol,
                           Mode mode, time_t deadline, bool doAccept, const std::string &bind_local_iface)
:
  protocol(protocol),
  mode(mode),
  hostname(hostname),
  port(_port),
  listen_sk(-1)
{  
  const std::string description = str(boost::format("%s:%s:%s")
      % (protocol == TCP ? "tcp" : "udp")
      % hostname
      % _port);

  for(;;) {
    try {
      // Not all potentially blocking calls below have a timeout arg.
      // So check early every round. It may abort hanging tests early (=good).
      if (deadline > 0 && deadline <= time(0))
        THROW(TimeOutException, "SocketStream");

      // Resolve host + port to a 'struct addrinfo'
      safeAddrInfo result;
      safeGetAddrInfo(result, protocol == SocketStream::TCP, hostname, port);

      // result is a linked list of resolved addresses, we only use the first

      if ((fd = socket(result.addrinfo->ai_family, result.addrinfo->ai_socktype, result.addrinfo->ai_protocol)) < 0)
        THROW_SYSCALL("socket");

      if (bind_local_iface != "") {
        // Bind socket to a specific network interface, causing packets to be
        // emitted through this interface.
        //
        // Requires CAP_NET_RAW (or root)
        LOG_DEBUG_STR("Binding socket " << description << " to interface " << bind_local_iface);

        struct ifreq ifr;
        memset(&ifr, 0, sizeof ifr);
        snprintf(ifr.ifr_name, sizeof ifr.ifr_name, "%s", bind_local_iface.c_str());

        if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof ifr) < 0)
          try {
            THROW_SYSCALL("setsockopt(SO_BINDTODEVICE)");
          } catch(Exception &ex) {
            LOG_ERROR_STR("Could not bind socket to device " << bind_local_iface << ": " << ex);
          }
      }

      if (mode == Client) {
        if (bind_local_iface != "") {
          struct sockaddr sa = getInterfaceIP(bind_local_iface);
          LOG_DEBUG_STR("Binding socket " << description << " to IP " << inet_ntoa(((struct sockaddr_in*)&sa)->sin_addr) << " of interface " << bind_local_iface);

          if (bind(fd, &sa, sizeof sa) < 0)
            THROW_SYSCALL(str(boost::format("bind [%s]") % description));
        }

        while (connect(fd, result.addrinfo->ai_addr, result.addrinfo->ai_addrlen) < 0)
          if (errno == ECONNREFUSED || errno == ETIMEDOUT) {
            if (deadline > 0 && time(0) >= deadline)
              throw TimeOutException("client socket", THROW_ARGS);

            if (usleep(999999) < 0) { // near 1 sec; max portably safe arg val
              // interrupted by a signal handler -- abort to allow this thread to
              // be forced to continue after receiving a SIGINT, as with any other
              // system call in this constructor 
              THROW_SYSCALL("usleep");
            }
          } else
            THROW_SYSCALL(str(boost::format("connect [%s]") % description));
      } else {
        const int on = 1;

        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on) < 0)
          THROW_SYSCALL("setsockopt(SO_REUSEADDR)");

        if (bind(fd, result.addrinfo->ai_addr, result.addrinfo->ai_addrlen) < 0)
          THROW_SYSCALL(str(boost::format("bind [%s]") % description));

        if (port == 0) {
          // we let OS search for a free port
          port = getSocketPort(fd);

          LOG_DEBUG(str(boost::format("Bound socket %s to port %s") % description % port));
        }

        if (protocol == TCP) {
          listen_sk = fd;
          fd	= -1;

          const int listenBacklog = 15;
          if (listen(listen_sk, listenBacklog) < 0)
            THROW_SYSCALL(str(boost::format("listen [%s]") % description));

          if (doAccept)
            accept(deadline);
          else
            break;
        }
      }

      // we have an fd! break out of the infinite loop
      break;
    } catch (...) {
      if (listen_sk >= 0) { close(listen_sk); listen_sk = -1; }
      if (fd >= 0)        { close(fd);        fd = -1; }

      throw;
    }
  }
}


SocketStream::~SocketStream()
{
  ScopedDelayCancellation dc; // close() can throw as it is a cancellation point

  if (listen_sk >= 0 && close(listen_sk) < 0) {
    // try/throw/catch to match patterns elsewhere. 
    //
    // This ensures a proper string for errno, a
    // backtrace if available, and the proper representation
    // of exceptions in general.
    try {
      THROW_SYSCALL("close listen_sk");
    } catch (Exception &ex) {
      LOG_ERROR_STR("Exception in destructor: " << ex);
    }
  }
}


FileDescriptorBasedStream *SocketStream::detach()
{
  ASSERT( mode == Server );

  FileDescriptorBasedStream *client = new FileDescriptorBasedStream(fd);

  fd = -1;

  return client;
}


void SocketStream::reaccept(time_t deadline)
{
  ASSERT( mode == Server );

  if (fd >= 0 && close(fd) < 0)
    THROW_SYSCALL("close");

  accept(deadline);
}


void SocketStream::accept(time_t deadline)
{
  if (deadline > 0) {
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(listen_sk, &fds);

    struct timeval timeval;

    time_t now = time(0);
    
    if (now > deadline)
      THROW(TimeOutException, "server socket");

    timeval.tv_sec  = deadline - now;
    timeval.tv_usec = 0;

    switch (select(listen_sk + 1, &fds, 0, 0, &timeval)) {
      case -1 : THROW_SYSCALL("select");

      case  0 : THROW(TimeOutException, "server socket");
    }
  }

  if ((fd = ::accept(listen_sk, 0, 0)) < 0)
    THROW_SYSCALL("accept");
}


void SocketStream::setReadBufferSize(size_t size)
{
  if (fd >= 0 && setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof size) < 0)
    THROW_SYSCALL("setsockopt(SO_RCVBUF)");
}


void SocketStream::setTimeout(double timeout)
{
  ASSERT(timeout >= 0.0);

  struct timeval tv;
  tv.tv_sec = static_cast<long>(timeout);
  tv.tv_usec = static_cast<long>((timeout - floor(timeout)) * 1000.0 * 1000.0);

  if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof tv) < 0)
    THROW_SYSCALL("setsockopt(SO_RCVTIMEO)");

  if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof tv) < 0)
    THROW_SYSCALL("setsockopt(SO_SNDTIMEO)");
}


#if defined __linux__ && __GLIBC_PREREQ(2,12)
// Actually, recvmmsg is supported by Linux 2.6.32+ using glibc 2.12+
#define HAVE_RECVMMSG
#else
// Equalize data structures to share more code. Declared within LOFAR namespace.
// Need it as vector template arg, so cannot be locally declared (ok w/ C++11).
struct mmsghdr {
  struct msghdr msg_hdr;
};
#endif

unsigned SocketStream::recvmmsg( void *bufBase, unsigned maxMsgSize,
                                 std::vector<unsigned> &recvdMsgSizes ) const
{
  ASSERT(protocol == UDP);
  ASSERT(mode == Server);

  // If recvmmsg() is not available, then use recvmsg() (1 call) as fall-back.
#ifdef HAVE_RECVMMSG
  const unsigned numBufs = recvdMsgSizes.size();
#else
  const unsigned numBufs = 1;
#endif

  // register our receive buffer(s)
  std::vector<struct iovec> iov(numBufs);
  for (unsigned i = 0; i < numBufs; i++) {
    iov[i].iov_base = (char*)bufBase + i * maxMsgSize;
    iov[i].iov_len  = maxMsgSize;
  }

  std::vector<struct mmsghdr> msgs(numBufs);
  for (unsigned i = 0; i < numBufs; ++i) {
    msgs[i].msg_hdr.msg_name    = NULL; // we don't need to know who sent the data
    msgs[i].msg_hdr.msg_iov     = &iov[i];
    msgs[i].msg_hdr.msg_iovlen  = 1;
    msgs[i].msg_hdr.msg_control = NULL; // we're not interested in OoB data
  }

  int numRead;
#ifdef HAVE_RECVMMSG
  // Note: the timeout parameter doesn't work as expected: only between datagrams
  // is the timeout checked, not when waiting for one (i.e. numBufs=1 or MSG_WAITFORONE).
  numRead = ::recvmmsg(fd, &msgs[0], numBufs, 0, NULL);
  if (numRead < 0)
    THROW_SYSCALL("recvmmsg");

  for (int i = 0; i < numRead; ++i) {
    recvdMsgSizes[i] = msgs[i].msg_len; // num bytes received is stored in msg_len by recvmmsg()
  }
#else
  numRead = ::recvmsg(fd, &msgs[0].msg_hdr, 0);
  if (numRead < 0)
    THROW_SYSCALL("recvmsg");

  recvdMsgSizes[0] = static_cast<unsigned>(numRead); // num bytes received is returned by recvmsg()
  numRead = 1; // equalize return val semantics to num msgs received
#endif

  return static_cast<unsigned>(numRead);
}

} // namespace LOFAR
