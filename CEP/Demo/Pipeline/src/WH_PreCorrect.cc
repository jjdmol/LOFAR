//  WH_PreCorrect: WorkHolder class
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

#include "BaseSim/Step.h"
#include "BaseSim/Profiler.h"
#include "Common/Debug.h"

#include "Pipeline/WH_PreCorrect.h"

// Set static variables
int WH_PreCorrect::theirProcessProfilerState=0; 

WH_PreCorrect::WH_PreCorrect (const string& name, 
			    unsigned int channels,
			    int stationDim,
			    int freqDim,
			    int pols)  // input side
: WorkHolder    (channels, channels, name),
  itsInHolders  (0),
  itsOutHolders (0),
  itsTime       (0),  
  itsStationDim (stationDim),
  itsFreqDim    (freqDim),
  itsPols       (pols)
{
  TRACER4("WH_PreCorrect C'tor");
  
  itsInHolders  = new DH_2DMatrix* [channels];
  itsOutHolders = new DH_2DMatrix* [channels];
  char str[8];
  for (int i=0; i<getInputs(); i++) {
    sprintf (str, "%d", i);
    TRACER3("Create WH_PreCorrect InhHolder[" << i << "]");
    itsInHolders[i] = new DH_2DMatrix (std::string("in_") + str,
				       itsStationDim, std::string("Station"),
				       itsFreqDim, std::string("Frequency"),
				       std::string("Time"),
				       itsPols);
  }
  for (int i=0; i<getOutputs(); i++) {
    sprintf (str, "%d", i);
    TRACER3("Create WH_PreCorrect OutHolder[" << i << "]");
    itsOutHolders[i] = new DH_2DMatrix(std::string("out_") + str,
				       itsStationDim, std::string("Station"),
				       itsFreqDim, std::string("Frequency"),
				       std::string("Time"),
				       itsPols); 
   
}
  // allocate the correctionvector and initialise to unit
  itsCorrectionVector = new DH_2DMatrix::DataType[stationDim];
  for (int i=0; i<stationDim; i++) setCorrectionVector(i,DH_2DMatrix::DataType(1,0));

  if (theirProcessProfilerState == 0) {
    theirProcessProfilerState = Profiler::defineState("WH_PreCorrect","purple");
  }
}


WH_PreCorrect::~WH_PreCorrect()
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

void  WH_PreCorrect::setCorrectionVector(int Field, myComplex8 Value) {
  DbgAssertStr(((Field >= 0) && (Field < itsStationDim)), "Field argument out of boundaries" );
  itsCorrectionVector[Field] = Value;
}

void  WH_PreCorrect::setCorrectionVector(myComplex8 *Value) {
  for (int i=0; i<itsStationDim; i++) itsCorrectionVector[i] = Value[i];
}

WorkHolder* WH_PreCorrect::make(const string& name) const
{
  TRACER4("WH_Correlator::make()");
  return new WH_PreCorrect(name, 
			  getInputs(), 
			  itsStationDim,
			  itsFreqDim,
			  itsPols);
}

void WH_PreCorrect::preprocess() {
  TRACER4("WH_Correlator::preprocess");
  return;
}


void WH_PreCorrect::process()
{  
  Profiler::enterState (theirProcessProfilerState);
  
  DbgAssertStr(getOutputs() == getInputs(), 
	       "inputs and outputs must be equal");
  
  
  for (int channel=0; channel<getOutputs(); channel++) {
    DH_2DMatrix *OutDHptr, *InDHptr;
    OutDHptr = getOutHolder(channel);
    InDHptr  = getInHolder(channel);
    DbgAssertStr(OutDHptr != 0, "GetOutHolder returned NULL");
    DbgAssertStr(InDHptr != 0, "GetInHolder returned NULL");
    int Xsize = OutDHptr->getXSize();
    for (int x=0; x < Xsize; x++) {
      int Ysize = OutDHptr->getYSize();
      for (int y=0; y < Ysize; y++) {
	for (int pol=0; pol<itsPols; pol++) {
	  OutDHptr->getBuffer(x,y,pol)->mult(*InDHptr->getBuffer(x,y,pol),itsCorrectionVector[x]);
	  // both polarisations use same correction factor
	}
      }
    }
    OutDHptr->setZ(InDHptr->getZ());      
    OutDHptr->setYOffset(InDHptr->getYOffset());
    OutDHptr->setTimeStamp(itsTime);
    OutDHptr->setXOffset(itsTime); 
  }
  Profiler::leaveState (theirProcessProfilerState);
}

void WH_PreCorrect::dump() const
{
}


DH_2DMatrix* WH_PreCorrect::getInHolder (int channel)
{
  DbgAssertStr (channel >= 0,          "input channel too low");
  DbgAssertStr (channel < getInputs(), "input channel too high");
  TRACER4("channel = " << channel);
  return itsInHolders[channel];
}

DH_2DMatrix* WH_PreCorrect::getOutHolder (int channel)
{
  DbgAssertStr (channel >= 0,           "output channel too low");
  DbgAssertStr (channel < getOutputs(), "output channel too high");
  return itsOutHolders[channel];
}
