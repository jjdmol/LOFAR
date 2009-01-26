//#  Strategy.cc: Strategy (sequence of Steps) to calibrate the visibility data.
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

#include <BBSControl/Strategy.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/MultiStep.h>
#include <BBSControl/Step.h>
#include <BBSControl/StreamUtil.h>
#include <BBSControl/Types.h>
#include <Common/ParameterSet.h>
#include <Common/Exceptions.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{

  namespace BBS
  {
    using LOFAR::operator<<;

    //##--------   P u b l i c   m e t h o d s   --------##//
      
    Strategy::Strategy(const ParameterSet& ps)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      try {
        itsRoot = Step::create("Root", ps, 0);
      } catch(BBSControlException &e) {
        THROW(BBSControlException, "Unable to find root step of strategy;"
          " please ensure a (multi-)step called 'Root' (case sensitive) is"
          " defined in the parset");
      }
    }

    Strategy::~Strategy()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    }

    void Strategy::print(ostream& os) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      ASSERT(itsRoot);
      itsRoot->print(os);
    }
 
    void Strategy::write(ParameterSet& ps) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      itsRoot->write(ps);
    }

    void Strategy::read(const ParameterSet& ps)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      itsRoot->read(ps);
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
      itsStack.push(strategy.getRoot());
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
