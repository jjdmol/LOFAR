//# NewHandler.h: a new handler that throws a LOFAR exception
//#
//# Copyright (C) 2008
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
//# $Id: NewHandler.h 17606 2011-03-22 12:49:57Z mol $

#ifndef LOFAR_COMMON_NEWHANDLER_H
#define LOFAR_COMMON_NEWHANDLER_H

// \file
//

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
#include <Common/Exception.h>

namespace LOFAR {

// BadAllocException is to be thrown instead of std::bad_alloc. We also
// inherit from std::bad_alloc to be able to throw it from within a new_handler,
// and for compatibility with code depending on new throwing std::bad_alloc.
//
// In fact, this class uses diamond inheritance since both Exception and
// std::bad_alloc are derived from std::exception.
class BadAllocException: public Exception, public std::bad_alloc {
public:
  BadAllocException(const std::string& text, const std::string& file="",
       int line=0, const std::string& function="",
       Backtrace* bt=0) :
    Exception(text, file, line, function, bt), std::bad_alloc() {}

    virtual const std::string& type() const {
      static const std::string itsType("BadAllocException");
      return itsType;
    }

  static void newHandler();
};


class NewHandler {
public:
   NewHandler( void (*handler)() );
   ~NewHandler();
private:
   void (*itsOldHandler)();
};

} // namespace LOFAR

#endif
