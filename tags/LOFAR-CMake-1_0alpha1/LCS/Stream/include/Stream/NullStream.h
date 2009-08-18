#ifndef LOFAR_LCS_STREAM_NULL_STREAM_H
#define LOFAR_LCS_STREAM_NULL_STREAM_H

#include <Stream/Stream.h>

#include <errno.h>


namespace LOFAR {

class NullStream : public Stream
{
  public:
    virtual	 ~NullStream();

    virtual void read(void *ptr, size_t size);
    virtual void write(const void *ptr, size_t size);
};

} // namespace LOFAR

#endif
