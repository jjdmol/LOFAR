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
//  Revision 1.1.1.1  2002/11/13 15:58:06  schaaf
//  %[BugId: 117]%
//
//  Initial working version
//
//  Revision 1.3  2002/08/19 20:42:14  schaaf
//  %[BugId: 11]%
//  Removed dump() call
//
//  Revision 1.2  2002/06/07 11:42:43  schaaf
//  %[BugId: 11]%
//  Removed Asserts in C'tor
//
//  Revision 1.1  2002/05/23 15:40:44  schaaf
//
//  %[BugId: 11]%
//  Added WH_Correlate
//
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

#include "Pipeline/WH_Correlate.h"

// Set static variables
int WH_Correlate::theirProcessProfilerState=0; 

WH_Correlate::WH_Correlate (const string& name, 
			    unsigned int nin, 
			    unsigned int nout,
			    int stationDim,
			    int freqDim,
			    int pols)  // input side
: WorkHolder    (nin, nout, name),
  itsInHolders  (0),
  itsOutHolders (0),
  itsTime       (0),  
  itsStationDim (stationDim),
  itsFreqDim    (freqDim),
  itsInPols     (pols),
  itsOutPols    (pols*pols),
  itsReset      (true)
{
  TRACER4("WH_Correlate C'tor");
  DbgAssertStr(nout == 1, "Need exactly one output");
  
  itsInHolders  = new DH_2DMatrix* [nin];
  itsOutHolders = new DH_Correlations* [nout];
  char str[8];
  for (int i=0; i<getInputs(); i++) {
    sprintf (str, "%d", i);
    TRACER3("Create WH_Correlate InhHolder[" << i << "]");
    itsInHolders[i] = new DH_2DMatrix (std::string("in_") + str,
				       itsStationDim, std::string("Station"),
				       itsFreqDim, std::string("Frequency"),
				       std::string("Time"),
				       itsInPols); // pols input side
  }
  for (int i=0; i<getOutputs(); i++) {
    sprintf (str, "%d", i);
    TRACER3("Create WH_Correlate OutHolder[" << i << "]");
    itsOutHolders[i] = new DH_Correlations(getName(),
					   stationDim,
					   1,   // integrate complete input into 1 output channel
					   itsOutPols);  // polarisations
   
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
  TRACER4("WH_Correlator::make()");
  return new WH_Correlate(name, 
			  getInputs(), 
			  getOutputs(),
			  itsStationDim,
			  itsFreqDim,
			  itsInPols);
}

void WH_Correlate::preprocess() {
  TRACER4("WH_Correlator::preprocess");
  return;
}


void WH_Correlate::process()
{  
  Profiler::enterState (theirProcessProfilerState);
  
  if (itsReset) {
    // after write of the data to the next step, the outputholder should be reset to contain only 0 values.
    itsReset = false;
    getOutHolder(0)->reset();
    // now start filling the housekeeping data
    getOutHolder(0)->setStartTime(getInHolder(0)->getZ());
    getOutHolder(0)->setStartChannel(getInHolder(0)->getYOffset());
    getOutHolder(0)->setEndChannel(getInHolder(0)->getYOffset() + getInHolder(0)->getYSize() -1);
  }

   int Stations,Frequencies;
   DH_Correlations *OutDHptr;
   DH_2DMatrix     *InDHptr;

     for (int time=0; time<getInputs(); time++) {
      InDHptr = getInHolder(time);
      OutDHptr = getOutHolder(0);
      Stations = InDHptr->getXSize();
      Frequencies = InDHptr->getYSize();
      for (short stationA = 0; stationA < Stations; stationA++) {
	for (short stationB = 0; stationB <= stationA; stationB++) {
	  // loop over all input frequencies effectively integrating those inputs.
	  for (short freq=0; freq < Frequencies; freq++) {
	    DbgAssertStr((itsInPols == 2 && itsOutPols == 4),"polarisations incorrect");
	    // XX
	    OutDHptr->getBuffer(stationA,stationB,0)->cmac(*InDHptr->getBuffer(stationA,freq,0),
							   *InDHptr->getBuffer(stationB,freq,0)); 	       
	    // XY
	    OutDHptr->getBuffer(stationA,stationB,1)->cmac(*InDHptr->getBuffer(stationA,freq,0),
							   *InDHptr->getBuffer(stationB,freq,1) ); 			
	    // YX	     
	    OutDHptr->getBuffer(stationA,stationB,2)->cmac(*InDHptr->getBuffer(stationA,freq,1),
							   *InDHptr->getBuffer(stationB,freq,0) ); 		
	    // YY	     
	    OutDHptr->getBuffer(stationA,stationB,3)->cmac(*InDHptr->getBuffer(stationA,freq,1),
							   *InDHptr->getBuffer(stationB,freq,1) ); 							        
	  }
	}
      }
     }
     Profiler::leaveState (theirProcessProfilerState);
     DbgAssertStr(getOutputs() == 1,"reset only works for times=1");
     if (getOutHolder(0)->doHandle()) {
       //output data will be read; so clear the registers in next call.
       itsReset = true;
       getOutHolder(0)->setEndTime(getInHolder(0)->getZ());
     }
}

void WH_Correlate::dump() const
{
  cout << "WH_Correlate " << getName() << " ::dump()" << endl;
  cout << "Time:      " <<  (const_cast<WH_Correlate*>(this))->getOutHolder(0)->getStartTime() 
       << " - " << (const_cast<WH_Correlate*>(this))->getOutHolder(0)->getEndTime()
       << endl;
  cout << "Frequency: " <<  (const_cast<WH_Correlate*>(this))->getOutHolder(0)->getStartChannel() 
       << " - " << (const_cast<WH_Correlate*>(this))->getOutHolder(0)->getEndChannel()
       << endl;
  for (int pol=0; pol<itsOutPols; pol++) {
    cout << "Polarisation: " << pol << endl ;
    int Stations = (const_cast<WH_Correlate*>(this))->getInHolder(0)->getXSize();
    for (short stationA = 0; stationA < Stations; stationA++) {
      cout << stationA << ":  ";
      for (short stationB = 0; stationB <= stationA; stationB++) {
	DH_Correlations::DataType val = *(const_cast<WH_Correlate*>(this))->getOutHolder(0)->getBuffer(stationA,stationB,pol);
	cout << val
	     << " " ;
      }
      cout << endl;
    }
  }
}


DH_2DMatrix* WH_Correlate::getInHolder (int channel)
{
  DbgAssertStr (channel >= 0,          "input channel too low");
  DbgAssertStr (channel < getInputs(), "input channel too high");
  TRACER4("channel = " << channel);
  return itsInHolders[channel];
}

DH_Correlations* WH_Correlate::getOutHolder (int channel)
{
  DbgAssertStr (channel >= 0,           "output channel too low");
  DbgAssertStr (channel < getOutputs(), "output channel too high");
  return itsOutHolders[channel];
}
