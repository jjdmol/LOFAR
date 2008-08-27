#include <lofar_config.h>

#include <Stream/SocketStream.h>
#include <Stream/SystemCallException.h>

#include <cstring>

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
    throw SystemCallException("socket");

  struct sockaddr_in sa;
  struct hostent     *host;

  if ((host = gethostbyname(hostname)) == 0)
    throw SystemCallException("gethostbyname");

  memset(&sa, 0, sizeof sa);
  sa.sin_family = AF_INET;
  sa.sin_port   = htons(port);
  memcpy(&sa.sin_addr, host->h_addr, host->h_length);

  if (mode == Client) {
    while (connect(fd, (struct sockaddr *) &sa, sizeof sa) < 0)
      if (errno == ECONNREFUSED)
	sleep(1);
      else
	throw SystemCallException("connect");
  } else {
    if (bind(fd, (struct sockaddr *) &sa, sizeof sa) < 0)
      throw SystemCallException("bind");

    if (protocol == TCP) {
      if (listen(fd, 5) < 0)
	throw SystemCallException("listen");

      int sk;

      if ((sk = accept(fd, 0, 0)) < 0)
	throw SystemCallException("accept");

      if (close(fd) < 0)
	throw SystemCallException("close");

      fd = sk;
    }
  }
}


SocketStream::~SocketStream()
{
}

} // namespace LOFAR
