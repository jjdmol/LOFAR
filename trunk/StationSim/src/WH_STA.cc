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

#ifndef STATIONSIM_PASTD_H
#include <StationSim/PASTd.h>
#endif

WH_STA::WH_STA (const string& name, int nout, int nrcu, int nsubband, 
				int maxnrfi, int buflength, string alg,int pdinit, int pdupdate, 
				int pdinterval, double pdbeta, const int detrfi)
: WorkHolder      (1, nout, name, "WH_STA"),
  itsInHolder     ("in", nrcu, nsubband),
  itsOutHolders   (0),
  itsNumberOfRFIs (string("out_mdl"), nsubband, 1),
  itsNrcu         (nrcu),
  itsNsubband     (nsubband),
  itsMaxRFI       (maxnrfi),
  itsBufLength    (buflength),
  itsBuffer       (nrcu, nsubband, buflength),
  itsPos          (0),
  itsEvectors     (nrcu, nrcu, nsubband),
  itsEvalues      (nrcu, nsubband),
  itsAcm          (nrcu, nrcu), 
  itsAlg          (alg),
  itsPASTdInterval(pdinterval),
  itsPASTdBeta    (pdbeta),
  itsPASTdInit    (pdinit),
  itsUpdateRate   (pdupdate),
  itsCount        (0),
  itsDetRFI       (detrfi)
{
  char str[8];
  if (nout - 1 > 0) {
    itsOutHolders = new DH_CubeC* [nout];
  }
  for (int i = 0; i < nout - 1; i++) {
    sprintf (str, "%d",i);
    // Define a space large enough to contain the max number of 
    // RFI sources. This should be implemented more elegantly.
    itsOutHolders[i] = new DH_CubeC (string("out_") + str, itsNrcu, itsNsubband, itsMaxRFI);    
  }
  itsBuffer = 0;

  // DEBUG
  itsC = 0;
}

WH_STA::~WH_STA()
{  
  for (int i = 0; i < getOutputs() - 1; i++) {
	delete itsOutHolders[i];
  }
  delete [] itsOutHolders;
}

WH_STA* WH_STA::make (const string& name) const
{
  return new WH_STA (name, getOutputs(), itsNrcu, itsNsubband, itsMaxRFI, itsBufLength, 
					 itsAlg, itsPASTdInit, itsUpdateRate, itsPASTdInterval, itsPASTdBeta,
					 itsDetRFI);
}

void WH_STA::process()
{
  using namespace blitz;
  Range all = Range::all ();

  // Place the next incoming sample vector in the snapshot buffer 
  // Keep a cylic buffer for the input snapshots
  for (int r = 0; r < itsNrcu; r++) {
	for (int s = 0; s < itsNsubband; s++) {
 	  itsBuffer (r, s, itsPos) = itsInHolder.getBuffer()[s + r * itsNsubband];
	}
  }
  itsPos = ++itsPos % itsBuffer.extent (thirdDim);

  if (getOutHolder(0)->doHandle ()) {

	// Debug
	//	cout << "WH_STA, oncontigeous buffer: " << endl << itsBuffer << endl;

	// Create contigeous buffer
	itsBuffer = CreateContigeousBuffer (itsBuffer, itsPos);
	itsPos = 0; // Set cyclic buffer pointer to the first entry because it is ordered now

	// Debug
	//	cout << "WH_STA, contigeous buffer: " << endl << itsBuffer << endl;
	
	if (itsAlg == "EVD") {
	  for (int s = 0; s < itsNsubband; s++) {
		LoMat_dcomplex b = itsBuffer (all, s, all).copy ();
		LoMat_dcomplex evec (itsNrcu, itsNrcu);
		LoVec_double eval (itsNrcu);

		// Debug
		//		cout << "WH_STA, contigeous subband buffer: " << endl << b << endl;

		itsAcm = LCSMath::acm (b);     // calculate the ACM
		LCSMath::eig (itsAcm, evec, eval); 

		// Debug
// 		cout << "WH_STA, acm: " << endl << itsAcm << endl;
// 		cout << "WH_STA, evec: " << endl << evec << endl;
// 		cout << "WH_STA, eval: " << endl << eval << endl;
		
		// Use these functions to reverse a matrix or vector and not the blitz ones, due to memory
		// incontiguous return values.
		itsEvectors (all, all, s) = ReverseMatrix (evec, firstDim);
		itsEvalues (all, s) = ReverseVector (eval);	  
	  }
	  ++itsCount;
	} else if (itsAlg == "SVD") {
	  for (int s = 0; s < itsNsubband; s++) {
		LoMat_dcomplex dummy (itsBuffer.extent(blitz::secondDim), itsBuffer.extent(blitz::secondDim));
		LoMat_dcomplex b = itsBuffer (all, s, all).copy ();
		LoMat_dcomplex evec;
		LoVec_double eval;

		LCSMath::svd (b, evec, dummy, eval);		  

		itsEvectors (all, all, s) = evec;
		itsEvalues (all, s) = eval;
	  }
	  ++itsCount;
	} else if (itsAlg == "PASTd") {
	  // Do PASTd
	  if (itsPASTdInit == 0 && itsCount == 0) {
		for (int s = 0; s < itsNsubband; s++) {
		  LoMat_dcomplex b = itsBuffer (all, s, all).copy ();
		  LoMat_dcomplex evec (itsNrcu, itsNrcu);
		  LoVec_double eval (itsNrcu);
		  
		  itsAcm = LCSMath::acm (b);     // calculate the ACM
		  //		  LCSMath::eig (itsAcm, evec, eval); 
		  
		  // Use these functions to reverse a matrix or vector and not the blitz ones, due to memory
		  // incontiguous return values.
		  itsEvectors (all, all, s) = ReverseMatrix (evec, firstDim);
		  itsEvalues (all, s) = ReverseVector (eval);	  
		}
		itsCount = ++itsCount % itsUpdateRate;
	  } else if (itsPASTdInit == 1 && itsCount == 0) {
	      // Do an SVD to initialize PASTd
		for (int s = 0; s < itsNsubband; s++) {
		  LoMat_dcomplex dummy (itsBuffer.extent(blitz::secondDim), itsBuffer.extent(blitz::secondDim));
		  LoMat_dcomplex b = itsBuffer (all, s, all).copy ();
		  LoMat_dcomplex evec;
		  LoVec_double eval;
		  
		  LCSMath::svd (b, evec, dummy, eval);		  
		  
		  itsEvectors (all, all, s) = evec;
		  itsEvalues (all, s) = eval;
		}
		itsCount = ++itsCount % itsUpdateRate;
	  } else if (itsCount > 0) {
	    // PASTd step
		for (int s = 0; s < itsNsubband; s++) {
		  LoMat_dcomplex b = itsBuffer (all, s, all).copy ();
		  LoVec_double eval = itsEvalues (all, s).copy ();
		  LoMat_dcomplex evec = itsEvectors (all, all, s).copy ();

		  pastd (b, itsBufLength, itsPASTdInterval, itsPASTdBeta, eval, evec);

		  itsEvectors (all, all, s) = evec;
		  itsEvalues (all, s) = eval;	  		  
		}
		itsCount = ++itsCount % itsUpdateRate;
	  }
	} else {
	  itsAlg = "EVD";	  
	} 

	// Detect the number of sources in the signal
	// make sure the number of sources detected does not exceed the 
	// maximum number of sources that can be handled by the
	// algorithm. If no adaptive beamforming is done, assume 0
	// interfering sources.
	// Determine the number of sources in the signal
	LoVec_double RFI (itsNsubband);
	if (itsDetRFI == 0) {
	  for (int s = 0; s < itsNsubband; s++) {
		RFI (s)= (double) mdl (itsEvalues (all, s), itsNrcu, itsBufLength);
		
		if (RFI (s) > itsMaxRFI) RFI (s) = itsMaxRFI;
	  }
	} else {
	  // The user provided a number of RFI's detected. This overrides 
	  // the detected number of RFI's
	  RFI = itsDetRFI;
	}

	// Find the appropriate Eigen vectors
	LoCube_dcomplex B (itsNrcu, itsMaxRFI, itsNsubband);
	B = itsEvectors (all, Range (itsEvectors.lbound (firstDim), itsMaxRFI - 1), all);

	// Now copy the Eigen vectors to the output
	for (int i = 0; i < getOutputs() - 1; i++) {
	  memcpy (itsOutHolders[i]->getBuffer (), B.data (), 
			  itsNrcu * itsNsubband * itsMaxRFI * sizeof (DH_CubeC::BufferType));	  
	}

	// Copy the mdl results to the output
	memcpy (itsNumberOfRFIs.getBuffer (), RFI.data (), sizeof (DH_SampleR::BufferType) * itsNsubband);

	// DEBUG
	cout << "WH_STA : " << itsC++ << endl;	

  } else {
    // The STA is now dormant. Do nothing.
    if (itsCount != 0 && itsAlg == "PASTd") {
      itsCount = ++itsCount % itsUpdateRate;
    }
  }
}

void WH_STA::dump() const
{
}

DH_SampleC* WH_STA::getInHolder (int channel)
{
  AssertStr (channel < getInputs(), "Input channel too high");
  return &itsInHolder;
}

DataHolder* WH_STA::getOutHolder (int channel)
{
  AssertStr (channel < (getOutputs()), "Output channel too high"); 
  if (channel < getOutputs() - 1)
	return itsOutHolders[channel];
  else
	return &itsNumberOfRFIs;  
}

LoCube_dcomplex WH_STA::CreateContigeousBuffer (const LoCube_dcomplex& aBuffer, int pos)
{
  using namespace blitz;
  AssertStr (pos <= aBuffer.ubound(thirdDim), "Can't create contigeous buffer, pos too high!");
  Range all = Range::all ();
  LoCube_dcomplex output (aBuffer.shape ());

  output (all, all, Range (output.lbound (thirdDim), itsBufLength - pos - 1)) = 
	aBuffer (all, all, Range (pos, aBuffer.ubound (thirdDim)));

  output (all, all, Range (itsBufLength - pos, output.ubound (thirdDim))) = 
	aBuffer (all, all, Range (aBuffer.lbound (thirdDim), pos - 1));

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
