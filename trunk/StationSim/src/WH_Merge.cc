//#  WH_Merge.cc:
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

#include <StationSim/WH_Merge.h>
#include <BaseSim/ParamBlock.h>
#include <Common/Debug.h>


WH_Merge::WH_Merge (const string& name,
		    unsigned int nin,
		    unsigned int nout)
: WorkHolder   (nin, nout, name, "WH_Merge"),
  itsInHolders (0),
  itsOutHolders(0)
{
  // Allocate blocks to hold pointers to input and output DH-s.
  if (nin > 0) {
    itsInHolders  = new DH_RCU* [nin];
  }
  if (nout > 0) {
    itsOutHolders = new DH_SampleR* [nout];
  }
  // Create the input DH-s.
  char str[8];
  for (unsigned int i=0; i<nin; i++) {
    sprintf (str, "%d", i);
    itsInHolders[i] = new DH_RCU (string("in_") + str);
  }
  // Create the output DH-s.
  if (nin == 0) {
    nin = 1;
  }
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_SampleR (string("out_") + str, nin, 1);
  }
}

WH_Merge::~WH_Merge()
{
  for (int i=0; i<getInputs(); i++) {
    delete itsInHolders[i];
  }
  delete [] itsInHolders;
  for (int i=0; i<getOutputs(); i++) {
    delete itsOutHolders[i];
  }
  delete [] itsOutHolders;
}

WorkHolder* WH_Merge::construct (const string& name, int ninput, int noutput,
				 const ParamBlock&)
{
  return new WH_Merge (name, ninput, noutput);
}

WH_Merge* WH_Merge::make (const string& name) const
{
  return new WH_Merge (name, getInputs(), getOutputs());
}

void WH_Merge::process()
{
  if (getOutputs() > 0) {
    DH_RCU::BufferType* buf = itsOutHolders[0]->getBuffer();
    buf[0] = 0;
    for (int i=0; i<getInputs(); i++) {
      buf[i] = itsInHolders[i]->getBuffer()[0];
    }
  }
}

void WH_Merge::dump() const
{
  cout << "WH_Merge " << getName() << " Buffer:" << endl;
  if (getOutputs() > 0) {
    cout << itsOutHolders[0]->getBuffer()[0];
    if (getInputs() > 0) {
      cout << ',' << itsOutHolders[0]->getBuffer()[getInputs()-1];
    }
    cout << endl;
  }
}


DH_RCU* WH_Merge::getInHolder (int channel)
{
  AssertStr (channel < getInputs(),
	     "input channel too high");
  return itsInHolders[channel];
}
DH_SampleR* WH_Merge::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs(),
	     "output channel too high");
  return itsOutHolders[channel];
}
