//#  WH_DetNulls.cc:
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

#include <StationSim/WH_DetNulls.h>
#include <Common/Debug.h>
#include <Common/Lorrays.h>
#include <Math/LCSMath.h>
#include <blitz/blitz.h>

WH_DetNulls::WH_DetNulls(const string& name, unsigned int nin, unsigned int nout, 
			 unsigned int nant, unsigned int detnulls, string s, string detnullsfile)
  : WorkHolder     (nin, nout, name, "WH_DetNulls"),
    itsOutHolders  (0),
    itsNrcu        (nant),
    itsNDetNulls   (detnulls),
    itsArray       (s)
{
  char str[8];
  if (nout > 0) {
    itsOutHolders = new DH_SampleC* [nout];
  }
  for (unsigned int i = 0; i < nout; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_SampleC (string("out_") + str, itsNrcu, itsNDetNulls);
  }

  // resize coordinate file to fit the number of deterministic nulls
  // and the nulls matrix to fit the resulting vectors
  itsCoordinates.resize(itsNDetNulls, 2);
  itsDetNulls.resize(itsNrcu, itsNDetNulls);
  // Read in file containing the coordinates of the fixed interfering sources

  if ( itsNDetNulls > 0 ) {
    itsDetNullsFile.open(detnullsfile.data());
    itsDetNullsFile >> itsCoordinates;
    itsDetNullsFile.close();
    itsDetNullsFileName = detnullsfile;
  }

  using namespace blitz;

  for (int i = 0; i < itsNDetNulls; i++) {
    LoVec_dcomplex sv(itsNrcu);
    sv = steerv(itsCoordinates(i,0), itsCoordinates(i,1), itsArray.getPointX(), itsArray.getPointY());
    itsDetNulls(Range::all(), i) = sv;
  }
  
}

WH_DetNulls::~WH_DetNulls()
{
  
}

WH_DetNulls* WH_DetNulls::make (const string& name) const
{
  return new WH_DetNulls (name, getInputs(), getOutputs(), itsNrcu, itsNDetNulls, 
			  itsArray.conf_file, itsDetNullsFileName);
}

void WH_DetNulls::preprocess()
{
}


void WH_DetNulls::process()
{
  if (getOutputs() > 0) {
    for (int i = 0; i < getOutputs(); i++) {
      memcpy(itsOutHolders[i]->getBuffer(), itsDetNulls.data(), 
	     itsNrcu * itsNDetNulls * sizeof(DH_SampleC::BufferType));

    }
  }
}

void WH_DetNulls::dump() const
{
  using namespace blitz;

  LoVec_dcomplex weight(itsOutHolders[0]->getBuffer(), itsNrcu, duplicateData);    

  //  cout << "Weight vector Buffer: " << endl;
  //  cout << weight<< endl;
}


DataHolder* WH_DetNulls::getInHolder (int channel)
{
  AssertStr (channel < 0, "input channel too high");
  return 0;
}

DH_SampleC* WH_DetNulls::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs(), "output channel too high");  
  return itsOutHolders[channel];
}

LoVec_dcomplex WH_DetNulls::steerv (double phi, double theta, LoVec_double px, LoVec_double py) {
  
  FailWhen1( px.size() != py.size(),"vector size mismatch" );

  LoVec_dcomplex res( px.size() );
  dcomplex i = dcomplex (0,1);

  res = exp(-2*M_PI*i*( px*sin(theta)*cos(phi) + py*sin(theta)*sin(phi) ) );
  return res;
}

