//#  WH_Cancel.cc:
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

#include <StationSim/WH_Cancel.h>
#include <BaseSim/ParamBlock.h>
#include <Common/Debug.h>
#include <Common/lofar_vector.h>


WH_Cancel::WH_Cancel (const string& name,
			  unsigned int nout, unsigned int nrcu,
			  unsigned int nsub1)
: WorkHolder    (2, nout, name, "WH_Cancel"),
  itsInHolder   ("in", nrcu, nsub1),
  itsOutHolders (0),
  itsNrcu       (nrcu),
  itsNsub1      (nsub1),
  itsFlags       ("flag", nrcu, nsub1)
{
  if (nout > 0) {
    itsOutHolders = new DH_SampleC* [nout];
  }
  char str[8];
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_SampleC (string("out_") + str, nrcu,
				       nsub1);
  }
}

WH_Cancel::~WH_Cancel()
{
  for (int i=0; i<getOutputs(); i++) {
    delete itsOutHolders[i];
  }
  delete [] itsOutHolders;
}

WorkHolder* WH_Cancel::construct (const string& name,
				    int ninput, int noutput,
				    const ParamBlock& params)
{
  Assert (ninput == 2);
  return new WH_Cancel (name, noutput,
			  params.getInt ("nrcu", 10),
			  params.getInt ("nsub1", 10));
}

WH_Cancel* WH_Cancel::make (const string& name) const
{
  return new WH_Cancel (name, getOutputs(),
			 itsNrcu, itsNsub1);
}

void WH_Cancel::process()
{
  if (getOutputs() > 0) 
    {
      DH_SampleC::BufferType* bufin = itsInHolder.getBuffer();
      DH_SampleR::BufferType* flags = itsFlags.getBuffer();
      DH_SampleC::BufferType* bufout = itsOutHolders[0]->getBuffer();

      Matrix<DH_SampleC::BufferType> InputMatrix(IPosition(2, itsNrcu, itsNsub1), bufin, SHARE);
      Matrix<DH_SampleR::BufferType> FlagMatrix(IPosition(2, itsNrcu, itsNsub1), flags, SHARE);
    
      for (int rcu = 0; rcu < itsNrcu; rcu++) 
	{
	  DH_SampleC::BufferType *out = bufout + rcu;
	  for (int sub = 0; sub < itsNsub1; sub++) 
	    {
	      out[itsNrcu * sub] = FlagMatrix(rcu, sub) * InputMatrix(rcu, sub);
	    }
	}
 
      // Copy the output if multiple outputs are used.
      for (int i = 1; i < getOutputs(); i++) 
	{
	  memcpy (itsOutHolders[i]->getBuffer(), bufout,
		  itsNrcu * itsNsub1 * sizeof(DH_SampleC::BufferType));
	}      
    }
}

void WH_Cancel::dump() const
{
  cout << "WH_Cancel " << getName() << " Buffers:" << endl;
  if (getOutputs() > 0) {
    cout << itsOutHolders[0]->getBuffer()[0] << ','
	 << itsOutHolders[0]->getBuffer()[itsNrcu*itsNsub1-1] << endl;
  }
}


DataHolder* WH_Cancel::getInHolder (int channel)
{
  AssertStr (channel < 2,
	     "input channel too high");
  if (channel == 0) 
  {
    return &itsInHolder;
  } 
  return &itsFlags;
}
DH_SampleC* WH_Cancel::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs(),
	     "output channel too high");
  return itsOutHolders[channel];
}
