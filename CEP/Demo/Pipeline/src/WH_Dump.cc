//  WH_Dump: WorkHolder class 
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
//////////////////////////////////////////////////////////////////////

#include <stdio.h>             // for sprintf
#include <unistd.h>
#include <math.h>

//#include "CEPFrame/Step.h"
#include "CEPFrame/Profiler.h"
#include "Common/Debug.h"

#include "Pipeline/WH_Dump.h"

using namespace LOFAR;

// Set static variables
int WH_Dump::theirProcessProfilerState=0; 

WH_Dump::WH_Dump (const string& name, 
		  unsigned int nin, 
		  int stationDim,
		  int pols)
: WorkHolder    (nin, 1, name),
  itsStationDim (stationDim),
  itsPols       (pols)
{
  char str[8];
  for (unsigned int i=0; i<nin; i++) {
    sprintf (str, "%d", i);
    TRACER3("Create WH_Dump InhHolder[" << i << "]");
    
    getDataManager().addInDataHolder(i, 
				     new DH_Correlations (std::string("in_") + str,
				     stationDim,
				     1,
				     pols*pols));
  }

  for (unsigned int i=0; i<1; i++) {
    sprintf (str, "%d", i);
    TRACER3("Create WH_Dump OutHolder[" << i << "]");

    getDataManager().addOutDataHolder(i,
				      new DH_Empty(std::string("out_") + str));

  }

  if (theirProcessProfilerState == 0) {
    theirProcessProfilerState = Profiler::defineState("WH_Dump","green");
  }

}


WH_Dump::~WH_Dump()
{
}

WorkHolder* WH_Dump::make(const string& name)
{
  return new WH_Dump(name, 
		     getDataManager().getInputs(), 
		     itsStationDim,
		     itsPols);
}

void WH_Dump::preprocess() {
  return;
}


void WH_Dump::process()
{ 

}

void WH_Dump::dump()
{
}

