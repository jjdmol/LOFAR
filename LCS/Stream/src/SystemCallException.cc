#include <lofar_config.h>

#include <Stream/SystemCallException.h>

#include <cstring>


namespace LOFAR {

SystemCallException::SystemCallException(const char *syscall, int error) throw()
:
  runtime_error(std::string(syscall) + ": " + errorMessage(error)),
  error(error)
{
}


SystemCallException::~SystemCallException() throw()
{
}


std::string SystemCallException::errorMessage(int error)
{
  char buffer[128];

  // there are two incompatible versions of versions of strerror_r()

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
  if (strerror_r(error, buffer, sizeof buffer) == 0)
    return std::string(buffer);
  else
    return "could not convert error to string";
#else
  return strerror_r(error, buffer, sizeof buffer);
#endif
}


} // namespace LOFAR
