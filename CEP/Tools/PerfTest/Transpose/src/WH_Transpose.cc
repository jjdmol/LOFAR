//  WH_Transpose: WorkHolder class 
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
/////////////////////////////////////////////////////////////////////

#include <stdio.h>             // for sprintf
#include <unistd.h>
#include <math.h>

#include "CEPFrame/Step.h"
#include "CEPFrame/Profiler.h"
#include "Common/Debug.h"

#include "Transpose/WH_Transpose.h"

using namespace LOFAR;

// Set static variables
int WH_Transpose::theirProcessProfilerState=0; 

WH_Transpose::WH_Transpose (const string& name, 
			    unsigned int nin, 
			    unsigned int nout,
			    int timeDim,
			    int freqDim)
: WorkHolder    (nin, nout, name),
  itsTimeDim    (timeDim),
  itsFreqDim    (freqDim)
{
  AssertStr (nin > 0,     "0 input for WH_Transpose is not possible");
  AssertStr (nout > 0,    "0 output for WH_Transpose is not possible");
  //   AssertStr (nout == nin, "number of inputs and outputs must match");
  
  char str[8];
  for (unsigned int i=0; i<nin; i++) {
    sprintf (str, "%d", i);
    TRACER3("Create WH_Transpose InhHolder[" << i << "]");
    getDataManager().addInDataHolder(i, new DH_2DMatrix (std::string("in_") + str,
				       timeDim, std::string("Time"),
				       freqDim, std::string("Frequency"),
							 std::string("Station")));
  }
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    TRACER3("Create WH_Transpose OutHolder[" << i << "]");
    getDataManager().addOutDataHolder(i, new DH_2DMatrix (std::string("out_") + str,
				       nin, std::string("Station"),
				       freqDim, std::string("Frequency"),
				       std::string("Time")));
  }

  if (theirProcessProfilerState == 0) {
    theirProcessProfilerState = Profiler::defineState("WH_Transpose","green");
  }
}


WH_Transpose::~WH_Transpose()
{
}

WorkHolder* WH_Transpose::make(const string& name)
{
  return new WH_Transpose(name, 
			  getDataManager().getInputs(), 
			  getDataManager().getOutputs(),
			  itsTimeDim,
			  itsFreqDim);
}

void WH_Transpose::preprocess() {
  return;
}


void WH_Transpose::process()
{  

  // check wheteher all timestamps of the inputs are the same
  for (int input=1; input<getDataManager().getInputs(); input++) {
    AssertStr(getDataManager().getInHolder(0)->compareTimeStamp 
	      (*getDataManager().getInHolder(input)) == 0,
	      "Input timestamps must be the same!");
  }
  unsigned long localtime = getDataManager().getInHolder(0)->getTimeStamp();

  Profiler::enterState (theirProcessProfilerState);
  
  DH_2DMatrix *InDHptr, *OutDHptr;
  DbgAssertStr(((DH_2DMatrix*)getDataManager().getOutHolder(0))->getXSize() == 
	       getDataManager().getInputs(),"nr of stations not correct");
  DbgAssertStr(((DH_2DMatrix*)getDataManager().getOutHolder(0))->getYSize() 
	       == ((DH_2DMatrix*)getDataManager().getInHolder(0))->getYSize(),
	       "nr of freqs not correct");
  DbgAssertStr(getDataManager().getOutputs() == 
	       ((DH_2DMatrix*)getDataManager().getInHolder(0))->getXSize(),
	       "nr of times not correct");
  DbgAssertStr(((DH_2DMatrix*)getDataManager().getInHolder(0))->getYSize() == 
	       ((DH_2DMatrix*)getDataManager().getOutHolder(0))->getYSize(),
	       "Y sizes not equal");
  
  // The X and Y sizes are the same for all outputs (see C'tor),
  //   so we can obtain them outside the loop
  int Xsize = ((DH_2DMatrix*)getDataManager().getOutHolder(0))->getXSize();
  int Ysize = ((DH_2DMatrix*)getDataManager().getOutHolder(0))->getYSize();
  int Ysize_bytes = Ysize*sizeof(int); // OK; hard coded data type==int
  int StationOffset = ((DH_2DMatrix*)getDataManager().getInHolder(0))->getZ();
  
  for (int time=0; time<getDataManager().getOutputs(); time++) {
    OutDHptr = (DH_2DMatrix*)getDataManager().getOutHolder(time); 
    OutDHptr->setTimeStamp(localtime);
    for (int station=0; station < Xsize; station++) {
      InDHptr = (DH_2DMatrix*)getDataManager().getInHolder (station);
      // DH_2DMatrix::getBuffer(x,y) contiguous for fixed x.
      memcpy(OutDHptr->getBuffer(station,0),
             InDHptr->getBuffer(time,0),
             Ysize_bytes   );
      OutDHptr->setXOffset(StationOffset);  // set station offset
      OutDHptr->setYOffset(InDHptr->getYOffset());   // set freq offset
      OutDHptr->setZ(InDHptr->getXOffset()); 	       // set time
    }
  }
  
  Profiler::leaveState (theirProcessProfilerState);
  //dump();

}

void WH_Transpose::dump()
{
  cout << "WH_Transpose " << getName() << " ::dump()" << endl;
  for (int outch=0; outch<std::min(10,getDataManager().getOutputs()); outch++) {
    cout << "Output " << outch << "   "
	 << ((DH_2DMatrix*)getDataManager().getOutHolder(outch))->getZName() << " "
	 << ((DH_2DMatrix*)getDataManager().getOutHolder(outch))->getZ() << "   "
	 << ((DH_2DMatrix*)getDataManager().getOutHolder(outch))->getXName() << "Offset = "  
	 << ((DH_2DMatrix*)getDataManager().getOutHolder(outch))->getXOffset() << "    "
	 << ((DH_2DMatrix*)getDataManager().getOutHolder(outch))->getYName() << "Offset = "  
	 << ((DH_2DMatrix*)getDataManager().getOutHolder(outch))->getYOffset() ;
    for (int x=0; 
	 x < std::min(10,((DH_2DMatrix*)getDataManager().getOutHolder(outch))->getXSize());
		 x++) {
	   cout << endl 
		<< ((DH_2DMatrix*)getDataManager().getOutHolder(outch))->getXName()
		<< x << "   ";
      for (int y=0; 
	   y < std::min(10,((DH_2DMatrix*)getDataManager().getOutHolder(outch))->getYSize());
	   y++) {
	cout << *((DH_2DMatrix*)getDataManager().getOutHolder(outch))->getBuffer(x,y) << " ";
      }
    }
    cout << endl;
  }
  cout << "=====================================" <<endl;

}
