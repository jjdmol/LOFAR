//  WH_Dest.cc: WorkHolder class using DH_VarSize() objects and 
//                  measuring performance
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>             // for sprintf
#include <math.h>

#include "CEPFrame/Step.h"
#include <Common/Debug.h>

#include "P2Perf/WH_Dest.h"
#include "P2Perf/StopWatch.h"
#include "P2Perf/P2Perf.h"

using namespace LOFAR;

                
WH_Dest::WH_Dest (DHGrowStrategy* DHGS,// the object that will grow the DH's
		  const string& name, 
		  unsigned int nin, 
		  unsigned int size,    // size of the packet in bytes
		  unsigned int packetsPerGrowStep)
         : WorkHolder    (nin, 0, name),
           itsSize (size),
           itsDHGrowStrategy(DHGS),
           itsPacketsPerGrowStep(packetsPerGrowStep),
           itsIteration(0)
{
  char str[8];

  for (unsigned int i=0; i<nin; i++) {
    sprintf (str, "%d", i);
    getDataManager().addInDataHolder(i, 
                                     new DH_VarSize (std::string("in_") + str,
                                                     itsSize));
  }

  watch = StopWatch::getGlobalStopWatch();  
}


WH_Dest::~WH_Dest()
{
}

WorkHolder* WH_Dest::make(const string& name)
{
  return new WH_Dest(itsDHGrowStrategy,
		     name, 
		     getDataManager().getInputs(),
		     itsSize,
		     itsPacketsPerGrowStep);
}

void WH_Dest::preprocess()
{
  // this is needed to synchronize with the WH_Src
  itsIteration++;
  if (itsIteration == itsPacketsPerGrowStep)
  {
    itsIteration = 0;
    // change size of the DataHolder
    TRACER2("growing dataSize of " << getName() << " from " << getDataManager().getInHolder(0)->getDataSize() << " to ");
    itsDHGrowStrategy->growDHs (&getDataManager());
    TRACER2(getDataManager().getInHolder(0)->getDataSize());

  }
};

void WH_Dest::process()
{  
  getDataManager().getInHolder(0);
  itsIteration++;
  if (itsIteration == itsPacketsPerGrowStep)
    {
      itsIteration = 0;
      // pausing the watch is nice to get more accurate measurements, but
      // it does not work when using multiple threads or processes
      // because each process will pause its own stopwatch and the time
      // will not be subtracted from the time of the measuring stopwatch
      watch->pause();
      // change size of the DataHolder
      //      TRACER2("growing dataSize of " << getName() << " from " << getDataManager().getInHolder(0)->getDataSize() << " to ");
      itsDHGrowStrategy->growDHs (&getDataManager());
      //TRACER2(getDataManager().getInHolder(0)->getDataSize());

      watch->resume();
    }
}
