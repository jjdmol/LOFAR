//  WH_GrowSize.cc:
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
//  $Log$
//
//////////////////////////////////////////////////////////////////////

#include "WH_GrowSize.h"
#include "Step.h"
#include "firewalls.h"
#include <stdio.h>             // for sprintf


WH_GrowSize::WH_GrowSize (const string& name, bool first,
			unsigned int nin, unsigned int nout,
			unsigned int nbuffer)
: WorkHolder    (nin, nout, name),
  itsInHolders  (0),
  itsOutHolders (0),
  itsBufLength  (nbuffer),
  itsFirst      (first)
{
  Firewall::Assert (nin > 0,
		    __HERE__,
		    "0 input DH_IntArray is not possible");
  Firewall::Assert (nout > 0,
		    __HERE__,
		    "0 output DH_IntArray is not possible");
  Firewall::Assert (nout == nin,
		    __HERE__,
		    "number of inputs and outputs must match");
  itsInHolders  = new DH_GrowSize* [nin];
  itsOutHolders = new DH_GrowSize* [nout];
  char str[8];
  for (unsigned int i=0; i<nin; i++) {
    sprintf (str, "%d", i);
    itsInHolders[i] = new DH_GrowSize (std::string("in_") + str, nbuffer);
  }
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_GrowSize (std::string("out_") + str, nbuffer);
  }
}


WH_GrowSize::~WH_GrowSize()
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

void WH_GrowSize::process()
{
  static bool firstCall = true;
  cout << getName() << endl;

#if 0
  // for all inputs take each input element and put the square
  // of the input element in the output element
  for (int i=0; i<getInputs(); i++)
  {
    DH_GrowSize::BufferType* inbuf  = itsInHolders[i]->getBuffer();
    DH_GrowSize::BufferType* outbuf = itsOutHolders[i]->getBuffer();

    cout << "Processing Data! "
	 << inbuf[0] << ',' << inbuf[itsBufLength-1] << "   --->  ";

    for (int j=0; j<itsBufLength; j++)
    {
      if (itsFirst) {
	inbuf[j] = j + 1;
      }
      outbuf[j] = inbuf[j] + 1; //* 2; //inbuf[j];

      cout << endl << "outbuf[" << j << "] = " << outbuf[j] << endl;
    }

    cout << outbuf[0] << ',' << outbuf[itsBufLength-1] << endl;
  }
#else
  if (false == firstCall)
  {
    // for all inputs take each input element and put the square
    // of the input element in the output element
    for (int i=0; i<getInputs(); i++)
    {
      DH_GrowSize::BufferType* inbuf  = itsInHolders[i]->getBuffer();
      DH_GrowSize::BufferType* outbuf = itsOutHolders[i]->getBuffer();
      
      if (itsInHolders[i]->increaseSize(2))
      {
	cout << "Increasing size for itsInHolders " << i << " to "
	     << itsInHolders[i]->getDataPacketSize() << endl;
      }
      if (itsOutHolders[i]->increaseSize(2))
      {
	cout << "Increasing size for itsOutHolders " << i << " to "
	     << itsOutHolders[i]->getDataPacketSize() << endl;
      }
    }
  }
  else
  {
    firstCall = false;
  }
#endif
}

void WH_GrowSize::dump() const
{
#if 0
  cout << "WH_GrowSize " << getName() << " Buffers:" << endl;
  for (int i=0; i<getOutputs(); i++)
  {
    cout << "Buf = [ ";
    for (int j=0; j<itsBufLength; j++)
    {
      cout << itsOutHolders[i]->getBuffer()[j];
      
      if (j < itsBufLength - 1) cout << ",";
    }
    cout << " ]" << endl;
  }
#endif
}

DH_GrowSize* WH_GrowSize::getInHolder (int channel)
{
  Firewall::Assert (channel < getInputs(),
		    __HERE__,
		    "input channel too high");
  return itsInHolders[channel];
}
DH_GrowSize* WH_GrowSize::getOutHolder (int channel)
{
  Firewall::Assert (channel < getOutputs(),
		    __HERE__,
		    "output channel too high");
  return itsOutHolders[channel];
}
