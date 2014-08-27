//# BudgetTimer.h: A timer that warns if a run takes longer than is acceptable
//#
//#  Copyright (C) 2007
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

#ifndef LOFAR_COINTERFACE_BUDGET_TIMER_H
#define LOFAR_COINTERFACE_BUDGET_TIMER_H

#include <Common/Timer.h>
#include <Common/LofarLogger.h>

#include <iomanip>

namespace LOFAR {

  namespace Cobalt {

    class BudgetTimer: public NSTimer
    {
    public:
      BudgetTimer(
        const std::string &name,
        double budget = 0.0,
        bool print_on_destruction = false,
        bool log_on_destruction = false);

      ~BudgetTimer();

      void stop();

    private:
        const double budget;

        double prev_elapsed;
    };

    BudgetTimer::BudgetTimer(
        const std::string &name,
        double budget,
        bool print_on_destruction,
        bool log_on_destruction):
      NSTimer(name, print_on_destruction, log_on_destruction),
      budget(budget),
      prev_elapsed(0.0)
    {
    }

    BudgetTimer::~BudgetTimer()
    {
      if (print_on_destruction) {
        const double realTimePerc = 100.0 * getAverage() / budget;

        if (log_on_destruction) {
          LOG_INFO_STR(std::left << std::setw(25) << itsName << ": ran at " << realTimePerc << "% of run-time budget");
        } else {
          // TODO
        }
      }
    }

    void BudgetTimer::stop()
    {
      NSTimer::stop();

      // Calculate duration of last run
      const double elapsed = getElapsed() - prev_elapsed;
      prev_elapsed = getElapsed();

      if (budget > 0.0 && elapsed > budget) {
        const double realTimePerc = 100.0 * elapsed / budget;
        LOG_WARN_STR("Run-time budget exceeded: " << itsName << " ran at " << realTimePerc << "% (took " << elapsed << " s, budget is " << budget << " s)");
      }
    }


  } // namespace Cobalt

} // namespace LOFAR

#endif 
