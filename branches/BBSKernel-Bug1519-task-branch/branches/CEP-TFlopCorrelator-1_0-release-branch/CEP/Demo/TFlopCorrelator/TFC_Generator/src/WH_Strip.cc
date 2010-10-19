//#  WH_Strip.cc: Take a signal from a DH and send only the bare data over a TH
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
#include <WH_Strip.h>
// for timer
#include <signal.h>
#include <sys/time.h>
// for rand
#include <stdlib.h>
// for sleep (yield)
#include <boost/thread.hpp>

using namespace LOFAR;

int WH_Strip::theirNoRunningWHs = 0;
int WH_Strip::theirNoAlarms = 0;
bool WH_Strip::theirTimerSet = 0;

WH_Strip::WH_Strip(const string& name, 
		   TransportHolder& th,
		   const ParameterSet ps)
  : WorkHolder (1, 0,
		name, 
		"WH_Strip"),
    itsTH(th),
    itsPS(ps)
{
  itsFrequency = ps.getDouble("Generator.OutputRate") / ps.getInt32("Input.NPacketsInFrame");
  
  getDataManager().addInDataHolder(0, new DH_RSP("strip_in", itsPS));
}

WH_Strip::~WH_Strip() {
}

WorkHolder* WH_Strip::construct(const string& name,
				TransportHolder& th,
				const ParameterSet ps)
{
  return new WH_Strip(name, th, ps);
}

WH_Strip* WH_Strip::make(const string& name)
{
  return new WH_Strip(name, itsTH, itsPS);
}

void WH_Strip::preprocess() 
{
  if (itsFrequency == 0) {
    // frequency was set to zero, which means output should go as fast as possible
    // we do that by setting theirNoAlarms to the number of runs
    theirNoAlarms += itsPS.getDouble("Generator.NoRuns");
  } else {
    if (!theirTimerSet) {
      sighandler_t ret = signal(SIGALRM, *WH_Strip::timerSignal);
      ASSERTSTR(ret != SIG_ERR, "WH_Strip couldn't set signal handler for timer");    
      struct itimerval value;
      memset (&value, 0, sizeof(itimerval));

      double time = 1/itsFrequency;    
      __time_t secs = floor(time);
      __time_t usecs = 1e6 * (time - secs);
      // this means 1MHz is the highest frequency
      if (secs + usecs == 0) usecs = 1;
      value.it_interval.tv_sec = secs;
      value.it_interval.tv_usec = usecs;
      value.it_value.tv_sec = secs;
      value.it_value.tv_usec = usecs;
      cout << "Setting timer interval to " << secs << "secs and " << usecs << "ms" << endl;

      setitimer(ITIMER_REAL, &value, 0);
      
      theirTimerSet = true;
    }
  }
  theirNoRunningWHs++;
}  

void WH_Strip::process() 
{
  EthernetFrame& myEthFrame = ((DH_RSP*)getDataManager().getInHolder(0))->getEthernetFrame();

  int timerCount = 0;
  while (theirNoAlarms == 0) 
  {
    timerCount ++;
    // wait for alarm to go off
    boost::thread::yield();
  };
  //cout<<"yield was called "<< timerCount << " times"<<endl;

  cout<<"WH_Strip sending "<<myEthFrame.getPayloadSize()<<" bytes"<<endl;
  bool ret = itsTH.sendBlocking(myEthFrame.getPayloadp(), myEthFrame.getPayloadSize(), 0);
  ASSERTSTR(ret, "TH couldn't send data");
  // we handled one alarm, so decrease it
  theirNoAlarms--;
}

void WH_Strip::postprocess() 
{
  theirNoRunningWHs--;
  if (theirNoRunningWHs == 0)
  {
    theirTimerSet = false;
    if (itsFrequency != 0) {
      // unset timer
      struct itimerval value;
      memset (&value, 0, sizeof(itimerval));
      setitimer(ITIMER_REAL, &value, 0);
      // remove sig handler
      sighandler_t ret = signal(SIGALRM, SIG_DFL);
      ASSERTSTR(ret != SIG_ERR, "WH_Strip couldn't unset signal handler for timer");    
    }
  }
}  

void WH_Strip::timerSignal(int sig)
{
  // set the number of frames that can be sent
  theirNoAlarms += theirNoRunningWHs;
}
  
