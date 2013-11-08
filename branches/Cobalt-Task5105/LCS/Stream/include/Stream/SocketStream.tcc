#include "SocketStream.h"
#include <sys/socket.h>
#include <sys/types.h>

#ifdef __linux__
// Actually, recvmmsg is supported by Linux 2.6.32+ using glibc 2.12+
#define HAVE_RECVMMSG
#endif

namespace LOFAR {

template<typename T> size_t SocketStream::recvmmsg( std::vector<T> &buffers, bool oneIsEnough )
{
  ASSERT(protocol == UDP);
  ASSERT(mode == Server);

#ifdef HAVE_RECVMMSG
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
  int numRead = ::recvmmsg(fd, &msgs[0], n, oneIsEnough ? MSG_WAITFORONE : 0, 0);

  if (numRead < 0)
    THROW_SYSCALL("recvmmsg");

  /* msgs[i].msg_len contains the number of bytes received for packet i */

  return numRead;
#else
  // recvmmsg not available: use recvmsg fall-back

  struct iovec iov;
  iov.iov_base = &buffers[0];
  iov.iov_len  = sizeof (T);

  struct msghdr msg;
  msg.msg_iov     = &iov;
  msg.msg_iovlen  = 1;
  msg.msg_name    = NULL; // we don't need to know who sent the data
  msg.msg_control = NULL; // we're not interested in OoB data

  if (recvmsg(fd, &msg, 0) < 0)
    THROW_SYSCALL("recvmsg");

  /* the recvmsg return value is the number of bytes received */

  return 1;
#endif
}

} // namespace LOFAR

