#include <lofar_config.h>

#include <Stream/FileDescriptorBasedStream.h>
#include <Stream/SystemCallException.h>

#include <unistd.h>

#include <stdexcept>


namespace LOFAR {

FileDescriptorBasedStream::~FileDescriptorBasedStream()
{
  if (close(fd) < 0)
    throw SystemCallException("close", errno, THROW_ARGS);
}


void FileDescriptorBasedStream::read(void *ptr, size_t size)
{
  while (size > 0) {
    const ssize_t bytes = ::read(fd, ptr, size);
    
    if (bytes < 0)
      throw SystemCallException("read", errno, THROW_ARGS);

    if (bytes == 0) 
      throw EndOfStreamException("read", THROW_ARGS);

    size -= bytes;
    ptr   = static_cast<char *>(ptr) + bytes;
  }
}


void FileDescriptorBasedStream::write(const void *ptr, size_t size)
{
  while (size > 0) {
    const ssize_t bytes = ::write(fd, ptr, size);

    if (bytes < 0)
      throw SystemCallException("write", errno, THROW_ARGS);

    size -= bytes;
    ptr   = static_cast<const char *>(ptr) + bytes;
  }
}

} // namespace LOFAR
