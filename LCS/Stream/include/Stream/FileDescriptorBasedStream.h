#ifndef LOFAR_LCS_STREAM_FILE_DESRIPTOR_BASED_STREAM_H
#define LOFAR_LCS_STREAM_FILE_DESRIPTOR_BASED_STREAM_H

#include <Stream/Stream.h>


namespace LOFAR {

class FileDescriptorBasedStream : public Stream
{
  public:
    virtual	 ~FileDescriptorBasedStream();

    virtual void read(void *ptr, size_t size);
    virtual void write(const void *ptr, size_t size);

  protected:
		 FileDescriptorBasedStream() {} // do not create directly

    int		 fd;
};

} // namespace LOFAR

#endif
