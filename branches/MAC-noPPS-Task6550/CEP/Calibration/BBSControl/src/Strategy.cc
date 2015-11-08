//# Strategy.cc: Strategy (sequence of Steps) to calibrate the visibility data.
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
//# $Id$

#include <lofar_config.h>

#include <BBSControl/Strategy.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/MultiStep.h>
#include <BBSControl/SolveStep.h>
#include <BBSControl/Step.h>
#include <BBSControl/StreamUtil.h>
#include <BBSControl/Types.h>

#include <Common/ParameterSet.h>
#include <Common/Exceptions.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iomanip.h>

namespace LOFAR
{

  namespace BBS
  {
    using LOFAR::operator<<;

    //##--------   P u b l i c   m e t h o d s   --------##//

    Strategy::Strategy(const ParameterSet& parset)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Create a subset of \a aParSet, containing only the relevant keys for
      // the Strategy.
      //ParameterSet ps(parset.makeSubset("Strategy."));

      // Get the name of the input column
      itsInputColumn = parset.getString("Strategy.InputColumn", "DATA");

      // Read data selection.
      itsBaselines = parset.getString("Strategy.Baselines", "*&");
      itsCorrelations = parset.getStringVector("Strategy.Correlations", vector<string>());

      // Get the time range.
      itsTimeRange = parset.getStringVector("Strategy.TimeRange", vector<string>());

      // Get the chunk size.
      itsChunkSize = parset.getUint32("Strategy.ChunkSize");

      // This strategy consists of the following steps.
      vector<string> steps(parset.getStringVector("Strategy.Steps"));

      if(steps.empty()) {
        THROW(BBSControlException, "Strategy contains no steps");
      }

      // Try to create a step for each name in \a steps.
      for(size_t i = 0; i < steps.size(); ++i) {
        itsSteps.push_back(Step::create(steps[i], parset, 0));
      }

      itsUseSolver = findGlobalSolveStep();

      int checkparset = 0;
      try {
        checkparset = parset.getInt ("checkparset", 0);
      } catch (...) {
        LOG_WARN_STR ("Parameter checkparset should be an integer value");
        checkparset = parset.getBool ("checkparset") ? 1:0;
      }

      if (checkparset>=0) {
        vector<string> unused = parset.unusedKeys();
        if (! unused.empty()) {
           LOG_WARN_STR (endl << 
             "*** WARNING: the following parset keywords were not used ***"
             << endl
             << "             maybe they are misspelled"
             << endl
             << "    " << unused << endl);
           ASSERTSTR (checkparset==0, "Unused parset keywords found");   
        }
      }
    }

    Strategy::~Strategy()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    }

    void Strategy::print(ostream& os) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      os << indent << "Strategy:";
      Indent indent0;
      os << endl << indent << "Input column: " << itsInputColumn
        << endl << indent << "Baselines: " << itsBaselines
        << endl << indent << "Correlations: " << itsCorrelations
        << endl << indent << "Time range: " << itsTimeRange
        << endl << indent << "Chunk size: " << itsChunkSize
        << boolalpha
        << endl << indent << "Use global solver: " << boolalpha << itsUseSolver
        << noboolalpha
        << endl << indent << "Steps:";
      Indent indent1;
      for(size_t i = 0; i < itsSteps.size(); ++i) {
    	  os << endl << indent << *itsSteps[i];
      }
    }

    bool Strategy::findGlobalSolveStep() const
    {
      StrategyIterator it(*this);
      while(!it.atEnd()) {
        shared_ptr<const SolveStep> step =
          dynamic_pointer_cast<const SolveStep>(*it);

        if(step && step->globalSolution()) {
          return true;
        }

        ++it;
      }

      return false;
    }

    //##--------   G l o b a l   m e t h o d s   --------##//

    ostream& operator<<(ostream& os, const Strategy& in)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      in.print(os);
      return os;
    }


    //##--------   P u b l i c   m e t h o d s   --------##//

    StrategyIterator::StrategyIterator(const Strategy &strategy) {
      // Push the top-level Steps in reverse order such that the first Step
      // is popped first.
      vector<shared_ptr<const Step> >::const_reverse_iterator revIt =
        strategy.itsSteps.rbegin();
      vector<shared_ptr<const Step> >::const_reverse_iterator revItEnd =
        strategy.itsSteps.rend();
      while(revIt != revItEnd)
      {
        itsStack.push(*revIt);
        ++revIt;
      }

      // Traverse to the first leaf Step.
      this->operator++();
    }

    bool StrategyIterator::atEnd() const {
      return !itsCurrent;
    }

    shared_ptr<const Step> StrategyIterator::operator*() const {
      return itsCurrent;
    }

    void StrategyIterator::operator++() {
      if(itsStack.empty()) {
        itsCurrent.reset();
        return;
      }

      shared_ptr<const Step> step(itsStack.top());
      itsStack.pop();

      // If the current Step is not a leaf Step, push all its children in
      // reverse order (such that the first child will be popped from the stack
      // first).
      shared_ptr<const MultiStep> multi =
        dynamic_pointer_cast<const MultiStep>(step);

      if(multi) {
        MultiStep::const_reverse_iterator revIt = multi->rbegin();
        MultiStep::const_reverse_iterator revItEnd = multi->rend();
        while(revIt != revItEnd) {
          itsStack.push(*revIt);
          ++revIt;
        }

        // Recursive call to traverse to a leaf Step.
        this->operator++();
      } else {
        // Iterator arrived at a leaf Step; update the current Step and exit.
        itsCurrent = step;
      }
    }

  } // namespace BBS

} // namespace LOFAR
