//#  CorrectStep.cc:
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
#include <BBSControl/CorrectStep.h>
#include <BBSControl/CommandVisitor.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/StreamUtil.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iomanip.h>
#include <Common/ParameterSet.h>

namespace LOFAR
{
  namespace BBS
  {
    using LOFAR::operator<<;

    //##--------   P u b l i c   m e t h o d s   --------##//

    CorrectStep::CorrectStep(const Step* parent) :
      SingleStep(parent)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    }


    CorrectStep::CorrectStep(const string& name,
                             const ParameterSet& parSet,
                             const Step* parent) :
      SingleStep(name, parent)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      read(parSet.makeSubset("Step." + name + "."));
    }


    CorrectStep::~CorrectStep()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    }


    const string& CorrectStep::type() const
    {
      static const string theType("Correct");
      return theType;
    }


    void CorrectStep::print(ostream& os) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      SingleStep::print(os);
      Indent id;
      os << endl << indent << "Use MMSE: " << boolalpha << itsUseMMSE
        << noboolalpha;
      if(itsUseMMSE)
      {
        Indent id;
        os << endl << indent << "Sigma: " << itsSigmaMMSE;
      }
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


    bool CorrectStep::useMMSE() const
    {
      return itsUseMMSE;
    }


    double CorrectStep::sigmaMMSE() const
    {
      return itsSigmaMMSE;
    }


    //##--------   P r i v a t e   m e t h o d s   --------##//

    void CorrectStep::write(ParameterSet& ps) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      SingleStep::write(ps);
      const string prefix("Step." + name() + ".Correct.");
      ps.replace(prefix + "MMSE.Enable", toString(itsUseMMSE));
      ps.replace(prefix + "MMSE.Sigma", toString(itsSigmaMMSE));
      LOG_TRACE_VAR_STR("\nContents of ParameterSet ps:\n" << ps);
    }


    void CorrectStep::read(const ParameterSet& ps)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      SingleStep::read(ps);
      ParameterSet pss(ps.makeSubset("Correct."));
      itsUseMMSE = pss.getBool("MMSE.Enable", false);
      itsSigmaMMSE = pss.getDouble("MMSE.Sigma", 0.0);
      if(itsSigmaMMSE < 0.0)
      {
        THROW(BBSControlException, "MMSE.Sigma should be positive: "
          << itsSigmaMMSE);
      }
    }

  } // namespace BBS

} // namespace LOFAR
