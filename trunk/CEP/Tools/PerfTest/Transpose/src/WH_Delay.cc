//  WH_Delay: WorkHolder class 
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
//
//
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>             // for sprintf
#include <unistd.h>
#include <math.h>

#include "BaseSim/Step.h"
#include "BaseSim/Profiler.h"
#include "BaseSim/CyclicBuffer.h"
#include "Common/Debug.h"

#include "Transpose/WH_Delay.h"


// Set static variables
int WH_Delay::theirProcessProfilerState=0; 

WH_Delay::WH_Delay (const string& name, 
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
  AssertStr (nin > 0,     "0 input for WH_Delay is not possible");
  AssertStr (nout > 0,    "0 output for WH_Delay is not possible");
  AssertStr (nout == nin, "number of inputs and outputs must match");

  itsBuffer = new CyclicBuffer<DH_2DMatrix*>[10];
  itsInHolders  = new DH_2DMatrix* [nin];
  itsOutHolders = new DH_2DMatrix* [nout];
  char str[8];
  for (unsigned int i=0; i<nin; i++) {
    sprintf (str, "%d", i);
    TRACER3("Create WH_Delay InhHolder[" << i << "]");
    itsInHolders[i] = new DH_2DMatrix (std::string("in_") + str,
				       timeDim, std::string("Time"),
				       freqDim, std::string("Frequency"),
				       std::string("Station"));

    
    for (int i=0;i<10;i++) {
      itsBuffer[i].AddBufferElement(new DH_2DMatrix (std::string("in_") + str,
						      timeDim, std::string("Time"),
						      freqDim, std::string("Frequency"),
						      std::string("Station")));
    }
  }
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    TRACER3("Create WH_Delay OutHolder[" << i << "]");
    itsOutHolders[i] = new DH_2DMatrix (std::string("out_") + str,
				       timeDim, std::string("Time"),
				       freqDim, std::string("Frequency"),
				       std::string("Station"));
  }

  if (theirProcessProfilerState == 0) {
    theirProcessProfilerState = Profiler::defineState("WH_Delay","orange");
  }
}


WH_Delay::~WH_Delay()
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

WorkHolder* WH_Delay::make(const string& name) const
{
  return new WH_Delay(name, 
			  getInputs(), 
			  getOutputs(),
			  itsTimeDim,
			  itsFreqDim);
}

void WH_Delay::preprocess() {
  return;
}


void WH_Delay::process()
{  
  for (int input=0; input<getInputs(); input++) {
    
    void* bufferptr;
    int ID;
    bufferptr = (void*) itsBuffer[input].GetWriteLockedDataItem(&ID);
    memcpy (bufferptr,
	    getInHolder(input)->getDataPtr(),
	    getInHolder(input)->getCurDataPacketSize());
    itsBuffer[input].WriteUnlockElement(ID);
    
    bufferptr = (void*) itsBuffer[input].GetReadDataItem(&ID);
    memcpy (getOutHolder(input)->getDataPtr(),
	    bufferptr,
	    getInHolder(input)->getCurDataPacketSize());  
    itsBuffer[input].ReadUnlockElement(ID);
  }
}

void WH_Delay::dump() const
{
  cout << "WH_Delay " << getName() << " ::dump()" << endl;
  for (int outch=0; outch<min(10,getOutputs()); outch++) {
    cout << "Output " << outch << "   "
	 << (const_cast<WH_Delay*>(this))->getOutHolder(outch)->getZName() << " "
	 << (const_cast<WH_Delay*>(this))->getOutHolder(outch)->getZ() << "   "
	 << (const_cast<WH_Delay*>(this))->getOutHolder(outch)->getXName() << "Offset = "  
	 << (const_cast<WH_Delay*>(this))->getOutHolder(outch)->getXOffset() << "    "
	 << (const_cast<WH_Delay*>(this))->getOutHolder(outch)->getYName() << "Offset = "  
	 << (const_cast<WH_Delay*>(this))->getOutHolder(outch)->getYOffset() ;
    for (int x=0; 
	 x < min(10,(const_cast<WH_Delay*>(this))->getOutHolder(outch)->getXSize());
		 x++) {
	   cout << endl 
		<< (const_cast<WH_Delay*>(this))->getOutHolder(outch)->getXName()
		<< x << "   ";
      for (int y=0; 
	   y < min(10,(const_cast<WH_Delay*>(this))->getOutHolder(outch)->getYSize());
	   y++) {
	cout << *(const_cast<WH_Delay*>(this))->getOutHolder(outch)->getBuffer(x,y) << " ";
      }
    }
    cout << endl;
  }
  cout << "=====================================" <<endl;

}

DH_2DMatrix* WH_Delay::getInHolder (int channel)
{
  DbgAssertStr (channel >= 0,          "input channel too low");
  DbgAssertStr (channel < getInputs(), "input channel too high");
  return itsInHolders[channel];
}

DH_2DMatrix* WH_Delay::getOutHolder (int channel)
{
  DbgAssertStr (channel >= 0,           "output channel too low");
  DbgAssertStr (channel < getOutputs(), "output channel too high");
  return itsOutHolders[channel];
}
