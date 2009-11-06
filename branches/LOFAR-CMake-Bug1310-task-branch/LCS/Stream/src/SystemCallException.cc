#include <lofar_config.h>

#include <Stream/SystemCallException.h>

#include <cstring>


namespace LOFAR {

SystemCallException::SystemCallException(const char *syscall, int error, const std::string& file, int line, 
			const std::string& func, Backtrace* bt) throw()
: Exception(std::string(syscall) + ": " + errorMessage(error), file, line, func, bt),
  error(error)
{
}


SystemCallException::~SystemCallException() throw()
{
}


const std::string& SystemCallException::type() const
{
  static const std::string theType("SystemCallException");
  return theType;
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
