//#  SolveStep.cc: 
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

#include <BBSControl/SolveStep.h>
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

    SolveStep::SolveStep(const Step* parent) : 
      SingleStep(parent),
      itsMaxIter(0), itsEpsilon(0), itsMinConverged(0)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    }


    SolveStep::SolveStep(const string& name, 
			       const ParameterSet& parset,
			       const Step* parent) :
      SingleStep(name, parset, parent)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Create a subset of \a parset, containing only the relevant keys for
      // the current SingleStep.
      ParameterSet ps(parset.makeSubset("Step." + name + ".Solve."));

      // Get the relevant parameters from the Parameter Set \a parset. 
      // \note A missing key is considered an error.
      itsMaxIter                 = ps.getUint32("MaxIter");
      itsEpsilon                 = ps.getDouble("Epsilon");
      itsMinConverged            = ps.getDouble("MinConverged");
      itsParms                   = ps.getStringVector("Parms");
      itsExclParms               = ps.getStringVector("ExclParms");
      itsDomainSize.bandWidth    = ps.getDouble("DomainSize.Freq");
      itsDomainSize.timeInterval = ps.getDouble("DomainSize.Time");
    }


    SolveStep::~SolveStep()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    }


    void SolveStep::accept(CommandVisitor &visitor) const
    {
      visitor.visit(*this);
    }


    void SolveStep::print(ostream& os) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      SingleStep::print(os);
      Indent id;
      os << endl << indent << "Solve: ";
      {
	Indent id;
	os << endl << indent << "Max nr. of iterations: "  << itsMaxIter
	   << endl << indent << "Convergence threshold: "  << itsEpsilon
	   << endl << indent << "Min fraction converged: " << itsMinConverged
	   << endl << indent << "Solvable parameters: "    << itsParms
	   << endl << indent << "Excluded parameters: "    << itsExclParms
	   << endl << indent << itsDomainSize;
      }
    }


    const string& SolveStep::type() const
    {
      static const string theType("Solve");
      return theType;
    }


    const string& SolveStep::operation() const
    {
      static string theOperation("Solve");
      return theOperation;
    }

    //##--------   P r i v a t e   m e t h o d s   --------##//

    void SolveStep::write(ParameterSet& ps) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      SingleStep::write(ps);
      ostringstream oss;
      oss << endl << "MaxIter = " << itsMaxIter
          << endl << "Epsilon = " << itsEpsilon
          << endl << "MinConverged = " << itsMinConverged
          << endl << "Parms = " << itsParms
          << endl << "ExclParms = " << itsExclParms
          << endl << "DomainSize.Freq = " << itsDomainSize.bandWidth
          << endl << "DomainSize.Time = " << itsDomainSize.timeInterval;
      ps.adoptBuffer(oss.str(), "Step." + name() + ".Solve.");
    }


    void SolveStep::read(const ParameterSet& ps)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      SingleStep::read(ps);
      ParameterSet pss(ps.makeSubset("Step." + name() + ".Solve."));
//       ParameterSet pss(ps);
      itsMaxIter                 = pss.getInt32("MaxIter");
      itsEpsilon                 = pss.getDouble("Epsilon");
      itsMinConverged            = pss.getDouble("MinConverged");
      itsParms                   = pss.getStringVector("Parms");
      itsExclParms               = pss.getStringVector("ExclParms");
      itsDomainSize.bandWidth    = pss.getDouble("DomainSize.Freq");
      itsDomainSize.timeInterval = pss.getDouble("DomainSize.Time");
    }


  } // namespace BBS

} // namespace LOFAR
