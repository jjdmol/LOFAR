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
//  $Log$
//  Revision 1.8  2002/07/18 09:35:57  schaaf
//  %[BugId: 11]%
//  Modified time handling
//
//  Revision 1.7  2002/06/10 09:07:13  schaaf
//  %[BugId: 11]%
//  Removed ^M characters
//
//  Revision 1.6  2002/06/07 11:35:20  schaaf
//  %[BugId: 11]%
//  modified process() inner loop; make use of contiguousness of the data
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

#include "Transpose/WH_Transpose.h"


// Set static variables
int WH_Transpose::theirProcessProfilerState=0; 

WH_Transpose::WH_Transpose (const string& name, 
			    unsigned int nin, 
			    unsigned int nout,
			    int timeDim,
			    int freqDim)
: WorkHolder    (nin, nout, name),
  itsInHolders  (0),
  itsOutHolders (0),
  itsTimeDim    (timeDim),
  itsFreqDim    (freqDim)
{
  AssertStr (nin > 0,     "0 input for WH_Transpose is not possible");
  AssertStr (nout > 0,    "0 output for WH_Transpose is not possible");
  //   AssertStr (nout == nin, "number of inputs and outputs must match");
  
  itsInHolders  = new DH_2DMatrix* [nin];
  itsOutHolders = new DH_2DMatrix* [nout];
  char str[8];
  for (unsigned int i=0; i<nin; i++) {
    sprintf (str, "%d", i);
    TRACER3("Create WH_Transpose InhHolder[" << i << "]");
    itsInHolders[i] = new DH_2DMatrix (std::string("in_") + str,
				       timeDim, std::string("Time"),
				       freqDim, std::string("Frequency"),
				       std::string("Station"));
  }
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    TRACER3("Create WH_Transpose OutHolder[" << i << "]");
    itsOutHolders[i] = new DH_2DMatrix (std::string("out_") + str,
				       nin, std::string("Station"),
				       freqDim, std::string("Frequency"),
				       std::string("Time"));
  }

  if (theirProcessProfilerState == 0) {
    theirProcessProfilerState = Profiler::defineState("WH_Transpose","green");
  }
}


WH_Transpose::~WH_Transpose()
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

WorkHolder* WH_Transpose::make(const string& name) const
{
  return new WH_Transpose(name, 
			  getInputs(), 
			  getOutputs(),
			  itsTimeDim,
			  itsFreqDim);
}

void WH_Transpose::preprocess() {
  return;
}


void WH_Transpose::process()
{  

  // check wheteher all timestamps of the inputs are the same
  for (int input=1; input<getInputs(); input++) {
    AssertStr(getInHolder(0)->compareTimeStamp (*getInHolder(input)) == 0,
	      "Input timestamps must be the same!");
  }
  unsigned long localtime = getInHolder(0)->getTimeStamp();

  Profiler::enterState (theirProcessProfilerState);
  
  DH_2DMatrix *InDHptr, *OutDHptr;
  DbgAssertStr(getOutHolder(0)->getXSize() == getInputs(),
	       "nr of stations not correct");
  DbgAssertStr(getOutHolder(0)->getYSize() == getInHolder(0)->getYSize(),
	       "nr of freqs not correct");
  DbgAssertStr(getOutputs() == getInHolder(0)->getXSize(),
	       "nr of times not correct");
  DbgAssertStr(getInHolder(0)->getYSize() == getOutHolder(0)->getYSize(),
	       "Y sizes not equal");
  
  // The X and Y sizes are the same for all outputs (see C'tor),
  //   so we can obtain them outside the loop
  int Xsize = getOutHolder(0)->getXSize();
  int Ysize = getOutHolder(0)->getYSize();
  int Ysize_bytes = Ysize*sizeof(int); // OK; hard coded data type==int
  int StationOffset = getInHolder(0)->getZ();
  
  for (int time=0; time<getOutputs(); time++) {
    OutDHptr = getOutHolder(time); 
    OutDHptr->setTimeStamp(localtime);
    for (int station=0; station < Xsize; station++) {
      InDHptr = getInHolder (station);
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

void WH_Transpose::dump() const
{
  cout << "WH_Transpose " << getName() << " ::dump()" << endl;
  for (int outch=0; outch<min(10,getOutputs()); outch++) {
    cout << "Output " << outch << "   "
	 << (const_cast<WH_Transpose*>(this))->getOutHolder(outch)->getZName() << " "
	 << (const_cast<WH_Transpose*>(this))->getOutHolder(outch)->getZ() << "   "
	 << (const_cast<WH_Transpose*>(this))->getOutHolder(outch)->getXName() << "Offset = "  
	 << (const_cast<WH_Transpose*>(this))->getOutHolder(outch)->getXOffset() << "    "
	 << (const_cast<WH_Transpose*>(this))->getOutHolder(outch)->getYName() << "Offset = "  
	 << (const_cast<WH_Transpose*>(this))->getOutHolder(outch)->getYOffset() ;
    for (int x=0; 
	 x < min(10,(const_cast<WH_Transpose*>(this))->getOutHolder(outch)->getXSize());
		 x++) {
	   cout << endl 
		<< (const_cast<WH_Transpose*>(this))->getOutHolder(outch)->getXName()
		<< x << "   ";
      for (int y=0; 
	   y < min(10,(const_cast<WH_Transpose*>(this))->getOutHolder(outch)->getYSize());
	   y++) {
	cout << *(const_cast<WH_Transpose*>(this))->getOutHolder(outch)->getBuffer(x,y) << " ";
      }
    }
    cout << endl;
  }
  cout << "=====================================" <<endl;

}

DH_2DMatrix* WH_Transpose::getInHolder (int channel)
{
  DbgAssertStr (channel >= 0,          "input channel too low");
  DbgAssertStr (channel < getInputs(), "input channel too high");
  return itsInHolders[channel];
}

DH_2DMatrix* WH_Transpose::getOutHolder (int channel)
{
  DbgAssertStr (channel >= 0,           "output channel too low");
  DbgAssertStr (channel < getOutputs(), "output channel too high");
  return itsOutHolders[channel];
}
