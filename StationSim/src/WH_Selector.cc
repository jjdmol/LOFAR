//#  WH_Selector.cc:
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

#include <StationSim/WH_Selector.h>
#include <BaseSim/ParamBlock.h>
#include <Common/Debug.h>
#include <Common/lofar_vector.h>


WH_Selector::WH_Selector (const string& name,
			  unsigned int nout, unsigned int nrcu,
			  unsigned int nsubbandin, unsigned int nsubbandout)
: WorkHolder    (2, nout, name, "WH_Selector"),
  itsInHolder   ("in", nrcu, nsubbandin),
  itsInSel      ("sel", nsubbandin),
  itsOutHolders (0),
  itsNrcu       (nrcu),
  itsNbandin    (nsubbandin),
  itsNbandout   (nsubbandout)
{
  if (nout > 0) {
    itsOutHolders = new DH_SampleC* [nout];
  }
  char str[8];
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_SampleC(string("out_") + str, nrcu, nsubbandout);
  }
}

WH_Selector::~WH_Selector()
{
  for (int i=0; i<getOutputs(); i++) {
    delete itsOutHolders[i];
  }
  delete [] itsOutHolders;
}

WorkHolder* WH_Selector::construct (const string& name,
				    int ninput, int noutput,
				    const ParamBlock& params)
{
  Assert (ninput == 2);
  return new WH_Selector (name, noutput,
			  params.getInt ("nrcu", 10),
			  params.getInt ("nsubbandin", 10),
			  params.getInt ("nsubbandout", 10));
}

WH_Selector* WH_Selector::make (const string& name) const
{
  return new WH_Selector (name, getOutputs(),
			 itsNrcu, itsNbandin, itsNbandout);
}

void WH_Selector::process()
{
  if (getOutputs() > 0) {
    // Keep track of which output subbands have been filled.
    vector<bool> outDone(itsNbandout, false);
    DH_SampleC::BufferType* bufin = itsInHolder.getBuffer();
    const int* sel = itsInSel.getBuffer();
    DH_SampleC::BufferType* bufout = itsOutHolders[0]->getBuffer();
    
    // Copy the selected input subband to the given output subband.
    // Check if the selection is given correctly.
    for (int i=0; i<itsNbandin; i++) {
      if (sel[i] >= 0) {
	Assert (sel[i] < itsNbandout);
	int out = sel[i];
	AssertStr (outDone[out] == false,
		   "Output subband " << out << " multiply used");
	outDone[out] = true;
	memcpy (bufout+out*itsNrcu, bufin,
		itsNrcu * sizeof(DH_SampleC::BufferType));
      }
      bufin += itsNrcu;
    }
    // Clear the buffers for which no input subband is selected.
    for (int i=0; i<itsNbandout; i++) {
      if (! outDone[i]) {
	DH_SampleC::BufferType* buf = bufout + i*itsNrcu;
	for (int j=0; j<itsNrcu; j++) {
	  buf[j] = 0;
	}
      }
    }
    // Copy the output if multiple outputs are used.
    for (int i=1; i<getOutputs(); i++) {
      memcpy (itsOutHolders[i]->getBuffer(), bufout,
	      itsNrcu * itsNbandout * sizeof(DH_SampleC::BufferType));
    }
  }
}

void WH_Selector::dump() const
{
  cout << "WH_Selector " << getName() << " Buffers:" << endl;
  if (getOutputs() > 0) {
    cout << itsOutHolders[0]->getBuffer()[0] << ','
	 << itsOutHolders[0]->getBuffer()[itsNrcu*itsNbandout-1] << endl;
  }
}


DataHolder* WH_Selector::getInHolder (int channel)
{
  AssertStr (channel < 2,
	     "input channel too high");
  if (channel == 0) 
  {
    return &itsInHolder;
  } 
  return &itsInSel;
}
DH_SampleC* WH_Selector::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs(),
	     "output channel too high");
  return itsOutHolders[channel];
}
