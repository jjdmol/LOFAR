//  WH_AddSignals.cc:
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
#include <StationSim/WH_AddSignals.h>
#include <stdio.h>		// for sprintf


WH_AddSignals::WH_AddSignals (const string& name, 
							  unsigned int nin,
							  unsigned int nout,
							  unsigned int nrcu)
: WorkHolder    (nin, nout, name, "WH_AddSignals"),
  itsInHolders  (0),
  itsOutHolders (0),
  itsNrcu       (nrcu)
{
  // Allocate blocks to hold pointers to input and output DH-s.
  if (nin > 0) {
    itsInHolders = new DH_SampleC *[nin];
  }
  if (nout > 0) {
    itsOutHolders = new DH_SampleC *[nout];
  }
  // Create the input DH-s.
  char str[8];

  for (unsigned int i = 0; i < nin; i++) {
    sprintf (str, "%d", i);
    itsInHolders[i] = new DH_SampleC (string ("in_") + str, nrcu, 1);
  }
  // Create the output DH-s.
  if (nin == 0) {
    nin = 1;
  }
  for (unsigned int i = 0; i < nout; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_SampleC (string ("out_") + str, 1, 1);
  }
//   // DEBUG
//   itsCount = 0;
//   itsFileOutReal.open ("/home/alex/data/add_real.txt");
//   itsFileOutComplex.open ("/home/alex/data/add_complex.txt");
//   itsFileOutReal.precision(20);
//   itsFileOutComplex.precision(20);
}

WH_AddSignals::~WH_AddSignals ()
{
  for (int i = 0; i < getInputs (); i++) {
    delete itsInHolders[i];
  }
  delete[]itsInHolders;
  for (int i = 0; i < getOutputs (); i++) {
    delete itsOutHolders[i];
  }
  delete[]itsOutHolders;
//   itsFileOutReal.close ();
//   itsFileOutComplex.close ();
}

WorkHolder* WH_AddSignals::construct (const string& name, 
									  int ninput,
									  int noutput, 
									  const ParamBlock&)
{
  return new WH_AddSignals (name, ninput, noutput, 1);
}

WH_AddSignals* WH_AddSignals::make (const string& name) const
{
  return new WH_AddSignals (name, getInputs (), getOutputs (), itsNrcu);
}

void WH_AddSignals::process ()
{
  if (getOutputs () > 0) {
    for (int i = 0; i < itsNrcu; i++) {
      itsOutHolders[i]->getBuffer ()[0] = 0;
    }

    for (int i = 0; i < getInputs (); i++) {
      for (int j = 0; j < itsNrcu; j++) {
	itsOutHolders[j]->getBuffer ()[0] += itsInHolders[i]->getBuffer ()[j];
      }
    }
	
// 	// DEBUG
//     for (int i = 0; i < itsNrcu && real (itsOutHolders[i]->getBuffer ()[0]) != 0; i++) {
//       itsFileOutReal << real (itsOutHolders[i]->getBuffer ()[0]) << " ";
// 	  itsFileOutComplex << imag (itsOutHolders[i]->getBuffer ()[0]) << " ";
// 	}
//     itsFileOutReal << endl;
//     itsFileOutComplex << endl;

	//cout << itsCount++ << endl;
  }
}

void WH_AddSignals::dump () const
{
  cout << "WH_AddSignals " << getName () << " Buffer:" << endl;
  if (getOutputs () > 0) {
    cout << itsOutHolders[0]->getBuffer ()[0];
    if (getInputs () > 0) {
      cout << ',' << itsOutHolders[0]->getBuffer ()[getInputs () - 1];
    }
    cout << endl;
  }
}


DH_SampleC* WH_AddSignals::getInHolder (int channel)
{
  AssertStr (channel < getInputs (), "input channel too high");
  return itsInHolders[channel];
}

DH_SampleC* WH_AddSignals::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs (), "output channel too high");
  return itsOutHolders[channel];
}
