//#  WH_Projection.cc:
//#
//#  Copyright (C) 2002
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  Chris Broekema, january 2003
//#
//#  $Id$
//#

#include <stdio.h>             // for sprintf
#include <blitz/blitz.h>

#include <StationSim/WH_Projection.h>
#include <BaseSim/ParamBlock.h>
#include <Common/Debug.h>
#include <Common/Lorrays.h>
#include <Math/LCSMath.h>
#include <blitz/blitz.h>

using namespace blitz;

WH_Projection::WH_Projection (const string& name, unsigned int nin, unsigned int nout,
			      unsigned int nant, unsigned int maxnrfi)
: WorkHolder    (nin, nout, name, "WH_Projection"),
  itsInHolders  (0),
  itsOutHolder  (0),
  itsNrcu       (nant),
  itsMaxRFI     (maxnrfi),
  itsWeight     ()
{
  if (nin > 0) {
    itsInHolders = new DH_SampleC* [nin];
  }
  for (unsigned int i=0; i < nin; i++) {
    // Define a space large enough to contain the max number of 
    // RFI sources. This should be implemented more elegantly.
    itsInHolders[0]= new DH_SampleC("in-sta", itsNrcu, itsMaxRFI);
    // The weight vector from the Weight Determination
    itsInHolders[1]= new DH_SampleC("in-wd", itsNrcu, 1);
  }
  
  if (nout > 0) {
    // The resulting Weight vector  
    itsOutHolder = new DH_SampleC("out", itsNrcu, 1);
  }
}

WH_Projection::~WH_Projection()
{
}

WH_Projection* WH_Projection::make (const string& name) const
{
  return new WH_Projection (name, getInputs(), getOutputs(), itsNrcu, itsMaxRFI);
}

void WH_Projection::preprocess()
{
}


void WH_Projection::process()
{
  LoVec_dcomplex steerv (itsInHolders[1]->getBuffer(), itsNrcu, duplicateData);
  itsWeight.resize(itsNrcu);

  itsWeight = steerv;

  memcpy(itsOutHolder->getBuffer(), itsWeight.data(), itsNrcu * sizeof(DH_SampleC::BufferType));
}

void WH_Projection::dump() const
{

}


DH_SampleC* WH_Projection::getInHolder (int channel)
{
  AssertStr (channel < getInputs(), "Input channel too high");  

  return itsInHolders[channel];
}

DH_SampleC* WH_Projection::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs(), "Output channel too high");  
  return itsOutHolder;
}


LoVec_dcomplex WH_Projection::getWeights (LoVec_dcomplex B, LoVec_dcomplex d) {
  LoVec_dcomplex w (B.size());
  
  if (B.size() != d.size()) {
    cout << "Error. WH_Projection::getWeights() encountered non equal size arrays" << endl;
  } else {
    LoVec_dcomplex temp(B.size());
    
	//    temp = LCSMath::matMult(B, 1/LCSMath::matMultReduce(B, B)) ;
	//    w = d - LCSMath::matMult( LCSMath::matMult( B, temp ), d );
  }
  return w;
}

LoVec_dcomplex WH_Projection::getWeights (LoMat_dcomplex B, LoVec_dcomplex d) {
  LoVec_dcomplex w (B.rows());
  
  if (B.rows() != d.size()) {
    cout << "Error. WH_Projection::getWeights() encountered non equal size arrays" << endl;
  } else {
    LoMat_dcomplex temp(B.cols(),B.rows());

//     temp = LCSMath::matMult(pow(LCSMath::matMult(B.transpose(firstDim, secondDim), B), -1), 
// 				    B.transpose(firstDim,secondDim)) ;

    //    w = d - LCSMath::matMult( LCSMath::matMult( B, temp ), d );
  }
  return w;
}
