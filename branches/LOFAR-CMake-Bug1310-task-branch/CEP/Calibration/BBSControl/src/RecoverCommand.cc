//# RecoverCommand.cc: 
//#
//# Copyright (C) 2007
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <lofar_config.h>
#include <BBSControl/RecoverCommand.h>
#include <BBSControl/CommandVisitor.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iostream.h>

namespace LOFAR
{
  namespace BBS 
  {

    // Register RecoverCommand with the CommandFactory. Use an anonymous
    // namespace. This ensures that the variable `dummy' gets its own private
    // storage area and is only visible in this compilation unit.
    namespace
    {
      bool dummy = CommandFactory::instance().
        registerClass<RecoverCommand>("recover");
    }


    //##--------   P u b l i c   m e t h o d s   --------##//

    void RecoverCommand::accept(CommandVisitor &visitor) const
    {
      visitor.visit(*this);
    }


    const string& RecoverCommand::type() const
    {
      static const string theType("Recover");
      return theType;
    }


    void RecoverCommand::print(ostream& os) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      Command::print(os);
    }


    //##--------   P r i v a t e   m e t h o d s   --------##//

    void RecoverCommand::write(ParameterSet&) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    }


    void RecoverCommand::read(const ParameterSet&)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    }


  } //# namespace BBS

} //# namespace LOFAR
