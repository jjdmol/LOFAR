//#  Copyright (C) 2014
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

#include "BudgetTimer.h"
#include <Common/LofarLogger.h>

#include <iomanip>

namespace LOFAR {

  namespace Cobalt {

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
        LOG_DEBUG_STR("Run-time budget exceeded: " << itsName << " ran at " << realTimePerc << "% (took " << elapsed << " s, budget is " << budget << " s)");
      }
    }
  } // namespace Cobalt
} // namespace LOFAR
