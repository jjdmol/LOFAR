//# FinalizeCommand.cc: 
//#
//# Copyright (C) 2007
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

#include <lofar_config.h>
#include <BBSControl/FinalizeCommand.h>
#include <BBSControl/CommandVisitor.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iostream.h>

namespace LOFAR
{
  namespace BBS 
  {

    // Register FinalizeCommand with the CommandFactory. Use an anonymous
    // namespace. This ensures that the variable `dummy' gets its own private
    // storage area and is only visible in this compilation unit.
    namespace
    {
      bool dummy = CommandFactory::instance().
	registerClass<FinalizeCommand>("finalize");
    }


    //##--------   P u b l i c   m e t h o d s   --------##//

    CommandResult FinalizeCommand::accept(CommandVisitor &visitor) const
    {
      return visitor.visit(*this);
    }


    const string& FinalizeCommand::type() const
    {
      static const string theType("Finalize");
      return theType;
    }


    void FinalizeCommand::print(ostream& os) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      Command::print(os);
    }


    //##--------   P r i v a t e   m e t h o d s   --------##//

    void FinalizeCommand::write(ParameterSet&) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    }


    void FinalizeCommand::read(const ParameterSet&)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    }


  } //# namespace BBS

} //# namespace LOFAR
