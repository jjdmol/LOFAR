//#  WH_Calibration.cc:
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
//#  $Id$
//#

#include <stdio.h>             // for sprintf

#include <StationSim/WH_Calibration.h>
#include <BaseSim/ParamBlock.h>
#include <Common/Debug.h>
#include <aips/Arrays/ArrayMath.h>

WH_Calibration::WH_Calibration(const string& name,
			   unsigned int nout,
			   unsigned int nrcu, unsigned int nsub1)
: WorkHolder   (1, nout, name, "WH_Calibration"),
  itsInHolder  ("in", nrcu, nsub1),
  itsOutHolders(0),
  itsNrcu      (nrcu),
  itsNsub1     (nsub1),
  itsAveraging (10),// BUG this needs to passed by the command line
  itsPos       (0),
  itsCount     (0)
{
  if (nout > 0) 
  {
    itsOutHolders = new DH_SampleR* [nout];
  }
  char str[8];
  // Create the output DH-s.
  for (unsigned int i = 0; i < nout; i++) 
  {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_SampleR(string("out_") + str, 1, 1);
  }

  itsPowerBuffer.resize(itsNrcu, itsNsub1, itsAveraging);
  itsMeanPower.resize(itsNsub1);
  itsMedianPower.resize(itsNsub1);
  itsMaxPower.resize(itsNsub1);
  itsMinPower.resize(itsNsub1);
  itsVariancePower.resize(itsNsub1);
}

WH_Calibration::~WH_Calibration()
{
  for (int i = 0; i < getOutputs(); i++) 
  {
    delete itsOutHolders[i];
  }
  delete [] itsOutHolders;
}

WorkHolder* WH_Calibration::construct(const string& name, int ninput,
				    int noutput,
				    const ParamBlock& params)
{
  Assert (ninput == 1);
  return new WH_Calibration(name, noutput,
                          params.getInt ("nrcu", 10),
                          params.getInt ("nsub1", 10));
}

WH_Calibration* WH_Calibration::make(const string& name) const
{
  return new WH_Calibration(name, getOutputs(), itsNrcu, itsNsub1);
}

void WH_Calibration::process()
{
  // Process receives each time a buffer of nRCU * nSub1 samples.
  if (getOutputs() > 0) 
  {
    DH_SampleC::BufferType* bufin = itsInHolder.getBuffer();
    DH_SampleR::BufferType* bufout = itsOutHolders[0]->getBuffer();
    
    Matrix<DH_SampleC::BufferType> InputMatrix(IPosition(2, itsNrcu, itsNsub1), bufin, SHARE);


    // Add the incoming sample matrix to the cube
    itsPowerBuffer.xyPlane(itsPos) = real(InputMatrix * conj(InputMatrix));
    itsPos = ++itsPos % itsAveraging;
    if (itsCount++ == 0)
      {
	for (int avr = 1; avr < itsAveraging; ++avr)
	  {
	    itsPowerBuffer.xyPlane(avr) = real(InputMatrix * conj(InputMatrix));
	  }
      }

    Matrix<double> AveragedRCUPower(itsNsub1, itsAveraging, 0);
    
    // Average over all RCU's
    for (int rcu = 0; rcu < itsNrcu; ++rcu)
      {
	for (int sub = 0; sub < itsNsub1; ++sub)
	  {
	    for (int avr = 0; avr < itsAveraging; ++avr)
	      {
		AveragedRCUPower(sub, avr) += itsPowerBuffer(rcu, sub, avr);
	      }
	  }
      }
    AveragedRCUPower = AveragedRCUPower / (double)itsNrcu;

    // Calculate statistics
    for (int sub = 0; sub < itsNsub1; ++sub)
      {
	itsMeanPower(sub) = mean(AveragedRCUPower.row(sub));
	itsMedianPower(sub) = median(AveragedRCUPower.row(sub));
	itsMaxPower(sub) = max(AveragedRCUPower.row(sub));
	itsMinPower(sub) = min(AveragedRCUPower.row(sub));
	itsVariancePower(sub) = variance(AveragedRCUPower.row(sub));
      }
    
    // Calculate the threshold
    Vector<double> CleanMeanPower;
    CleanMeanPower = itsMeanPower;
    
    // Get rid of intermittend RFI, make use of the fact that the mean and the median
    // are different for intermittend RFI.
    for (int sub = 0; sub < itsNsub1; ++sub)
      {
	if (itsMeanPower(sub) > itsMedianPower(sub))
	  {
	    CleanMeanPower(sub) = itsMedianPower(sub); // Get rid of the intermittend RFI
	  }
      }
    
    // Get rid of the stationary RFI by asuming that this isn't much in the signal left,
    // so the median will give a good intermediate threshold. An extra thirty percent is
    // added for safety.
    for (int sub = 0; sub < itsNsub1; ++sub)
      {
	if (CleanMeanPower(sub) > median(CleanMeanPower) * 1.3)
	  {
	    CleanMeanPower(sub) =  median(CleanMeanPower);
	  }
      }
    
    // The threshold is the mean plus n times sigma
    double CleanVariance = variance(CleanMeanPower);
    double Threshold = mean(CleanMeanPower) + 13 * sqrt(CleanVariance);
    
    *bufout = Threshold;
    
//     if (itsCount == 12)
//       {
//	cout << Threshold << endl << endl;
// 	cout << AveragedRCUPower << endl << endl;
// 	cout << CleanMeanPower << endl << endl;
// 	cout << CleanVariance << endl << endl;
//       }

//     *bufout = 0.0005; // DEBUG
 
    // Copy to other output buffers.
    for (int i = 1; i < getOutputs(); i++) 
      {
	memcpy (getOutHolder(i)->getBuffer(), bufout, sizeof(DH_SampleR::BufferType));
      }
  }
}

void WH_Calibration::dump() const
{
  cout << "WH_Calibration " << getName() << " Buffer:" << endl;
  if (getOutputs() > 0) 
  {
    cout << itsOutHolders[0]->getBuffer()[0] << ','
	 << itsOutHolders[0]->getBuffer()[itsNrcu * itsNsub1 - 1] << endl;
  }
}

DH_SampleC* WH_Calibration::getInHolder(int channel)
{
  AssertStr (channel == 0, "input channel too high");
  return &itsInHolder;
}

DH_SampleR* WH_Calibration::getOutHolder(int channel)
{
  AssertStr (channel < getOutputs(), "output channel too high");
  return itsOutHolders[channel];
}
