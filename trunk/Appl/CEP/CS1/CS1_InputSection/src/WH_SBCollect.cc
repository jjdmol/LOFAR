//#  WH_SBCollect.cc: Joins all data (stations, pols) for a subband
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
#include <CS1_InputSection/WH_SBCollect.h>
#include <CS1_Interface/DH_RSP.h>
#include <CS1_Interface/DH_Subband.h>
#include <Common/hexdump.h>
#include <tinyCEP/Sel_RoundRobin.h>
#include <CEPFrame/DataManager.h>

// for timer
#include <signal.h>
#include <sys/time.h>
// for rand
#include <stdlib.h>
// for sleep (yield)
#include <boost/thread.hpp>

namespace LOFAR
{
  namespace CS1
  {

    int WH_SBCollect::theirNoRunningWHs = 0;
    int WH_SBCollect::theirNoAlarms = 0;
    bool WH_SBCollect::theirTimerSet = 0;

    WH_SBCollect::WH_SBCollect(const string& name,
			       const ACC::APS::ParameterSet pset,
			       const int noutputs) 
      : WorkHolder          (pset.getInt32("Observation.NStations"), 
			     noutputs,
			     name,
			     "WH_SBCollect"),
	itsPS               (pset),
	itsNStations        (pset.getInt32("Observation.NStations")),
	itsNSubbandsPerCell (pset.getInt32("General.SubbandsPerPset") * pset.getInt32("BGLProc.PsetsPerCell"))
    {
      char str[32];
      for (int i=0; i<itsNinputs; i++)
	{
	  sprintf(str, "DH_in_%d", i);
	  getDataManager().addInDataHolder(i, new DH_RSP(str, itsPS));
	}
      vector<int> channels;
      for(int i=0;i<itsNoutputs; i++)
	{
	  sprintf(str, "DH_out_%d", i);
	  getDataManager().addOutDataHolder(i, new DH_Subband(str, itsPS));
	  getDataManager().setAutoTriggerOut(i, false);
	  channels.push_back(i);
	}
      // Set a round robin output selector
      getDataManager().setOutputSelector(new Sel_RoundRobin(channels));
    }

    WH_SBCollect::~WH_SBCollect() {
    }

    WorkHolder* WH_SBCollect::construct(const string& name,
					const ACC::APS::ParameterSet pset,
					const int noutputs) 
    {
      return new WH_SBCollect(name, pset, noutputs);
    }

    WH_SBCollect* WH_SBCollect::make(const string& name)
    {
      return new WH_SBCollect(name, itsPS, itsNoutputs);
    }

    void WH_SBCollect::preprocess() {
      sleep(6);
      if (!theirTimerSet) {
#define USE_TIMER 0
#if USE_TIMER
	sighandler_t ret = signal(SIGALRM, *WH_SBCollect::timerSignal);
	ASSERTSTR(ret != SIG_ERR, "WH_SBCollect couldn't set signal handler for timer");    
	struct itimerval value;
	memset (&value, 0, sizeof(itimerval));

	double interval = itsPS.getUint32("Observation.NSubbandSamples") / itsPS.getDouble("Observation.SampleRate") / itsNSubbandsPerCell;
	__time_t secs = static_cast<__time_t>(floor(interval));
	__time_t usecs = static_cast<__time_t>(1e6 * (interval - secs));
	// this means 1MHz is the highest frequency
	value.it_interval.tv_sec = secs;
	value.it_interval.tv_usec = usecs;
	value.it_value.tv_sec = secs;
	value.it_value.tv_usec = usecs;
	clog << "Setting timer interval to " << secs << "secs and " << usecs << "ms" << endl;

	setitimer(ITIMER_REAL, &value, 0);
#else
	theirNoAlarms = -1; //This will make sure data is written at maximum speed
#endif
	theirTimerSet = true;
      }
      theirNoRunningWHs++;
    }

    void WH_SBCollect::process() 
    {
      // Copy every subband to one BG/L core
      for (unsigned subband = 0; subband < itsNSubbandsPerCell; subband ++) {
	// ask the round robin selector for the next output
	DH_Subband *outHolder = static_cast<DH_Subband *>(getDataManager().getOutHolder(getDataManager().getOutputSelector()->getCurrentSelection()));

	// Copy one subband from every input
	for (unsigned station = 0; station < itsNStations; station ++) {
	  DH_RSP *inHolder = static_cast<DH_RSP *>(getDataManager().getInHolder(station));

	  if (subband == 0)
	    inHolder->getExtraData();

	  memcpy(outHolder->getSamples3D()[station].origin(),
		 inHolder->getSamples()[subband].origin(),
		 outHolder->getSamples3D()[station].num_elements() * sizeof(DH_Subband::SampleType));

	  // copy other information (delayInfo, flags etc)
	  outHolder->getDelays()[station].delayAtBegin	= inHolder->getFineDelayAtBegin();
	  outHolder->getDelays()[station].delayAfterEnd = inHolder->getFineDelayAfterEnd();
	  outHolder->getFlags()[station]		= inHolder->getFlags();
	}

	outHolder->fillExtraData();

	while (theirNoAlarms == 0) 
	  {
	    // wait for alarm to go off
	    boost::thread::yield();
	  };
	// we handled one alarm, so decrease it
	theirNoAlarms--;
	getDataManager().readyWithOutHolder(getDataManager().getOutputSelector()->getCurrentSelection());
	getDataManager().getOutputSelector()->selectNext();
      }
    }

    void WH_SBCollect::postprocess() 
    {
      theirNoRunningWHs--;
      if (theirNoRunningWHs == 0)
	{
	  theirTimerSet = false;
	  if (itsFrequency != 0) {
	    // unset timer
#if USE_TIMER
	    struct itimerval value;
	    memset (&value, 0, sizeof(itimerval));
	    setitimer(ITIMER_REAL, &value, 0);
	    // remove sig handler
	    sighandler_t ret = signal(SIGALRM, SIG_DFL);
	    ASSERTSTR(ret != SIG_ERR, "WH_SBCollect couldn't unset signal handler for timer");    
#endif
	  }
	}
      cout<<"Sleeping to enable shutdown of threads"<<endl;
      sleep(30);
    }  

    void WH_SBCollect::timerSignal(int sig)
    {
      // set the number of frames that can be sent
      theirNoAlarms += theirNoRunningWHs;
    }
  } // namespace CS1

} // namespace LOFAR
