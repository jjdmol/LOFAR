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


WH_STA::WH_STA (const string& name,  int nin,  int nout,
				 int nant,  int maxnrfi,  int buflength)
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
  itsAcm          (nant, nant)
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
  //DEBUG
//   delay = 3;
  itsTestVector.resize(itsNrcu, 300);
  itsFileInput.open ("/home/alex/gerdes/test_vectorSTA_sin.txt");
  itsFileInput >> itsTestVector;
  itsCount = 0;
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
}

WH_STA* WH_STA::make (const string& name) const
{
  return new WH_STA (name, getInputs(), getOutputs(), itsNrcu, itsMaxRFI, itsBufLength);
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
	//itsBuffer(i, itsPos) = itsInHolders[i]->getBuffer()[0];
	itsBuffer(i, itsPos) = itsTestVector(i, itsCount);
  }
  itsPos = (itsPos + 1) % itsBuffer.cols();
  itsCount = (itsCount + 1) % itsTestVector.cols();

//   // Select the appropriate algorithm
//   // See if the PASTd algorithm need updating
//   // Use either EVD or SVD for updating

  if (getOutHolder(0)->doHandle ()) {
	// Create contigeous buffer
	itsBuffer = CreateContigeousBuffer (itsBuffer, itsPos);
	itsPos = 0; // Set cyclic buffer pointer to the first entry because it is ordered now

	// calculate the ACM
 	itsAcm = LCSMath::acm (itsBuffer);

	// using the ACM, calculate eigen vectors and values.
	LCSMath::eig (itsAcm, itsEvectors, itsEvalues);

	// Determine the number of sources in the signal

	// put in a vector with two large eigen values and see what MDL finds
	itsEvalues = -1;
//  	itsEvalues(itsNrcu-1) = 200000000.0;
//    	itsEvalues(itsNrcu-2) = 100000000.0;
//  	itsEvalues(itsNrcu-3) = 10000.0;
	//	itsEvalues(itsNrcu-4) = 1000.0;

	double RFI = (double) mdl (itsEvalues, itsNrcu, itsBufLength);
	// 	cout << RFI << endl;
	if (RFI > itsMaxRFI) RFI = itsMaxRFI;

	// DEBUG
//  	RFI = 1;

// 	// DEBUG
// 	for (int i = 0; i < itsNrcu; i++) {
// 	  cout << itsEvalues(i) << endl;
// 	  if (itsEvalues(i) > 10) {
// 		for (int j = 0; j < itsNrcu; j++) {
// 		  cout << itsEvectors(i, j) << endl;
// 		}
// 	  }
// 	}
	
	// Find the appropriate Eigen vectors
	LoMat_dcomplex B = itsEvectors(Range(itsEvectors.ubound(secondDim) - RFI + 1,
										 itsEvectors.ubound(secondDim)), Range::all());

	// 	cout << B << endl;
	//	cout << itsEvalues << endl;
	
	// Now assign the Eigen vectors to the output
	for (int i = 0; i < getOutputs() - 1; i++) {
	  memcpy(itsOutHolders[i]->getBuffer(), B.data(), itsNrcu * (int)RFI * sizeof(DH_SampleC::BufferType));
	}
	memcpy(itsNumberOfRFIs.getBuffer(), &RFI, sizeof(DH_SampleR::BufferType));	
  }

  // SVD 

  // PASTd 
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
