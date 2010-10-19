//#  SingleStep.cc:
//#
//# Copyright (C) 2002-2007
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
//#  $Id$

#include <lofar_config.h>

#include <BBSControl/SingleStep.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/CommandVisitor.h>
#include <BBSControl/StreamUtil.h>
#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iomanip.h>

namespace LOFAR
{
  namespace BBS
  {
    using LOFAR::operator<<;

    //##--------   P u b l i c   m e t h o d s   --------##//

    SingleStep::~SingleStep()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    }


    void SingleStep::print(ostream& os) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      Step::print(os);
      Indent id;
      os << endl << indent << "Output column: " << itsOutputColumn
        << boolalpha
        << endl << indent << "Write flags: " << itsWriteFlags
	    << noboolalpha;
    }


    //##--------   P r o t e c t e d   m e t h o d s   --------##//

    SingleStep::SingleStep(const Step* parent) :
      Step(parent)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    }


    SingleStep::SingleStep(const string& name, const Step* parent) :
      Step(name, parent)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    }


    void SingleStep::write(ParameterSet& ps) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      Step::write(ps);
      const string prefix("Step." + name() + ".");
      ps.replace(prefix + "Output.Column", itsOutputColumn);
      ps.replace(prefix + "Output.WriteFlags", toString(itsWriteFlags));
      ps.replace(prefix + "Operation",  toUpper(operation()));
      LOG_TRACE_VAR_STR("\nContents of ParameterSet ps:\n" << ps);
    }


    void SingleStep::read(const ParameterSet& ps)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      Step::read(ps);
      itsOutputColumn = ps.getString("Output.Column", "");
      itsWriteFlags = ps.getBool("Output.WriteFlags", false);
    }

  } // namespace BBS

} // namespace LOFAR
