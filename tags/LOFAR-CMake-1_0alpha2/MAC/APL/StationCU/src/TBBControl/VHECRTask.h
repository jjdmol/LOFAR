//#  VHECRTask.h: Controller for the TBBDriver
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

#ifndef STATIONCU_TBBCONTROL_VHECRTASK_H
#define STATIONCU_TBBCONTROL_VHECRTASK_H

//# Common Includes
#include <Common/LofarLogger.h>
#include <Common/lofar_string.h>
#include <Common/lofar_datetime.h>
#include <Common/lofar_vector.h>

//# local includes
#include "TBBTrigger.h"
#include "TBBReadCmd.h"

// forward declaration


namespace LOFAR {
  namespace StationCU {
   
    class VHECRTask
    {
    public:
      static VHECRTask*	instance();
	   ~VHECRTask();
            
      // define responsefunctionType
      void readTBBdata(vector<TBBReadCmd> cmdVector);
      void addTrigger(const TBBTrigger&	trigger);
      
    private:

#define VHECR_TASK_BUFFER_LENGTH (3*96)

      int itsNoCoincidenceChannels;
      double itsCoincidenceTime;

      // avoid defaultconstruction and copying
      VHECRTask();
      VHECRTask(const VHECRTask&);
      VHECRTask& operator=(const VHECRTask&);
      
      // remote function to call for saving triggers
      uint32 itsNrTriggers;	// just for statistics
      bool   itsInitialized;
      vector<TBBReadCmd> itsCommandVector;    // used as temporarely buffer
      

      // Buffer for trigger messages

      // Single element in the buffer
      struct triggerBuffElem {
	uint32	next;
	uint32	prev;	
	uint32	RcuNr;
	uint32	SeqNr;
	uint32	Time;
	uint32	SampleNr;
	uint32	Sum;
	uint32	NrSamples;
	uint32	PeakValue;
	double  date;
	uint32	meanval;
	uint32	afterval;
      };

      // All buffered triggers
      struct triggerBuffElem trigBuffer[VHECR_TASK_BUFFER_LENGTH];

      // Index of first and last element in buffer
      uint32 first, last;

      // Insert element into buffer, returns index of inserted element
      uint32 add2buffer(const TBBTrigger& trigger);

      int coincidenceCheck(uint32 latestindex, uint32 nChannles, double timeWindow);

    };
    
  };//StationCU
};//LOFAR
#endif
