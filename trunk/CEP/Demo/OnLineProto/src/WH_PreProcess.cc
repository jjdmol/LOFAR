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
  : WorkHolder    (nbeamlets, nbeamlets, name,"WH_PreProcess"),
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
  dump();

  std::cout << "its number of beamlets: " << itsMac.getNumberOfBeamlets () << std::endl;

  TRACER4("WH_PreProcess::Process()");
  for (int i = 0; i < itsMac.getNumberOfBeamlets (); i++) {
    // set elapsed time
    ((DH_Beamlet*)getDataManager().getInHolder(i))->setElapsedTime(((DH_Beamlet*)getDataManager().getOutHolder(i))->getElapsedTime());
    //    std::cout << "Elapsed time: " << ((DH_Beamlet*)getDataManager().getOutHolder(i))->getElapsedTime() << std::endl;

    for (int j = 1; j < itsMac.getBeamletSize (); j++) { 
      *((DH_Beamlet*)getDataManager().getInHolder(i))->getBufferElement(j) 
	= *((DH_Beamlet*)getDataManager().getOutHolder(i))->getBufferElement(j);
      std::cout << *((DH_Beamlet*)getDataManager().getOutHolder(i))->getBufferElement(j) << ' ';
    }
    std::cout << std::endl;
  }
}

void WH_PreProcess::dump()
{    
  cout << "WH_PreProcess " << getName () << " Buffers:" << endl;
  for (int i = 0; i < itsMac.getNumberOfBeamlets(); i++) {
    for (int j = 0; j < itsMac.getBeamletSize(); j++) {
      cout << *((DH_Beamlet*)getDataManager().getInHolder(i))->getBufferElement(j) << ' ';
    }
    cout << endl;
  }
}

}// namespace LOFAR
