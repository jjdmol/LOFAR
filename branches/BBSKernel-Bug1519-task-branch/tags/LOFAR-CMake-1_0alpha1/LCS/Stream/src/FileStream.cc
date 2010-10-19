#include <lofar_config.h>

#include <Stream/FileStream.h>
#include <Stream/SystemCallException.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


namespace LOFAR {

FileStream::FileStream(const char *name)
{
  if ((fd = open(name, O_RDONLY)) < 0)
    throw SystemCallException("open", errno, THROW_ARGS);
}


FileStream::FileStream(const char *name, int mode)
{
  if ((fd = open(name, O_RDWR | O_CREAT | O_TRUNC, mode)) < 0)
    throw SystemCallException("open", errno, THROW_ARGS);
}


FileStream::FileStream(const char *name, int flags, int mode)
{
  if ((fd = open(name, flags, mode)) < 0) 
    throw SystemCallException("open", errno, THROW_ARGS);
}

FileStream::~FileStream()
{
}

} // namespace LOFAR
