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
//
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>             // for sprintf
#include <math.h>

#include "Step.h"
#include "Debug.h"

#include "WH_FillTFMatrix.h"
#include "StopWatch.h"

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
  itsSourceID   (sourceID),
  itsTimeDim    (timeDim),
  itsFreqDim    (freqDim)
{
  TRACER4("Enter WH_FillTFMatrix C'tor " << name);
  AssertStr (nin > 0,     "0 input DH_IntArray is not possible");
  AssertStr (nout > 0,    "0 output DH_IntArray is not possible");
  //   AssertStr (nout == nin, "number of inputs and outputs must match");
  
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
  {
    int timestep;
    itsTime += (timestep = getOutHolder(0)->getXSize()); // increase the local clock
    // the time step is the Xsize; 
    for (int outch=0; outch<getOutputs(); outch++) {
      DbgAssertStr(timestep == getOutHolder(0)->getXSize(),
		   "All Output DataHolders must have the same time (X) dimension");
      getOutHolder(outch)->setTimeStamp(itsTime);
      getOutHolder(outch)->setXOffset(itsTime);
      getOutHolder(outch)->setZ(itsSourceID);      
      getOutHolder(outch)->setYOffset(getOutHolder(outch)->getYSize()*outch);
      for (int x=0; x < getOutHolder(outch)->getXSize(); x++) { 
	for (int y=0; y < getOutHolder(outch)->getYSize(); y++) {
	  // fill autput buffer with random integer 0-99
	  int* iptr = getOutHolder(outch)->getBuffer(x,y);
	  *(getOutHolder(outch)->getBuffer(x,y)) = 
	    (int)(100.0*rand()/RAND_MAX+1.0);
	}
      }
    }
  }
  dump();
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
 
