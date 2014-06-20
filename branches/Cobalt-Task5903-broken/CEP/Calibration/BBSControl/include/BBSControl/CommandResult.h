//# CommandResult.h: Result of execution of a command by the local controller
//#
//# Copyright (C) 2002-2004
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

#ifndef LOFAR_BBSCONTROL_COMMANDRESULT_H
#define LOFAR_BBSCONTROL_COMMANDRESULT_H

// \file
// Result of execution of a command by the local controller

//# Includes
#include <Common/lofar_string.h>
#include <Common/lofar_iosfwd.h>

namespace LOFAR
{
  namespace BBS
  {

    // \addtogroup BBSControl
    // @{

    // Class representing the result of the execution of a command by the
    // local controller. It encapsulates the return value (an enumerated
    // value), with its associated return value as a string, and an optional
    // additional error message.
    class CommandResult
    {
    public:
      // Result values that can be set by the local controller.
      enum Result {
        UNKNOWN = -1,  ///< An unknown error occurred.
        OK,            ///< No errors.
        OUT_OF_DATA,   ///< Local controller was "Out of data"
        ERROR,         ///< Something definitely went wrong.
        //# Insert new return values HERE !!
        N_Result       ///< Number of result values.
      };

      // Default constructor.
      CommandResult(Result result = UNKNOWN, const string& msg = string());

      // This constructor was added to make the life of the programmer a
      // little brighter. The job of converting the \c int \a result to an \c
      // enum Result is now done by the constructor.
      CommandResult(int result, const string& msg = string());

      // Return the result code as an integer.
      int asInt() const
      { return itsResult; }

      // Return the result code as a string.
      const string& asString() const;

      // Return the user-supplied message.
      const string& message() const
      { return itsMessage; }

      // Return \c true if result is OK, else return \c false.
      operator bool() const
      { return itsResult == OK; }

      // Return \c true if result is equal to \a result, else return \c false.
      bool is(Result result) const
      { return itsResult == result; }

    private:
      // If \a result matches with one of the enumeration values in Result,
      // then itsResult is set to \a result, else itsResult is set to \c
      // UNKNOWN. 
      void set(Result result);

      // The return result
      Result itsResult;

      // User-supplied optional message.
      string itsMessage;

    };

    // Output in ASCII format.
    ostream& operator<<(ostream& os, const CommandResult& result);

    // @}

  } // namespace BBS

} // namespace LOFAR

#endif
