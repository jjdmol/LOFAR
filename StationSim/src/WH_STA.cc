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
//#  Chris Broekema, january 2003.
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

WH_STA::WH_STA (const string& name, unsigned int nin, unsigned int nout,
		unsigned int nant, unsigned int maxnrfi, unsigned int buflength)
: WorkHolder    (nin, nout, name, "WH_STA"),
  itsInHolders  (0),
  itsOutHolder  (0),
  itsNrcu       (nant),
  itsMaxRFI     (maxnrfi),
  itsBufLength  (buflength)
{
  if (nin > 0) {
    itsInHolders = new DH_SampleC* [nin];
  }
  char str[8];
  for (unsigned int i=0; i<nin; i++) {
    sprintf (str, "%d",i);
    itsInHolders[i] = new DH_SampleC (string("in_") + str, 1, 1);
  }
  
  if (nout > 0) {
    // Define a space large enough to contain the max number of 
    // RFI sources. This should be implemented more elegantly.
    itsOutHolder = new DH_SampleC("out", itsNrcu, itsMaxRFI);
  }
}

WH_STA::~WH_STA()
{
  for (int i=0; i< getInputs(); i++) {
    delete itsInHolders[i];
  }
  delete [] itsInHolders;

  delete itsOutHolder;
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
  /// REMOVE ME!!

  // Find the snapshot array for the current AWE snapshot
  itsBuffer.resize(itsNrcu, itsBufLength);
  itsBuffer = -1;


  // This is only a single snapshot.. Make a buffer to really to something 
  // useful
//   DH_SampleC::BufferType* bufin = itsInHolders->getBuffer();

//   itsBuffer = *bufin;

//   // Select the appropriate algorithm
//   // See if the PASTd algorithm need updating
//   // Use either EVD or SVD for updating

//   // EVD - first calculate the ACM
//   LoMat_dcomplex itsAcm (itsNrcu, itsNrcu) ;
//   itsAcm = LCSMath::acm(itsBuffer);
  
//   // EVD - using the ACM, calculate eigen vectors and values.
//   LoMat_dcomplex itsEvectors;
//   LoVec_double   itsEvalues;
//   LCSMath::eig(itsAcm, itsEvectors, itsEvalues);

//   // EVD - Determine the number of sources in the signal
//   // TODO: fit MDL into Common library.
//   unsigned int RFI = mdl(itsEvalues, itsNrcu, itsBufLength);
  
//   // EVD - Find the appropriate Eigen vectors
//   // Assume the most powerfull sources are in front. This may not be true!!
//   // TODO check if this is a good assumption -- possible overflow
//   LoMat_dcomplex B(itsNrcu, RFI);
// //   B = itsEvectors(Range::all(), Range(itsEvectors.lbound(firstDim), 
// // 				     itsEvectors.lbound(firstDim) + RFI - 1));

//   LoVec_dcomplex w (itsNrcu) ; // the weight vector
//   dcomplex *w_ptr = w.data() ;

  LoVec_dcomplex d(itsNrcu);
  //  w = getWeights(B, d);
  
  // Now assign the calculated weight vector to the output
  

  memcpy(itsOutHolder->getBuffer(), d.data(), itsNrcu * sizeof(DH_SampleC::BufferType));
  
  
//   cout << itsBuffer << endl;
//   cout << itsAcm << endl;

  // SVD 

  // PASTd
 
}

void WH_STA::dump() const
{
  using namespace blitz;

//   LoVec_dcomplex weight(itsOutHolder->getBuffer(), itsNrcu, duplicateData);    

//   cout << "Weight vector Buffer: " << endl;
//   cout << weight<< endl;
}


DH_SampleC* WH_STA::getInHolder (int channel)
{
  AssertStr (channel < getInputs(), "Input channel too high");

  return itsInHolders[channel];
}

DH_SampleC* WH_STA::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs(), "output channel too high");
  
  return itsOutHolder;
}

