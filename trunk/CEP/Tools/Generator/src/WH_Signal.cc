//#  WH_Signal.cc: Create a signal
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
#include <Generator/WH_Signal.h>
// for timer
#include <signal.h>
#include <sys/time.h>
// for sleep (yield)
#include <boost/thread.hpp>

namespace LOFAR {
  namespace Generator {

    int WH_Signal::theirNoRunningWHs = 0;
    int WH_Signal::theirNoAlarms = 0;
    bool WH_Signal::theirTimerSet = 0;

    WH_Signal::WH_Signal(const string& name, 
			 const int noOutputs,
			 const ParameterSet ps)
      : WorkHolder (0, noOutputs,
		    name, 
		    "WH_Signal"),
	itsPS(ps),
	itsSignal(0)
    {
      itsFrequency = ps.getDouble("Generator.OutputRate") / ps.getInt32("Generator.NTimesPerFrame");
      double deltaT = 1 / ps.getDouble("Generator.SampleFreq");

      string signalType = ps.getString("Generator.SignalType");
      if (signalType == "RANDOM") {
	itsSignal = new Sig_Random();
      } else if (signalType == "MULTICHROME") {
	int noFreqs = ps.getInt32("Generator.NFreqs");
	vector<Sig_MultiChrome::SignalInfo> signals;
	char key[60];
	for (int f = 0; f < noFreqs; f++)
	{
	  Sig_MultiChrome::SignalInfo s;
	  snprintf(key, 60, "Generator.Signals.%d.Amplitude", f);
	  s.amplitude = ps.getDouble(key);
	  snprintf(key, 60, "Generator.Signals.%d.Frequency", f);
	  s.frequency = ps.getDouble(key);
	  snprintf(key, 60, "Generator.Signals.%d.Phase", f);
	  s.phase = ps.getDouble(key);
	  signals.push_back(s);
	}
	
	itsSignal = new Sig_MultiChrome(signals, deltaT);
      } else {
	// send zeros by default
	itsSignal = new Sig_Zero();
      }
 
      for (int outDH = 0; outDH < noOutputs; outDH++) {
	getDataManager().addOutDataHolder(outDH, new DH_RSP("signal_out", itsPS));
      }  
    }

    WH_Signal::~WH_Signal() {
      delete itsSignal;
    }

    WorkHolder* WH_Signal::construct(const string& name,
				     const int noOutputs,
				     const ParameterSet ps)
    {
      return new WH_Signal(name, noOutputs, ps);
    }

    WH_Signal* WH_Signal::make(const string& name)
    {
      return new WH_Signal(name, itsNoutputs, itsPS);
    }

    void WH_Signal::preprocess() 
    {
      if (itsFrequency == 0) {
	// frequency was set to zero, which means output should go as fast as possible
	// we do that by setting theirNoAlarms to -1
	theirNoAlarms = -1;
      } else {
	if (!theirTimerSet) {
	  sighandler_t ret = signal(SIGALRM, *WH_Signal::timerSignal);
	  ASSERTSTR(ret != SIG_ERR, "WH_Signal couldn't set signal handler for timer");    
	  struct itimerval value;
	  memset (&value, 0, sizeof(itimerval));

	  double time = 1/itsFrequency;    
	  time_t secs = static_cast<time_t>(floor(time));
	  time_t usecs = static_cast<time_t>(1e6 * (time - secs));
	  // this means 1MHz is the highest frequency
	  if (secs + usecs == 0) usecs = 1;
	  value.it_interval.tv_sec = secs;
	  value.it_interval.tv_usec = usecs;
	  value.it_value.tv_sec = secs;
	  value.it_value.tv_usec = usecs;
	  cout << "Setting timer interval to " << secs << "secs and " << usecs << "us" << endl;

	  setitimer(ITIMER_REAL, &value, 0);
      
	  theirTimerSet = true;
	}
      }
      theirNoRunningWHs++;
    }  


    void WH_Signal::process() 
    {
      Frame& myEthFrame = *((DH_RSP*)getDataManager().getOutHolder(0))->getFrame();
      //myEthFrame.reset();
      Header& header = *myEthFrame.getHeader();
      header.setProtocol(0xAAAB);
      header.setSeqId(itsStamp.getSeqId());
      header.setBlockId(itsStamp.getBlockId());

      itsSignal->fillNext(myEthFrame.getData());

      char* src = myEthFrame.getBufferp();
      int  size = myEthFrame.getSize();
      for (int o = 1; o < itsNoutputs; o++) {
	memcpy(((DH_RSP*)getDataManager().getOutHolder(o))->getFrame()->getBufferp(),
	       src, size);
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
      itsStamp += myEthFrame.getData()->getNTimes();
    }

    void WH_Signal::postprocess() 
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
	    ASSERTSTR(ret != SIG_ERR, "WH_Signal couldn't unset signal handler for timer");    
	  }
	}
    }  

    void WH_Signal::timerSignal(int sig)
    {
      // set the number of frames that can be sent
      theirNoAlarms += theirNoRunningWHs;
    }
  
  } // namespace Generator
} // namespace LOFAR
