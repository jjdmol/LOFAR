//#  WH_Detection.cc:
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

#include <StationSim/WH_Detection.h>
#include <BaseSim/ParamBlock.h>
#include <Common/Debug.h>

#include <aips/Arrays/ArrayMath.h>

WH_Detection::WH_Detection(const string& name,
			   unsigned int nout,
			   unsigned int nrcu, unsigned int nsub1)
: WorkHolder   (2, nout, name, "WH_Detection"),
  itsInHolder  ("in", nrcu, nsub1),
  itsOutHolders(0),
  itsNrcu      (nrcu),
  itsNsub1     (nsub1),
  itsThreshold ("threshold", 1, 1),
  itsFlags     (nrcu, nsub1, 1)
{
  if (nout > 0) 
  {
    itsOutHolders = new DH_SampleR* [nout];
  }
  char str[8];
  // Create the output DH-s.
  for (unsigned int i = 0; i < nout; i++) 
  {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_SampleR(string("out_") + str, nrcu, nsub1);
  }
}

WH_Detection::~WH_Detection()
{
  for (int i = 0; i < getOutputs(); i++) 
  {
    delete itsOutHolders[i];
  }
  delete [] itsOutHolders;
}

WorkHolder* WH_Detection::construct(const string& name, int ninput,
				    int noutput,
				    const ParamBlock& params)
{
  Assert (ninput == 1);
  return new WH_Detection(name, noutput,
                          params.getInt ("nrcu", 10),
                          params.getInt ("nsub1", 10));
}

WH_Detection* WH_Detection::make(const string& name) const
{
  return new WH_Detection(name, getOutputs(), itsNrcu, itsNsub1);
}

void WH_Detection::process()
{
  // Process receives each time a buffer of nrcu * nsub1 samples.
  if (getOutputs() > 0) 
  {
    DH_SampleC::BufferType* bufin = itsInHolder.getBuffer();
    DH_SampleR::BufferType* bufout = itsOutHolders[0]->getBuffer();
    const DH_SampleR::BufferType* threshold = itsThreshold.getBuffer();

    Matrix<DH_SampleC::BufferType> InputMatrix(IPosition(2, itsNrcu, itsNsub1), bufin, SHARE);
    Matrix<double> itsPower = real(InputMatrix * conj(InputMatrix));

    // Do thresholding
    for (int rcu = 0 ; rcu < itsNrcu; ++rcu)
      {
	DH_SampleR::BufferType *out = bufout + rcu;
	for (int sub = 0; sub < itsNsub1; ++sub)
	  {
	    if (itsPower(rcu, sub) > *threshold)
	      {
		itsFlags(rcu, sub) = 0;
	      }
	    else
	      {
		itsFlags(rcu, sub) = 1;
	      }
	    out[itsNrcu * sub] = itsFlags(rcu, sub);
	  }
      }

    // Copy to other output buffers.
    for (int i = 1; i < getOutputs(); i++) 
    {
	memcpy (getOutHolder(i)->getBuffer(), bufout,
		itsNrcu * itsNsub1 * sizeof(DH_SampleR::BufferType));
    }
  }
}

void WH_Detection::dump() const
{
  cout << "WH_Detection " << getName() << " Buffer:" << endl;
  if (getOutputs() > 0) 
  {
    cout << itsOutHolders[0]->getBuffer()[0] << ','
	 << itsOutHolders[0]->getBuffer()[itsNrcu * itsNsub1 - 1] << endl;
  }
}

DataHolder* WH_Detection::getInHolder(int channel)
{
  AssertStr (channel < 2, "input channel too high");
  if (channel == 0)
  {
    return &itsInHolder;
  }
  return &itsThreshold;
}

DH_SampleR* WH_Detection::getOutHolder(int channel)
{
  AssertStr (channel < getOutputs(), "output channel too high");
  return itsOutHolders[channel];
}
