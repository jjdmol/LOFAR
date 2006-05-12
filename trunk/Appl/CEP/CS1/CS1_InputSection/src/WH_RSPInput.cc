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
#include <AMCBase/TimeCoord.h>
#include <CS1_InputSection/WH_RSPInput.h>
#include <CS1_Interface/DH_RSP.h>
#include <CS1_Interface/DH_Delay.h>
#include <Common/hexdump.h>
#include <Common/Timer.h>
#include <APS/ParameterSet.h>
#include <Transport/TransportHolder.h>
#include <CS1_Interface/RSPTimeStamp.h>
#include <CS1_InputSection/BeamletBuffer.h>
#include <CS1_InputSection/InputThread.h>

namespace LOFAR {
  namespace CS1 {

    WH_RSPInput::WH_RSPInput(const string& name, 
			     ACC::APS::ParameterSet& ps,
			     TransportHolder& th,
			     uint stationNr)
      : WorkHolder (1, 
		    ps.getInt32("Observation.NSubbands") * ps.getInt32("Observation.NStations") / (ps.getInt32("Input.NRSPBoards") * ps.getInt32("General.SubbandsPerCell")), 
		    name, 
		    "WH_RSPInput"),
	itsTH(th),
	itsStationNr(stationNr),
	itsPS (ps),
	itsBBuffer(0)
    {
      LOG_TRACE_FLOW_STR("WH_RSPInput constructor");    

      char str[32];

      // get parameters
      itsNSubbands = ps.getInt32("Observation.NSubbands") * ps.getInt32("Observation.NStations") / ps.getInt32("Input.NRSPBoards");
      itsNSubbandsPerCell = ps.getInt32("General.SubbandsPerCell");
      itsNSamplesPerSec = ps.getInt32("Observation.NSubbandSamples");
      itsNHistorySamples = (ps.getInt32("BGLProc.NPPFTaps") - 1) * ps.getInt32("Observation.NChannels");
 
      // create incoming dataholder holding the delay information 
      getDataManager().addInDataHolder(0, new DH_Delay("DH_Delay", ps.getInt32("Input.NRSPBoards")));
 
      // create a outgoing dataholder for each subband
      for (int s=0; s < itsNoutputs; s++) {
	snprintf(str, 32, "DH_RSP_out_%d", s);
	getDataManager().addOutDataHolder(s, new DH_RSP(str, itsPS)); 
      }
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
      args.nSubbandsPerFrame  = itsNSubbands;

      args.frameSize          = args.frameHeaderSize + args.nSubbandsPerFrame * args.nTimesPerFrame * sizeof(Beamlet);

  
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
      itsBBuffer = new BeamletBuffer(cyclicBufferSize, itsNSubbands, cyclicBufferSize/6, cyclicBufferSize/6);
      startThread();
      itsPrePostTimer = new NSTimer("pre/post");
      itsProcessTimer = new NSTimer("process");
      itsGetElemTimer = new NSTimer("getElem");
      itsTimers.push_back(itsPrePostTimer);
      itsTimers.push_back(itsProcessTimer);
      itsTimers.push_back(itsGetElemTimer);
      itsPrePostTimer->start();

      // determine starttime
      double startTime = itsPS.getDouble("Observation.StartTime");
      if (startTime > 0) {
	double utc = AMC::TimeCoord(startTime).utc();
	int sampleFreq = itsPS.getInt32("Observation.SampleRate");
	int seconds = floor(utc);
	int samples = (utc - seconds) * sampleFreq;

	itsSyncedStamp = TimeStamp(seconds, samples);
      } else { 
	itsSyncedStamp = TimeStamp(0, 0);
      }

      cout<<"Starting buffer at "<<itsSyncedStamp<<endl;cout.flush();
      itsBBuffer->startBufferRead(itsSyncedStamp);
      cout<<"end of WH_RSPInput::preprocess"<<endl;cout.flush();
    }

    void WH_RSPInput::process() 
    { 
      cout<<"begin of WH_RSPInput::process"<<endl;cout.flush();
      itsProcessTimer->start();
      DH_RSP* rspDHp;
      DH_Delay* delayDHp;
      timestamp_t delayedstamp;

      // delay control
      delayDHp = (DH_Delay*)getDataManager().getInHolder(0);
      // Get delay from the delay controller
      delayedstamp = itsSyncedStamp + delayDHp->getCoarseDelay(itsStationNr);    

      /* startstamp is the synced and delay-controlled timestamp to 
	 start from in cyclic buffer */
      vector<Beamlet *>   subbandbuffer;
      SparseSet flags;

      // collect pointers to subbands in output dataholders
      for (int output = 0; output < itsNoutputs; output ++) {
	rspDHp = (DH_RSP*)getDataManager().getOutHolder(output);
	for (int subband = 0; subband < itsNSubbandsPerCell; subband++) {
	  subbandbuffer.push_back((Beamlet*)rspDHp->getBuffer());
	}
      }
	  
      // get the data from the cyclic buffer
      itsGetElemTimer->start();
      cout<<"reading from buffer"<<endl;cout.flush();

      itsBBuffer->getElements(subbandbuffer,
			      &flags,
			      delayedstamp - itsNHistorySamples, 
			      itsNSamplesPerSec + itsNHistorySamples);
      cout<<"done reading from buffer"<<endl;cout.flush();
      itsGetElemTimer->stop();

      // fill in the outgoing dataholders
      for (int output = 0; output < itsNoutputs; output++) { 
	rspDHp = (DH_RSP*)getDataManager().getOutHolder(output);
    
	// fill in the data
	rspDHp->getFlags() = flags;
	rspDHp->setStationID(itsStationID);
	rspDHp->setTimeStamp(delayedstamp);
	rspDHp->fillExtraData();
	rspDHp->setFineDelayAtBegin(delayDHp->getFineDelayAtBegin(itsStationID));
	rspDHp->setFineDelayAfterEnd(delayDHp->getFineDelayAfterEnd(itsStationID));
      }    

      itsSyncedStamp += itsNSamplesPerSec;
      itsProcessTimer->stop();
    }

    void WH_RSPInput::postprocess()
    {
      sleep(1);
      itsPrePostTimer->stop();

      cout<<"\nWH_Timers:"<<endl;
      vector<NSTimer*>::iterator it = itsTimers.begin();
      for (; it != itsTimers.end(); it++) {
	(*it)->print(cout);
      }

      delete itsPrePostTimer;
      delete itsProcessTimer;
      delete itsGetElemTimer;
      itsPrePostTimer = 0;
      itsProcessTimer = 0;
      itsGetElemTimer = 0;

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

  } // namespace CS1
} // namespace LOFAR
