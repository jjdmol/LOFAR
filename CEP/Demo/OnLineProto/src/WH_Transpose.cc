//  WH_Transpose.cc:
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
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>             // for sprintf
#include <Common/Debug.h>

// CEPFrame general includes
#include "CEPFrame/DH_Empty.h"
#include "CEPFrame/Step.h"
#include <Common/KeyValueMap.h>

// OnLineProto specific include
#include "OnLineProto/WH_Transpose.h"
#include "OnLineProto/DH_Beamlet.h"
#include "OnLineProto/DH_CorrCube.h"

#define min(a,b) (a<b?a:b)
namespace LOFAR
{

WH_Transpose::WH_Transpose (const string& name,
			    unsigned int nin,
			    unsigned int nout,
			    const int FBW)
  : WorkHolder    (nin, nout, name,"WH_Transpose"),
    itsFBW(FBW)
{
  char str[8];
  // create the input dataholders
  for (unsigned int i=0; i<nin; i++) {
    sprintf (str, "%d", i);
    getDataManager().addInDataHolder(i, 
				     new DH_Beamlet (string("out_") + str, itsFBW), 
				     true);
  }
  // create the output dataholders
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    getDataManager().addOutDataHolder(i, 
				      new DH_CorrCube (string("out_") + str), 
				      true);
  }
}

WH_Transpose::~WH_Transpose()
{
}

WorkHolder* WH_Transpose::construct (const string& name, 
				     unsigned int nin,
				     unsigned int nout,
				     const int FBW)
{
  return new WH_Transpose (name, nin, nout, FBW);
}

WH_Transpose* WH_Transpose::make (const string& name)
{
  return new WH_Transpose (name, 
			   getDataManager().getInputs(), 
			   getDataManager().getOutputs(),
			   itsFBW);
}

void WH_Transpose::process()
{
  TRACER4("WH_Transpose::Process()");
  // Transpose receives one beamlet per station
  //DbgAssertStr(getDataManager().getInputs() == NSTATIONS,"Nr inputs");
  DbgAssertStr(getDataManager().getOutputs() == NVis,"Nr outputs");
  
  DbgAssertStr(NVis == BFBW,"Assume one freq channel per corrcube");

  // copy all data in the input beamlets into 
  // the CorrCube plane corresponding to the timestamp in the beamlets.

  // firts get the timestamp of the first input channel;
  // all other channels will be checked against this timestamp later.
  
  int OutChannel;
  int OutStation;
  int OutFreqBin;
  int OutTime = 1;//(int)((DH_Beamlet*)getDataManager().getInHolder(0))->getElapsedTime();

  // loop over all input beamlets and channels therein
  for (int InChannel=0; InChannel< getDataManager().getInputs(); InChannel++) {
    for (int InFreqBin=0; InFreqBin<BFBW; InFreqBin++) {
      // ToDo: calculate the correct output channel and freq channels therein
      // 
      OutChannel = InFreqBin;
      OutFreqBin = 0; // single frequency assumed
      OutStation=InChannel; // channel nr == station nr
     
      // now copy the data
      *((DH_CorrCube*)getDataManager().getOutHolder(OutChannel))
	->getBufferElement(OutStation, OutTime, OutFreqBin) 
	= 
	*((DH_Beamlet*)getDataManager().getInHolder(InChannel))
	->getBufferElement(InFreqBin) ;
    } 
  }

  
}

void WH_Transpose::dump()
{
  cout << "--------------------------------------------------------" << endl;
  cout << "Dump WH_Transpose " << getName() << endl;
  cout << " input " << endl;
  for (int s=0; s<10; s++) {
    cout << "in station=" << s << ":  ";
    for (int f=0; f<10; f++) {
      cout << 	*((DH_Beamlet*)getDataManager().getInHolder(0))
	->getBufferElement(f*BFBW+s) << "  ";
    }
    cout << endl;
  }

  cout << " output " << endl;
  for (int s=0; s<10; s++) {
    cout << "out station=" << s << ":  ";
    for (int t=0; t<10; t++) {
      cout << 	*((DH_Beamlet*)getDataManager().getOutHolder(0))
	->getBufferElement(t*NSTATIONS+s) << "  ";
    }
    cout << endl;
  }
  cout << "--------------------------------------------------------" << endl;
}

}// namespace LOFAR
