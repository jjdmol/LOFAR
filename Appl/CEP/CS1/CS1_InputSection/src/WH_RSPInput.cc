//#  WH_RSPInput.cc: Catch RSP ethernet frames and synchronize RSP inputs 
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
#include <AMCBase/Epoch.h>
#include <CS1_InputSection/WH_RSPInput.h>
#include <CS1_Interface/DH_RSP.h>
#include <CS1_Interface/DH_Delay.h>
#include <Common/hexdump.h>
#include <APS/ParameterSet.h>
#include <Transport/TransportHolder.h>
#include <CS1_Interface/RSPTimeStamp.h>
#include <CS1_InputSection/BeamletBuffer.h>
#include <CS1_InputSection/InputThread.h>
#include <tinyCEP/Sel_RoundRobin.h>


// for timer
#include <signal.h>
#include <sys/time.h>
// for rand
#include <stdlib.h>
// for sleep (yield)
#include <boost/thread.hpp>

namespace LOFAR {
  namespace CS1 {

    int WH_RSPInput::theirNoRunningWHs = 0;
    int WH_RSPInput::theirNoAlarms = 0;
    bool WH_RSPInput::theirTimerSet = 0;

    WH_RSPInput::WH_RSPInput(const string& name, 
			     ACC::APS::ParameterSet& ps,
			     TransportHolder& th,
			     uint stationNr)
      : WorkHolder (1, 
		    ps.getInt32("Observation.NSubbands") * ps.getInt32("Observation.NStations") / (ps.getInt32("Input.NRSPBoards") * ps.getInt32("General.SubbandsPerPset") * ps.getInt32("BGLProc.PsetsPerCell")), 
		    name, 
		    "WH_RSPInput"),
	itsTH(th),
	itsStationNr(stationNr),
	itsPS (ps),
	itsBBuffer(0),
	itsPrePostTimer("pre/post"),
	itsProcessTimer("process"),
	itsGetElemTimer("getElem")
    {
      LOG_TRACE_FLOW_STR("WH_RSPInput constructor");    

      char str[32];

      // get parameters
      itsNSubbandsPerCell = ps.getInt32("General.SubbandsPerPset") * ps.getInt32("BGLProc.PsetsPerCell");
      itsNSamplesPerSec = ps.getInt32("Observation.NSubbandSamples");
      itsNHistorySamples = (ps.getInt32("BGLProc.NPPFTaps") - 1) * ps.getInt32("Observation.NChannels");
 
      // create incoming dataholder holding the delay information 
      getDataManager().addInDataHolder(0, new DH_Delay("DH_Delay", ps.getInt32("Input.NRSPBoards")));
 
      // create a outgoing dataholder for each subband
      vector<int> subbands;
     for (int s=0; s < itsNoutputs; s++) {
	snprintf(str, 32, "DH_RSP_out_%d", s);
	getDataManager().addOutDataHolder(s, new DH_RSP(str, itsPS)); 
	subbands.push_back(s);
      }
     // set round-robin output selector, but set start index as well
     // to optimize P2P transpose
     getDataManager().setOutputSelector(new Sel_RoundRobin(subbands,itsStationNr));

    }


    WH_RSPInput::~WH_RSPInput() 
    {
    }


    WorkHolder* WH_RSPInput::construct(const string& name,
				       ACC::APS::ParameterSet& ps,
				       TransportHolder& th,
				       uint stationNr)
    {
      return new WH_RSPInput(name, ps, th, stationNr);
    }


    WH_RSPInput* WH_RSPInput::make(const string& name)
    {
      return new WH_RSPInput(name, itsPS, itsTH, itsStationNr);
    }


    void WH_RSPInput::startThread()
    {
  
      /* start up thread which writes RSP data from ethernet link
	 into cyclic buffers */
      LOG_TRACE_FLOW_STR("WH_RSPInput starting thread");   
  
      ThreadArgs args;
      args.BBuffer            = itsBBuffer;
      args.th                 = &itsTH;
      args.ipHeaderSize       = itsPS.getInt32("Input.IPHeaderSize");
      args.frameHeaderSize    = itsPS.getInt32("Input.SzEPAheader");
      args.nTimesPerFrame     = itsPS.getInt32("Input.NTimesInFrame");
      args.nSubbandsPerFrame  = itsPS.getInt32("Input.NSubbandsPerFrame");

      args.frameSize          = args.frameHeaderSize + args.nSubbandsPerFrame * args.nTimesPerFrame * sizeof(Beamlet);
      args.ID                 = itsStationNr;

  
      if ((itsTH.getType() == "TH_File") || (itsTH.getType() == "TH_Null")) {
	// if we are reading from file, overwriting the buffer should not be allowed
	// this way we can work with smaller files
	itsBBuffer->setAllowOverwrite(false);
      }

      itsInputThreadObject = new InputThread(args);
      itsInputThread = new boost::thread(*itsInputThreadObject);
    }

    void WH_RSPInput::preprocess()
    {
      // create the buffer controller.
      int cyclicBufferSize = itsPS.getInt32("Input.NSamplesToBuffer");
      int subbandsToReadFromFrame = itsPS.getInt32("Observation.NSubbands") * itsPS.getInt32("Observation.NStations") / itsPS.getInt32("Input.NRSPBoards");
      ASSERTSTR(subbandsToReadFromFrame <= itsPS.getInt32("Input.NSubbandsPerFrame"), subbandsToReadFromFrame << " < " << itsPS.getInt32("Input.NSubbandsPerFrame"));

      itsBBuffer = new BeamletBuffer(cyclicBufferSize, subbandsToReadFromFrame, cyclicBufferSize/6, cyclicBufferSize/6);
      startThread();
      itsPrePostTimer.start();

      itsDelayCompensation = itsPS.getBool("Observation.DelayCompensation");

      // determine starttime
      double startTime = itsPS.getDouble("Observation.StartTime");
      double utc = 0;

#if 1
      // interpret the time as utc
      utc = startTime;
#else
      utc = AMC::Epoch(startTime).utc();
#endif
      int sampleFreq = itsPS.getInt32("Observation.SampleRate");
      int seconds = (int)floor(utc);
      int samples = (int)((utc - floor(utc)) * sampleFreq);

      itsSyncedStamp = TimeStamp(seconds, samples);

      cout<<"Starting buffer at "<<itsSyncedStamp<<endl;cout.flush();
      itsBBuffer->startBufferRead(itsSyncedStamp);
      cout<<"end of WH_RSPInput::preprocess"<<endl;cout.flush();


      if (!theirTimerSet) {
#define USE_TIMER 0
#if USE_TIMER
	sighandler_t ret = signal(SIGALRM, *WH_RSPInput::timerSignal);
	ASSERTSTR(ret != SIG_ERR, "WH_RSPInput couldn't set signal handler for timer");    
	struct itimerval value;
	memset (&value, 0, sizeof(itimerval));

	__time_t secs = 1;
	__time_t usecs = 0;
	// this means 1MHz is the highest frequency
	value.it_interval.tv_sec = secs;
	value.it_interval.tv_usec = usecs;
	value.it_value.tv_sec = sec;
	value.it_value.tv_usec = usecs;
	cout << "Setting timer interval to " << secs << "secs and " << usecs << "ms" << endl;

	setitimer(ITIMER_REAL, &value, 0);
#else
	theirNoAlarms = -1; //This will make sure data is written at maximum speed
#endif
      
	theirTimerSet = true;
      }
      theirNoRunningWHs++;

    }

    void WH_RSPInput::process() 
    { 
      //cout<<"begin of WH_RSPInput::process"<<endl;cout.flush();
      itsProcessTimer.start();

      // delay control
      DH_Delay *delayDHp = static_cast<DH_Delay *>(getDataManager().getInHolder(0));
      // Get delay from the delay controller
      timestamp_t delayedstamp = itsSyncedStamp;

      if (itsDelayCompensation) {
	delayedstamp += (*delayDHp)[itsStationNr].coarseDelay;
      }

      /* startstamp is the synced and delay-controlled timestamp to 
	 start from in cyclic buffer */
      vector<Beamlet *> subbandbuffer;
      SparseSet flags;

      // collect pointers to subbands in output dataholders
      for (int output = 0; output < itsNoutputs; output ++) {
	DH_RSP *rspDHp = static_cast<DH_RSP *>(getDataManager().getOutHolder(output));

	for (int subband = 0; subband < itsNSubbandsPerCell; subband++) {
	  subbandbuffer.push_back(reinterpret_cast<Beamlet *>(rspDHp->getSamples()[subband].origin()));
	}
      }
	  
      // get the data from the cyclic buffer
      itsGetElemTimer.start();

      itsBBuffer->getElements(subbandbuffer,
			      &flags,
			      delayedstamp - itsNHistorySamples, 
			      itsNSamplesPerSec + itsNHistorySamples);
      itsGetElemTimer.stop();

      // print flags
      cout<<"WH_RSP out "<<itsStationNr<<" "<<delayedstamp<<" flags: "<< flags <<endl;
      // printsamples

      // fill in the outgoing dataholders
      for (int output = 0; output < itsNoutputs; output++) { 
	DH_RSP *rspDHp = static_cast<DH_RSP *>(getDataManager().getOutHolder(output));
    
	// fill in the data
	rspDHp->getFlags() = flags;
	rspDHp->setStationID(itsStationNr);
	rspDHp->setTimeStamp(delayedstamp - itsNHistorySamples);
	rspDHp->fillExtraData();
 	rspDHp->setFineDelayAtBegin((*delayDHp)[itsStationNr].fineDelayAtBegin);
 	rspDHp->setFineDelayAfterEnd((*delayDHp)[itsStationNr].fineDelayAfterEnd);
      }    



      itsSyncedStamp += itsNSamplesPerSec;
      itsProcessTimer.stop();
      while (theirNoAlarms == 0) 
	{
	  // wait for alarm to go off
	  boost::thread::yield();
	};
      
      // we handled one alarm, so decrease it
      theirNoAlarms--;
    }

    void WH_RSPInput::postprocess()
    {
      theirNoRunningWHs--;
      if (theirNoRunningWHs == 0)
	{
	  theirTimerSet = false;
	  if (itsFrequency != 0) {
#if USE_TIMER
	    // unset timer
	    struct itimerval value;
	    memset (&value, 0, sizeof(itimerval));
	    setitimer(ITIMER_REAL, &value, 0);
	    // remove sig handler
	    sighandler_t ret = signal(SIGALRM, SIG_DFL);
	    ASSERTSTR(ret != SIG_ERR, "WH_RSPInput couldn't unset signal handler for timer");    
#endif
	  }
	}


      sleep(1);
      itsPrePostTimer.stop();

      cout<<"\nWH_Timers:"<<endl;

      itsPrePostTimer.print(clog);
      itsProcessTimer.print(clog);
      itsGetElemTimer.print(clog);

      cout<<"in WH_RSPInput postprocess"<<endl;
      //args.Connection         = 0;
      // stop writer thread
      InputThread::stopThreads();
      itsBBuffer->clear();
      cout<<"buffer cleared"<<endl;
      itsInputThread->join();
      delete itsInputThread;
      delete itsInputThreadObject;

      delete itsBBuffer;
      sleep(2);
    }

    void WH_RSPInput::dump() const 
    {
    }

    void WH_RSPInput::timerSignal(int)
    {
      // set the number of frames that can be sent
      theirNoAlarms += theirNoRunningWHs;
    }

  } // namespace CS1
} // namespace LOFAR
