//  WH_Random.cc:
//
//  Copyright (C) 2000, 2001
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
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>             // for sprintf
#include <sys/time.h>          // for gettimeofday
#include <Common/Debug.h>

// CEPFrame general includes
#include <Common/KeyValueMap.h>

// OnLineProto specific include
#include <WH_Random.h>
#include <DH_Empty.h>
#include <DH_CorrCube.h>

namespace LOFAR
{

WH_Random::WH_Random (const string& name,
			    unsigned int nin,
			    unsigned int nout,
			    const int FBW)
  : WorkHolder    (nin, nout, name,"WH_Random"),
    itsFBW(FBW),
    itsIntegrationTime(0)
{
  char str[8];
  // create the input dataholders
  for (unsigned int i=0; i<nin; i++) {
    sprintf (str, "%d", i);
    getDataManager().addInDataHolder(i, 
				     new DH_Empty (string("out_") + str));
  }
  // create the output dataholders
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    getDataManager().addOutDataHolder(i, 
				      new DH_CorrCube (string("out_") + str));
  }

  // seed the random generator with a random int
  timeval tv;
  gettimeofday(&tv, NULL);
  srandom(tv.tv_sec);
}

WH_Random::~WH_Random()
{
}

WorkHolder* WH_Random::construct (const string& name, 
				     unsigned int nin,
				     unsigned int nout,
				     const int FBW)
{
  return new WH_Random (name, nin, nout, FBW);
}

WH_Random* WH_Random::make (const string& name)
{
  return new WH_Random (name, 
			   getDataManager().getInputs(), 
			   getDataManager().getOutputs(),
			   itsFBW);
}

void WH_Random::process()
{
  TRACER4("WH_Random::Process()");
  // loop over input stations and channels within the corresponding beamlets
  for (int station=0; station< NSTATIONS; station++) {
    for (int freq=0; freq < BFBW; freq++) {
     
      // a random complex float number
      //      complex<float> rval = (complex<float>) std::polar( 2.5, (2 * M_PI * rand()/(RAND_MAX)) );

      complex<float> rval = complex<float> ( -2.5 + 5.0 * rand()/(RAND_MAX), 0.0 );

      ((DH_CorrCube*)getDataManager().getOutHolder(freq))
	   ->setBufferElement(station, 
			      itsIntegrationTime, 
			      0, // remember we assume monochromatic correlators
			      &rval);      
    } 
  }
  // keep track of the time channel
  if (itsIntegrationTime++ == TSIZE-1) itsIntegrationTime=0;

  for (int i = 0; i < BFBW; i++) {
    getDataManager().readyWithOutHolder(i);
  }
}

void WH_Random::dump()
{
  cout << "--------------------------------------------------------" << endl;
  cout << "Dump WH_Random " << getName() << endl;
   
  for (int s=0; s<10; s++) {
    cout << "out station=" << s << ":  ";
    for (int t=0; t<10; t++) {
      cout << *((DH_CorrCube*)getDataManager().getOutHolder(1))
	->getBufferElement(s, 0, t) ;

    }
    cout << endl;
  }
  cout << "--------------------------------------------------------" << endl;
}

}// namespace LOFAR
