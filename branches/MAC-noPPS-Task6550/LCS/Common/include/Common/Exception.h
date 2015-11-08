//# Exception.h: LCS Exception base class.
//#
//# Copyright (C) 2002
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_COMMON_EXCEPTION_H
#define LOFAR_COMMON_EXCEPTION_H

// \file
// LCS Exception base class.

//# Includes
#include <exception>
#include <string>
#include <iosfwd>

# include <boost/shared_ptr.hpp>

#ifdef HAVE_BACKTRACE
# include <Common/Backtrace.h>
#else
namespace LOFAR { class Backtrace {}; }
#endif

#include <Common/Thread/Cancellation.h>

//# This might be undefined if used by an external package like ASKAP.
#ifndef AUTO_FUNCTION_NAME
# define AUTO_FUNCTION_NAME __FUNCTION__
#endif


namespace LOFAR
{
  // \ingroup Common
  // @{

  //
  // This is the base class for all LCS related exceptions.
  //
  class Exception : public std::exception
  {
  public:
    // Wrapper class to define a different terminate handler. The terminate
    // handler is the function that will be called by the runtime system when
    // exception handling must be abandoned.
    class TerminateHandler
    {
    public:
      // Constructor. Register \a handler as the current terminate handler.
      explicit TerminateHandler(std::terminate_handler handler) 
      { itsOldHandler = std::set_terminate(handler); }
      
      // Destructor. Unregister the current terminate handler; reset to
      // previous handler.
      ~TerminateHandler()
      { std::set_terminate(itsOldHandler); }
      
    private:
      // This is the old terminate handler. We need it in the destructor.
      std::terminate_handler itsOldHandler;
    };

    // Constructor. Create an exception object that may contain detailed
    // information about the exact location where it was constructed:
    // filename, line number, function name, and backtrace (if available).
    Exception(const std::string& text, const std::string& file="",
 	      int line=0, const std::string& func="", Backtrace* bt=0) :
      itsText(text), itsFile(file), itsLine(line), itsFunction(func), 
      itsBacktrace(bt)
    {
      // Don't allow cancellation points when an exception is being thrown.
      // This assumes that Exception objects are only created for the purpose
      // of throwing them, and will be destroyed at the end of a catch block.
      Cancellation::push_disable();
    }

    // Terminate handler. This terminate handler provides more feedback than
    // the default terminate handler. When terminate is called due to an
    // uncaught exception it will not only print the message, but also
    // filename, line number, function name, and backtrace (if available).
    static void terminate();

    virtual ~Exception() throw() {
      Cancellation::pop_disable();
    }

    // Implementation of std::exception::what().
    // Returns the user-supplied text as C-style char array.
    virtual const char* what() const throw() {
      return itsText.c_str();
    }

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

    // Print the exception object in a human-readable form, consisting of:
    // exception type, user-supplied text, filename, line number, function
    // name, and backtrace (if available).
    void print(std::ostream& os) const;
    
    // Return the exception object as a formatted string using the
    // Exception::print() method.
    const std::string message() const;

  private:
    std::string itsText;
    std::string itsFile;
    int         itsLine;
    std::string itsFunction;
    boost::shared_ptr<Backtrace> itsBacktrace;
  };

  // Print the exception \a ex into the output stream \a os.
  std::ostream& operator<<(std::ostream& os, const Exception& ex);

  // @}

} // namespace LOFAR


//
// \name Useful macros for lazy people
//@{

//
// The macro \c THROW_ARGS contains the arguments for the Exception object
// being constructed in the THROW macro.
//
#ifdef HAVE_BACKTRACE
# define THROW_ARGS __FILE__, __LINE__, AUTO_FUNCTION_NAME,	\
                          new LOFAR::Backtrace
#else
# define THROW_ARGS __FILE__, __LINE__, AUTO_FUNCTION_NAME
#endif

//
// Declare and define an exception class of type \c excp, which is derived
// from the exception class \c super.
//
#define EXCEPTION_CLASS(excp,super)				\
  class excp : public super					\
  {								\
  public:							\
    excp(const std::string& text, const std::string& file="",	\
         int line=0, const std::string& function="",		\
         LOFAR::Backtrace* bt=0) :					\
      super(text, file, line, function, bt) {}			\
      virtual const std::string& type() const {			\
        static const std::string itsType(#excp);		\
        return itsType;						\
      }								\
  }

//
// Throw an exception of type \c excp; use \c strm for the message,
// and \c THROW_ARGS for the other constructor arguments.
// Do not use this to 'throw SystemCallException(...)' (unless an errno was
// already avail (e.g pthread functions return it)). Use THROW_SYSCALL() instead.
//
#if !defined(THROW)
# include <sstream>
# define THROW(excp,strm)			\
  do {						\
    std::ostringstream oss;			\
    try {					\
      oss << strm;				\
    } catch (std::exception&) {}		\
    throw excp(oss.str(), THROW_ARGS);		\
  } while(0)
#endif

//@}

#endif
