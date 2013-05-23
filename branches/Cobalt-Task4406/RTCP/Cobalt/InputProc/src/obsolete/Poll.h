//# Poll.h
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#ifndef LOFAR_INPUT_PROC_POLL_H
#define LOFAR_INPUT_PROC_POLL_H

#include <sys/epoll.h>

#include <Stream/FileDescriptorBasedStream.h>
#include <Common/SystemCallException.h>

class Poll : protected FileDescriptorBasedStream
{
public:
  Poll();

  // Caveats:
  //   * Don't add a stream that's already in the set
  //   * You might want to call s->setnonblocking() as well,
  //     or your read()/write() can still block.
  void add( FileDescriptorBasedStream *s, bool reading, bool writing );

  // Note: closing the file descriptor automatically removes
  // the stream from the list, see man epoll.
  void remove( FileDescriptorBasedStream *s );

  // Wait for timeout_ms milliseconds for events, and return
  // the relevant streams. Up to maxevents streams are returned.
  std::vector<FileDescriptorBasedStream *> poll( int timeout_ms, size_t maxevents );
};

Poll::Poll()
{
  fd = epoll_create1(EPOLL_CLOEXEC);

  if( fd == -1 )
    throw SystemCallException("epoll_create1", errno, THROW_ARGS);
}

void Poll::add( FileDescriptorBasedStream *s, bool reading, bool writing )
{
  ASSERT( s->fd >= 0 );

  struct epoll_event ev;
  ev.events = (reading ? EPOLLIN : 0) | (writing ? EPOLLOUT : 0);
  ev.data.ptr = s;

  if (epoll_ctl(fd, EPOLL_CTL_ADD, s->fd, &ev) == -1)
    throw SystemCallException("epoll_ctl", errno, THROW_ARGS);
}

void Poll::remove( FileDescriptorBasedStream *s )
{
  ASSERT( s->fd >= 0 );

  struct epoll_event ev;

  if (epoll_ctl(fd, EPOLL_CTL_DEL, s->fd, &ev) == -1)
    throw SystemCallException("epoll_ctl", errno, THROW_ARGS);
}

std::vector<FileDescriptorBasedStream *> Poll::poll( int timeout_ms, size_t maxevents )
{
  // In theory, starvation can occur under heavy I/O if maxevents < #streams. If
  // this is to be avoided, extend this class to employ a ready list as
  // described in 'man epoll'.
  std::vector<struct epoll_event> events(maxevents);
  int nfds;

  nfds = epoll_wait(fd, &events[0], events.size(), timeout_ms );

  if (nfds == -1)
    throw SystemCallException("epoll_wait", errno, THROW_ARGS);

  std::vector<FileDescriptorBasedStream *> result(nfds, 0);

  for (int i = 0; i < nfds; ++i) {
    FileDescriptorBasedStream *s = static_cast<FileDescriptorBasedStream*>(events[i].data.ptr);

    results[i] = s;
  }

  return result;
}

#endif

