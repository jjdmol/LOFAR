//#  CorrectStep.cc:
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
#include <BBSControl/CorrectStep.h>
#include <BBSControl/CommandVisitor.h>
#include <BBSControl/StreamUtil.h>
#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iomanip.h>

namespace LOFAR
{
  namespace BBS
  {

    CorrectStep::CorrectStep(const string& name,
                             const ParameterSet& parSet,
                             const Step* parent) :
      SingleStep(name, parent)
    {
      read(parSet.makeSubset("Step." + name + "."));
    }


    CommandResult CorrectStep::accept(CommandVisitor &visitor) const
    {
        return visitor.visit(*this);
    }


    const string& CorrectStep::operation() const
    {
      static string theOperation("Correct");
      return theOperation;
    }


    void CorrectStep::print(ostream& os) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      SingleStep::print(os);
      Indent id;
      os << endl << indent << "Correct: ";
      {
        Indent id;
        os << endl << indent << "Condition number flagging:";
        {
          Indent id;
          os << endl << indent << "Enabled: " << boolalpha << itsUseCondFlagging
             << noboolalpha;
          if(itsUseCondFlagging)
          {
             os << endl << indent << "Threshold: " << itsThreshold;
          }
        }
      }
    }

    const string& CorrectStep::type() const
    {
      static const string theType("Correct");
      return theType;
    }

    void CorrectStep::write(ParameterSet& ps) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      SingleStep::write(ps);
      const string prefix = "Step." + name() + ".Correct.";
      ps.replace(prefix + "ConditionNumberFlagging.Enabled",
                 toString(itsUseCondFlagging));
      if(itsUseCondFlagging) {
        ps.replace(prefix + "ConditionNumberFlagging.Threshold",
                 toString(itsThreshold));
      }
      LOG_TRACE_VAR_STR("\nContents of ParameterSet ps:\n" << ps);
    }

    void CorrectStep::read(const ParameterSet& ps)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      SingleStep::read(ps);
      ParameterSet pss(ps.makeSubset("Correct."));
      itsUseCondFlagging = pss.getBool("ConditionNumberFlagging.Enabled");
      if(itsUseCondFlagging) {
        itsThreshold = pss.getDouble("ConditionNumberFlagging.Threshold");
      }
    }

  } // namespace BBS

} // namespace LOFAR
