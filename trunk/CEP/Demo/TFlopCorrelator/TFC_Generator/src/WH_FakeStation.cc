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
#include <WH_FakeStation.h>
#include <signal.h>
#include <sys/time.h>
#include <boost/thread.hpp>

using namespace LOFAR;

WH_FakeStation::WH_FakeStation(const string& name, 
			       const ParameterSet ps,
			       TransportHolder& th)
  : WorkHolder (0, 0,
		name, 
		"WH_FakeStation"),
    itsPS(ps),
    itsTH(th)
{
}

WH_FakeStation::~WH_FakeStation() {
}

WorkHolder* WH_FakeStation::construct(const string& name,
			       const ParameterSet ps,
			       TransportHolder& th)
{
  return new WH_FakeStation(name, ps, th);
}

WH_FakeStation* WH_FakeStation::make(const string& name)
{
  return new WH_FakeStation(name, itsPS, itsTH);
}

void WH_FakeStation::preprocess() 
{
  if (!theirTimerSet) {
    theirTimerSet = true;
    sighandler_t ret = signal(SIGALRM, *WH_FakeStation::timerSignal);
    ASSERTSTR(ret != SIG_ERR, "WH_FakeStation couldn't set signal handler for timer");    
    struct itimerval value;
    memset (&value, 0, sizeof(itimerval));
    value.it_interval.tv_sec = ;
    value.it_interval.tv_usec = ;
    value.it_value.tv_sec = ;
    value.it_value.tv_usec = ;
    setitimer(ITIMER_REAL, &value, 0);
  }
  theirNoRunningWHs++;
}  

void WH_FakeStation::process() 
{
  itsEthFrame.reset();
  EpaHeader myHeader;
  myHeader.protocol = 0; // what was the number here?
  myHeader.stationId = itsStationId;

  for (int epap = 0; epap < noEpaP; epap++) {
    myHeader.seqId = itsStamp.getSeqId();
    myHeader.blockId = itsStamp.getBlockId();
    itsEthFrame.getEpaPacket(epap).setHeader(myHeader);
    itsStamp++;
    // don't touch the data, it is already zero
  }

  while (theirNoAlarms == 0) 
  {
    // wait for alarm to go off
    boost::thread::yield();
  };

  // we handled one alarm, so decrease it
  theirNoAlarms--;
  itsTH.sendBlocking(itsEthFrame.getPayloadp(), itsEthFrame.getPayloadSize(), 0);
}

void WH_FakeStation::postprocess() 
{
  theirNoRunningWHs--;
  if (theirNoRunningWHs == 0)
  {
    theirTimerSet = false;
    // unset timer
    struct itimerval value;
    memset (&value, 0, sizeof(itimerval));
    setitimer(ITIMER_REAL, &value, 0);
    // remove sig handler
    sighandler_t ret = signal(SIGALRM, SIG_DFL);
    ASSERTSTR(ret != SIG_ERR, "WH_FakeStation couldn't unset signal handler for timer");    
  }
}  

void WH_FakeStation::timerSignal(int sig)
{
  // set the number of frames that can be sent
  theirNoAlarms += theirNoRunningWHs;
}
  
