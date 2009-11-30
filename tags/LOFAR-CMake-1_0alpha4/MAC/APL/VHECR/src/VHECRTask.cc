//#  VHECRTask.cc: Implementation of the VHECR algoritms
//#
//#  Copyright (C) 2007
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
#include <Common/LofarLogger.h>
// #include <APS/ParameterSet.h>
#include <time.h>

#include <VHECR/VHECRTask.h>

namespace LOFAR {
  //	using namespace ACC::APS;
  namespace VHECR {
    
    //
    // VHECRTask()
    //
    VHECRTask::VHECRTask() :
      itsNrTriggers (0),
      itsSamplingRate(200000000), // NB. Sampling rate 200 MHz assumed without info! Needs to be changed in the future.
    itsInitialized (false)
    {
      // set default parameters for coincidence
      itsNoCoincidenceChannels = 8;
      itsCoincidenceTime = 10.3e-6;
      cout << "Number of required coincident channels = " << itsNoCoincidenceChannels << ", coincidence time window = " << itsCoincidenceTime << endl;
      // Initialize the trigger messages buffer
      for (uint32 i=0; i<VHECR_TASK_BUFFER_LENGTH; i++){
	trigBuffer[i].next = i+1;
	trigBuffer[i].prev = i-1;
	trigBuffer[i].Time = 0;
	trigBuffer[i].SampleNr = 0;
	trigBuffer[i].date = 0;
      };
      first = 0;
      last = VHECR_TASK_BUFFER_LENGTH-1;
      trigBuffer[first].prev = VHECR_TASK_BUFFER_LENGTH; //This means "not there"

      LOG_DEBUG ("VHECR construction");
    }
    
    
    //
    // ~VHECRTask()
    //
    VHECRTask::~VHECRTask()
    {
      LOG_DEBUG ("VHECR destruction");
    }
    
    //
    // readTBBdata(vector<TBBReadCmd>	cmdVector)
    //
    void VHECRTask::readTBBdata(std::vector<TBBReadCmd>	cmdVector)
    {
      // for now we only print the info that comes in.
      std::vector<TBBReadCmd>::iterator	iter = cmdVector.begin();
      std::vector<TBBReadCmd>::iterator	end  = cmdVector.end();
      while (iter != end) {
	LOG_INFO_STR(*iter);
	iter++;
      }
    }
    
    string VHECRTask::readableTime(const uint64 date)
    {
      time_t unixtime = date / itsSamplingRate;
      uint32 sample = date % itsSamplingRate;
      double secfraction = sample / (double) itsSamplingRate;
      
      tm * timeRec;
      timeRec = gmtime ( &unixtime );
      
      char timeStr[50];
      sprintf (timeStr, "%02d:%02d:%02.6f", (timeRec->tm_hour)%24, timeRec->tm_min, (double) timeRec->tm_sec + secfraction);
      string outString = timeStr;      
      return outString;
    }
    
    //
    // addTrigger(trigger)
    //
    // THIS IS WERE THE DEVELOPMENT SHOULD TAKE PLACE.
    //

    void VHECRTask::addTrigger(const TBBTrigger& trigger) {
      int newindex, coincidenceIndex;
      //      cout << "Received trigger: " << trigger.itsRcuNr << ", " << trigger.itsTime <<endl;
      
      newindex = add2buffer(trigger);
//       cout << "Added trigger: " << trigBuffer[newindex].RcuNr << ", " << newindex << ", " 
//       	   << trigBuffer[newindex].date-1.1991456e+09 << endl;
//       itsNoCoincidenceChannels = 2;
//       itsCoincidenceTime = 10.3e-6;
      uint64 timeWindow = (uint64)(itsSamplingRate * itsCoincidenceTime);
      
      coincidenceIndex = coincidenceCheck(newindex, itsNoCoincidenceChannels, itsCoincidenceTime);
//      cout << "Done coincidence check for new index: " << newindex << ", for " << itsNoCoincidenceChannels << " coincindence channels, for window = " << itsCoincidenceTime << " seconds; result = " << coincidenceIndex << endl;

      static uint64 latestCoincidenceTime = 0; // we'll ensure that all coincidences we find are at least 1 time window apart.
                                               // That way we won't see every coincidence (n-8) times (n = # single triggers in one pulse)
      // warning: static vars are class vars, not instance vars! Should be done differently if more than one VHECRtask
      if ( (coincidenceIndex >= 0) && (trigBuffer[coincidenceIndex].date >= latestCoincidenceTime + timeWindow) ){
	cout << "Detected coincidence: " << trigBuffer[coincidenceIndex].RcuNr << ", " 
        << readableTime(trigBuffer[coincidenceIndex].date) << endl; //"; " << trigBuffer[coincidenceIndex].SampleNr << endl;

        latestCoincidenceTime = trigBuffer[coincidenceIndex].date;
	// This adds the trigger to the command queue.
	uint32 RcuNr      = trigBuffer[coincidenceIndex].RcuNr;
	uint32 Time       = trigBuffer[coincidenceIndex].Time;
	uint32 sampleTime = trigBuffer[coincidenceIndex].SampleNr;
	uint32 prePages   = 1;
	uint32 postPages  = 2;
	itsCommandVector.push_back(TBBReadCmd(RcuNr, Time, sampleTime, prePages, postPages));	
	itsNrTriggers++;
      };
      
      // All code for this event is [TEST] code
      if (!itsCommandVector.empty()) {
	readTBBdata(itsCommandVector);			// report that we want everything
	itsCommandVector.clear();					// clear buffer
      }
    }
    
    // Check the contents of the buffer if a coincidence is found
    int VHECRTask::coincidenceCheck(uint32 latestindex, uint32 nChannles, double timeWindow){
      uint32 i,foundRCUs[nChannles],nfound;
      uint32 startindex,runindex;
      
      uint64 refdate;
      uint64 timeWindow64 = (uint64)(itsSamplingRate * timeWindow);
      
      startindex = first;
      while ((startindex!=trigBuffer[latestindex].next) && (startindex < VHECR_TASK_BUFFER_LENGTH)) {
	runindex = trigBuffer[startindex].next;
	refdate=trigBuffer[startindex].date-timeWindow64;
	nfound=0;
	while ((runindex < VHECR_TASK_BUFFER_LENGTH) && (trigBuffer[runindex].date >= refdate)){
	  for (i=0; i<nfound; i++){
	    if (foundRCUs[i] == trigBuffer[runindex].RcuNr) { 
	      break; //break the for-loop;
	    };
	  };
	  if (i == nfound) { 
	    if (nfound+2 >= nChannles) { return startindex; };
	    foundRCUs[nfound] = trigBuffer[runindex].RcuNr;
	    nfound++;
	  };
	  runindex = trigBuffer[runindex].next;
	};
	startindex = trigBuffer[startindex].next;
      };
      return -1;
    };

    // Add a trigger message to the buffer. 
    uint32 VHECRTask::add2buffer(const TBBTrigger& trigger){
      uint64 date;
      uint32 newindex,runindex;

      date = (uint64) trigger.itsTime * itsSamplingRate + trigger.itsSampleNr;
      
      newindex = last;
      last = trigBuffer[last].prev;
      trigBuffer[last].next = VHECR_TASK_BUFFER_LENGTH;
      
      trigBuffer[newindex].RcuNr     = trigger.itsRcuNr;
      trigBuffer[newindex].SeqNr     = trigger.itsSeqNr;
      trigBuffer[newindex].Time      = trigger.itsTime;
      trigBuffer[newindex].SampleNr  = trigger.itsSampleNr;
      trigBuffer[newindex].Sum       = trigger.itsSum;
      trigBuffer[newindex].NrSamples = trigger.itsNrSamples;
      trigBuffer[newindex].PeakValue = trigger.itsPeakValue;
      trigBuffer[newindex].date      = date;
      
      runindex = first;
      while (runindex < VHECR_TASK_BUFFER_LENGTH){
	if (trigBuffer[runindex].date <= date) { 
	  break; 
	};
	runindex = trigBuffer[runindex].next;
      };
      trigBuffer[newindex].next = runindex;
      if (runindex == first){
	first = newindex;
	trigBuffer[newindex].prev = VHECR_TASK_BUFFER_LENGTH;
	trigBuffer[runindex].prev = newindex;
      } else {
	if (runindex >= VHECR_TASK_BUFFER_LENGTH){
	  trigBuffer[last].next = newindex;
	  trigBuffer[newindex].prev = last;
	  last = newindex;
	} else {
	  trigBuffer[(trigBuffer[runindex].prev)].next = newindex;
	  trigBuffer[newindex].prev = trigBuffer[runindex].prev;
	  trigBuffer[runindex].prev = newindex;
	};
      };
      return newindex;
    };
   
    
  }; // VHECR
}; // LOFAR
