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
//////////////////////////////////////////////////////////////////////

#include <stdio.h>             // for sprintf
#include <unistd.h>
#include <math.h>

#include "CEPFrame/Step.h"
#include "CEPFrame/Profiler.h"
#include "Common/Debug.h"

#include "Pipeline/WH_Correlate.h"

using namespace LOFAR;

// Set static variables
int WH_Correlate::theirProcessProfilerState=0; 

WH_Correlate::WH_Correlate (const string& name, 
			    unsigned int nin, 
			    unsigned int nout,
			    int stationDim,
			    int freqDim,
			    int pols)  // input side
: WorkHolder    (nin, nout, name),
  itsTime       (0),  
  itsStationDim (stationDim),
  itsFreqDim    (freqDim),
  itsInPols     (pols),
  itsOutPols    (pols*pols),
  itsReset      (true)
{
  TRACER4("WH_Correlate C'tor");
  DbgAssertStr(nout == 1, "Need exactly one output");
  
  char str[8];
  for (int i=0; i<getDataManager().getInputs(); i++) {
    sprintf (str, "%d", i);
    TRACER3("Create WH_Correlate InhHolder[" << i << "]");

    getDataManager().addInDataHolder(i,
				     new DH_2DMatrix (std::string("in_") + str,
				     itsStationDim, std::string("Station"),
				     itsFreqDim, std::string("Frequency"),
				     std::string("Time"),
				     itsInPols));
  }
  for (int i=0; i<getDataManager().getOutputs(); i++) {
    sprintf (str, "%d", i);
    TRACER3("Create WH_Correlate OutHolder[" << i << "]");
    
    getDataManager().addOutDataHolder(i,
				      new DH_Correlations(getName(),
				      stationDim,
				      1, // integrate complete input into 1 output channel
				      itsOutPols));   
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
  TRACER4("WH_Correlator::make()");
  return new WH_Correlate(name, 
			  getDataManager().getInputs(), 
			  getDataManager().getOutputs(),
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
    
    DbgAssertStr(getDataManager().getOutHolder(0)->getType() == "DH_Correlations", 
		 "DataHolder is not of type DH_Correlations");
    DH_Correlations* dhOut = (DH_Correlations*)getDataManager().getOutHolder(0);

    DbgAssertStr(getDataManager().getInHolder(0)->getType() == "DH_2DMatrix",
		 "DataHolder is not of type DH_2DMatrix");
    DH_2DMatrix* dhIn = (DH_2DMatrix*)getDataManager().getInHolder(0);

    dhOut->reset();
    // now start filling the housekeeping data
    dhOut->setStartTime(dhIn->getZ());
    dhOut->setStartChannel(dhIn->getYOffset());
    dhOut->setEndChannel(dhIn->getYOffset() + dhIn->getYSize() -1);
  }

   int Stations,Frequencies;
   DH_Correlations *OutDHptr;
   DH_2DMatrix     *InDHptr;

   DbgAssertStr(getDataManager().getOutHolder(0)->getType() == "DH_Correlations", 
		 "DataHolder is not of type DH_Correlations");
   OutDHptr = (DH_Correlations*)getDataManager().getOutHolder(0);

   for (int time=0; time<getDataManager().getInputs(); time++) {
      DbgAssertStr(getDataManager().getInHolder(time)->getType() == "DH_2DMatrix",
		 "DataHolder is not of type DH_2DMatrix"); 
      InDHptr = (DH_2DMatrix*)getDataManager().getInHolder(time);

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
     DbgAssertStr(getDataManager().getOutputs() == 1,"reset only works for times=1");
     if (OutDHptr->doHandle()) {
       //output data will be read; so clear the registers in next call.
       itsReset = true;
       DbgAssertStr(getDataManager().getInHolder(0)->getType() == "DH_2DMatrix",
		 "DataHolder is not of type DH_2DMatrix");
       OutDHptr->setEndTime(((DH_2DMatrix*)getDataManager().getInHolder(0))->getZ());
     }

}

void WH_Correlate::dump()
{
  DbgAssertStr(getDataManager().getOutHolder(0)->getType() == "DH_Correlations", 
		 "DataHolder is not of type DH_Correlations");
  DH_Correlations* dhOut = (DH_Correlations*)getDataManager().getOutHolder(0);

  cout << "WH_Correlate " << getName() << " ::dump()" << endl;
  cout << "Time:      " << dhOut->getStartTime() 
       << " - " << dhOut->getEndTime() << endl;
  cout << "Frequency: " << dhOut->getStartChannel() 
       << " - " << dhOut->getEndChannel() << endl;

  for (int pol=0; pol<itsOutPols; pol++) {
    cout << "Polarisation: " << pol << endl ;
    DbgAssertStr(getDataManager().getInHolder(0)->getType() == "DH_2DMatrix",
		 "DataHolder is not of type DH_2DMatrix");
    int Stations = ((DH_2DMatrix*)getDataManager().getInHolder(0))->getXSize();
    for (short stationA = 0; stationA < Stations; stationA++) {
      cout << stationA << ":  ";
      for (short stationB = 0; stationB <= stationA; stationB++) {
	DH_Correlations::DataType val = *dhOut->getBuffer(stationA,stationB,pol);
	cout << val
	     << " " ;
      }
      cout << endl;
    }
  }
}


