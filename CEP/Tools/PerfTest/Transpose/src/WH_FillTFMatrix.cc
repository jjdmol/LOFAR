
//  WH_FillTFMatrix: WorkHolder class filling DH_TFMatrix
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
//  Revision 1.9  2002/11/12 14:24:20  schaaf
//
//  %[BugId: 11]%
//  ongoing development
//
//  Revision 1.8  2002/08/19 20:36:59  schaaf
//  %[BugId: 11]%
//  Layout
//  little performance enhancement
//
//  Revision 1.7  2002/06/07 11:37:41  schaaf
//  %[BugId: 11]%
//  Removed unnessesary Assert in C'tor
//  Improved performance of process() inner loop
//
//  Revision 1.6  2002/05/24 10:47:52  schaaf
//  %[BugId: 11]%
//  Removed ^M characters
//
//  Revision 1.5  2002/05/16 15:05:40  schaaf
//  Added profiler state for process() method
//
//  Revision 1.3  2002/05/07 14:59:16  schaaf
//  optimised performance of process()
//
//  Revision 1.2  2002/05/07 11:15:38  schaaf
//  minor
//
//  Revision 1.1.1.1  2002/05/06 11:49:20  schaaf
//  initial version
//
//
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>             // for sprintf
#include <math.h>
#include <unistd.h>

#include "BaseSim/Step.h"
#include "BaseSim/Profiler.h"
#include "Common/Debug.h"

#include "Transpose/WH_FillTFMatrix.h"

// Set static variables
int WH_FillTFMatrix::theirProcessProfilerState=0; 

WH_FillTFMatrix::WH_FillTFMatrix (const string& name, 
				  int sourceID,
				  unsigned int nin, 
				  unsigned int nout,
				  int timeDim,
				  int freqDim)
: WorkHolder    (nin, nout, name),
  itsInHolders  (0),
  itsOutHolders (0),
  itsTime       (0),
  itsTimeDim    (timeDim),
  itsFreqDim    (freqDim),
  itsSourceID   (sourceID)
{
  TRACER4("Enter WH_FillTFMatrix C'tor " << name);
  DbgAssertStr (nout > 0,    "0 output DH_IntArray is not possible");
  
  itsInHolders  = new DH_Empty* [nin];
  itsOutHolders = new DH_2DMatrix* [nout];
  char str[8];
  for (unsigned int i=0; i<nin; i++) {
    sprintf (str, "%d", i);
    itsInHolders[i] = new DH_Empty (std::string("in_") + str);
  }
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_2DMatrix (std::string("out_") + str,
					timeDim, std::string("Time"),
					freqDim, std::string("Frequency"),
					std::string("Station"));
  }
  if (theirProcessProfilerState == 0) {
    theirProcessProfilerState = Profiler::defineState("WH_FillTFMatrix",
						      "gray");

  }
  TRACER4("End of WH_FillMatrix C'tor " << getName());
}


WH_FillTFMatrix::~WH_FillTFMatrix()
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

WorkHolder* WH_FillTFMatrix::make(const string& name) const
{
  return new WH_FillTFMatrix(name, 
			     itsSourceID,
			     getInputs(), 
			     getOutputs(),
			     itsTimeDim,
			     itsFreqDim);
}

void WH_FillTFMatrix::preprocess() {
    
  return;
}


void WH_FillTFMatrix::process()
{  
  Profiler::enterState (theirProcessProfilerState);

  ///////////////////////// START OF PREPROCESS BLOCK ////////////////
  int cnt=0;
  int *Rowstartptr;
  DH_2DMatrix *DHptr;
  int Xsize,Ysize; 
  // the time step is the Xsize; 
  for (int outch=0; outch<getOutputs(); outch++) {
    DHptr = getOutHolder(outch);
    AssertStr(DHptr != 0, "GetOutHolder returned NULL");
    //    DHptr->setZ(itsSourceID);      
    //    DHptr->setYOffset(DHptr->getYSize()*outch);
    Xsize = DHptr->getXSize();
      for (int x=0; x < Xsize; x++) {
      Ysize = DHptr->getYSize();
      Rowstartptr = DHptr->getBuffer(x,0);
        for (int y=0; y < Ysize; y++) {
  	*(Rowstartptr+y) = 
      	  cnt++;
//    	// fill output buffer with random integer 0-99
//  	//(int)(100.0*rand()/RAND_MAX+1.0);
        }
    }
  }
  ///////////////////////// END OF PREPROCESS BLOCK ////////////////
  

  {
    int timestep;
    DH_2DMatrix *DHptr;
    
    itsTime += (timestep = getOutHolder(0)->getXSize()); // increase local clock
    // the time step is the Xsize; 
    for (int outch=0; outch<getOutputs(); outch++) {
      DbgAssertStr(timestep == getOutHolder(0)->getXSize(),
		   "All Output DataHolders must have the same time (X) dimension");
      DHptr = getOutHolder(outch);
      DHptr->setZ(itsSourceID);      
      DHptr->setYOffset(DHptr->getYSize()*outch);
      DHptr->setTimeStamp(itsTime);
      DHptr->setXOffset(itsTime);
    }
  }
  Profiler::leaveState (theirProcessProfilerState);
}


void WH_FillTFMatrix::dump() const
{
  cout << "WH_FillTFMatrix " << getName() << " ::dump()" << endl;
  for (int outch=0; outch<min(10,getOutputs()); outch++) {
    cout << "Output " << outch << "   "
	 << (const_cast<WH_FillTFMatrix*>(this))->getOutHolder(outch)->getZName() << " "
	 << (const_cast<WH_FillTFMatrix*>(this))->getOutHolder(outch)->getZ() << "   "
	 << (const_cast<WH_FillTFMatrix*>(this))->getOutHolder(outch)->getXName() << "Offset = "  
	 << (const_cast<WH_FillTFMatrix*>(this))->getOutHolder(outch)->getXOffset() << "    "
	 << (const_cast<WH_FillTFMatrix*>(this))->getOutHolder(outch)->getYName() << "Offset = "  
	 << (const_cast<WH_FillTFMatrix*>(this))->getOutHolder(outch)->getYOffset() ;
    for (int x=0; 
	 x < min(10,(const_cast<WH_FillTFMatrix*>(this))->getOutHolder(outch)->getXSize());
		 x++) {
	   cout << endl 
		<< (const_cast<WH_FillTFMatrix*>(this))->getOutHolder(outch)->getXName()
		<< x << "   ";
      for (int y=0; 
	   y < min(10,(const_cast<WH_FillTFMatrix*>(this))->getOutHolder(outch)->getYSize());
	   y++) {
	cout << *(const_cast<WH_FillTFMatrix*>(this))->getOutHolder(outch)->getBuffer(x,y) << " ";
      }
    }
    cout << endl;
  }
  cout << "=====================================" <<endl;
}

DH_Empty* WH_FillTFMatrix::getInHolder (int channel)
{
  AssertStr (channel >= 0,          "input channel too low");
  AssertStr (channel < getInputs(), "input channel too high");
  AssertStr (itsInHolders[channel] != 0, "InHolder does not exist");
  return itsInHolders[channel];
}
DH_2DMatrix* WH_FillTFMatrix::getOutHolder (int channel)
{
  AssertStr (channel >= 0,           "output channel too low");
  AssertStr (channel < getOutputs(), 
	     "output channel too high; name = " << getName() 
	     << " channel = " << channel);
  AssertStr (itsOutHolders[channel] != NULL, "OutHolder does not exist");
  return itsOutHolders[channel];
}
 
