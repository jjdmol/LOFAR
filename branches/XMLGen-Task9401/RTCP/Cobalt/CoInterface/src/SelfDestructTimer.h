//# SelfDestructTimer.h: Kill the application using an alarm().
//#
//#  Copyright (C) 2007
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
//#  $Id: BudgetTimer.h 30919 2015-02-05 15:26:22Z amesfoort $

#ifndef LOFAR_COINTERFACE_SELF_DESTRUCT_TIMER_H
#define LOFAR_COINTERFACE_SELF_DESTRUCT_TIMER_H

#include <CoInterface/Exceptions.h>
#include <CoInterface/Parset.h>

namespace LOFAR {
  namespace Cobalt {
    // Return the maximum runtime from now, if we linger `lingerTime' after the observation ends.
    //
    // Returns 0 if the maximum runtime has already passed, or if the observation is not real time.
    size_t getMaxRunTime(const Parset &parset, time_t lingerTime);

    // Sets a self-destruct timer after `lingerTime` after the observation ends. If that time has
    // already passed, a CoInterfaceException is thrown.
    //
    // This function is a no-op in the following cases:
    //
    //  * The observation is not real time.
    //  * The environment variable 'COBALT_NO_ALARM' is defined.
    void setSelfDestructTimer(const Parset &parset, time_t lingerTime);

  } // namespace Cobalt
} // namespace LOFAR

#endif 
