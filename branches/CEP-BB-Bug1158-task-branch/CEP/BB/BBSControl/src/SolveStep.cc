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
#include <Common/lofar_iomanip.h>

namespace LOFAR
{
  namespace BBS
  {
    using ACC::APS::ParameterSet;
    using LOFAR::operator<<;


    //##--------   P u b l i c   m e t h o d s   --------##//

    SolveStep::SolveStep(const Step* parent) : 
      SingleStep(parent)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    }


    SolveStep::SolveStep(const string& name, 
			       const ParameterSet& parset,
			       const Step* parent) :
      SingleStep(name, parent)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Get the relevant parameters from the Parameter Set \a parset. 
      read(parset.makeSubset("Step." + name + "."));
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
	os << endl << indent << "Solvable parameters: " << itsParms
	   << endl << indent << "Excluded parameters: " << itsExclParms
       << endl << indent << "Kernel groups: "       << itsKernelGroups
	   << endl << indent << itsCellSize
	   << endl << indent << "Cell chunk size: " << itsCellChunkSize
	   << boolalpha
	   << endl << indent << "Propagate solutions: " << itsPropagateFlag
	   << noboolalpha
       << endl << indent << itsSolverOptions;
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
      const string prefix = "Step." + name() + ".Solve.";
      ps.replace(prefix + "Parms", 
                 toString(itsParms));
      ps.replace(prefix + "ExclParms", 
                 toString(itsExclParms));
      ps.replace(prefix + "KernelGroups",
                 toString(itsKernelGroups));
      ps.replace(prefix + "CellSize.Freq",
                 toString(itsCellSize.freq));
      ps.replace(prefix + "CellSize.Time", 
                 toString(itsCellSize.time));
      ps.replace(prefix + "CellChunkSize", 
                 toString(itsCellChunkSize));
      ps.replace(prefix + "PropagateSolutions", 
                 toString(itsPropagateFlag));
      ps.replace(prefix + "Options.MaxIter", 
                 toString(itsSolverOptions.maxIter));
      ps.replace(prefix + "Options.EpsValue", 
                 toString(itsSolverOptions.epsValue));
      ps.replace(prefix + "Options.EpsDerivative", 
                 toString(itsSolverOptions.epsDerivative));
      ps.replace(prefix + "Options.CollFactor", 
                 toString(itsSolverOptions.collFactor));
      ps.replace(prefix + "Options.LMFactor", 
                 toString(itsSolverOptions.lmFactor));
      ps.replace(prefix + "Options.BalancedEqs", 
                 toString(itsSolverOptions.balancedEqs));
      ps.replace(prefix + "Options.UseSVD", 
                 toString(itsSolverOptions.useSVD));
      LOG_TRACE_VAR_STR("\nContents of ParameterSet ps:\n" << ps);
    }


    void SolveStep::read(const ParameterSet& ps)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      SingleStep::read(ps);
      ParameterSet pss(ps.makeSubset("Solve."));
      itsParms                       =
        pss.getStringVector("Parms");
      itsExclParms                   = 
        pss.getStringVector("ExclParms",    vector<string>());
      itsKernelGroups                = 
        pss.getUint32Vector("KernelGroups");//, vector<uint32>());
      itsCellSize.freq               =
        pss.getUint32      ("CellSize.Freq");
      itsCellSize.time               = 
        pss.getUint32      ("CellSize.Time");
      itsCellChunkSize               = 
        pss.getUint32      ("CellChunkSize", 0);
      itsPropagateFlag               =
        pss.getBool("PropagateSolutions", false);
      itsSolverOptions.maxIter       = 
        pss.getUint32      ("Options.MaxIter");
      itsSolverOptions.epsValue      = 
        pss.getDouble      ("Options.EpsValue");
      itsSolverOptions.epsDerivative = 
        pss.getDouble      ("Options.EpsDerivative");
      itsSolverOptions.collFactor     = 
        pss.getDouble      ("Options.CollFactor");
      itsSolverOptions.lmFactor      = 
        pss.getDouble      ("Options.LMFactor");
      itsSolverOptions.balancedEqs   = 
        pss.getBool        ("Options.BalancedEqs");
      itsSolverOptions.useSVD        = 
        pss.getBool        ("Options.UseSVD");
    }


  } // namespace BBS

} // namespace LOFAR
