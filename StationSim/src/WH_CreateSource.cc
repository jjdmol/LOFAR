//  WH_CreateSource.cc:
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
//

#include <Common/Debug.h>
#include <BaseSim/ParamBlock.h>
#include <DataGen/WH_CreateSource.h>
#include <stdio.h>		// for sprintf


WH_CreateSource::WH_CreateSource (const string& name, 
								  unsigned int nin, 
								  unsigned int nout)
: WorkHolder    (nin, nout, name, "WH_CreateSource"), 
  itsInHolders  (0),
  itsOutHolders (0)
{
  // Allocate blocks to hold pointers to input and output DH-s.
  if (nin > 0) {
    itsInHolders = new DH_SampleR* [nin];
  }
  if (nout > 0) {
    itsOutHolders = new DH_SampleR* [nout];
  }
  // Create the input DH-s.
  char str[8];

  for (unsigned int i = 0; i < nin; i++) {
    sprintf (str, "%d", i);
    itsInHolders[i] = new DH_SampleR (string ("in_") + str, 1, 1);
  }
  // Create the output DH-s.
  if (nin == 0) {
    nin = 1;
  }
  for (unsigned int i = 0; i < nout; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_SampleR (string ("out_") + str, 1, 1);
  }
}

WH_CreateSource::~WH_CreateSource ()
{
  for (int i = 0; i < getInputs (); i++) {
    delete itsInHolders[i];
  }
  delete[]itsInHolders;
  for (int i = 0; i < getOutputs (); i++) {
    delete itsOutHolders[i];
  }
  delete[]itsOutHolders;
}

WorkHolder* WH_CreateSource::construct (const string& name, 
										int ninput,
										int noutput, 
										const ParamBlock&)
{
  return new WH_CreateSource (name, ninput, noutput);
}

WH_CreateSource* WH_CreateSource::make (const string & name) const
{
  return new WH_CreateSource (name, getInputs (), getOutputs ());
}

void WH_CreateSource::process ()
{
  if (getOutputs () > 0) {
    DH_SampleR::BufferType* buf = itsOutHolders[0]->getBuffer ();
    buf[0] = 0;

    for (int i = 0; i < getInputs (); i++) {
      buf[0] += itsInHolders[i]->getBuffer ()[0];
    }
  }
}

void WH_CreateSource::dump () const
{
}


DH_SampleR* WH_CreateSource::getInHolder (int channel)
{
  AssertStr (channel < getInputs (), "input channel too high");
  return itsInHolders[channel];
}

DH_SampleR* WH_CreateSource::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs (), "output channel too high");
  return itsOutHolders[channel];
}
