#include <lofar_config.h>

#include <Stream/SystemCallException.h>

#include <cstdio>
#include <string>


namespace LOFAR {

SystemCallException::SystemCallException(const char *syscall, int error) throw()
:
  runtime_error(std::string(syscall) + ": " + sys_errlist[error]),
  error(error)
{
}


SystemCallException::~SystemCallException() throw()
{
}


} // namespace LOFAR
