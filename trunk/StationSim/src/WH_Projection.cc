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
  itsOutHolders    (0),
  itsNumberOfRFIs  ("in_mdl", 1, 1),
  itsRFISources    ("in_rfi", nant, maxnrfi),
  itsNrcu          (nant),
  itsMaxRFI        (maxnrfi),
  itsWeight        (itsNrcu)
{
  char str[8];
  // nin is the number of steering vectors that will be used as an input (at least one
  // for the weight determination and possibly more for deterministic nulls).
  if (nin - 2 > 0) {
    itsInHolders = new DH_SampleC* [nin - 2];
  }
  for (unsigned int i = 0; i < nin - 2; i++) { // The weight vector from the Weight Determination
    sprintf (str, "%d",i);
    itsInHolders[i]= new DH_SampleC(string("in_") + str, itsNrcu, 1);
  }
  if (nout > 0) {
    itsOutHolders = new DH_SampleC* [nout];
  }
  for (unsigned int i = 0; i < nout; i++) {  // The resulting Weight vector
    sprintf (str, "%d",i);                      
    itsOutHolders[i] = new DH_SampleC (string("out_") + str, itsNrcu, 1);
  }
}

WH_Projection::~WH_Projection()
{
  for (int i = 0; i < getInputs() - 2; i++) {
    delete itsInHolders[i];
  }
  delete [] itsInHolders;
  
 for (int i = 0; i < getOutputs(); i++) {
  delete itsOutHolders[i];
 }
 delete [] itsOutHolders;
}

WH_Projection* WH_Projection::make (const string& name) const
{
  return new WH_Projection (name, getInputs(), getOutputs(), itsNrcu, itsMaxRFI);
}

void WH_Projection::process()
{
  if (getOutputs() > 0) {
	// Get the number of detected RFI sources from the STA comp. (internally calc. by MDL)
	int detectedRFIs = (int)(itsNumberOfRFIs.getBuffer()[0]);
	detectedRFIs = 2;

	// Read in the eigenvectors from the STA component. They are the detected rfi sources and
	// should be nulled.
	//  LoMat_dcomplex V (itsInHolders[0]->getBuffer(), shape(itsNrcu, itsMaxRFI), duplicateData);
	LoMat_dcomplex V (itsRFISources.getBuffer(), shape(itsNrcu, detectedRFIs), duplicateData);
	
	// Get the steering vector form the weight determination comp. assume that only one will be put in.
	LoVec_dcomplex a (itsInHolders[0]->getBuffer(), itsNrcu, duplicateData);

	// Calculate the weights using the eigenvectors and the steering vector
	itsWeight = getWeights(V, a);
  
	// DEBUG matrix inverse 
// 	int N = 3; 
// 	LoMat_dcomplex Orig(N,N); 
// 	LoMat_dcomplex A(N, N);
// 	LoVec_dcomplex Vec(N);

// 	Vec = 1, 1, 1;
// 	Orig = 1,2,3,
// 	  4,5,6,
// 	  7,8,9;
// 	Orig(0,0)=dcomplex(0,1);

// 	cout << Orig << endl;
// 	A = LCSMath::invert(Orig);
// 	cout << A << endl;
	
// 	cout << LCSMath::matMult(A, Orig) << endl;

// 	cout << Orig << endl << Vec << endl << vm_mult(Vec,Orig) << endl;
	// END DEBUG matrix inverse

	// Copy output to the next step
	for (int i = 0; i < getOutputs(); i++) {
	  memcpy(itsOutHolders[i]->getBuffer(), itsWeight.data(), itsNrcu * sizeof(DH_SampleC::BufferType));
	}
  }
}

void WH_Projection::dump() const
{
}

DataHolder* WH_Projection::getInHolder (int channel)
{
  AssertStr (channel < getInputs(), "Input channel too high");  
  if (channel < getInputs() - 2) {
	return itsInHolders[channel];
  } else if (channel == getInputs() - 2) {
	return &itsNumberOfRFIs;
  } else {
	return &itsRFISources;
  }
}

DH_SampleC* WH_Projection::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs(), "Output channel too high");  
  return itsOutHolders[channel];
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

  //cout << w << endl << Pv << endl << I << endl << V << endl << a << endl;

  LoMat_dcomplex temp = LCSMath::matMult(LCSMath::hermitianTranspose(V), V);
  //  cout << temp << endl;
  Pv = LCSMath::invert(temp); // (VHV)^-1
  //  cout << Pv << endl;
  Pv = LCSMath::matMult(V, LCSMath::matMult(Pv, LCSMath::hermitianTranspose(V))); // VPvVH
  //  cout << Pv << endl;
  Pv = I - Pv;
  //  cout << Pv << endl;

  w = mv_mult(Pv, a); // w = Pv a;
  //  cout << w << endl;  

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
