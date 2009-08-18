#ifndef LOFAR_LCS_STREAM_STREAM_H
#define LOFAR_LCS_STREAM_STREAM_H

#include <cstddef>
#include <Common/Exception.h>


namespace LOFAR {

class Stream
{
  public:
    EXCEPTION_CLASS( EndOfStreamException, LOFAR::Exception );

    virtual	 ~Stream();

    virtual void read(void *ptr, size_t size) = 0;
    virtual void write(const void *ptr, size_t size) = 0;
};

} // namespace LOFAR

#endif
