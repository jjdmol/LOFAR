//  WH_PreProcess.cc:
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
#include "CEPFrame/Step.h"
#include <Common/KeyValueMap.h>

// OnLineProto specific include
#include "OnLineProto/WH_PreProcess.h"

namespace LOFAR
{
WH_PreProcess::WH_PreProcess (const string& name, const int nbeamlets, MAC mac, 
			      int StationID)
  : WorkHolder    (nbeamlets+1, nbeamlets, name,"WH_PreProcess"),
    itsMac (mac),
    itsStationID (StationID)
{
  char str[8];

  // create the input dataholders
  for (int i = 0; i < nbeamlets; i++) {
    sprintf (str, "%d", i);
    int bs = mac.getBeamletSize (); 
    getDataManager().addInDataHolder(i, new DH_Beamlet (string("in_") + str,
							itsStationID,
							mac.getFrequency(i),
							mac.getChannelBandwidth(),
							mac.getStartHourangle(),
							mac.getBeamletSize()));
  }

  // connect to fringe control
  getDataManager().addInDataHolder(nbeamlets, new DH_Phase ("in_fringe", itsStationID));  

  // create the output dataholders
  for (int i = 0; i < nbeamlets; i++) {
    sprintf (str, "%d", i);
    int bs = mac.getBeamletSize (); 
    getDataManager().addOutDataHolder(i, new DH_Beamlet (string("out_") + str,
							 itsStationID,
							 mac.getFrequency(i),
							 mac.getChannelBandwidth(),
							 mac.getStartHourangle(),
							 mac.getBeamletSize()));
				      }
}
  
WH_PreProcess::~WH_PreProcess()
{
}

WorkHolder* WH_PreProcess::construct (const string& name, 
				      const int nchan,
				      MAC mac,
				      int StationID)
{
  return new WH_PreProcess (name, nchan, mac, StationID);
}

WH_PreProcess* WH_PreProcess::make (const string& name)
{
  return new WH_PreProcess (name, getDataManager().getOutputs(), itsMac, itsStationID);
}

void WH_PreProcess::process()
{
  TRACER4("WH_PreProcess::Process()");

  int b =  itsMac.getNumberOfBeamlets();
  for (int i = 0; i < b; i++) {
    // set elapsed time
    ((DH_Beamlet*)getDataManager().getOutHolder(i))->setElapsedTime(((DH_Beamlet*)getDataManager().getInHolder(i))->getElapsedTime());
    // cout << "WH_PreProcess : " << ((DH_Beamlet*)getDataManager().getOutHolder(i))->getElapsedTime() << endl;
    for (int j = 1; j < itsMac.getBeamletSize (); j++) {
       if (ENABLE_FS == 1) {
	  Station* station = itsMac.getStations();
//	  float t_i = station[StationID].getY * sin(
	  complex<float> phase = 1.0;
	    
	    
	    
	    
	    //*((DH_Phase*)getDataManager().getInHolder(b))->getBuffer();
	  *((DH_Beamlet*)getDataManager().getOutHolder(i))->getBufferElement(j) 
	    = phase * *((DH_Beamlet*)getDataManager().getInHolder(i))->getBufferElement(j);
       } else {
	  *((DH_Beamlet*)getDataManager().getOutHolder(i))->getBufferElement(j)
	    = *((DH_Beamlet*)getDataManager().getInHolder(i))->getBufferElement(j);
       }
    }
  }
}

void WH_PreProcess::dump()
{    
  cout << "WH_PreProcess " << getName () << " Buffers:" << endl;
  for (int i = 0; i < MIN(itsMac.getNumberOfBeamlets(),1); i++) {
    for (int j = 0; j < MIN(itsMac.getBeamletSize(),10); j++) {
      cout << *((DH_Beamlet*)getDataManager().getInHolder(i))->getBufferElement(j) << ' ';
    }
    cout << endl;
  }
}

}// namespace LOFAR
