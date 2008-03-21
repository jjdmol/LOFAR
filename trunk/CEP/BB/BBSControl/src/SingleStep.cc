//#  SingleStep.cc: 
//#
//#  Copyright (C) 2002-2007
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

#include <lofar_config.h>

#include <BBSControl/SingleStep.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/CommandVisitor.h>
#include <BBSControl/StreamUtil.h>
#include <APS/ParameterSet.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
  namespace BBS
  {
    using ACC::APS::ParameterSet;
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
      os << endl << indent << "Output data: " << itsOutputData;
    }


    //##--------   P r o t e c t e d   m e t h o d s   --------##//

    SingleStep::SingleStep(const Step* parent) :
      Step(parent)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    }


    SingleStep::SingleStep(const string& name, 
				 const ParameterSet& parset,
				 const Step* parent) :
      Step(name, parset, parent)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Create a subset of \a parset, containing only the relevant keys for
      // the current SingleStep.
      ParameterSet ps(parset.makeSubset("Step." + name + "."));

      // Get the name of the data column to write to
      itsOutputData = ps.getString("OutputData");
    }


    void SingleStep::write(ParameterSet& ps) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      Step::write(ps);
      ps.replace("Step." + name() + ".OutputData", itsOutputData);
      ps.replace("Step." + name() + ".Operation", toUpper(operation()));
    }


    void SingleStep::read(const ParameterSet& ps)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      Step::read(ps);
      itsOutputData = ps.getString("Step." + name() + ".OutputData");
    }

  } // namespace BBS

} // namespace LOFAR
