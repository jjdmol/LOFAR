//#  Command.cc: Interface for classes that can be saved to and 
//#                      restored from a ParameterSet.
//#
//#  Copyright (C) 2007
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
#include <BBSControl/Command.h>
#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
  namespace BBS
  {

    //##--------   P u b l i c   m e t h o d s   --------##//

    void Command::print(ostream& os) const
    {
      os << "Command: " << type();
    }


    //##--------   G l o b a l   m e t h o d s   --------##//

    ostream& operator<<(ostream& os, const Command& cmd)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      cmd.print(os);
      return os;
    }


    ParameterSet& operator<<(ParameterSet& ps, const Command& cmd)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      cmd.write(ps); 
      return ps;
    }


    ParameterSet& operator>>(ParameterSet& ps, Command& cmd)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      cmd.read(ps); 
      return ps;
    }


  } // namespace BBS

} // namespace LOFAR
