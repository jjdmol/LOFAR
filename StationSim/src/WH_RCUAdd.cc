//#  WH_RCUAdd.cc:
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

#include <StationSim/WH_RCUAdd.h>
#include <BaseSim/ParamBlock.h>
#include <Common/Debug.h>


WH_RCUAdd::WH_RCUAdd (const string& name,
		      unsigned int nin,
		      unsigned int nout)
: WorkHolder   (nin, 1, name,"WH_RCUAdd"),
  itsInHolders (0),
  itsOutHolders(0)
{
  if (nin > 0) {
    itsInHolders  = new DH_RCU* [nin];
  }
  if (nout > 0) {
    itsOutHolders = new DH_RCU* [nout];
  }
  char str[8];
  // Create the input DH-s.
  for (unsigned int i=0; i<nin; i++) {
    sprintf (str, "%d", i);
    itsInHolders[i] = new DH_RCU (string("in_") + str);
  }
  // Create the output DH-s.
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_RCU (string("out_") + str);
  }
}

WH_RCUAdd::~WH_RCUAdd()
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

WorkHolder* WH_RCUAdd::construct (const string& name, int ninput, int noutput,
				   const ParamBlock&)
{
  return new WH_RCUAdd (name, ninput, noutput);
}

WH_RCUAdd* WH_RCUAdd::make (const string& name) const
{
  return new WH_RCUAdd (name, getInputs(), getOutputs());
}

void WH_RCUAdd::process()
{
  if (getOutputs() > 0) {
    DH_RCU::BufferType* buf = itsOutHolders[0]->getBuffer();
    if (getInputs() == 0) {
      buf[0] = 0;
    } else {
      buf[0] = itsInHolders[0]->getBuffer()[0];
      for (int i=1; i<getInputs(); i++) {
	buf[0] += itsInHolders[i]->getBuffer()[0];
      }
    }
    for (int i=1; i<getOutputs(); i++) {
      itsOutHolders[i]->getBuffer()[0] = buf[0];
    }
  }
}

void WH_RCUAdd::dump() const
{
  cout << "WH_RCUAdd " << getName() << " Buffer:" << endl;
  if (getOutputs() > 0) {
    cout << itsOutHolders[0]->getBuffer()[0] << endl;
  }
}


DH_RCU* WH_RCUAdd::getInHolder (int channel)
{
  AssertStr (channel < getInputs(),
	     "input channel too high");
  return itsInHolders[channel];
}
DH_RCU* WH_RCUAdd::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs(),
	     "output channel too high");
  return itsOutHolders[channel];
}
