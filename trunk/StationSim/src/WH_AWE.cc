//#  WH_AWE.cc:
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

#include <StationSim/WH_AWE.h>
#include <BaseSim/ParamBlock.h>
#include <Common/Debug.h>
#include <Common/lofar_vector.h>


WH_AWE::WH_AWE (const string& name,
		unsigned int nout,
		unsigned int nsubband,
		unsigned int nbeam)
: WorkHolder    (1, nout, name, "WH_AWE"),
  itsInHolder   ("in", nbeam, nsubband),
  itsOutHolders (0),
  itsNsubband   (nsubband),
  itsNbeam      (nbeam)
{
  if (nout > 0) {
    itsOutHolders = new DH_Weight* [nout];
  }
  char str[8];
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_Weight (string("out_") + str, nbeam);
  }
}

WH_AWE::~WH_AWE()
{
  for (int i=0; i<getOutputs(); i++) {
    delete itsOutHolders[i];
  }
  delete [] itsOutHolders;
}

WorkHolder* WH_AWE::construct (const string& name,
			       int ninput, int noutput,
			       const ParamBlock& params)
{
  Assert (ninput == 1);
  return new WH_AWE (name, noutput,
		     params.getInt ("nsubband", 10),
		     params.getInt ("nbeam", 10));
}

WH_AWE* WH_AWE::make (const string& name) const
{
  return new WH_AWE (name, getOutputs(),
		     itsNsubband, itsNbeam);
}

void WH_AWE::process()
{
  if (getOutputs() > 0) {
    DH_SampleC::BufferType* bufin = itsInHolder.getBuffer();
    DH_Weight::BufferType* bufout = itsOutHolders[0]->getBuffer();
    for (int i=0; i<itsNbeam; i++) {
      bufout[i] += 1;
    }
    // Copy the output if multiple outputs are used.
    for (int i=1; i<getOutputs(); i++) {
      memcpy (itsOutHolders[i]->getBuffer(), bufout,
	      itsNbeam * sizeof(DH_Weight::BufferType));
    }
  }
}

void WH_AWE::dump() const
{
  cout << "WH_AWE " << getName() << " Buffers:" << endl;
  if (getOutputs() > 0) {
    cout << itsOutHolders[0]->getBuffer()[0] << ','
	 << itsOutHolders[0]->getBuffer()[itsNbeam-1] << endl;
  }
}


DH_SampleC* WH_AWE::getInHolder (int channel)
{
  AssertStr (channel < 1,
	     "input channel too high");
  return &itsInHolder;
}
DH_Weight* WH_AWE::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs(),
	     "output channel too high");
  return itsOutHolders[channel];
}
