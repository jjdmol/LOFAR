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
// #include <Common/ParameterSet.h>

#include "VHECRTask.h"

namespace LOFAR {
  namespace StationCU {
    
   //
   // [global] VHECRTask::instance()
   //
   VHECRTask* VHECRTask::instance()
   {
   	static	VHECRTask*		theirVHECRTask;
   
   	if (theirVHECRTask == 0) {
   		theirVHECRTask = new VHECRTask();
   	}
   	return (theirVHECRTask);
   }
    
    //
    // VHECRTask()
    //
    VHECRTask::VHECRTask() :
      itsNrTriggers (0),
      itsInitialized (false)
    {
      // set default parameters for coincidence
      itsNoCoincidenceChannels = 8;
      itsCoincidenceTime = 10.3e-6;
      // Initialize the trigger messages buffer
      for (uint32 i=0; i<VHECR_TASK_BUFFER_LENGTH; i++){
      	trigBuffer[i].next = i+1;
      	trigBuffer[i].prev = i-1;
      	trigBuffer[i].Time = 0;
      	trigBuffer[i].SampleNr = 0;
      	trigBuffer[i].date = 0.;
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
    void VHECRTask::readTBBdata(vector<TBBReadCmd>	cmdVector)
    {
      // for now we only print the info that comes in.
      vector<TBBReadCmd>::iterator	iter = cmdVector.begin();
      vector<TBBReadCmd>::iterator	end  = cmdVector.end();
      while (iter != end) {
      	LOG_INFO_STR(*iter);
      	iter++;
      }
    }
    
    //
    // addTrigger(trigger)
    //
    // THIS IS WERE THE DEVELOPMENT SHOULD TAKE PLACE.
    //
    void VHECRTask::addTrigger(const TBBTrigger& trigger)
    {
      int newindex, coincidenceIndex;
      //      cout << "Received trigger: " << trigger.itsRcuNr << ", " << trigger.itsTime <<endl;
      
      newindex = add2buffer(trigger);
//       cout << "Added trigger: " << trigBuffer[newindex].RcuNr << ", " << newindex << ", " 
//       	   << trigBuffer[newindex].date-1.1991456e+09 << endl;
//       itsNoCoincidenceChannels = 2;
//       itsCoincidenceTime = 10.3e-6;
      

      coincidenceIndex = coincidenceCheck(newindex, itsNoCoincidenceChannels, itsCoincidenceTime);
      if ( coincidenceIndex >= 0){
      	cout << "Detected coincidence: " << trigBuffer[coincidenceIndex].RcuNr << ", " 
      	     << (uint)trigBuffer[coincidenceIndex].date%(60*60*24)/(60*60) << ":"
      	     <<	(uint)trigBuffer[coincidenceIndex].date%(60*60)/60 << ":"
      	     << fmod(trigBuffer[coincidenceIndex].date,60) 
      	     << endl;

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
    int VHECRTask::coincidenceCheck(uint32 latestindex, uint32 nChannles, double timeWindow)
    {
      uint32 i,foundRCUs[nChannles],nfound;
      uint32 startindex,runindex;
      double refdate;
      
      startindex = first;
      while ((startindex!=trigBuffer[latestindex].next) && (startindex < VHECR_TASK_BUFFER_LENGTH)) {
      	runindex = trigBuffer[startindex].next;
      	refdate=trigBuffer[startindex].date-timeWindow;
      	nfound=0;
      	while ((runindex < VHECR_TASK_BUFFER_LENGTH) && (trigBuffer[runindex].date >= refdate)) {
      	  for (i=0; i<nfound; i++){
      	    if (foundRCUs[i] == trigBuffer[runindex].RcuNr) { 
      	      break; //break the for-loop;
      	    };
      	  };
      	  if (i == nfound) { 
      	    if (nfound+2 >= nChannles) { return startindex; }
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
    uint32 VHECRTask::add2buffer(const TBBTrigger& trigger)
    {
      double date;
      uint32 newindex,runindex;

      date = trigger.itsTime + trigger.itsSampleNr/200e6;
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
      while (runindex < VHECR_TASK_BUFFER_LENGTH) {
      	if (trigBuffer[runindex].date <= date) { 
      	  break; 
      	};
      	runindex = trigBuffer[runindex].next;
      };
      trigBuffer[newindex].next = runindex;
      if (runindex == first) {
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
  }; // StationCU
}; // LOFAR
