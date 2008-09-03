//#  WH_FakeStation.cc: Emulate a station
//#
//#  Copyright (C) 2006
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Transport/TransportHolder.h>
#include <Generator/WH_FakeStation.h>
#include <Generator/DH_RSP.h>
#include <Generator/RSPTimeStamp.h>

// This WH can be used with CEPFrame or tinyCEP, but to be able
// to test if we are in CEPFrame, we need to include DataManager.h
#include <CEPFrame/DataManager.h>

namespace LOFAR {
  namespace Generator {

    WH_FakeStation::WH_FakeStation(const string& name, 
				   const ParameterSet ps,
				   const int stationID,
				   const int delay,
				   TransportHolder* th)
      : WorkHolder (1, 1,
		    name, 
		    "WH_FakeStation"),
	itsPS(ps),
	itsDelay(delay),
	itsTH(th)
    {
      itsStationId = stationID;
      getDataManager().addInDataHolder(0, new DH_RSP("incoming_DH_RSP", itsPS));
      getDataManager().addOutDataHolder(0, new DH_RSP("outgoing_DH_RSP", itsPS));
      // To skip sending some data
      itsFlagger = new RangeFlagger(0 + 1000*stationID, 2000 + stationID * 1000);
      //itsFlagger = new Flagger();
    }

    WH_FakeStation::~WH_FakeStation() {
    }

    WorkHolder* WH_FakeStation::construct(const string& name,
					  const ParameterSet ps,
					  const int stationID,
					  const int delay,
					  TransportHolder* th)
    {
      return new WH_FakeStation(name, ps, stationID, delay, th);
    }

    WH_FakeStation* WH_FakeStation::make(const string& name)
    {
      return new WH_FakeStation(name, itsPS, itsStationId, itsDelay, itsTH);
    }

    void WH_FakeStation::preprocess() {
      DataManager* dm = dynamic_cast<DataManager*> (&getDataManager());
      ASSERTSTR(dm, "WH_FakeStation only works in CEPFrame, not tinyCEP. This is because we need shared input and output DHs.");
      itsTH->init();
    }

    void WH_FakeStation::process() 
    {
      Frame& myEthFrame = *(dynamic_cast<DH_RSP*>(getDataManager().getInHolder(0)))->getFrame();
      TimeStamp myStamp;

      Header& myHeader = *myEthFrame.getHeader();
      myHeader.setStationId(itsStationId);
      // get the stamp
      myStamp.setStamp(myHeader.getSeqId(), myHeader.getBlockId());
      // add the delay
      myStamp += itsDelay;
      // set the stamp
      myHeader.setSeqId(myStamp.getSeqId());
      myHeader.setBlockId(myStamp.getBlockId());
      
      if (itsTH != 0) {
	if (itsFlagger->sendData(myStamp)){
	  itsTH->sendBlocking(myEthFrame.getBufferp(), myEthFrame.getSize(), 0);
	}
      }
    }

  } // namespace Generator
} // namespace LOFAR
