//  WH_FillTFMatrix.cc: WorkHolder class filling DH_TFMatrix
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
///////////////////////////////////////////////////////////////////////////

#include <stdio.h>             // for sprintf
#include <math.h>
#include <unistd.h>

#include "CEPFrame/Step.h"
#include "CEPFrame/Profiler.h"
#include "Common/Debug.h"

#include "Transpose/WH_FillTFMatrix.h"

using namespace LOFAR;

// Set static variables
int WH_FillTFMatrix::theirProcessProfilerState=0; 

WH_FillTFMatrix::WH_FillTFMatrix (const string& name, 
				  int sourceID,
				  unsigned int nin, 
				  unsigned int nout,
				  int timeDim,
				  int freqDim)
: WorkHolder    (nin, nout, name),
  itsTime       (0),
  itsTimeDim    (timeDim),
  itsFreqDim    (freqDim),
  itsSourceID   (sourceID)
{
  TRACER4("Enter WH_FillTFMatrix C'tor " << name);
  DbgAssertStr (nout > 0,    "0 output DH_IntArray is not possible");
  
  char str[8];
  for (unsigned int i=0; i<nin; i++) {
    sprintf (str, "%d", i);
    getDataManager().addInDataHolder(i, new DH_Empty (std::string("in_") + str));
  }
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    getDataManager().addOutDataHolder(i, new DH_2DMatrix (std::string("out_") + str,
					timeDim, std::string("Time"),
					freqDim, std::string("Frequency"),
							  std::string("Station")));
  }
  if (theirProcessProfilerState == 0) {
    theirProcessProfilerState = Profiler::defineState("WH_FillTFMatrix",
						      "gray");

  }
  TRACER4("End of WH_FillMatrix C'tor " << getName());
}


WH_FillTFMatrix::~WH_FillTFMatrix()
{
}

WorkHolder* WH_FillTFMatrix::make(const string& name)
{
  return new WH_FillTFMatrix(name, 
			     itsSourceID,
			     getDataManager().getInputs(), 
			     getDataManager().getOutputs(),
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
  for (int outch=0; outch<getDataManager().getOutputs(); outch++) {
    DHptr = (DH_2DMatrix*)getDataManager().getOutHolder(outch);
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
    
    itsTime += (timestep = ((DH_2DMatrix*)getDataManager().getOutHolder(0))->getXSize()); // increase local clock
    // the time step is the Xsize; 
    for (int outch=0; outch<getDataManager().getOutputs(); outch++) {
      DbgAssertStr(timestep == ((DH_2DMatrix*)getDataManager().getOutHolder(0))->getXSize(),
		   "All Output DataHolders must have the same time (X) dimension");
      DHptr = (DH_2DMatrix*)getDataManager().getOutHolder(outch);
      DHptr->setZ(itsSourceID);      
      DHptr->setYOffset(DHptr->getYSize()*outch);
      DHptr->setTimeStamp(itsTime);
      DHptr->setXOffset(itsTime);
    }
  }
  Profiler::leaveState (theirProcessProfilerState);
}


void WH_FillTFMatrix::dump()
{
  cout << "WH_FillTFMatrix " << getName() << " ::dump()" << endl;
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

 
