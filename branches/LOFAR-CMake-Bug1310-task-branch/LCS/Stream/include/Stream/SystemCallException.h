#ifndef LOFAR_LCS_STREAM_SYSTEM_CALL_EXCEPTION_H
#define LOFAR_LCS_STREAM_SYSTEM_CALL_EXCEPTION_H

#include <cerrno>
#include <Common/Exception.h>
#include <string>


namespace LOFAR {

class SystemCallException : public Exception
{
  public:
    SystemCallException(const char *syscall, int error=errno, const std::string& file="", int line=0, 
			const std::string& func="", Backtrace* bt=0) throw();
			
    virtual		~SystemCallException() throw();
    virtual const std::string& type() const;
    
    const int		error;

  private:
    static std::string	errorMessage(int error);
};

} // namespace LOFAR

#endif
