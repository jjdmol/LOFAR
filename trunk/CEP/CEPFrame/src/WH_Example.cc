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
//  $Log$
//  Revision 1.17  2002/05/03 11:21:32  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.16  2002/03/15 13:28:09  gvd
//  Added construct function to WH classes (for XML parser)
//  Added getX functions to ParamBlock
//  Added SAX classes for XML parser
//  Improved testing scripts (added .run)
//
//  Revision 1.15  2002/03/14 14:25:27  wierenga
//  system includes before local includes
//
//  Revision 1.14  2002/03/01 08:27:57  gvd
//  Replaced firewall by Debug and changed code accordingly
//  Added lofar_*.h for correct use of namespaces (for KAI and Intel C++)
//
//  Revision 1.13  2002/01/02 14:02:00  rdam
//  Made getType() return class name
//
//  Revision 1.12  2001/10/19 06:01:46  gvd
//  Added checkConnections
//  Cleaned up Transport and StepRep classes
//
//  Revision 1.11  2001/09/24 14:04:09  gvd
//  Added preprocess and postprocess functions
//
//  Revision 1.10  2001/09/21 12:19:02  gvd
//  Added make functions to WH classes to fix memory leaks
//
//  Revision 1.9  2001/03/23 10:00:40  gvd
//  Improved documentation and test programs
//  Added clearEventCount function to Step
//
//  Revision 1.8  2001/03/01 13:15:47  gvd
//  Added type argument in DataHolder constructor which is used in
//  the connect functions to check if the DH types match
//  Improved the simulator parser
//  Improved documentation
//
//  Revision 1.7  2001/02/05 14:53:05  loose
//  Added GPL headers
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>             // for sprintf

#include "BaseSim/WH_Example.h"
#include "BaseSim/Step.h"
#include "BaseSim/ParamBlock.h"
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
