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

// \file Exception.h
// LCS Exception base class.

//# Includes
//# Never #include <config.h> or #include <lofar_config.h> in a header file!
#include <Common/lofar_string.h>
#include <exception>


namespace LOFAR {

  // \addtogroup Common
  // @{

  //
  // This is the base class for all LCS related exceptions.
  //
  class Exception : public std::exception
  {
  public:
    Exception(const string& text, const string& file="",
	      int line=0, const string& func="");

    virtual ~Exception() throw();

    // Implementation of std::exception::what().
    // Returns the user-supplied text as C-style char array.
    virtual const char* what() const throw();

    // Return the class type of the exception.
    virtual const string& type() const {
      static const string itsType("Exception");
      return itsType;
    }

    // Return the user-supplied text
    const string& text() const {
      return itsText;
    }

    // Return the name of the file where the exception occurred.
    const string& file() const {
      return itsFile;
    }

    // Return the line number where the exception occurred.
    int line() const {
      return itsLine;
    }

    // Return the function name where the exception occurred.
    const string& function() const {
      return itsFunction;
    }

    // Return exception type, user-supplied text, function name,
    // filename, and line number as a formatted string.
    const string message() const;
    
  private:
    string itsText;
    string itsFile;
    int         itsLine;
    string itsFunction;

  };

  // Put the exception message into an ostream.
  inline std::ostream& operator<<(std::ostream& os, const Exception& ex)
  {
    return os << ex.message();
  }

  // @}

} // namespace LOFAR


//
// Declare and define an exception class of type \c excp, which is derived
// from the exception class \c super.
//
#define EXCEPTION_CLASS(excp,super) \
class excp : public super \
{ \
public: \
  excp( const LOFAR::string& text, const LOFAR::string& file="", \
	int line=0, const LOFAR::string& function="" ) : \
    super(text, file, line, function) {} \
  virtual const LOFAR::string& type( void ) const \
  { \
    static const LOFAR::string itsType(#excp); \
    return itsType; \
  } \
};

#endif
