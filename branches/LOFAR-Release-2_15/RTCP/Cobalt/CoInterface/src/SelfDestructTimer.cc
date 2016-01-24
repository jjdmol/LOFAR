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
//#  $Id: BudgetTimer.cc 30919 2015-02-05 15:26:22Z amesfoort $
#include <lofar_config.h>

#include "SelfDestructTimer.h"
#include <Common/LofarLogger.h>

#include <unistd.h>
#include <time.h>


namespace LOFAR {
  namespace Cobalt {

    size_t getMaxRunTime(const Parset &parset, time_t lingerTime)
    {
      if (!parset.settings.realTime)
        return 0;

      const time_t now = time(0);
      const double stopTime = parset.settings.stopTime;

      if (now < stopTime + lingerTime)
        return stopTime + lingerTime - now;
      else
        return 0;
    }

    void setSelfDestructTimer(const Parset &parset, time_t lingerTime) {
      if (!parset.settings.realTime)
        return;

      if (getenv("COBALT_NO_ALARM") != NULL)
        return;

      size_t maxRunTime = getMaxRunTime(parset, lingerTime);

      if (maxRunTime > 0) {
        LOG_INFO_STR("Self-destruct: alarm set to " << maxRunTime << " seconds");
        alarm(maxRunTime);
      } else {
        LOG_FATAL_STR("Self-destruct: Observation.stopTime has passed long ago, but observation is real time. Nothing to do. Bye bye.");
	THROW(CoInterfaceException, "Self-destruct timer is in the past.");
      }
    }
  } // namespace Cobalt
} // namespace LOFAR
