//  WH_Example.cc:
//
//  Copyright (C) 2000, 2001
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
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>             // for sprintf

#include "CEPFrame/WH_Example.h"
#include "CEPFrame/Step.h"
#include "CEPFrame/ParamBlock.h"
#include "Common/Debug.h"

WH_Example::WH_Example (const string& name,
			unsigned int nin, unsigned int nout,
			unsigned int nbuffer)
: WorkHolder    (nin, nout, name,"WH_Example"),
  itsInHolders  (0),
  itsOutHolders (0),
  itsBufLength  (nbuffer)
{
  if (nin > 0) {
    itsInHolders  = new DH_Example* [nin];
  }
  if (nout > 0) {
    itsOutHolders = new DH_Example* [nout];
  }
  char str[8];
  for (unsigned int i=0; i<nin; i++) {
    sprintf (str, "%d", i);
    itsInHolders[i] = new DH_Example (string("in_") + str, nbuffer);
  }
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_Example (string("out_") + str, nbuffer);
  }
}

WH_Example::~WH_Example()
{
  for (int i=0; i<getInputs(); i++) {
    delete itsInHolders[i];
  }
  for (int i=0; i<getOutputs(); i++) {
    delete itsOutHolders[i];
  }
  delete [] itsInHolders;
  delete [] itsOutHolders;
}

WorkHolder* WH_Example::construct (const string& name, int ninput, int noutput,
				   const ParamBlock& params)
{
  return new WH_Example (name, ninput, noutput,
			 params.getInt ("nbuffer", 10));
}

WH_Example* WH_Example::make (const string& name) const
{
  return new WH_Example (name, getInputs(), getOutputs(),
			 itsBufLength);
}

void WH_Example::process()
{
  cout << getName() << endl;
  DH_Example::BufferType* buf = itsOutHolders[0]->getBuffer();
  for (int j=0; j<itsBufLength; j++) {
    buf[j] = 0;
  }
  cout << "Processing Data! "
       << buf[0] << ',' << buf[itsBufLength-1] << "   --->  ";
  const complex<float> cp1(1,0);
  if (getInputs() == 0) {
    for (int j=0; j<itsBufLength; j++) {
      buf[j] = cp1 + DH_Example::BufferType (Step::getEventCount());
    }
  } else {
    for (int i=0; i<getInputs(); i++) {
      DH_Example::BufferType* curbuf = itsInHolders[i]->getBuffer();
      for (int j=0; j<itsBufLength; j++) {
	buf[j] += curbuf[j] + cp1;
      }
    }
  }
  for (int i=1; i<getOutputs(); i++) {
    DH_Example::BufferType* curbuf = itsOutHolders[i]->getBuffer();
    for (int j=0; j<itsBufLength; j++) {
      curbuf[j] = buf[j];
    }
  }
  cout << buf[0] << ',' << buf[itsBufLength-1] << endl;
}

void WH_Example::dump() const
{
  cout << "WH_Example " << getName() << " Buffers:" << endl;
  for (int i=0; i<getOutputs(); i++) {
    cout << itsOutHolders[i]->getBuffer()[0] << ','
	 << itsOutHolders[i]->getBuffer()[itsBufLength-1] << endl;
  }
}


DH_Example* WH_Example::getInHolder (int channel)
{
  AssertStr (channel < getInputs(),
	     "input channel too high");
  return itsInHolders[channel];
}
DH_Example* WH_Example::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs(),
	     "output channel too high");
  return itsOutHolders[channel];
}
