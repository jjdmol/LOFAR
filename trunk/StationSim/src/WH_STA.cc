//#  WH_STA.cc:
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
//#

#include <stdio.h>             // for sprintf
#include <blitz/blitz.h>

#include <StationSim/WH_STA.h>
#include <StationSim/MDL.h>
#include <BaseSim/ParamBlock.h>
#include <Common/Debug.h>
#include <Common/Lorrays.h>
#include <Math/LCSMath.h>
#include <blitz/blitz.h>
#include <StationSim/PASTd.h>


WH_STA::WH_STA (const string& name,  int nin,  int nout,
				int nant,  int maxnrfi,  int buflength, int alg)
: WorkHolder      (nin, nout, name, "WH_STA"),
  itsInHolders    (0),
  itsOutHolders   (0),
  itsNumberOfRFIs ("out_mdl", 1, 1),
  itsNrcu         (nant),
  itsMaxRFI       (maxnrfi),
  itsBufLength    (buflength),
  itsBuffer       (nant, buflength),
  itsPos          (0),
  itsEvectors     (nant,nant),
  itsEvalues      (nant),
  itsAcm          (nant, nant),
  itsAlg          (alg),
  itsPASTdInterval(50),
  itsPASTdBeta    (0.95)  
{
  char str[8];
  if (nant > 0) {
    itsInHolders = new DH_SampleC* [nant];
  }
  for (int i = 0; i < nant; i++) {
    sprintf (str, "%d",i);
    itsInHolders[i] = new DH_SampleC (string("in_") + str, 1, 1);
  } 
  if (nout - 1 > 0) {
    itsOutHolders = new DH_SampleC* [nout - 1];
  }
  for (int i = 0; i < nout - 1; i++) {
    sprintf (str, "%d",i);
    // Define a space large enough to contain the max number of 
    // RFI sources. This should be implemented more elegantly.
    itsOutHolders[i] = new DH_SampleC (string("out_") + str, itsNrcu, itsMaxRFI);    
  }
//   //DEBUG
//   itsTestVector.resize(itsNrcu, 300);
//   itsFileInput.open ("/home/alex/temp/test_vectorSTA_sin.txt");
//   itsFileInput >> itsTestVector;
//   itsCount = 0;
}

WH_STA::~WH_STA()
{  
  for (int i = 0; i < getInputs(); i++) {
    delete itsInHolders[i];
  }
  delete [] itsInHolders;
  
 for (int i = 0; i < getOutputs() - 1; i++) {
  delete itsOutHolders[i];
 }
 delete [] itsOutHolders;

//  // DEBUG
//  itsFileInput.close ();
}

WH_STA* WH_STA::make (const string& name) const
{
  return new WH_STA (name, getInputs(), getOutputs(), itsNrcu, itsMaxRFI, itsBufLength, itsAlg);
}

void WH_STA::preprocess()
{

}


void WH_STA::process()
{
  using namespace blitz;

  // Place the next incoming sample vector in the snapshot buffer 
  // Keep a cylic buffer for the input snapshots
  for (int i = 0; i < itsNrcu; i++) {
 	itsBuffer(i, itsPos) = itsInHolders[i]->getBuffer()[0];
// 	// DEBUG
// 	itsBuffer(i, itsPos) = itsTestVector(i, itsCount);
  }
  itsPos = (itsPos + 1) % itsBuffer.cols();
//   // DEBUG
//   itsCount = (itsCount + 1) % itsTestVector.cols();

  if (getOutHolder(0)->doHandle ()) {
	// Create contigeous buffer
	itsBuffer = CreateContigeousBuffer (itsBuffer, itsPos);
	itsPos = 0; // Set cyclic buffer pointer to the first entry because it is ordered now
	
	switch (itsAlg) {
	case 0 : // ACM and EVD
	  {
		itsAcm = LCSMath::acm (itsBuffer);		              // calculate the ACM
		LCSMath::eig (itsAcm, itsEvectors, itsEvalues);       // using the ACM, calc eigen vect/ values
		itsEvectors = ReverseMatrix (itsEvectors, firstDim);
		itsEvalues = ReverseVector (itsEvalues);
		break;
	  }
	case 1 : // SVD
	  {
		LoMat_dcomplex dummy (itsBuffer.extent(blitz::secondDim), itsBuffer.extent(blitz::secondDim));
 		LCSMath::svd (itsBuffer, itsEvectors, dummy, itsEvalues);
		//itsEvalues = sqr(itsEvalues) - 1;
		break;
	  }
	case 2 : // PASTd
	  {
		// Do an SVD or EVD to initialize PASTd
		itsAcm = LCSMath::acm (itsBuffer);		              // calculate the ACM
		LCSMath::eig (itsAcm, itsEvectors, itsEvalues);       // using the ACM, calc eigen vect/ values
		itsEvectors = ReverseMatrix (itsEvectors, firstDim);
		itsEvalues = ReverseVector (itsEvalues);
		break;
	  }
	default :
	  itsAlg = 0;
	  break;
	} 

	// Determine the number of sources in the signal
	double RFI = (double) mdl (itsEvalues, itsNrcu, itsBufLength);
	if (RFI > itsMaxRFI) RFI = itsMaxRFI;

	// Find the appropriate Eigen vectors
	LoMat_dcomplex B (itsNrcu, RFI);
	B = itsEvectors (Range::all (), Range(itsEvectors.lbound(firstDim), RFI - 1));

	// Now assign the Eigen vectors to the output
	for (int i = 0; i < getOutputs() - 1; i++) {
	  memcpy(itsOutHolders[i]->getBuffer(), B.data(), 
			 itsNrcu * (int)RFI * sizeof(DH_SampleC::BufferType));
	}
	memcpy(itsNumberOfRFIs.getBuffer(), &RFI, sizeof(DH_SampleR::BufferType));
  } else if (itsAlg == 2) {
	// do PASTd step
	//pastd (itsBuffer, itsBufLength, itsPASTdInterval, itsPASTdBeta, itsEvalues, itsEvectors);
  }
}

void WH_STA::dump() const
{
}

DH_SampleC* WH_STA::getInHolder (int channel)
{
  AssertStr (channel < getInputs(), "Input channel too high");
  return itsInHolders[channel];
}

DataHolder* WH_STA::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs(), "Output channel too high"); 
  if (channel < getOutputs() - 1)
	return itsOutHolders[channel];
  else
	return &itsNumberOfRFIs;  
}

LoMat_dcomplex WH_STA::CreateContigeousBuffer (const LoMat_dcomplex& aBuffer, int pos)
{
  using namespace blitz;
  AssertStr (pos <= aBuffer.ubound(secondDim), "Can't create contigeous buffer, pos too high!");
  Range all = Range::all();
  LoMat_dcomplex output (aBuffer.shape());

  int p = 0;
  for (int i = pos - 1; i >= 0; i--) {
	output (all, p++) = aBuffer(all, i);
  }
  for (int i = aBuffer.ubound(secondDim); i >= pos; i--) {
	output (all, p++) = aBuffer(all, i);
  }
  return output;
}

LoMat_dcomplex WH_STA::TransposeMatrix (const LoMat_dcomplex& aMat)
{
  LoMat_dcomplex invMat (aMat.cols (), aMat.rows ());
  for (int i = aMat.lbound (blitz::firstDim); i < aMat.rows (); i++) {
	for (int j = aMat.lbound (blitz::secondDim); j < aMat.cols (); j++) {
	  invMat (j, i) = aMat (i, j);
	}
  }
  return invMat;
}

LoVec_double WH_STA::ReverseVector (const LoVec_double& aVec)
{
  LoVec_double revVec (aVec.size ());
  int j = 0;
  for (int i = aVec.size () - 1; i >= aVec.lbound (blitz::firstDim); i--) {
	revVec (i) = aVec (j++);
  }
  return revVec;
}

LoMat_dcomplex WH_STA::ReverseMatrix (const LoMat_dcomplex& aMat, int dim)
{
  LoMat_dcomplex revMat (aMat.shape ());
  if (dim) {
	for (int i = aMat.lbound (blitz::firstDim); i < aMat.rows (); i++) {
	  for (int j = aMat.lbound (blitz::secondDim); j < aMat.cols (); j++) {
		revMat (i, j) = aMat (i, aMat.cols () - j - 1);
	  }
	}
  } else {
	for (int i = aMat.lbound (blitz::firstDim); i < aMat.rows (); i++) {
	  for (int j = aMat.lbound (blitz::secondDim); j < aMat.cols (); j++) {
		revMat (i, j) = aMat (aMat.rows () - i - 1, j);
	  }
	}
  }
  return revMat;
}
