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
#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_string.h>


//# local includes
#include "VHECR/TBBTrigger.h"
#include "VHECR/TBBReadCmd.h"
#include "VHECR/VHECRsettings.h"

// forward declaration


namespace LOFAR {
  namespace VHECR {
    
    class VHECRTask
    {
    public:
      explicit VHECRTask();
      VHECRTask(const string& cntlrName);
      ~VHECRTask();
      
      // define responsefunctionType
      int getReadCmd(std::vector<TBBReadCmd> &readCmd);
      
      void readTBBdata(std::vector<TBBReadCmd>	cmdVector);
      void addTrigger(const TBBTrigger&	trigger);
      void readAntennaPositions(string fileName, string antennaSelection);
      
      /*!
	\brief Put the parameters from the parset file into the VHECRTast object
	
	\param AntennaSet - E.g. "LBA_INNER"
	\param AntennaPositionsFile - Absolute path to the file with the antenna positions.
	\param Clock - Sampling clock speed in MHz (i.e.160 or 200) 
	\param NoCoincChann - The number of channels needed to detect a coincidence.
	\param CoincidenceTime - The time-range in seconds during which triggers are considered part of a coincidence.
	\param DoDirectionFit - Do a direction fit: none [0], simple [1], more fancy [2], etc. 
	\param MinElevation - Minimum elevation to accept a trigger in degrees.
	\param MaxFitVariance - Maximum variance (``badness of fit'') of the direction fit to still accept a trigger. 
	\param ParamExtension - String with "keyword=value;" pairs for additional parameters during development.
      */
      void setParameters(string AntennaSet, string AntennaPositionsFile, int Clock,
			 int NoCoincChann=48, float CoincidenceTime=1e-6, int DoDirectionFit=0, 
			 float MinElevation=30., float MaxFitVariance=100., string ParamExtension="",
			 float forcedDeadTime=10.);
      struct positionStruct {
        double x, y, z;
      };
      struct fitResultStruct {
        double theta, phi, mse;
      };
      string itsOutputFilename;
      string itsConfigurationFile;
      
uint32 totalCoincidences, badFits;

    private:

      string itsAntennaPositionsFile;
      string itsAntennaSelection;
      //uint32 itsDoDirectionFit;
      double itsForcedDeadTime;
      //uint32 itsNoCoincidenceChannels;
      //double itsMinElevation;
      //double itsMaxFitVariance;
      string itsParamExtension;
      uint32 itsSamplingRate;   // in Hz!!!
      //double itsCoincidenceTime;

      /*!
	\brief Setup and clear internal values, after setParameters() or reading in a configuration file

	\return <tt>true</tt> if everything went fine.

	Only opens output file if not already opened!
      */      
      bool setup();


#define VHECR_TASK_BUFFER_LENGTH (2*96)
#define NOFANTENNAS (96)

      ParameterSet*  itsParameterSet;
      VHECRsettings* itsSettings;
    
      FILE * itsLogfile;
      // avoid defaultconstruction and copying
      VHECRTask(const VHECRTask&);
      VHECRTask& operator=(const VHECRTask&);
      
      // remote function to call for saving triggers
      uint32 itsNrTriggers;	// just for statistics

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
      
          
      positionStruct antennaPositions[96]; // much faster than vector<position> !!
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
      fitResultStruct fitDirectionToCoincidence(int coincidenceIndex, uint32 nofChannels);
      void fitDirectionAndDistanceToCoincidence(int coincidenceIndex, uint32 nofChannels);

    };
    
  };//VHECR
};//LOFAR
#endif
