//#  WH_BeamFormer.cc:
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
//#  



#include <stdio.h>             // for sprintf

#include <StationSim/WH_BeamFormer.h>
#include <BaseSim/ParamBlock.h>
#include <Common/Debug.h>
#include <Common/lofar_vector.h>
#include <Math/LCSMath.h>


using namespace blitz;

WH_BeamFormer::WH_BeamFormer (const string& name,
							  unsigned int nin,
							  unsigned int nout,
							  unsigned int nrcu,
							  unsigned int nbeam,
							  unsigned int maxNtarget, 
							  unsigned int maxNrfi,
							  bool tapstream)
  : WorkHolder    (nin, nout, name,"WH_BeamFormer"),  // Check number of inputs and outputs
	itsInHolders  (0),
	itsOutHolders (0),
	itsWeight     ("weights", nrcu, 1),
	itsNrcu       (nrcu),
	itsNbeam      (nbeam),
	itsMaxNtarget (maxNtarget),
	itsMaxNrfi    (maxNrfi),
	sample        (nrcu),
	itsTapStream  (tapstream)
{
  char str[8];
  // the number of inputs is equal to the number of reveiving elements
  if (nin > 0) {
    itsInHolders = new DH_SampleC* [nrcu];
  }
  for (unsigned int i = 0; i < nrcu; i++) {
    sprintf (str, "%d",i);
    itsInHolders[i] = new DH_SampleC (string ("in_") + str, 1, 1);
  }
  // idem for the number of outputs
  if (nout > 0) {
    itsOutHolders = new DH_SampleC* [nout];
  }
  for (unsigned int i = 0; i < nout; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_SampleC (string ("out_") + str, 1, 1);

	if (itsTapStream) {
	  itsOutHolders [i]->setOutFile (string ("BF_") + str + string (".dat"));
	}
  }

  //DEBUG
//   itsFileInput.open ("/home/alex/temp/test_vectorEssai.txt");
//   itsTestVector.resize(itsNrcu, 25445);
//   itsFileInput >> itsTestVector;
  handle = gnuplot_init ();
}


WH_BeamFormer::~WH_BeamFormer()
{
  for (int i = 0; i < itsNrcu; i++) {
    delete itsInHolders[i];
  }
  delete [] itsInHolders;
  for (int i = 0; i < getOutputs(); i++) {
    delete itsOutHolders[i];
  }
  delete [] itsOutHolders;

  gnuplot_close (handle);
//   DEBUG
//   itsFileInput.close ();
}


WorkHolder* WH_BeamFormer::construct (const string& name,
									  int ninput, int noutput, const ParamBlock& params)
{
  return new WH_BeamFormer (name, ninput, noutput,
							params.getInt ("nrcu", 10),
							params.getInt ("nbeam", 10),
							params.getInt ("maxntarget", 10),
							params.getInt ("maxnrfi", 10),
							false);
}

WH_BeamFormer* WH_BeamFormer::make (const string& name) const
{
  return new WH_BeamFormer (name, getInputs(), getOutputs(),
							itsNrcu, itsNbeam, itsMaxNtarget, itsMaxNrfi, itsTapStream);
}


void WH_BeamFormer::process()
{
  if (getOutputs () > 0) {
	dcomplex temp;
	for (int i = 0; i < itsNrcu; i++) {
	  sample(i) = (dcomplex)itsInHolders[i]->getBuffer()[0]; 
// 	  //DEBUG
// 	  sample(i) = itsTestVector(i, itsPos);
	}
	LoVec_dcomplex weight(itsWeight.getBuffer(), itsNrcu, duplicateData);   
	
	dcomplex output = 0; 
	for (int i = 0; i < itsNrcu; i++) {
	  output += (2 * real(weight(i)) - weight(i)) * sample(i); // w^H * x(t)
	}

    for (int j = 0; j < getOutputs(); j++) {
      // copy the sample to the outHolders
      itsOutHolders[j]->getBuffer ()[0] = output;
    }

// 	int t = itsInHolders[0]->getTimeStamp ();
// 	string n = getName ();
// 	if (t % 500 == 0 && n == "0") {
// 	  gnuplot_resetplot (handle);
// 	  LoVec_double w = LCSMath::absVec (weight);
// 	  gnuplot_plot_x (handle, w.data (), w.size (), "Power spectrum.");
// 	}
  }
}



void WH_BeamFormer::dump() const
{
  cout << "WH_BeamFormer " << getName() << " Buffers:" << endl;
  dcomplex sum = 0;
  if (getOutputs() > 0) {
    for (int i=0; i < itsNrcu; i++) {
      sum = sum + itsOutHolders[i]->getBuffer()[0];
      //      cout << itsOutHolders[i]->getBuffer()[0] << '.' << endl ;
    }
    cout << "Sum of beamformer outputs: "<< sum.real() << endl;
  }
}


DataHolder* WH_BeamFormer::getInHolder (int channel)
{
  AssertStr (channel < getInputs(), "input channel too high");
  if (channel < getInputs() - 1) {
    return itsInHolders[channel];
  } else {
    return &itsWeight;
  }
}

DataHolder* WH_BeamFormer::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs(), "output channel too high");
  return itsOutHolders[channel];
}

