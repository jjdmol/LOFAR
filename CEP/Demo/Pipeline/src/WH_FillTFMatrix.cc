
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
///
//////////////////////////////////////////////////////////////////////

#include <stdio.h>             // for sprintf
#include <math.h>
#include <unistd.h>
#include <stdlib.h>

//#include "CEPFrame/Step.h"
#include "CEPFrame/Profiler.h"
#include "Common/Debug.h"

#include "Pipeline/WH_FillTFMatrix.h"

// Set static variables
int WH_FillTFMatrix::theirProcessProfilerState=0; 

WH_FillTFMatrix::WH_FillTFMatrix (const string& name, 
				  int sourceID,
				  unsigned int nin, 
				  unsigned int nout,
				  int timeDim,
				  int freqDim,
				  int pols)
: WorkHolder    (nin, nout, name),
  itsTime       (0),
  itsTimeDim    (timeDim),
  itsFreqDim    (freqDim),
  itsSourceID   (sourceID),
  itsPols       (pols)
{
  TRACER4("Enter WH_FillTFMatrix C'tor " << name);
  DbgAssertStr (nout > 0,    "0 output DH_IntArray is not possible");
  
  char str[8];
  for (unsigned int i=0; i<nin; i++) {
    sprintf (str, "%d", i);
    getDataManager().addInDataHolder(i,
				     new DH_Empty (std::string("in_") + str));
  }

  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);

    getDataManager().addOutDataHolder(i,
				      new DH_2DMatrix (std::string("out_") + str,
				      timeDim, std::string("Time"),
				      freqDim, std::string("Frequency"),
				      std::string("Station"),
				      2), true);
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
			     itsFreqDim,
			     itsPols);
}

void WH_FillTFMatrix::preprocess() {
  // initialise the random number generation
  srandom(itsSourceID*random());
    
  return;
}


void WH_FillTFMatrix::process()
{  
  Profiler::enterState (theirProcessProfilerState);

  int cnt_real=0;
  int cnt_imag=0;
  DH_2DMatrix *DHptr;
  int Xsize,Ysize; 
  // the time step is the Xsize; 
  int timestep;
  DbgAssertStr(getDataManager().getOutHolder(0)->getType() == "DH_2DMatrix",
	       "DataHolder is not of type DH_2DMatrix");    
  timestep = ((DH_2DMatrix*)getDataManager().getOutHolder(0))->getXSize();
  itsTime += timestep; // increase local clock
  
  // initialise the counters
  //cnt_real=(int)(127.0*random()/RAND_MAX+1.0);
  //cnt_imag=(int)(127.0*random()/RAND_MAX+1.0);;
  
  for (int outch=0; outch<getDataManager().getOutputs(); outch++) {
    DbgAssertStr(getDataManager().getOutHolder(outch)->getType() == "DH_2DMatrix",
	         "DataHolder is not of type DH_2DMatrix");
    DHptr = (DH_2DMatrix*)getDataManager().getOutHolder(outch);
    DbgAssertStr(DHptr != 0, "GetOutHolder returned NULL");
    //    DHptr->setZ(itsSourceID);      
    //    DHptr->setYOffset(DHptr->getYSize()*outch);
    Xsize = DHptr->getXSize();
      for (int x=0; x < Xsize; x++) {
      Ysize = DHptr->getYSize();
      for (int y=0; y < Ysize; y++) {
	for (int pol=0; pol<itsPols; pol++) {
	  // *DHptr->getBuffer(x,y,pol) = DH_2DMatrix::DataType(cnt_real++,cnt_imag++);
	  // *DHptr->getBuffer(x,y,pol) = DH_2DMatrix::DataType(cnt_real++ + itsSourceID +pol, 0);
	  *DHptr->getBuffer(x,y,pol) = DH_2DMatrix::DataType(itsTime + itsSourceID +2*pol, 0);
	}
      }
      
      DbgAssertStr(timestep == ((DH_2DMatrix*)getDataManager().getOutHolder(0))->getXSize(),
		   "All Output DataHolders must have the same time (X) dimension");
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
  cout << "WH_FillTFMatrix " << getName() << " ::dump() " << itsSourceID<< endl;
  
  for (int outch=0; outch<std::min(10, getDataManager().getOutputs()); outch++) {

    DbgAssertStr(getDataManager().getOutHolder(outch)->getType() == "DH_2DMatrix",
	         "DataHolder is not of type DH_2DMatrix");
    DH_2DMatrix* dhOut = (DH_2DMatrix*)getDataManager().getOutHolder(outch);
    cout << "Output " << outch << "   "
	 << dhOut->getZName() << " "
	 << dhOut->getZ() << "   "
	 << dhOut->getXName() << "Offset = "  
	 << dhOut->getXOffset() << "    "
	 << dhOut->getYName() << "Offset = "  
	 << dhOut->getYOffset() ;
    for (int pol=0; pol < itsPols; pol++) {
      cout << endl << "Polarisation: " << pol;
      for (int x=0; 
	   x < std::min(10, dhOut->getXSize());
	   x++) {
	cout << endl 
	     << dhOut->getXName()
	     << x << "   ";
	for (int y=0; 
	     y < std::min(10, dhOut->getYSize());
	     y++) {
	  cout << *dhOut->getBuffer(x,y,pol) << " ";
	}
      }
    }
    cout << endl;
  }
  cout << "=====================================" <<endl;

}

 
