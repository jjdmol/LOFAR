#ifndef POLL
#define POLL

#include <Stream/FileDescriptorBasedStream.h>
#include <Common/SystemCallException.h>
#include <sys/epoll.h>

class Poll: protected FileDescriptorBasedStream {
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

