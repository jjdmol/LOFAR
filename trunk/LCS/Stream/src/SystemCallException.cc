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

  if (strerror_r(error, buffer, sizeof buffer) == 0)
    return std::string(buffer);
  else
    return "could not convert error to string";
}


} // namespace LOFAR
