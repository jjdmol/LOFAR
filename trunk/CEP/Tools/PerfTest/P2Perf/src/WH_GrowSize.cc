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
//  Revision 1.3  2001/09/19 08:10:51  wierenga
//  Changes to do perform bandwidth tests.
//
//  Revision 1.2  2001/09/19 08:00:13  wierenga
//  Added code to do performance tests.
//
//  Revision 1.1  2001/08/16 15:14:23  wierenga
//  Implement GrowSize DH and WH for performance measurements. Timing code still needs to be added.
//
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>             // for sprintf
#include <math.h>

#include "Step.h"
#include "firewalls.h"

#include "WH_GrowSize.h"
#include "StopWatch.h"
#include "SeqSim.h"

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

  iteration = 0;
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

WorkHolder* WH_GrowSize::make(const string& name) const
{
  return new WH_GrowSize(name, itsFirst, getInputs(), getOutputs(), itsBufLength);
}

void WH_GrowSize::process()
{
  if (!strncmp("GrowSize[1]", getName().c_str(), 11))
  {
    if (iteration > 0)
    {
      watch.stop();
      printf("%d %g\n",
	     itsInHolders[0]->getDataPacketSize(),
	     (itsInHolders[0]->getDataPacketSize() / (1024.0 * 1024.0)) / watch.elapsed());
    }
    watch.start();
  }

  if (!strncmp("GrowSize[1]", getName().c_str(), 11)
      || (iteration > 0))
  {
    if (iteration % 5 == 0)
    {
      for (int i=0; i<getInputs(); i++)
      {
	(void)itsInHolders[i]->increaseSize(exp(log(MAX_GROW_SIZE)/1000));
	(void)itsOutHolders[i]->increaseSize(exp(log(MAX_GROW_SIZE)/1000));
      }
    }
  }

  iteration++;
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
