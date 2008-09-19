//#  Exception.h: LCS Exception base class.
//#
//#  Copyright (C) 2002
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_COMMON_EXCEPTION_H
#define LOFAR_COMMON_EXCEPTION_H

// \file
// LCS Exception base class.

//# Includes
//# Never #include <config.h> or #include <lofar_config.h> in a header file!
#include <exception>
#include <string>
#include <sstream>

#ifdef HAVE_BACKTRACE
# include <Common/Backtrace.h>
#endif

namespace LOFAR {

  // \addtogroup Common
  // @{

  //
  // This is the base class for all LCS related exceptions.
  //
  class Exception : public std::exception
  {
  public:
    Exception(const std::string& text, const std::string& file,
	      int line, const std::string& func) :
      itsText(text), itsFile(file), itsLine(line), itsFunction(func)
    {}
      

#if defined(HAVE_BACKTRACE)
    Exception(const std::string& text, const std::string& file,
	      int line, const std::string& func,
              const Backtrace& bt) :
      itsText(text), itsFile(file), itsLine(line), itsFunction(func), 
      itsBacktrace(bt)
    {}
#endif

    virtual ~Exception() throw() {}

    // Implementation of std::exception::what().
    // Returns the user-supplied text as C-style char array.
    virtual const char* what() const throw();

    // Return the class type of the exception.
    virtual const std::string& type() const {
      static const std::string itsType("Exception");
      return itsType;
    }

    // Return the user-supplied text
    const std::string& text() const {
      return itsText;
    }

    // Return the name of the file where the exception occurred.
    const std::string& file() const {
      return itsFile;
    }

    // Return the line number where the exception occurred.
    int line() const {
      return itsLine;
    }

    // Return the function name where the exception occurred.
    const std::string& function() const {
      return itsFunction;
    }

#if defined(HAVE_BACKTRACE)
    // Return the backtrace from where the exception occurred.
    const Backtrace& backtrace() const {
      return itsBacktrace;
    }
#endif

    // Return exception type, user-supplied text, filename, line number, 
    // function name, and (if available) backtrace as a formatted string.
    const std::string message() const;
    
  private:
    std::string itsText;
    std::string itsFile;
    int         itsLine;
    std::string itsFunction;
#if defined(HAVE_BACKTRACE)
    Backtrace   itsBacktrace;
#endif

  };

  // Put the exception message into an ostream.
  inline std::ostream& operator<<(std::ostream& os, const Exception& ex)
  {
    return os << ex.message();
  }

  // @}

} // namespace LOFAR


//
// \name Useful macros for lazy people
//@{

//
//  Define the \c THROW_ARGS macro, using \c AUTO_FUNCTION_NAME
//
#if defined(HAVE_BACKTRACE)
# define THROW_ARGS __FILE__, __LINE__, AUTO_FUNCTION_NAME, Backtrace()
#else
# define THROW_ARGS __FILE__, __LINE__, AUTO_FUNCTION_NAME
#endif

//
// Declare and define an exception class of type \c excp, which is derived
// from the exception class \c super.
//
#if defined(HAVE_BACKTRACE)
# define EXCEPTION_CLASS(excp,super)                       \
  class excp : public super                                \
  {                                                        \
  public:                                                  \
    excp(const std::string& text, const std::string& file, \
         int line, const std::string& function,            \
         const Backtrace& bt) :                            \
      super(text, file, line, function, bt) {}             \
      virtual const std::string& type() const              \
      {                                                    \
        static const std::string itsType(#excp);           \
        return itsType;                                    \
      }                                                    \
  }
#else
# define EXCEPTION_CLASS(excp,super)                       \
  class excp : public super                                \
  {                                                        \
  public:                                                  \
    excp(const std::string& text, const std::string& file, \
         int line, const std::string& function ) :         \
      super(text, file, line, function) {}                 \
      virtual const std::string& type() const              \
      {                                                    \
        static const std::string itsType(#excp);           \
        return itsType;                                    \
      }                                                    \
  }
#endif

//
// Throw an exception of type \c excp; use \c strm for the message. 
// Use this macro to insure that the  \c THROW_ARGS macro expands properly.
//
#if !defined(THROW)
# define THROW(excp,strm) \
do { \
  std::ostringstream oss; \
  oss << strm; \
  throw excp(oss.str(),THROW_ARGS); \
} while(0)
#endif

//@}

#endif
