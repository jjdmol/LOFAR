//  WH_ReadSignal.cc:
//
//  Copyright (C) 2002
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

#include <Common/Debug.h>
#include <BaseSim/ParamBlock.h>
#include <StationSim/WH_ReadSignal.h>
#include <stdio.h>		// for sprintf


WH_ReadSignal::WH_ReadSignal (const string & name, unsigned int nout, 
							  const string & fileName)  
: WorkHolder        (0, nout, name, "WH_ReadSignal"),
  itsOutHolders     (0),
  itsFileName       (fileName), 
  itsFile           (fileName.c_str ())
{
  AssertStr (itsFile, "Failed to open file " << fileName);
  // Set the file pointer to the begining of the data
  char s[256];
  strcpy (s, "Empty");

  while (strncmp (s, "Data :", 6) != 0)
    itsFile.getline (s, 256);

  if (nout > 0) {
    itsOutHolders = new DH_SampleR*[nout];
  }

  char str[8];

  for (unsigned int i = 0; i < nout; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_SampleR (string ("out_") + str, 1, 1);
  }
}

WH_ReadSignal::~WH_ReadSignal ()
{
  for (int i = 0; i < getOutputs (); i++) {
    delete itsOutHolders[i];
  }
  delete[]itsOutHolders;
}

WorkHolder* WH_ReadSignal::construct (const string& name,
									  int ninput, 
									  int noutput,
									  const ParamBlock& params)
{
  Assert (ninput == 0);
  return new WH_ReadSignal (name, 
							noutput,
							params.getString ("rcufilename", ""));
}

WH_ReadSignal* WH_ReadSignal::make (const string & name) const
{
  return new WH_ReadSignal (name, getOutputs (), itsFileName);
}

void WH_ReadSignal::process ()
{
  double sample;

  if (!itsFile.eof ()) {
    itsFile >> sample;
  } else {
	char s[256];
	itsFile.seekg (0);

	while (strncmp (s, "Data :", 6) != 0)
	  itsFile.getline (s, 256);
  
    itsFile >> sample;
  }

  for (int i = 0; i < getOutputs (); i++) {
    itsOutHolders[i]->getBuffer ()[0] = sample;
  }
}

void WH_ReadSignal::dump () const
{
  cout << "WH_ReadSignal " << getName () << " Buffers:" << endl;
  cout << itsOutHolders[0]->getBuffer ()[0] << ','
	   << itsOutHolders[getOutputs () - 1]->getBuffer ()[0] << endl;
}


DataHolder* WH_ReadSignal::getInHolder (int channel)
{
  AssertStr (channel < 0, "input channel too high");
  return 0;
}

DH_SampleR* WH_ReadSignal::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs (), "output channel too high");
  return itsOutHolders[channel];
}
