//#  WH_RCU.cc:
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

#include <StationSim/WH_RCU.h>
#include <BaseSim/ParamBlock.h>
#include <Common/Debug.h>


WH_RCU::WH_RCU (const string& name,
		unsigned int nrcu,
		const string& fileName)
: WorkHolder    (0, nrcu, name, "WH_RCU"),
  itsOutHolders (0),
  itsFileName   (fileName),
  itsFile       (fileName.c_str())
{
  AssertStr (itsFile, "Failed to open file " << fileName);
  if (nrcu > 0) {
    itsOutHolders = new DH_RCU* [nrcu];
  }
  char str[8];
  for (unsigned int i=0; i<nrcu; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_RCU (string("out_") + str);
  }
}

WH_RCU::~WH_RCU()
{
  for (int i=0; i<getOutputs(); i++) {
    delete itsOutHolders[i];
  }
  delete [] itsOutHolders;
}

WorkHolder* WH_RCU::construct (const string& name,
			       int ninput, int noutput,
			       const ParamBlock& params)
{
  Assert (ninput == 0);
  return new WH_RCU (name, noutput,
		     params.getString ("rcufilename", ""));
}

WH_RCU* WH_RCU::make (const string& name) const
{
  return new WH_RCU (name, getOutputs(), itsFileName);
}

void WH_RCU::process()
{
  for (int i=0; i<getOutputs(); i++) {
    itsFile >> itsOutHolders[i]->getBuffer()[0];
    if (itsFile.eof()) {
      itsFile.close();
      itsFile.open (itsFileName.c_str());
      itsFile >> itsOutHolders[i]->getBuffer()[0];
      AssertStr (!itsFile.eof(), "File " << itsFileName << " is empty");
    }
    AssertStr (!itsFile.fail(), "File " << itsFileName << " has wrong format");
  }
}

void WH_RCU::dump() const
{
  cout << "WH_RCU " << getName() << " Buffers:" << endl;
  cout << itsOutHolders[0]->getBuffer()[0] << ','
       << itsOutHolders[getOutputs()-1]->getBuffer()[0] << endl;
}


DataHolder* WH_RCU::getInHolder (int channel)
{
  AssertStr (channel < 0,
	     "input channel too high");
  return 0;
}
DH_RCU* WH_RCU::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs(),
	     "output channel too high");
  return itsOutHolders[channel];
}
