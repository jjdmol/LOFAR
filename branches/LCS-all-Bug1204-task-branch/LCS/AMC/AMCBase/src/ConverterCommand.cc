//#  ConverterCommand.cc:  Commands to be sent to the AMC Converter server.
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
#include <AMCBase/ConverterCommand.h>

namespace LOFAR
{
  namespace AMC
  {

    ConverterCommand::ConverterCommand(Commands cmd)
    {
      if (INVALID < cmd && cmd < N_Commands) itsCommand = cmd;
      else itsCommand = INVALID;
    }


    const string& ConverterCommand::showCommand() const
    {
      //# Caution: Always keep this array of strings in sync with the enum
      //#          Commands that is defined in the header file!
      static const string commands[N_Commands+1] = {
        "J2000 --> AZEL",
        "J2000 --> ITRF",
        "AZEL --> J2000",
        "ITRF --> J2000",
        "<INVALID>"
      };
      if (isValid()) return commands[itsCommand];
      else return commands[N_Commands];
    }


    bool operator==(const ConverterCommand& lhs, const ConverterCommand& rhs)
    {
      return lhs.get() == rhs.get();
    }


    ostream& operator<<(ostream& os, const ConverterCommand& cc)
    {
      return os << cc.showCommand();
    }

  } // namespace AMC
  
} // namespace LOFAR
