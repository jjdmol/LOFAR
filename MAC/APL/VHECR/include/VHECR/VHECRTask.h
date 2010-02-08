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

#ifndef VHECR_VHECRTASK_H
#define VHECR_VHECRTASK_H

//# Common Includes
#include <Common/LofarLogger.h>
#include <Common/lofar_string.h>
#include <Common/lofar_datetime.h>

//# local includes
#include "VHECR/TBBTrigger.h"
#include "VHECR/TBBReadCmd.h"

// forward declaration


namespace LOFAR {
  namespace VHECR {
    
    class VHECRTask
    {
    public:
      VHECRTask();
      ~VHECRTask();
      
      // define responsefunctionType
	  int getReadCmd(std::vector<TBBReadCmd> &readCmd);
	  	
      void readTBBdata(std::vector<TBBReadCmd>	cmdVector);
      void addTrigger(const TBBTrigger&	trigger);
      struct position {
        double x, y, z;
      };
      string itsOutputFilename;
      string itsConfigurationFile;
      string antennaPositionsFile;
      string antennaSelection;
      uint32 doDirectionFit;
      uint32 totalCoincidences, badFits;

    private:

#define VHECR_TASK_BUFFER_LENGTH (2*96)

      int itsNoCoincidenceChannels;
      double itsCoincidenceTime;
      FILE * itsLogfile;
      // avoid defaultconstruction and copying
      VHECRTask(const VHECRTask&);
      VHECRTask& operator=(const VHECRTask&);
      
      // remote function to call for saving triggers
      uint32 itsNrTriggers;	// just for statistics
      uint32 itsSamplingRate;

      bool   itsInitialized;
      std::vector<TBBReadCmd> itsCommandVector;    // used as temporarely buffer
      

      // Buffer for trigger messages

      // Single element in the buffer
      struct triggerBuffElem {
	uint32	next;
	uint32	prev;
	uint32  no;
	uint32	RcuNr;
	uint32	SeqNr;
	uint32	Time;
	uint32	SampleNr;
	uint32	Sum;
	uint32	NrSamples;
	uint32	PeakValue;
        uint64  date;
	uint32	meanval;
	uint32	afterval;
      };
      
          
      position antennaPositions[96]; // much faster than vector<position> !!
      void readConfigFile(string fileName);
      string readableTime(const uint64 date);

      // All buffered triggers
      struct triggerBuffElem trigBuffer[VHECR_TASK_BUFFER_LENGTH];

      // Index of first and last element in buffer
      uint32 first, last;

      // Insert element into buffer, returns index of inserted element
      uint32 add2buffer(const TBBTrigger& trigger);
      void printCoincidence(int coincidenceIndex);

      int coincidenceCheck(uint32 latestindex, uint32 nChannles, double timeWindow);
      void fitDirectionToCoincidence(int coincidenceIndex, int nofChannels);
      void readAntennaPositions(string fileName, string antennaSelection);

    };
    
  };//VHECR
};//LOFAR
#endif
