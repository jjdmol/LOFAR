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

//#include "CEPFrame/Step.h"
#include "CEPFrame/Profiler.h"
#include "Common/Debug.h"

#include "Pipeline/WH_PreCorrect.h"

using namespace LOFAR;

// Set static variables
int WH_PreCorrect::theirProcessProfilerState=0; 

WH_PreCorrect::WH_PreCorrect (const string& name, 
			    unsigned int channels,
			    int stationDim,
			    int freqDim,
			    int pols)  // input side
: WorkHolder    (channels, channels, name),
  itsTime       (0),  
  itsStationDim (stationDim),
  itsFreqDim    (freqDim),
  itsPols       (pols)
{
  TRACER4("WH_PreCorrect C'tor");
  
  char str[8];
  for (int i=0; i<getDataManager().getInputs(); i++) {
    sprintf (str, "%d", i);
    TRACER3("Create WH_PreCorrect InhHolder[" << i << "]");
    getDataManager().addInDataHolder(i, 
				     new DH_2DMatrix (std::string("in_") + str,
				     itsStationDim, std::string("Station"),
				     itsFreqDim, std::string("Frequency"),
				     std::string("Time"),
				     itsPols), true);
  }
  for (int i=0; i<getDataManager().getOutputs(); i++) {
    sprintf (str, "%d", i);
    TRACER3("Create WH_PreCorrect OutHolder[" << i << "]");
    getDataManager().addOutDataHolder(i,
				      new DH_2DMatrix(std::string("out_") + str,
				      itsStationDim, std::string("Station"),
				      itsFreqDim, std::string("Frequency"),
				      std::string("Time"),
				      itsPols), true);

   
}
  // allocate the correctionvector and initialise to unit
  itsCorrectionVector = new DH_2DMatrix::DataType[stationDim];
  for (int i=0; i<stationDim; i++) {
    //setCorrectionVector(i,DH_2DMatrix::DataType(0.5+1.0*random()/RAND_MAX,0.5+1.0*random()/RAND_MAX));
    setCorrectionVector(i,DH_2DMatrix::DataType(1,0));
  }
  if (theirProcessProfilerState == 0) {
    theirProcessProfilerState = Profiler::defineState("WH_PreCorrect","purple");
  }

}


WH_PreCorrect::~WH_PreCorrect()
{
}

void  WH_PreCorrect::setCorrectionVector(int Field, myComplex8 Value) {
  DbgAssertStr(((Field >= 0) && (Field < itsStationDim)), "Field argument out of boundaries" );
  itsCorrectionVector[Field] = Value;
}

void  WH_PreCorrect::setCorrectionVector(myComplex8 *Value) {
  for (int i=0; i<itsStationDim; i++) itsCorrectionVector[i] = Value[i];
}

WorkHolder* WH_PreCorrect::make(const string& name)
{
  TRACER4("WH_PreCorrect::make()");
  return new WH_PreCorrect(name, 
			  getDataManager().getInputs(), 
			  itsStationDim,
			  itsFreqDim,
			  itsPols);
}

void WH_PreCorrect::preprocess() {
  TRACER4("WH_PreCorrect::preprocess");
  return;
}


void WH_PreCorrect::process()
{  
  Profiler::enterState (theirProcessProfilerState);  
  DbgAssertStr(getDataManager().getOutputs() == getDataManager().getInputs(), 
	       "inputs and outputs must be equal");
  
  
  for (int channel=0; channel<getDataManager().getOutputs(); channel++) {
    DH_2DMatrix *OutDHptr, *InDHptr;
    OutDHptr = (DH_2DMatrix*)getDataManager().getOutHolder(channel);
    InDHptr  = (DH_2DMatrix*)getDataManager().getInHolder(channel);
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

void WH_PreCorrect::dump()
{
  cout << "WH_PreCorrect " << getName() << " ::dump()" << endl;
  for (int outch=0; outch<std::min(10,getDataManager().getOutputs()); outch++) {

    DbgAssertStr(getDataManager().getOutHolder(outch)->getType() == "DH_2DMatrix",
	         "DataHolder is not of type DH_2DMatrix");
    DH_2DMatrix* outDH = (DH_2DMatrix*)getDataManager().getOutHolder(outch);
    cout << "Output " << outch << "   " << outDH->getZName()  << " "<< outDH->getZ() 
	 << "   " << outDH->getXName() << "Offset = "  << outDH->getXOffset() 
	 << "    " << outDH->getYName() << "Offset = " << outDH->getYOffset() ;

    for (int pol=0; pol<itsPols; pol++) {
      cout << endl << "Polarisation: " << pol ;
      for (int x=0; 
	   x < std::min(10, outDH->getXSize());
	   x++) {
	cout << endl 
	     << outDH->getXName()
	     << x << "   ";
	for (int y=0; 
	     y < std::min(10, outDH->getYSize());
	     y++) {
	  cout << *outDH->getBuffer(x,y,pol) << " ";
	}
      }
    }
    cout << endl;
  }
  cout << "=====================================" <<endl;
}
