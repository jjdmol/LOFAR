//  WH_Correlate: WorkHolder class
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
//////////////////////////////////////////////////////////////////

#include <stdio.h>             // for sprintf
#include <unistd.h>
#include <math.h>

#include "CEPFrame/Step.h"
#include "CEPFrame/Profiler.h"
#include "Common/Debug.h"

#include "Transpose/WH_Correlate.h"

using namespace LOFAR;

// Set static variables
int WH_Correlate::theirProcessProfilerState=0; 

WH_Correlate::WH_Correlate (const string& name, 
			    unsigned int nin, 
			    unsigned int nout,
			    int stationDim,
			    int freqDim)
: WorkHolder    (nin, nout, name),
  itsTime       (0),  
  itsStationDim (stationDim),
  itsFreqDim    (freqDim)
{
  char str[8];
  for (unsigned int i=0; i<nin; i++) {
    sprintf (str, "%d", i);
    TRACER3("Create WH_Correlate InhHolder[" << i << "]");
    getDataManager().addInDataHolder(i, new DH_2DMatrix (std::string("in_") + str,
				       stationDim, std::string("Station"),
				       freqDim, std::string("Frequency"),
				       std::string("Time")));
  }
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    TRACER3("Create WH_Correlate OutHolder[" << i << "]");
    getDataManager().addOutDataHolder(i, new DH_Empty());
  }

  if (theirProcessProfilerState == 0) {
    theirProcessProfilerState = Profiler::defineState("WH_Correlate","blue");
  }
}


WH_Correlate::~WH_Correlate()
{
}

WorkHolder* WH_Correlate::make(const string& name)
{
  return new WH_Correlate(name, 
			  getDataManager().getInputs(), 
			  getDataManager().getOutputs(),
			  itsStationDim,
			  itsFreqDim);
}

void WH_Correlate::preprocess() {
  return;
}


void WH_Correlate::process()
{  
  Profiler::enterState (theirProcessProfilerState);
   
   int Stations,Frequencies;
   int vis;
   int *RowStartptrA, *RowStartptrB;
   DH_2DMatrix *InDHptr;
   for (int time=0; time<getDataManager().getOutputs(); time++) {
      InDHptr = (DH_2DMatrix*)getDataManager().getInHolder(time);
      Stations = InDHptr->getXSize();
      Frequencies = InDHptr->getYSize();
      for (int stationA = 0; stationA < Stations; stationA++) {
	 for (int stationB = 0; stationB <= stationA; stationB++) {
	    RowStartptrA = InDHptr->getBuffer(stationA,0);
	    RowStartptrB = InDHptr->getBuffer(stationB,0);
	    for (int freq=0; freq < Frequencies; freq++) {
	       vis += *(RowStartptrA+freq) * *(RowStartptrB+freq);
	    }
	 }
      }
   }
   Profiler::leaveState (theirProcessProfilerState);
}

void WH_Correlate::dump()
{
  cout << "WH_Correlate " << getName() << " ::dump()" << endl;
  for (int outch=0; outch<std::min(10,getDataManager().getInputs()); outch++) {
    cout << "Input " << outch << "   "
	 << ((DH_2DMatrix*)getDataManager().getInHolder(outch))->getZName() << " "
	 << ((DH_2DMatrix*)getDataManager().getInHolder(outch))->getZ() << "   "
	 << ((DH_2DMatrix*)getDataManager().getInHolder(outch))->getXName() << "Offset = "  
	 << ((DH_2DMatrix*)getDataManager().getInHolder(outch))->getXOffset() << "    "
	 << ((DH_2DMatrix*)getDataManager().getInHolder(outch))->getYName() << "Offset = "  
	 << ((DH_2DMatrix*)getDataManager().getInHolder(outch))->getYOffset() ;
    for (int x=0; 
	 x < std::min(10,((DH_2DMatrix*)getDataManager().getInHolder(outch))->getXSize());
		 x++) {
	   cout << endl 
		<< ((DH_2DMatrix*)getDataManager().getInHolder(outch))->getXName()
		<< x << "   ";
      for (int y=0; 
	   y < std::min(10,((DH_2DMatrix*)getDataManager().getInHolder(outch))->getYSize());
	   y++) {
	cout << *((DH_2DMatrix*)getDataManager().getInHolder(outch))->getBuffer(x,y) << " ";
      }
    }
    cout << endl;
  }
  cout << "=====================================" <<endl;

}

