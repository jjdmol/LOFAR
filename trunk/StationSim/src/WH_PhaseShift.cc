// WH_PhaseShift.cc: implementation of the WH_PhaseShift class.
//
//  Copyright (C) 2000,2001,2002
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

#include <Common/Debug.h>
#include <StationSim/WH_PhaseShift.h>

using namespace blitz;


WH_PhaseShift::WH_PhaseShift (int nin, 
							  int nout, 
							  int nrcu, 
							  DataGenerator* dg_config, 
							  int nfft, 
							  int source, 
							  int windowsize,
							  string name)
  : WorkHolder        (nin, nout, "aWorkHolder", "WH_PhaseShift"),
	itsNrcu           (nrcu),
	itsPos            (0),
	itsNfft           (nfft),
	itsSource         (source),
    itsPrevWindowSize (windowsize),
    itsCount          (0),
	itsConfig         (dg_config),
	itsName           (name)
{
  // Allocate blocks to hold pointers to input and output DH-s.
  if (nin > 0) {
    itsInHolders = new DH_SampleR* [nin];
  }
  if (nout > 0) {
    itsOutHolders = new DH_SampleC* [nout];
  }
  // Create the input DH-s.
  char str[8];

  for (int i = 0; i < nin; i++) {
    sprintf (str, "%d", i);
    itsInHolders[i] = new DH_SampleR (string ("in_") + str, 1, 1);
  }
  // Create the output DH-s.
  if (nin == 0) {
    nin = 1;
  }
  for (int i = 0; i < nout; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_SampleC (string ("out_") + str, nrcu, 1);

// 	// DEBUG
// 	itsOutHolders [i]->setOutFile (string ("phase_") + str + string ("_") + itsName  + string (".dat"));
  }

  itsInputBuffer.resize (nfft);
  itsForwardPlan = FFTW::initForwardPlan (nfft);
  itsInversePlan = FFTW::initInversePlan (nfft);
  itsFreqShift.resize (itsNfft);
  itsOutputBuffer.resize (itsNrcu, 
						  itsNfft);
  itsFreqShift = 
	PhaseShift::getFreqShift (itsConfig->itsSamplingFreq,
							  (itsConfig->itsSamplingFreq) / 2, 
							  itsNfft);

  itsTrajectCount = 0;
}

WH_PhaseShift::~WH_PhaseShift ()
{
  for (int ch = 0; ch < getInputs (); ch++) {
    delete itsInHolders[ch];
  }
  for (int ch = 0; ch < getOutputs (); ch++) {
    delete itsOutHolders[ch];
  }
}


WH_PhaseShift* WH_PhaseShift::make (const string&) const
{
  return new WH_PhaseShift (getInputs (), 
							getOutputs (), 
							itsNrcu, 
							itsConfig,
							itsNfft, 
							itsSource, 
							itsPrevWindowSize,
							itsName);
}


void WH_PhaseShift::process ()
{
  if (getOutputs () > 0) {
    if (itsCount > itsPrevWindowSize - 2) {
      DH_SampleR::BufferType* bufin = itsInHolders[0]->getBuffer ();
      itsInputBuffer (itsPos) = bufin[0];
      itsPos = (itsPos + 1) % itsInputBuffer.size ();

      if (itsPos == 0) {
		itsOutputBuffer = 
		  PhaseShift::phaseShift (itsNfft,
								  itsInputBuffer,
								  itsConfig->itsSources[itsSource]->itsTraject->getTheta (itsTrajectCount),
								  itsConfig->itsSources[itsSource]->itsTraject->getPhi (itsTrajectCount++),
								  *(itsConfig->itsArray),
								  itsFreqShift,
								  itsForwardPlan,
								  itsInversePlan);
		itsOutputBuffer /= itsNfft;
      }

      for (int i = 0; i < getOutputs (); i++) {
		LoVec_dcomplex temp (itsNrcu);
		for (int j = 0; j < itsNrcu; j++) {
		  temp (j) = itsOutputBuffer (j, itsPos);
		}
		memcpy (getOutHolder (i)->getBuffer (), 
				temp.data (),
				itsNrcu * sizeof (dcomplex));
      }
    }
	itsCount++;
  }
}


void WH_PhaseShift::dump () const
{
  cout << itsCount << endl;
  cout << "Phi   : "<<  itsConfig->itsSources[itsSource]->itsTraject->getPhi   (itsCount) << endl;  
  cout << "Theta : "<<  itsConfig->itsSources[itsSource]->itsTraject->getTheta (itsCount) << endl;
}

DH_SampleR* WH_PhaseShift::getInHolder (int channel)
{
  return itsInHolders[channel];
}

DH_SampleC* WH_PhaseShift::getOutHolder (int channel)
{
  return itsOutHolders[channel];
}
