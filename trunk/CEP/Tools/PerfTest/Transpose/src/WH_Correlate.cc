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
//  $Log$
//  Revision 1.5  2002/05/16 15:00:34  schaaf
//
//  overall update, added profiler states, removed command line processing, setZ/XOffset and Yoffset
//
//  Revision 1.3  2002/05/07 14:59:16  schaaf
//  optimised performance of process()
//
//  Revision 1.2  2002/05/07 11:15:12  schaaf
//  minor
//
//  Revision 1.1.1.1  2002/05/06 11:49:20  schaaf
//  initial version
//
//
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>             // for sprintf
#include <unistd.h>
#include <math.h>

#include "BaseSim/Step.h"
#include "BaseSim/Profiler.h"
#include "Common/Debug.h"

#include "Transpose/WH_Correlate.h"


// Set static variables
int WH_Correlate::theirProcessProfilerState=0; 

WH_Correlate::WH_Correlate (const string& name, 
			    unsigned int nin, 
			    unsigned int nout,
			    int stationDim,
			    int freqDim)
: WorkHolder    (nin, nout, name),
  itsInHolders  (0),
  itsOutHolders (0),
  itsTime       (0),  
  itsStationDim (stationDim),
  itsFreqDim    (freqDim)
{
  AssertStr (nin > 0,     "0 input for WH_Correlate is not possible");
  AssertStr (nout > 0,    "0 output for WH_Correlate is not possible");
  //   AssertStr (nout == nin, "number of inputs and outputs must match");
  
  itsInHolders  = new DH_2DMatrix* [nin];
  itsOutHolders = new DH_Empty* [nout];
  char str[8];
  for (unsigned int i=0; i<nin; i++) {
    sprintf (str, "%d", i);
    TRACER3("Create WH_Correlate InhHolder[" << i << "]");
    itsInHolders[i] = new DH_2DMatrix (std::string("in_") + str,
				       stationDim, std::string("Station"),
				       freqDim, std::string("Frequency"),
				       std::string("Time"));
  }
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    TRACER3("Create WH_Correlate OutHolder[" << i << "]");
    itsOutHolders[i] = new DH_Empty();
  }

  if (theirProcessProfilerState == 0) {
    theirProcessProfilerState = Profiler::defineState("WH_Correlate","blue");
  }
}


WH_Correlate::~WH_Correlate()
{
  for (int i=0; i<getInputs(); i++) {
    delete itsInHolders[i];
  }
  for (int i=0; i<getOutputs(); i++) {
    delete itsOutHolders[i];
  }
  delete [] itsInHolders;
  delete [] itsOutHolders;
}

WorkHolder* WH_Correlate::make(const string& name) const
{
  return new WH_Correlate(name, 
			  getInputs(), 
			  getOutputs(),
			  itsStationDim,
			  itsFreqDim);
}

void WH_Correlate::preprocess() {
  return;
}


void WH_Correlate::process()
{  

  Profiler::enterState (theirProcessProfilerState);
  itsOutHolders[0]->setTimeStamp(itsTime++);
  
  int Stations,Frequencies;
  int vis;
  int Ysize_bytes;
  DH_2DMatrix *InDHptr;
  
  for (int time=0; time<getOutputs(); time++) {
    InDHptr = getInHolder(time);
    Stations = InDHptr->getXSize();
    Frequencies = InDHptr->getYSize();
    for (int stationA = 0; stationA < Stations; stationA++) {
      for (int stationB = 0; stationB <= stationA; stationB++) {
	for (int freq=0; freq < Frequencies; freq++) {
	  vis += *InDHptr->getBuffer(stationA,freq) * 
	    *InDHptr->getBuffer(stationB,freq);
	}
      }
    }
  }
  Profiler::leaveState (theirProcessProfilerState);
  //dump();
}

void WH_Correlate::dump() const
{
  cout << "WH_Correlate " << getName() << " ::dump()" << endl;
  for (int outch=0; outch<min(10,getInputs()); outch++) {
    cout << "Input " << outch << "   "
	 << (const_cast<WH_Correlate*>(this))->getInHolder(outch)->getZName() << " "
	 << (const_cast<WH_Correlate*>(this))->getInHolder(outch)->getZ() << "   "
	 << (const_cast<WH_Correlate*>(this))->getInHolder(outch)->getXName() << "Offset = "  
	 << (const_cast<WH_Correlate*>(this))->getInHolder(outch)->getXOffset() << "    "
	 << (const_cast<WH_Correlate*>(this))->getInHolder(outch)->getYName() << "Offset = "  
	 << (const_cast<WH_Correlate*>(this))->getInHolder(outch)->getYOffset() ;
    for (int x=0; 
	 x < min(10,(const_cast<WH_Correlate*>(this))->getInHolder(outch)->getXSize());
		 x++) {
	   cout << endl 
		<< (const_cast<WH_Correlate*>(this))->getInHolder(outch)->getXName()
		<< x << "   ";
      for (int y=0; 
	   y < min(10,(const_cast<WH_Correlate*>(this))->getInHolder(outch)->getYSize());
	   y++) {
	cout << *(const_cast<WH_Correlate*>(this))->getInHolder(outch)->getBuffer(x,y) << " ";
      }
    }
    cout << endl;
  }
  cout << "=====================================" <<endl;

}

DH_2DMatrix* WH_Correlate::getInHolder (int channel)
{
  DbgAssertStr (channel >= 0,          "input channel too low");
  DbgAssertStr (channel < getInputs(), "input channel too high");
  return itsInHolders[channel];
}

DH_Empty* WH_Correlate::getOutHolder (int channel)
{
  DbgAssertStr (channel >= 0,           "output channel too low");
  DbgAssertStr (channel < getOutputs(), "output channel too high");
  return itsOutHolders[channel];
}
