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
: WorkHolder       (nin, nout, name, "WH_Projection"),
  itsInHolders     (0),
  itsOutHolder     (0),
  itsNumberOfRFIs  (0),
  itsNrcu          (nant),
  itsMaxRFI        (maxnrfi),
  itsWeight        ()
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
  itsNumberOfRFIs = new DH_SampleR("in-mdl", 1, 1);
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
  if (getOutputs() > 0) {
	// Get the number of detected RFI sources from the STA comp. (internally calc. by MDL)
	int detectedRFIs = (int)*itsNumberOfRFIs->getBuffer();
	
	// Read in the eigenvectors from the STA component. They are the detected rfi sources and
	// should be nulled.
	//  LoMat_dcomplex V (itsInHolders[0]->getBuffer(), shape(itsNrcu, itsMaxRFI), duplicateData);
	LoMat_dcomplex V (itsInHolders[0]->getBuffer(), shape(itsNrcu, detectedRFIs), duplicateData);
	
	// Get the steering vector form the weight determination comp.
	LoVec_dcomplex a (itsInHolders[1]->getBuffer(), itsNrcu, duplicateData);
	
	itsWeight.resize(itsNrcu);
	itsWeight = getWeights(V, a);
  


//   // DEBUG matrix inverse
//   int N = 3;
//   LoMat_dcomplex A(N,N);
//   LoMat_dcomplex Orig(N,N);
//   dcomplex d;

//   Orig = 1,2,3,
// 	     4,5,6,
//          7,8,9;
//   Orig(0,0)=dcomplex(0,1);

//   cout << Orig << endl;
//   A = LCSMath::invert(Orig);
//   cout << A << endl << d << endl;

//   cout << LCSMath::matMult(A, Orig) << endl;

	memcpy(itsOutHolder->getBuffer(), itsWeight.data(), itsNrcu * sizeof(DH_SampleC::BufferType));
  }
}

void WH_Projection::dump() const
{

}


DataHolder* WH_Projection::getInHolder (int channel)
{
  AssertStr (channel < getInputs(), "Input channel too high");  
  if (channel < 2)
	return itsInHolders[channel];
  else
	return itsNumberOfRFIs;
}

DH_SampleC* WH_Projection::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs(), "Output channel too high");  
  return itsOutHolder;
}


LoVec_dcomplex WH_Projection::getWeights (LoVec_dcomplex V, LoVec_dcomplex a) 
{
  AssertStr(V.size() == a.size(), "Error. WH_Projection::getWeights() encountered non equal size arrays");
  LoVec_dcomplex w (V.size());
  LoMat_dcomplex Pv (V.size(), V.size());
  w = 1;
  LoMat_dcomplex I = LCSMath::diag(w);        // Create Identity matrix
  LoVec_dcomplex temp (V.size());

  dcomplex VHV = 1 / sum((2 * real(V) - V) * V);
  temp = VHV * (2 * real(V) - V);
  Pv = LCSMath::matMult(V, temp);
  Pv = I - Pv;

  w = mv_mult(Pv, a); // w = Pv a;

  return w;
}

LoVec_dcomplex WH_Projection::getWeights (LoMat_dcomplex V, LoVec_dcomplex a) 
{
  AssertStr(V.rows() == a.size(), "Error. WH_Projection::getWeights() encountered non equal size arrays");

  LoVec_dcomplex w (V.rows());
  w = 1;
  LoMat_dcomplex Pv (V.shape());  
  LoMat_dcomplex I = LCSMath::diag(w);        // Create Identity matrix

  Pv = LCSMath::invert(LCSMath::matMult(LCSMath::hermitianTranspose(V), V)); // (VHV)^-1
  Pv = LCSMath::matMult(V, LCSMath::matMult(Pv, LCSMath::hermitianTranspose(V))); // VPvVH
  Pv = I - Pv;

  w = mv_mult(Pv, a); // w = Pv a;
  
  return w;
}

LoVec_dcomplex WH_Projection::mv_mult (LoMat_dcomplex A, LoVec_dcomplex B) 
{
  LoVec_dcomplex tmp(A.rows());
  for (int i=0; i < A.rows(); i++) {
    dcomplex res = 0;
    for (int j=0; j < A.cols(); j++) {
      res = res + A(i,j)* B(j);
      
    }
    tmp(i)=res;
  }
  return tmp;
}

LoVec_dcomplex WH_Projection::vm_mult (const LoVec_dcomplex& A, const LoMat_dcomplex& B) 
{
  AssertStr(A.size() == B.rows(), "Vector size and matrix columns aren't the same size!");
  LoVec_dcomplex out (A.size());

  for (int i = 0; i < B.cols(); i++) {
    dcomplex res = 0;
    for (int j = 0; j < B.rows(); j++) {
      res += B(j,i) *  A(j);     
    }
    out(i) = res;
  }
  return out;
}
