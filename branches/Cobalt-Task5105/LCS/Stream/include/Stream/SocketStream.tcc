#include "SocketStream.h"
#include <sys/socket.h>
#include <sys/types.h>

namespace LOFAR {

template<typename T> size_t SocketStream::recvmmsg( std::vector<T> &buffers )
{
  ASSERT(protocol == UDP);
  ASSERT(mode == Server);

  const size_t n = buffers.size();

  // set of receive buffers
  std::vector<struct iovec> iov(n);

  for(size_t i = 0; i < n; ++i) {
    iov[i].iov_base = &buffers[i];
    iov[i].iov_len  = sizeof (T);
  }

  // recvmsg parameter struct
  std::vector<struct mmsghdr> msgs(n);

  // register our receive buffers
  for(size_t i = 0; i < n; ++i) {
    msgs[i].msg_hdr.msg_iov     = &iov[i];
    msgs[i].msg_hdr.msg_iovlen  = 1;
    msgs[i].msg_hdr.msg_name    = NULL; // we don't need to know who sent the data
    msgs[i].msg_hdr.msg_control = NULL; // we're not interested in OoB data
  }

  // receive data
  int numRead = ::recvmmsg(fd, &msgs[0], n, MSG_WAITFORONE, 0);

  if (numRead < 0)
    THROW_SYSCALL("recvmmsg");

  return numRead;
}

} // namespace LOFAR

