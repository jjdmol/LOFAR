//#  WH_WeightDetermination.cc:
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
//#


#include <stdio.h>             // for sprintf
#include <blitz/blitz.h>

#include <StationSim/WH_WeightDetermination.h>
#include <Common/Debug.h>
#include <Common/Lorrays.h>
#include <Math/LCSMath.h>
#include <blitz/blitz.h>

WH_WeightDetermination::WH_WeightDetermination(const string& name, unsigned int nin, unsigned int nout, 
											   unsigned int nant, string s, string beamtrajectoryfile)
  : WorkHolder     (nin, nout, name, "WH_WeightDetermination"),
    itsOutHolders  (0),
    itsNrcu        (nant),
    itsArray       (s),
	itsBeamTraject (beamtrajectoryfile)
{
  char str[8];
  if (nout > 0) {
    itsOutHolders = new DH_SampleC* [nout];
  }
  for (unsigned int i = 0; i < nout; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_SampleC (string("out_") + str, itsNrcu, 1);
  }
}

WH_WeightDetermination::~WH_WeightDetermination()
{
  
}

WH_WeightDetermination* WH_WeightDetermination::make (const string& name) const
{
  return new WH_WeightDetermination (name, getInputs(), getOutputs(), itsNrcu, itsArray.conf_file, itsBeamTraject.conf_file);
}

void WH_WeightDetermination::preprocess()
{
}


void WH_WeightDetermination::process()
{
  if (getOutputs() > 0) {
	LoVec_dcomplex d = steerv(itsBeamTraject.getPhi (itsOutHolders [0]->getTimeStamp ()), 
							  itsBeamTraject.getTheta (itsOutHolders [0]->getTimeStamp ()), 
							  itsArray.getPointX (), itsArray.getPointY ());
	
	for (int i = 0; i < getOutputs(); i++) {
	  memcpy(itsOutHolders[i]->getBuffer(), d.data(), itsNrcu * sizeof(DH_SampleC::BufferType));
	}
  }
}

void WH_WeightDetermination::dump() const
{
  using namespace blitz;

  LoVec_dcomplex weight(itsOutHolders[0]->getBuffer(), itsNrcu, duplicateData);    

  //  cout << "Weight vector Buffer: " << endl;
  //  cout << weight<< endl;
}


DataHolder* WH_WeightDetermination::getInHolder (int channel)
{
  AssertStr (channel < 0, "input channel too high");
  return 0;
}

DH_SampleC* WH_WeightDetermination::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs(), "output channel too high");  
  return itsOutHolders[channel];
}

LoVec_dcomplex WH_WeightDetermination::steerv (double phi, double theta, LoVec_double px, LoVec_double py) {
  
  FailWhen1( px.size() != py.size(),"vector size mismatch" );

  LoVec_dcomplex res( px.size() );
  dcomplex i = dcomplex (0,1);

  res = exp( i * -2*M_PI*( px*sin(theta)*cos(phi) + py*sin(theta)*sin(phi) ) );
  return res;
}
