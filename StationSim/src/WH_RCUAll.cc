//#  WH_RCUAll.cc:
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

#include <StationSim/WH_RCUAll.h>
#include <BaseSim/ParamBlock.h>
#include <Common/Debug.h>


WH_RCUAll::WH_RCUAll (const string& name,
		      unsigned int nout,
		      unsigned int nrcu,
		      const string& fileName)
			
: WorkHolder   (0, nout, name, "WH_RCUAll"),
  itsOutHolders(0),
  itsNrcu      (nrcu),
  itsFileName  (fileName),
  itsFile      (fileName.c_str())
{
  AssertStr (itsFile, "Failed to open file " << fileName);
  if (nout > 0) {
    itsOutHolders = new DH_SampleR* [nout];
  }
  char str[8];
  // Create the output DH-s.
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_SampleR (string("out_") + str, nrcu, 1);
  }
}

WH_RCUAll::~WH_RCUAll()
{
  for (int i=0; i<getOutputs(); i++) {
    delete itsOutHolders[i];
  }
  delete [] itsOutHolders;
}

WorkHolder* WH_RCUAll::construct (const string& name, int ninput, int noutput,
				  const ParamBlock& params)
{
  Assert (ninput==0);
  return new WH_RCUAll (name, noutput,
			params.getInt ("nrcu", 10),
			params.getString ("rcufilename", ""));
}

WH_RCUAll* WH_RCUAll::make (const string& name) const
{
  return new WH_RCUAll (name, getOutputs(), itsNrcu, itsFileName);
}

void WH_RCUAll::process()
{
  if (getOutputs() > 0) {
    DH_SampleR::BufferType* bufout = itsOutHolders[0]->getBuffer();
    for (unsigned int i=0; i<itsNrcu; i++) {
      itsFile >> bufout[i];
      if (itsFile.eof()) {
	itsFile.close();
	itsFile.open (itsFileName.c_str());
	itsFile >> bufout[i];
	AssertStr (!itsFile.eof(), "File " << itsFileName << " is empty");
      }
      AssertStr (!itsFile.fail(), "File " << itsFileName
		 << " has wrong format");
    }
    // Copy data to other output buffers
    for (int i=0; i<getOutputs(); i++) {
      memcpy (getOutHolder(i)->getBuffer(), bufout,
	      itsNrcu * sizeof(DH_SampleR::BufferType));
    }
  }
}

void WH_RCUAll::dump() const
{
  cout << "WH_RCUAll " << getName() << " Buffer:" << endl;
  if (getOutputs() > 0) {
    cout << itsOutHolders[0]->getBuffer()[0];
    if (getInputs() > 0) {
      cout << ',' << itsOutHolders[0]->getBuffer()[itsNrcu-1];
    }
    cout << endl;
  }
}


DataHolder* WH_RCUAll::getInHolder (int channel)
{
  AssertStr (channel < 0,
	     "input channel too high");
  return 0;
}
DH_SampleR* WH_RCUAll::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs(),
	     "output channel too high");
  return itsOutHolders[channel];
}
