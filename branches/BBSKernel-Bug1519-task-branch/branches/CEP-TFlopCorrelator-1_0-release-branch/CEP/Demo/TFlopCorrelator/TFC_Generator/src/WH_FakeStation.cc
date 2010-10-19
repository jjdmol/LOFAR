//#  WH_FakeStation.cc: Emulate a station
//#
//#  Copyright (C) 2002-2005
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

#include <lofar_config.h>

// General includes
#include <Common/LofarLogger.h>

// Application specific includes
#include <TFC_Interface/RSPTimeStamp.h>
#include <DH_RSP.h>
#include <WH_FakeStation.h>

// This WH can be used with CEPFrame or tinyCEP, but to be able
// to test if we are in CEPFrame, we need to include DataManager.h
#include <CEPFrame/DataManager.h>

using namespace LOFAR;

WH_FakeStation::WH_FakeStation(const string& name, 
			       const ParameterSet ps,
			       const int stationID,
			       const int delay)
  : WorkHolder (1, 1,
		name, 
		"WH_FakeStation"),
    itsPS(ps),
    itsDelay(delay)
{
  itsStationId = stationID;
  getDataManager().addInDataHolder(0, new DH_RSP("incoming_DH_RSP", itsPS));
  getDataManager().addOutDataHolder(0, new DH_RSP("outgoing_DH_RSP", itsPS));
}

WH_FakeStation::~WH_FakeStation() {
}

WorkHolder* WH_FakeStation::construct(const string& name,
				      const ParameterSet ps,
				      const int stationID,
				      const int delay)
{
  return new WH_FakeStation(name, ps, stationID, delay);
}

WH_FakeStation* WH_FakeStation::make(const string& name)
{
  return new WH_FakeStation(name, itsPS, itsStationId, itsDelay);
}

void WH_FakeStation::preprocess() {
  DataManager* dm = dynamic_cast<DataManager*> (&getDataManager());
  ASSERTSTR(dm, "WH_FakeStation only works in CEPFrame, not tinyCEP. This is because we need shared input and output DHs.");
}

void WH_FakeStation::process() 
{
  EthernetFrame& myEthFrame = ((DH_RSP*)getDataManager().getInHolder(0))->getEthernetFrame();
  TimeStamp myStamp;

  for (int epap = 0; epap < myEthFrame.getNoPacketsInFrame(); epap++) {
    EpaHeader& header = myEthFrame.getEpaPacket(epap).getHeader();

    // set my station id
    header.setStationId(itsStationId);

    // get the stamp
    myStamp.setStamp(header.getSeqId(), header.getBlockId());
    // add the delay
    myStamp += itsDelay;
    // set the stamp
    header.setSeqId(myStamp.getSeqId());
    header.setBlockId(myStamp.getBlockId());
  }
}
