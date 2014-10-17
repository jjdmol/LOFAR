//# InitializeCommand.cc:
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

#include <BBSControl/InitializeCommand.h>
#include <BBSControl/CommandVisitor.h>
#include <BBSControl/Strategy.h>
#include <BBSControl/StreamUtil.h>

#include <Common/LofarLogger.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_iomanip.h>
#include <Common/ParameterSet.h>

namespace LOFAR
{
  namespace BBS
  {
    using LOFAR::operator<<;

    // Register InitializeCommand with the CommandFactory. Use an anonymous
    // namespace. This ensures that the variable `dummy' gets its own private
    // storage area and is only visible in this compilation unit.
    namespace
    {
      bool dummy = CommandFactory::instance().
        registerClass<InitializeCommand>("initialize");
    }


    //##--------   P u b l i c   m e t h o d s   --------##//

    InitializeCommand::InitializeCommand()
      : itsUseSolver(false)
    {
    }

    InitializeCommand::InitializeCommand(const Strategy& strategy)
    {
      itsInputColumn = strategy.inputColumn();
      itsBaselines = strategy.baselines();
      itsCorrelations = strategy.correlations();
      itsUseSolver = strategy.useSolver();
    }

    CommandResult InitializeCommand::accept(CommandVisitor &visitor) const
    {
      return visitor.visit(*this);
    }


    const string& InitializeCommand::type() const
    {
      static const string theType("Initialize");
      return theType;
    }


    void InitializeCommand::print(ostream& os) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      Command::print(os);
      os << endl << indent << "Input column: " << itsInputColumn
        << endl << indent << "Baselines: " << itsBaselines
        << endl << indent << "Correlations: " << itsCorrelations
        << endl << indent << "Use global solver: " << boolalpha << itsUseSolver
        << noboolalpha;
    }


    //##--------   P r i v a t e   m e t h o d s   --------##//

    void InitializeCommand::write(ParameterSet& ps) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      ps.add("InputColumn", itsInputColumn);
      ps.add("Baselines", itsBaselines);
      ps.add("Correlations", toString(itsCorrelations));
      ps.add("UseSolver", toString(itsUseSolver));
    }


    void InitializeCommand::read(const ParameterSet& ps)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Read input column.
      itsInputColumn = ps.getString("InputColumn");

      // Read data selection.
      itsBaselines = ps.getString("Baselines");
      itsCorrelations = ps.getStringVector("Correlations");

      // Read flag that controls use of the global solver.
      itsUseSolver = ps.getBool("UseSolver");
    }

  } //# namespace BBS

} //# namespace LOFAR
