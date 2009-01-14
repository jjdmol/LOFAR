#include <lofar_config.h>

#include <Stream/NullStream.h>

#include <cstring>


namespace LOFAR {

NullStream::~NullStream()
{
}


void NullStream::read(void *ptr, size_t size)
{
  memset(ptr, 0, size);
}


void NullStream::write(const void *, size_t)
{
}

} // namespace LOFAR
