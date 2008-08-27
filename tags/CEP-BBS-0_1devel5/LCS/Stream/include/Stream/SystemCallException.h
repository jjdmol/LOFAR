#ifndef LOFAR_LCS_STREAM_SYSTEM_CALL_EXCEPTION_H
#define LOFAR_LCS_STREAM_SYSTEM_CALL_EXCEPTION_H

#include <cerrno>
#include <stdexcept>
#include <string>


namespace LOFAR {

class SystemCallException : public std::runtime_error
{
  public:
			SystemCallException(const char *syscall, int error = errno) throw();
    virtual		~SystemCallException() throw();

    const int		error;

  private:
    static std::string	errorMessage(int error);
};

} // namespace LOFAR

#endif
