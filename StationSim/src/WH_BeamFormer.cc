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

#include <stdio.h>             // for sprintf

#include <StationSim/WH_BeamFormer.h>
#include <BaseSim/ParamBlock.h>
#include <Common/Debug.h>
#include <Common/lofar_vector.h>


WH_BeamFormer::WH_BeamFormer (const string& name,
			      unsigned int nout,
			      unsigned int nrcu,
			      unsigned int nsubband, unsigned int nbeam,
			      unsigned int maxNtarget, unsigned int maxNrfi,
			      const string& dipoleFileName)
: WorkHolder    (4, nout, name,"WH_BeamFormer"),
  itsInHolder   ("in", nrcu, nsubband),
  itsWeight     ("weight", nbeam),
  itsTarget     ("target", maxNtarget),
  itsRFI        ("rfi", maxNrfi),
  itsOutHolders (0),
  itsNrcu       (nrcu),
  itsNsubband   (nsubband),
  itsNbeam      (nbeam),
  itsMaxNtarget (maxNtarget),
  itsMaxNrfi    (maxNrfi),
  itsDipoleName (dipoleFileName)
{
  // The first time the weights should not be read.
  itsWeight.setReadDelay (1);
  if (nout > 0) {
    itsOutHolders = new DH_SampleC* [nout];
  }
  char str[8];
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_SampleC (string("out_") + str, nbeam, nsubband);
  }
}

WH_BeamFormer::~WH_BeamFormer()
{
  for (int i=0; i<getOutputs(); i++) {
    delete itsOutHolders[i];
  }
  delete [] itsOutHolders;
}

WorkHolder* WH_BeamFormer::construct (const string& name,
				      int ninput, int noutput,
				      const ParamBlock& params)
{
  Assert (ninput == 4);
  return new WH_BeamFormer (name, noutput,
			    params.getInt ("nrcu", 10),
			    params.getInt ("nsubband", 10),
			    params.getInt ("nbeam", 10),
			    params.getInt ("maxntarget", 10),
			    params.getInt ("maxnrfi", 10),
			    params.getString ("dipolefile", ""));
}

WH_BeamFormer* WH_BeamFormer::make (const string& name) const
{
  return new WH_BeamFormer (name, getOutputs(),
			    itsNrcu, itsNsubband, itsNbeam,
			    itsMaxNtarget, itsMaxNrfi, itsDipoleName);
}

void WH_BeamFormer::preprocess()
{
  if (itsDipoleLoc.size() == 0) {
    float val;
    ifstream dipoleFile (itsDipoleName.c_str());
    AssertStr (dipoleFile, "Dipole location file " << itsDipoleName
	       << " not found");
    itsDipoleLoc.reserve (100);
    while (true) {
      dipoleFile >> val;
      if (dipoleFile.eof()) {
	break;
      }
      AssertStr (!dipoleFile.fail(),
		 "Dipole location file " << itsDipoleName
		 << " has wrong format");
      itsDipoleLoc.push_back (val);
    }
    dipoleFile.close();
  }
}

void WH_BeamFormer::process()
{
  if (getOutputs() > 0) {
    DH_SampleC::BufferType* bufin = itsInHolder.getBuffer();
    const DH_Weight::BufferType* weight = itsWeight.getBuffer();
    DH_SampleC::BufferType* bufout = itsOutHolders[0]->getBuffer();
    // As a test copy the data of the first RCU to all beams.
    for (int i=0; i<itsNbeam; i++) {
      for (int j=0; j<itsNsubband; j++) {
	*bufout++ = weight[i] * bufin[j*itsNrcu];
      }
    }
    // Copy the output if multiple outputs are used.
    for (int i=1; i<getOutputs(); i++) {
      memcpy (itsOutHolders[i]->getBuffer(), bufout,
	      itsNbeam * itsNsubband * sizeof(DH_SampleC::BufferType));
    }
  }
}

void WH_BeamFormer::dump() const
{
  cout << "WH_BeamFormer " << getName() << " Buffers:" << endl;
  if (getOutputs() > 0) {
    cout << itsOutHolders[0]->getBuffer()[0] << ','
	 << itsOutHolders[0]->getBuffer()[itsNbeam*itsNsubband-1] << endl;
  }
}


DataHolder* WH_BeamFormer::getInHolder (int channel)
{
  AssertStr (channel < 4,
	     "input channel too high");
  if (channel == 0) {
    return &itsInHolder;
  } else if (channel == 1) {
    return &itsWeight;
  } else if (channel == 2) {
    return &itsTarget;
  }
  return &itsRFI;
}
DH_SampleC* WH_BeamFormer::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs(),
	     "output channel too high");
  return itsOutHolders[channel];
}
