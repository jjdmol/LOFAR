#ifndef LOFAR_LCS_STREAM_SOCKET_STREAM_H
#define LOFAR_LCS_STREAM_SOCKET_STREAM_H

#include <Stream/FileDescriptorBasedStream.h>


namespace LOFAR {

class SocketStream : public FileDescriptorBasedStream
{
  public:
    enum Protocol {
      TCP, UDP
    };

    enum Mode {
      Client, Server
    };

  	    SocketStream(const char *hostname, short port, Protocol, Mode);
    virtual ~SocketStream();

    void setReadBufferSize(size_t size);
};

} // namespace LOFAR

#endif
