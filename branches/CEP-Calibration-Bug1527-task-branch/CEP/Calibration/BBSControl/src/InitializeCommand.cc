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
      itsSelection = strategy.selection();
//      itsStations = strategy.getStations();
//      itsCorrelationFilter = strategy.getCorrelationFilter();
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
        << endl << indent << itsSelection
        << endl << indent << "UseSolver: " << boolalpha << itsUseSolver
        << noboolalpha;
//      os << endl << indent << "Stations: " << itsStations
//        << endl << indent << "Input column: " << itsInputColumn
//        << endl << indent << itsCorrelationFilter
//        << boolalpha
//        << endl << indent << "UseSolver: " << boolalpha << itsUseSolver
//        << noboolalpha;
    }


    //##--------   P r i v a t e   m e t h o d s   --------##//

    void InitializeCommand::write(ParameterSet& ps) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

//      ps.add("Stations", toString(itsStations));
      ps.add("InputColumn", itsInputColumn);
      ps.add("Selection.BaselineType", itsSelection.type);
      ps.add("Selection.Baselines", toString(itsSelection.baselines));
      ps.add("Selection.Correlations", toString(itsSelection.correlations));
//      ps.add("Correlation.Selection", itsCorrelationFilter.selection);
//      ps.add("Correlation.Type", toString(itsCorrelationFilter.type));
      ps.add("UseSolver", toString(itsUseSolver));
    }


    void InitializeCommand::read(const ParameterSet& ps)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
//      itsStations = ps.getStringVector("Stations", vector<string>());
      itsInputColumn = ps.getString("InputColumn", "DATA");
      fromParameterSet(ps, itsSelection);
//      itsCorrelationFilter.selection = ps.getString("Correlation.Selection",
//        "CROSS");
//      itsCorrelationFilter.type = ps.getStringVector("Correlation.Type",
//        vector<string>());
      itsUseSolver = ps.getBool("UseSolver", false);
    }

  } //# namespace BBS

} //# namespace LOFAR
