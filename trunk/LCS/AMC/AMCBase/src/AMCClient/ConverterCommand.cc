//#  ConverterCommand.cc: one line description
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
#include <AMCBase/AMCClient/ConverterCommand.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
  namespace AMC
  {
    //# Definition of static data members.
    ConverterCommand::cmdset_t  ConverterCommand::theirCmdSet;
    ConverterCommand::Init      ConverterCommand::theirInit;


    //# Implementation of non-inline member functions.
    ConverterCommand::ConverterCommand(int32 iCmd) : 
      itsCmd(ConverterCommand::INVALID)
    {
      cmdset_t::const_iterator it = theirCmdSet.find(static_cast<Cmd>(iCmd));
      if (it != theirCmdSet.end()) {
        itsCmd = *it;
      }
    }

    //# Implementation of non-inline non-member functions.
//     bool operator==(const ConverterCommand& lhs, const ConverterCommand& rhs)
//     {
//       return lhs.get() == rhs.get();
//     }

    ostream& operator<<(ostream& os, const ConverterCommand& cc)
    {
      return os << cc.get();
    }

 } // namespace AMC
  
} // namespace LOFAR
