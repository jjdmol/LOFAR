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
//#  Chris Broekema, january 2003
//#  



#include <stdio.h>             // for sprintf

#include <StationSim/WH_BeamFormer.h>
#include <BaseSim/ParamBlock.h>
#include <Common/Debug.h>
#include <Common/lofar_vector.h>

using namespace blitz;

WH_BeamFormer::WH_BeamFormer (const string& name,
			      unsigned int nin,
			      unsigned int nout,
			      unsigned int nrcu,
			      unsigned int nbeam,
			      unsigned int maxNtarget, unsigned int maxNrfi)
: WorkHolder    (nin, nout, name,"WH_BeamFormer"),  // Check number of inputs and outputs
  itsInHolders  (0),
  itsWeight     (0),
  itsOutHolders (0),
  itsNrcu       (nrcu),
  itsNbeam      (nbeam),
  itsMaxNtarget (maxNtarget),
  itsMaxNrfi    (maxNrfi)
{

  // the number of inputs is equal to the number of reveiving elements
  if (nin > 0) {
    itsInHolders = new DH_SampleC* [nrcu];
  }
  char str[8];
  for (unsigned int i=0; i<nrcu; i++) {
    sprintf (str, "%d",i);
    itsInHolders[i] = new DH_SampleC (string("in_") + str, 1, 1);
  }

  // The first time the weights should not be read.
  itsWeight = new DH_SampleC("weights", itsNrcu, 1);


  // idem for the number of outputs
  if (nout > 0) {
    itsOutHolders = new DH_SampleC* [nout];
  }
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_SampleC (string("out_") + str, 1, 1);
  }
  sample.resize(itsNrcu);

  //DEBUG
  itsFileOutReal.open ((string ("/home/alex/gerdes/BF_real_") + name + string (".txt")).c_str ());
  itsFileOutComplex.open ((string ("/home/alex/gerdes/BF_complex_") + name + string (".txt")).c_str ());
  itsFileInput.open ("/home/alex/gerdes/testvector.txt");
  itsTestVector.resize (itsNrcu);
  itsFileInput >> itsTestVector;
}


WH_BeamFormer::~WH_BeamFormer()
{
  for (int i=0; i< itsNrcu; i++) {
    delete itsInHolders[i];
  }
  delete itsWeight;

  delete [] itsInHolders;
  for (int i=0; i<getOutputs(); i++) {
    delete itsOutHolders[i];
  }
  delete [] itsOutHolders;

  // DEBUG
  itsFileOutReal.close ();
  itsFileOutComplex.close ();
  itsFileInput.close ();
}


WorkHolder* WH_BeamFormer::construct (const string& name,
									  int ninput, int noutput, const ParamBlock& params)
{
  Assert (ninput == 4);
  return new WH_BeamFormer (name, ninput, noutput,
							params.getInt ("nrcu", 10),
							params.getInt ("nbeam", 10),
							params.getInt ("maxntarget", 10),
							params.getInt ("maxnrfi", 10));
}

WH_BeamFormer* WH_BeamFormer::make (const string& name) const
{
  return new WH_BeamFormer (name, getInputs(), getOutputs(),
			    itsNrcu, itsNbeam, itsMaxNtarget, itsMaxNrfi);
}


void WH_BeamFormer::process()
{
  for (int i = 0; i < itsNrcu; i++) {
    sample(i) = (dcomplex)itsInHolders[i]->getBuffer()[0]; 
  }

  // the weights are calculated in WH_AWE
  if (getOutputs() > 0) {
    
    LoVec_dcomplex weight(itsWeight->getBuffer(), itsNrcu, duplicateData);    
    //sample *= weight;
	sample = itsTestVector * weight;

    for (int j = 0; j < itsNrcu; j++) {
      // copy the sample to the outHolders
      itsOutHolders[j]->getBuffer ()[0] = sample(j);
    }

	// DEBUG
	//	cout << sample << " " << real(sample(0)) << " " << imag(sample(0)) << endl;
	itsFileOutReal << real(sample) << endl;
	itsFileOutComplex << imag (sample) << " " << endl;
	//cout << sample << endl;	
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
  AssertStr (channel < getInputs(),
	     "input channel too high");
  if (channel < itsNrcu) {
    return itsInHolders[channel];
  } else {
    return itsWeight;
  }
}

DataHolder* WH_BeamFormer::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs(),
	     "output channel too high");
  return itsOutHolders[channel];
}

