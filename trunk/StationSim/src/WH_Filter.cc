//  WH_Filter.cc:
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
//  $Log$
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>             // for sprintf

#include "StationSim/WH_Filter.h"
#include "BaseSim/ParamBlock.h"
#include "Common/Debug.h"


WH_Filter::WH_Filter (const string& name,
		      unsigned int nout,
		      unsigned int nrcu, unsigned int nsubband,
		      const string& coeffFileName)
: WorkHolder   (1, nout, name, "WH_Filter"),
  itsInHolder  ("in", nrcu, 1),
  itsOutHolders(0),
  itsNrcu      (nrcu),
  itsNsubband  (nsubband),
  itsCoeffName (coeffFileName),
  itsBuffer    (0),
  itsBufPos    (0)
{
  if (nout > 0) {
    itsOutHolders = new DH_Sample* [nout];
  }
  char str[8];
  // Create the output DH-s.
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_Sample (string("out_") + str, nrcu, nsubband);
  }
}

WH_Filter::~WH_Filter()
{
  for (int i=0; i<getOutputs(); i++) {
    delete itsOutHolders[i];
  }
  delete [] itsOutHolders;
  if (itsBuffer) {
    for (int i=0; i<itsNrcu; i++) {
      delete [] itsBuffer[i];
    }
    delete [] itsBuffer;
  }
}

WorkHolder* WH_Filter::construct (const string& name, int ninput, int noutput,
				  const ParamBlock& params)
{
  Assert (ninput==1);
  return new WH_Filter (name, noutput,
			params.getInt ("nrcu", 10),
			params.getInt ("nsubband", 10),
			params.getString ("coeffname", "filter.coeff"));
}

WH_Filter* WH_Filter::make (const string& name) const
{
  return new WH_Filter (name, getOutputs(), itsNrcu, itsNsubband,
			itsCoeffName);
}

void WH_Filter::preprocess()
{
  // If first time, read the filter coefficients and allocate the buffer.
  if (itsBuffer == 0) {
    float val;
    ifstream coeffFile (itsCoeffName.c_str());
    AssertStr (coeffFile, "Coeff.file " << itsCoeffName << " not found");
    itsCoeff.reserve(100);
    itsNcoeff = 0;
    while (true) {
      coeffFile >> val;
      if (coeffFile.eof()) {
	break;
      }
      AssertStr (!coeffFile.fail(),
		 "Coeff.file " << itsCoeffName << " has wrong format");
      itsCoeff.push_back (val);
      itsNcoeff++;
    }
    coeffFile.close();
    // Allocate a circular buffer per rcu to be used by process.
    // Its principal size is the nr of coefficients.
    // Initialize the buffers.
    itsBuffer = new DH_Sample::BufferType* [itsNrcu];
    for (int i=0; i<itsNrcu; i++) {
      itsBuffer[i] = new DH_Sample::BufferType [itsNcoeff];
      for (int j=0; j<itsNcoeff; j++) {
	itsBuffer[i][j] = 0;
      }
    }
  }
  // Do the standard preprocess as well.
  WorkHolder::preprocess();
}

void WH_Filter::process()
{
  if (getOutputs() > 0) {
    DH_Sample::BufferType* bufin = itsInHolder.getBuffer();
    // Keep the received data per RCU in a circular buffer.
    if (itsBufPos == itsNcoeff) {
      itsBufPos = 0;
    }
    for (int i=0; i<itsNrcu; i++) {
      itsBuffer[i][itsBufPos] = bufin[i];
    }
    itsBufPos++;
    // Calculate the filter output. This only needs to be done if the filtered
    // sample is not discarded in the process of decimation. 
    // Decimation is controlled by Step::setOutRate() and checked by
    // DataHolder::doHandle()
    if (itsOutHolders[0]->doHandle()) {
      DH_Sample::BufferType* bufout = itsOutHolders[0]->getBuffer();
      for (int i=0; i<itsNrcu; i++) {
	bufout[i] = 0;
	int n1 = itsNcoeff - itsBufPos;
	int bufpos = itsBufPos;
	for (int j=0; j<n1; j++) {
	  bufout[i] += itsBuffer[i][bufpos++] * itsCoeff[j];
	}
	bufpos = 0;
	for (int j=n1; j<itsNcoeff; j++) {
	  bufout[i] += itsBuffer[i][bufpos++] * itsCoeff[j];
	}
      }
      for (int i=1; i<getOutputs(); i++) {
	memcpy (getOutHolder(i)->getBuffer(), bufout,
		itsNrcu *itsNsubband * sizeof(DH_Sample::BufferType));
      }
    }
  }
}

void WH_Filter::dump() const
{
  cout << "WH_Filter " << getName() << " Buffer:" << endl;
  if (getOutputs() > 0) {
    cout << itsOutHolders[0]->getBuffer()[0] << ','
	 << itsOutHolders[0]->getBuffer()[itsNrcu*itsNsubband-1] << endl;
  }
}


DH_Sample* WH_Filter::getInHolder (int channel)
{
  AssertStr (channel == 0,
	     "input channel too high");
  return &itsInHolder;
}
DH_Sample* WH_Filter::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs(),
	     "output channel too high");
  return itsOutHolders[channel];
}
