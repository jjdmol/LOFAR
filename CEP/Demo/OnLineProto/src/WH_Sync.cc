//  WH_Sync.cc:
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
#include "OnLineProto/WH_Sync.h"

namespace LOFAR
{
WH_Sync::WH_Sync (const string& name, 
			      const int nbeamlets, 
			      const ParameterSet& ps, 
			      const int StationID)
  : WorkHolder    (nbeamlets+1, nbeamlets, name,"WH_Sync"),
    itsPS (ps),
    itsStationID (StationID)
{
  char str[8];
  int bs = itsPS.getInt("station.nchannels"); 
    
  for (int i = 0; i < nbeamlets; i++) {
    sprintf (str, "%d", i);

    // create the input dataholders
    getDataManager().addInDataHolder(i, new DH_Beamlet (string("in_") + str,
							itsStationID,
							itsPS.getFloat(string("station.beamlet.") + str),
							itsPS.getFloat("station.chan_bw"),
							itsPS.getFloat("observation.ha_0"),
							itsPS.getInt("station.nchannels")));

    // create the output dataholders
    getDataManager().addOutDataHolder(i, new DH_Beamlet (string("out_") + str,
							 itsStationID,
							 itsPS.getFloat(string("station.beamlet.") + str),
							 itsPS.getFloat("station.chan_bw"),
							 itsPS.getFloat("observation.ha_0"),
							 itsPS.getInt("station.nchannels")));
  }
}
  
WH_Sync::~WH_Sync()
{
}

WorkHolder* WH_Sync::construct (const string& name, 
				      const int nbeamlets, 
				      const ParameterSet& ps,
				      const int StationID)
{
  return new WH_Sync (name, nbeamlets, ps, StationID);
}

WH_Sync* WH_Sync::make (const string& name)
{
  return new WH_Sync (name, getDataManager().getOutputs(), itsPS, itsStationID);
}

void WH_Sync::process()
{
  TRACER4("WH_Sync::Process()");

  //! todo: define b
  int b = 10;

  for (int i = 0; i < b; i++) {
    // set elapsed time
    ((DH_Beamlet*)getDataManager().getOutHolder(i))->
      setElapsedTime(((DH_Beamlet*)getDataManager().getInHolder(i))->getElapsedTime());

    for (int j = 0; j < itsPS.getInt("station.nchannels"); j++) {
      
      if (itsPS.getInt("general.enable_fs") == 1) 
	{
	  // apply phase shift
	  *((DH_Beamlet*)getDataManager().getOutHolder(i))->getBufferElement(j) 
	    = phase * *((DH_Beamlet*)getDataManager().getInHolder(i))->getBufferElement(j);
	} 
      else 
	{
	  // just copy input to output
	  *((DH_Beamlet*)getDataManager().getOutHolder(i))->getBufferElement(j)
	    = *((DH_Beamlet*)getDataManager().getInHolder(i))->getBufferElement(j);
	}    
    }
  }
}

void WH_Sync::dump()
{    
  cout << "WH_Sync " << getName () << " Buffers:" << endl;
  for (int i = 0; i < MIN(itsPS.getInt("station.nbeamlets"),1); i++) {
    for (int j = 0; j < MIN(itsPS.getInt("station.nchannels"),10); j++) {
      cout << *((DH_Beamlet*)getDataManager().getOutHolder(i))->getBufferElement(j) << ' ';
    }
    cout << endl;
  }
}

}// namespace LOFAR
