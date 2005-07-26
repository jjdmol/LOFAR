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

int WH_FakeStation::theirNoRunningWHs = 0;
int WH_FakeStation::theirNoAlarms = 0;
bool WH_FakeStation::theirTimerSet = 0;

WH_FakeStation::WH_FakeStation(const string& name, 
			       const ParameterSet ps,
			       TransportHolder& th,
			       const int stationID)
  : WorkHolder (0, 0,
		name, 
		"WH_FakeStation"),
    itsPS(ps),
    itsTH(th),
    itsEthFrame(ps)
{
  itsFrequency = ps.getDouble("Generator.OutputRate") / ps.getInt32("NoPacketsInFrame");
  itsStationId = stationID;
  // cout<<"myStationId = "<<itsStationId<<endl;
}

WH_FakeStation::~WH_FakeStation() {
}

WorkHolder* WH_FakeStation::construct(const string& name,
				      const ParameterSet ps,
				      TransportHolder& th,
				      const int stationID)
{
  return new WH_FakeStation(name, ps, th, stationID);
}

WH_FakeStation* WH_FakeStation::make(const string& name)
{
  return new WH_FakeStation(name, itsPS, itsTH, itsStationId);
}

void WH_FakeStation::preprocess() 
{
  if (!theirTimerSet) {
    theirTimerSet = true;
    sighandler_t ret = signal(SIGALRM, *WH_FakeStation::timerSignal);
    ASSERTSTR(ret != SIG_ERR, "WH_FakeStation couldn't set signal handler for timer");    
    struct itimerval value;
    memset (&value, 0, sizeof(itimerval));

    double time = 1/itsFrequency;    
    value.it_interval.tv_sec = (__time_t) floor(time);
    value.it_interval.tv_usec = (__time_t) (1e6 * (time - floor(time)));
    value.it_value.tv_sec = (__time_t) floor(time);
    value.it_value.tv_usec = (__time_t) (1e6 * (time - floor(time)));

    setitimer(ITIMER_REAL, &value, 0);
  }
  theirNoRunningWHs++;
}  

void WH_FakeStation::process() 
{
  itsEthFrame.reset();

  for (int epap = 0; epap < itsEthFrame.getNoPacketsInFrame(); epap++) {
    EpaHeader& header = itsEthFrame.getEpaPacket(epap).getHeader();
    header.setProtocol(43691);
    header.setStationId(itsStationId);
    header.setSeqId(itsStamp.getSeqId());
    header.setBlockId(itsStamp.getBlockId());
    itsStamp++;
    // don't touch the data, it is already zero

    RectMatrix<dataType>& myMatrix = itsEthFrame.getEpaPacket(epap).getMatrix();
    dimType sbDim = myMatrix.getDim("subband");
    RectMatrix<dataType>::cursorType cursor = myMatrix.getCursor(0*sbDim);
    for (int sb = 0; sb < myMatrix.getNElemInDim(sbDim); myMatrix.moveCursor(&cursor, sbDim), sb++) {
      myMatrix.setValue(cursor, makei16complex(sb, itsStamp.getBlockId()));  // the real value is the beamlet number, the imag value is the slice count
      // the y polarisations are all 0
    }
  }

  int timerCount = 0;
  while (theirNoAlarms == 0) 
  {
    timerCount ++;
    // wait for alarm to go off
    boost::thread::yield();
  };
  cout<<"yield was called "<< timerCount << " times"<<endl;

  // we handled one alarm, so decrease it
  theirNoAlarms--;
  //cout<<"sending from "<<(void*)itsEthFrame.getPayloadp()<<" size "<< itsEthFrame.getPayloadSize()<<endl;
  bool ret = itsTH.sendBlocking(itsEthFrame.getPayloadp(), itsEthFrame.getPayloadSize(), 0);
  ASSERTSTR(ret, "TH couldn't send data");
  //cout<<"succesfully sent"<<endl;
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
  
