//#  WH_DataReader.cc:
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

#include <StationSim/WH_DataReader.h>
#include <BaseSim/ParamBlock.h>
#include <Common/Debug.h>


WH_DataReader::WH_DataReader (const string& name,
							  unsigned int nrcu,
							  const string& fileName)
: WorkHolder    (0, nrcu, name, "WH_DataReader"),
  itsOutHolders (0),
  itsFileName   (fileName),
  itsFile       (fileName.c_str(), ios::binary),
  itsCount      (0)
{
  AssertStr (itsFile, "Failed to open file " << fileName);
  if (nrcu > 0) {
    itsOutHolders = new DH_SampleC* [nrcu];
  }
  char str[8];
  for (unsigned int i = 0; i < nrcu; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_SampleC (string("out_") + str, 1, 1);
  }
}

WH_DataReader::~WH_DataReader()
{
  for (int i = 0; i < getOutputs(); i++) {
    delete itsOutHolders[i];
  }
  delete [] itsOutHolders;
}

WorkHolder* WH_DataReader::construct (const string& name,
									  int ninput, int noutput,
									  const ParamBlock& params)
{
  Assert (ninput == 0);
  return new WH_DataReader (name, noutput,
							params.getString ("rcufilename", ""));
}

WH_DataReader* WH_DataReader::make (const string& name) const
{
  return new WH_DataReader (name, getOutputs(), itsFileName);
}

void WH_DataReader::process()
{
  short int sample;

//   if (itsCount == 0) {
	if (itsFile.eof ()) {
	  itsFile.close ();
	  itsFile.open (itsFileName.c_str ());	
	  AssertStr (!itsFile.eof (), "File " << itsFileName << " is empty");
	}
// 	itsFile.read(itsBuffer, BUFFER_SIZE);
//   }

//   sample = itsBuffer[itsCount];
//   itsCount = itsCount + 1 % BUFFER_SIZE;

	itsFile.read(&sample, 2);

  cout << sample << endl;

  for (int i = 0; i < getOutputs (); i++) {
	itsOutHolders[i]->getBuffer ()[0] = sample;
  }
}

void WH_DataReader::dump() const
{
  cout << "WH_DataReader " << getName() << " Buffers:" << endl;
  cout << itsOutHolders[0]->getBuffer()[0] << ','
       << itsOutHolders[getOutputs()-1]->getBuffer()[0] << endl;
}


DataHolder* WH_DataReader::getInHolder (int channel)
{
  AssertStr (channel < 0, "input channel too high");
  return 0;
}
DH_SampleC* WH_DataReader::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs(), "output channel too high");
  return itsOutHolders[channel];
}
