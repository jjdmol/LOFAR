//  WH_Transpose.cc:
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
#include <Common/Debug.h>

// CEPFrame general includes
#include "CEPFrame/DH_Empty.h"
#include "CEPFrame/Step.h"
#include <Common/KeyValueMap.h>

// OnLineProto specific include
#include "OnLineProto/WH_Transpose.h"
#include "OnLineProto/DH_Beamlet.h"
#include "OnLineProto/DH_CorrCube.h"

namespace LOFAR
{

WH_Transpose::WH_Transpose (const string& name,
			    unsigned int nin,
			    unsigned int nout,
			    const ACC::ParameterSet& ps)
  : WorkHolder    (nin, nout, name,"WH_Transpose"),
    itsFBW(ps.getInt("station.nchannels")),
    itsIntegrationTime(0),
    itsPS(ps)
{
  char str[8];
  // create the input dataholders
  for (unsigned int i=0; i<nin; i++) {
    sprintf (str, "%d", i);
    getDataManager().addInDataHolder(i, 
				     new DH_Beamlet (string("Transpose in_") + str, itsFBW));
  }
  // create the output dataholders
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    getDataManager().addOutDataHolder(i, 
				      new DH_CorrCube (string("Transpose out_") + str, ps));
  }
}

WH_Transpose::~WH_Transpose()
{
}

WorkHolder* WH_Transpose::construct (const string& name, 
				     unsigned int nin,
				     unsigned int nout,
				     const ACC::ParameterSet& ps)
{
  return new WH_Transpose (name, nin, nout, ps);
}

WH_Transpose* WH_Transpose::make (const string& name)
{
  return new WH_Transpose (name, 
			   getDataManager().getInputs(), 
			   getDataManager().getOutputs(),
			   itsPS);
}

void WH_Transpose::process()
{
  TRACER4("WH_Transpose::Process()");
  // Transpose receives one beamlet per station
 
   // DbgAssertStr(getOutRate() == TSIZE,"Outrate error");
   //DbgAssertStr(getDataManager().getInputs() == NSTATIONS,"Nr inputs");
  //DbgAssertStr(getDataManager().getOutputs() == NVis,"Nr outputs");
  
  //DbgAssertStr(NVis == BFBW,"Assume one freq channel per corrcube");

   // ToDo: optimise transpose speed by copiing larger freq intevals
   //       in one memcopy instead of one by one.

  // loop over input stations and channels within the corresponding beamlets
  for (int station=0; station< getDataManager().getInputs(); station++) {
    for (int freq=0; freq<itsPS.getInt("station.nchannels"); freq++) {
     
      // in this version we make the corrcube monochromatic (see assert above)
      // therefore  the outholder channel corresponds to the freq.

      ((DH_CorrCube*)getDataManager().getOutHolder(freq))
	   ->setBufferElement(station, 
			      itsIntegrationTime, 
			      0, // remember we assume monochromatic correlators
			      ((DH_Beamlet*)getDataManager().getInHolder(station))
	                               ->getBufferElement(freq)
			      );      
    
       
       // temporary test to find signal above noise level
       if (((DH_Beamlet*)getDataManager().getInHolder(station))->getBufferElement(freq)->real() > 0.01)
	 {
// 	    cout << "stations " << station
// 	      << "  time " << itsIntegrationTime
// 	      << "value " 
// 	      << ((DH_Beamlet*)getDataManager().getInHolder(station))->getBufferElement(freq)
// 		<< endl;
	 }

	 } 
  }
  // keep track of the time channel
  itsIntegrationTime++;
  if (itsIntegrationTime == itsPS.getInt("corr.tsize")-1) itsIntegrationTime=0;
  cout << itsIntegrationTime << " ";
}

void WH_Transpose::dump()
{
  cout << "--------------------------------------------------------" << endl;
  cout << "Dump WH_Transpose " << getName() << endl;
  cout << " input " << endl;
  int nrStations = getDataManager().getInputs();
  int nrFreq = itsPS.getInt("station.nchannels");
  for (int station=0; station < nrStations ; station++) {
    cout << "in station=" << station << ":  ";
    for (int freq=0; freq < nrFreq; freq++) {
      cout << 	*((DH_Beamlet*)getDataManager().getInHolder(station))
	->getBufferElement(freq) << "  ";
    }
    cout << endl;
  }

  cout << " output " << endl;

  
  for (int station=0; station < nrStations; station++) {
    cout << "out station=" << station << ":  ";
    for (int freq=0; freq < nrFreq; freq++) {
      cout << *((DH_CorrCube*)getDataManager().getOutHolder(0))
	->getBufferElement(station, 0, freq) ;

    }
    cout << endl;
  }
  cout << "--------------------------------------------------------" << endl;
}

}// namespace LOFAR
