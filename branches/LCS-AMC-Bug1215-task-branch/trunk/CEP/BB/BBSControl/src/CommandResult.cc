//#  CommandResult.cc: Result of execution of a command by the local controller
//#
//#  Copyright (C) 2002-2004
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <BBSControl/CommandResult.h>
#include <Common/lofar_iostream.h>

namespace LOFAR
{
  namespace BBS
  {

    CommandResult::CommandResult(Result result, const string& msg) :
      itsMessage(msg)
    {
      set(result);
    }


    CommandResult::CommandResult(int result, const string& msg) :
      itsMessage(msg)
    {
      set(static_cast<Result>(result));
    }


    const string& CommandResult::asString() const
    {
      //# Caution: Always keep this array of strings in sync with the enum
      //#          Result that is defined in the header file!
      static const string resultString[N_Result+1] = {
        "Success",
        "Out-of-data",
        "Error",
        "Unknown error"    //# This line should ALWAYS be last!
      };
      if (itsResult < 0) return resultString[N_Result];
      else return resultString[itsResult];
    }


    void CommandResult::set(Result result)
    {
      if (UNKNOWN < result && result < N_Result) itsResult = result;
      else itsResult = UNKNOWN;
    }
    

    ostream& operator<<(ostream& os, const CommandResult& result)
    {
      os << result.asString();
      if (!result.message().empty()) os << ": " << result.message();
      return os;
    }

  } // namespace BBS
  
} // namespace LOFAR
