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
		      const string& fileName,
		      bool multiFile)
			
: WorkHolder   (0, nout, name, "WH_RCUAll"),
  itsOutHolders(0),
  itsNrcu      (nrcu),
  itsFileName  (fileName),
  itsMultiFile (multiFile),
  itsFile      (0)
{
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
  delete [] itsFile;
}

WorkHolder* WH_RCUAll::construct (const string& name, int ninput, int noutput,
				  const ParamBlock& params)
{
  Assert (ninput==0);
  return new WH_RCUAll (name, noutput,
			params.getInt ("nrcu", 10),
			params.getString ("rcufilename", ""),
			params.getBool ("rcumultifile", true));
}

WH_RCUAll* WH_RCUAll::make (const string& name) const
{
  return new WH_RCUAll (name, getOutputs(), itsNrcu,
			itsFileName, itsMultiFile);
}

void WH_RCUAll::preprocess()
{
  postprocess();
  if (itsMultiFile) {
    itsFile = new ifstream [itsNrcu];
    unsigned int pos = itsFileName.find ('*');
    AssertStr (pos != string::npos, "Multi file name " << itsFileName
	       << " does not contain an *");
    char str[8];
    for (unsigned int i=0; i<itsNrcu; i++) {
      sprintf (str, "%d", i);
      string name (itsFileName);
      name.replace (pos, 1, str);
      itsFile[i].open (name.c_str());
      AssertStr (itsFile[i], "File " << name << " could not be opened");
    }
  } else {
    itsFile = new ifstream [1];
    itsFile[0].open (itsFileName.c_str());
    AssertStr (itsFile[0], "File " << itsFileName << " could not be opened");
  }
}

void WH_RCUAll::process()
{
  if (getOutputs() > 0) {
    DH_SampleR::BufferType* bufout = itsOutHolders[0]->getBuffer();
    for (unsigned int i=0; i<itsNrcu; i++) {
      if (itsMultiFile) {
	short val;
	itsFile[i].read ((char*)(&val), 2);
	AssertStr (itsFile[i], "File " << itsFileName << " at end");
	bufout[i] = double(val) / 2048;              // 11 significant bits
      } else {
	itsFile[0] >> bufout[i];
	if (itsFile[0].eof()) {
	  itsFile[0].close();
	  itsFile[0].open (itsFileName.c_str());
	  itsFile[0] >> bufout[i];
	  AssertStr (!itsFile[0].eof(), "File " << itsFileName << " is empty");
	}
	AssertStr (!itsFile[0].fail(), "File " << itsFileName
		   << " has wrong format");
      }
    }
    // Copy data to other output buffers
    for (int i=0; i<getOutputs(); i++) {
      memcpy (getOutHolder(i)->getBuffer(), bufout,
	      itsNrcu * sizeof(DH_SampleR::BufferType));
    }
  }
}

void WH_RCUAll::postprocess()
{
  delete [] itsFile;
  itsFile = 0;
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
