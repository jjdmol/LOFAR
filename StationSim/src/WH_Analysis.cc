//#  WH_Analysis.cc:
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

#include <StationSim/WH_Analysis.h>
#include <BaseSim/ParamBlock.h>
#include <Common/Debug.h>


WH_Analysis::WH_Analysis (const string& name,
			  unsigned int nout,
			  unsigned int nx, unsigned int ny,
			  double threshold)
: WorkHolder   (1, nout, name, "WH_Analysis"),
  itsInHolder  ("in", nx, ny),
  itsOutHolders(0),
  itsNx        (nx),
  itsNy        (ny),
  itsThreshold (threshold)
{
  if (nout > 0) {
    itsOutHolders = new DH_SampleR* [nout];
  }
  char str[8];
  // Create the output DH-s.
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_SampleR (string("out_") + str, nx, ny);
  }
}

WH_Analysis::~WH_Analysis()
{
  for (int i=0; i<getOutputs(); i++) {
    delete itsOutHolders[i];
  }
  delete [] itsOutHolders;
}

WorkHolder* WH_Analysis::construct (const string& name, int ninput,
				    int noutput,
				    const ParamBlock& params)
{
  Assert (ninput==1);
  return new WH_Analysis (name, noutput,
			  params.getInt ("nx", 10),
			  params.getInt ("ny", 10),
			  params.getDouble ("threshold", 0.));
}

WH_Analysis* WH_Analysis::make (const string& name) const
{
  return new WH_Analysis (name, getOutputs(), itsNx, itsNy, itsThreshold);
}

void WH_Analysis::process()
{
  // Process receives each time a buffer of nx*ny samples.
  if (getOutputs() > 0) {
    DH_SampleC::BufferType* bufin = itsInHolder.getBuffer();
    DH_SampleR::BufferType* bufout = itsOutHolders[0]->getBuffer();
    for (int i=0; i<itsNx*itsNy; i++) {
      if (abs(bufin[i]) > itsThreshold) {
	bufout[i] = 0;
      } else {
	bufout[i] = 1;
      }     
    }
    // Copy to other output buffers.
    for (int i=1; i<getOutputs(); i++) {
      memcpy (getOutHolder(i)->getBuffer(), bufout,
	      itsNx *itsNy * sizeof(DH_SampleR::BufferType));
    }
  }
}

void WH_Analysis::dump() const
{
  cout << "WH_Analysis " << getName() << " Buffer:" << endl;
  if (getOutputs() > 0) {
    cout << itsOutHolders[0]->getBuffer()[0] << ','
	 << itsOutHolders[0]->getBuffer()[itsNx*itsNy-1] << endl;
  }
}


DH_SampleC* WH_Analysis::getInHolder (int channel)
{
  AssertStr (channel == 0,
	     "input channel too high");
  return &itsInHolder;
}
DH_SampleR* WH_Analysis::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs(),
	     "output channel too high");
  return itsOutHolders[channel];
}
