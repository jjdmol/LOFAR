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
			      unsigned int nant, unsigned int maxnrfi, unsigned int ndetnull,
			      bool tapstream, string arraycfg)
: WorkHolder       (nin, nout, name, "WH_Projection"),
  itsInHolders     (0),
  itsOutHolders    (0),
  itsNumberOfRFIs  ("in_mdl", 1, 1),
  itsRFISources    ("in_rfi", nant, maxnrfi),
  itsNrcu          (nant),
  itsMaxRFI        (maxnrfi),
  itsDetectedRFIs  (nin - 3),
  itsDetNulls      (ndetnull),
  itsWeight        (itsNrcu),
  itsSteerv        (itsNrcu),
  itsV             (0,0),
  itsA             (itsNrcu),
  itsTapStream     (tapstream),
  itsArray         (arraycfg)

{
  char str[8];
  // nin is the number of steering vectors that will be used as an input (at least one
  // for the weight determination and possibly more for deterministic nulls).
  if (nin - 2 > 0) {
    itsInHolders = new DH_SampleC* [nin - 2];
  }
    
  for (unsigned int i = 0; i < nin - 2; i++) { // The weight vector from the Weight Determination
    sprintf (str, "%d",i);                     // And the deterministic nulls
    if (i == 1) {
      // make the inholder large enough to contain all the deterministic nulls 
      itsInHolders[i] = new DH_SampleC(string("in_") + str, itsNrcu, itsDetNulls);
    } else {
      itsInHolders[i] = new DH_SampleC(string("in_") + str, itsNrcu, 1);
    }
  }
  if (nout > 0) {
    itsOutHolders = new DH_SampleC* [nout];
  }
  for (unsigned int i = 0; i < nout; i++) {  // The resulting Weight vector
    sprintf (str, "%d",i);                   // And the steervector for spectrum plot
    itsOutHolders[i] = new DH_SampleC (string("out_") + str, itsNrcu, 2);

    if (itsTapStream) {
      itsOutHolders[i]->setOutFile (string ("Proj_") + str + string(".dat"));
    }
  }
  int Count = 0;
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
  return new WH_Projection (name, getInputs(), getOutputs(), itsNrcu, itsMaxRFI, 
			    itsDetNulls, itsTapStream, itsArray.conf_file);
}


void WH_Projection::process()
{

  if (getOutputs() > 0) {
    double phi_det;
    double theta_det;
    Count++;
    LoVec_dcomplex Vvec(itsNrcu);
    LoMat_dcomplex jVec(itsNrcu, 2); // concatenate the weightvector and the steervector to save connections
    

	if (itsRFISources.doHandle ()) {  


	  // Get the number of detected RFI sources from the STA comp. (internally calc. by MDL)
	  int NumberOfEigenVectors = (int)(itsNumberOfRFIs.getBuffer()[0]);
	  //	  itsDetectedRFIs = NumberOfEigenVectors + getInputs() - 3;
	  
	  // The number of sources = number of detected sources + deterministic nulls
	  itsDetectedRFIs = NumberOfEigenVectors + itsDetNulls;

	  // DEBUG
	  // cout << itsDetectedRFIs << endl;
 
	  // Read in the eigenvectors from the STA component. These represent the detected RFI sources and
	  // should be nulled. The deterministic nulls should be appended to this matrix.
	  LoMat_dcomplex V (itsRFISources.getBuffer(), shape (itsNrcu, NumberOfEigenVectors), duplicateData);
	  itsV.resize(V.shape());
	  itsV = V;
	  
  	  if (itsDetNulls != 0 && itsInHolders[1]->doHandle()) {
	    LoMat_dcomplex detnulls (itsInHolders[1]->getBuffer(), shape (itsNrcu, itsDetNulls), duplicateData);
	  //	  itsV.transposeSelf(secondDim, firstDim);
	  // 	  cout << itsV(9,Range::all()) << endl;

	  // Merge the deterministic nulls in the eigenvectors matrix
	  
	    itsV.resizeAndPreserve(itsNrcu, itsDetectedRFIs); // Add number of deterministic nulls
	    itsV(Range::all(), Range(NumberOfEigenVectors, toEnd)) = detnulls;

	    // Test function for deterministic nulls
//    	    cout << "Checking deterministic nulls" << endl;
//    	    for (int i = 0; i < itsDetNulls; i++) {
//    	      LoVec_dcomplex temp(itsNrcu);
//    	      temp = itsV(Range::all(), NumberOfEigenVectors+i);
//    	      qm_find_doa(temp , 1, phi_det, theta_det);
//    	    }
	  }
	}

	if (itsInHolders[0]->doHandle ()) {
	  // Get the steering vector from the weight determination comp. assume that only one will be put in.
	  LoVec_dcomplex steerv (itsInHolders[0]->getBuffer(), itsNrcu, duplicateData);
	  itsA = steerv;
	  itsSteerv = steerv;
	}
	
	if (itsOutHolders[0]->doHandle ()) {  
	  // Calculate the weights using the eigenvectors and the steering vector
	  if (itsDetectedRFIs == 1) {
	    Vvec = itsV(blitz::Range::all(), itsV.lbound(secondDim));
	    itsWeight = getWeights(Vvec, itsA);
	  } else if (itsDetectedRFIs > 1) {		
	    itsWeight = getWeights(itsV, itsA);
	  } else {
	    // itsDetectedRFIs == 0
	    itsWeight = itsA;
	  }
	}

	// Assign the weightvector and steervector to a matrix for transport
	jVec(Range::all(), 0) = itsWeight;
	jVec(Range::all(), 1) = itsSteerv;
	// Copy output to the next step
	memcpy(itsOutHolders[0]->getBuffer(), jVec.data(), itsNrcu * 2 * sizeof(DH_SampleC::BufferType));
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

  dcomplex VHV = (dcomplex)1 / sum((2 * real(V) - V) * V);
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
  LoMat_dcomplex I = LCSMath::diag(w);        // Create Identity matrix

  LoMat_dcomplex temp = LCSMath::matMult(LCSMath::hermitianTranspose(V), V);
  temp = LCSMath::invert(temp); // (VHV)^-1
  LoMat_dcomplex temp2 = LCSMath::matMult(temp, LCSMath::hermitianTranspose(V));
  LoMat_dcomplex Pv = LCSMath::matMult(V, temp2); // VPvVH
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

void WH_Projection::qm_find_doa(const LoVec_dcomplex& evec, const double accuracy, double& phi, double& theta)
{
  // Estimates the direction of arrival detected by the adaptive algorithms.
  // Accuracy of the estimation depends on the value of 'accuracy' which is 
  // given in degrees. Please note that the algorithm is extremely expensive
  // and inherently inaccurate.
  
  LoVec_double px (itsNrcu);
  LoVec_double py (itsNrcu);
  LoVec_dcomplex temp_sv(itsNrcu);

  px = itsArray.getPointX ();
  py = itsArray.getPointY ();

  double c = 0;
  double max = 0;
  double phimax = 0;
  double thetamax = 0;
  
  for (double a = -180; a <= 180; a = a + accuracy) {
    for (double b = 0; b <= 90; b = b + accuracy) {
      temp_sv = steerv ( (a*M_PI/180) , (b*M_PI/180) , px, py );
      c = abs ( sum ( LCSMath::conj(evec) * temp_sv ) ) ;
      if ( c > max ) {
	max = c ; 
  	phimax = a * M_PI / 180;
  	thetamax = b * M_PI / 180;
      }
    }
  }
  phi = phimax;
  theta = thetamax;
  cout << "Source found at (" << phimax << ", " << thetamax << ")"<< endl;
}

LoVec_dcomplex WH_Projection::steerv (double phi, double theta, LoVec_double px, LoVec_double py) {
  
  FailWhen1( px.size() != py.size(),"vector size mismatch" );

  LoVec_dcomplex res( px.size() );
  dcomplex i = dcomplex (0,1);

  res = exp( -2*M_PI*i*( px*sin(theta)*cos(phi) + py*sin(theta)*sin(phi) ) );
  return res;
}
