//#  SI_Peeling.cc:  The peeling calibration strategy
//#
//#  Copyright (C) 2002-2003
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

#include <PSS3/SI_Peeling.h>
#include <Common/Debug.h>
#include <PSS3/Calibrator.h>

SI_Peeling::SI_Peeling(Calibrator* cal, int argSize, char* args)
  : StrategyImpl(),
    itsCal(cal),
    itsCurIter(-1),
    itsFirstCall(true)
{
  AssertStr(argSize == sizeof(Peeling_data), "Incorrect argument list");
  SI_Peeling::Peeling_data* pData = (SI_Peeling::Peeling_data*)args;
  itsNIter = pData->nIter;
  itsNSources = pData->nSources;
  itsStartSource = pData->startSource;
  itsCurSource = itsStartSource;
  itsTimeInterval = pData->timeInterval;
  TRACER1("Creating Peeling strategy implementation with " 
	  << "number of iterations = " << itsNIter << ", "
	  << "number of sources = " << itsNSources << ", "
	  << " time interval = " << itsTimeInterval);

}

SI_Peeling::~SI_Peeling()
{}

bool SI_Peeling::execute(vector<string>& parmNames, 
			 vector<string>& resultParmNames,
			 vector<double>& resultParmValues,
			 Quality& resultQuality,
			 int& resultIterNo)
{
  AssertStr(itsCal != 0, 
	    "Calibrator pointer not set for this peeling strategy");

  if (itsFirstCall)
  {
    itsCal->Initialize();
    itsCal->clearSolvableParms();
    for (unsigned int i=0; i < parmNames.size(); i++)      // Add all parms
    { 
      itsCal->addSolvableParm(parmNames[i], itsCurSource);
    }
    itsCal->commitSolvableParms();
    itsCal->resetTimeIntervalIterator();
    itsCal->advanceTimeIntervalIterator();
    TRACER1("Next interval");
   
    itsFirstCall = false;
  }

  TRACER1("Next iteration: " << itsCurIter+1);
  if (++itsCurIter >= itsNIter)          // Next iteration
  {                                      // Finished with all iterations
    TRACER1("Next interval");
    if (itsCal->advanceTimeIntervalIterator() == false) // Next time interval
    {                                    // Finished with all time intervals
      TRACER1("Next source: " << itsCurSource+1);
     if (++itsCurSource >= itsStartSource+itsNSources)  // Next source
      {
	itsCurIter = -1;
	itsCurSource = itsStartSource;
	itsFirstCall = true;
	return false;                    // Finished with all sources
      }
      else
      {
	itsCal->clearSolvableParms();
	for (unsigned int i=0; i < parmNames.size(); i++) // Add all parms
	{ 
	  itsCal->addSolvableParm(parmNames[i], itsCurSource);
	}
	itsCal->commitSolvableParms();
	itsCal->resetTimeIntervalIterator();

	itsCal->advanceTimeIntervalIterator();
	TRACER1("Next interval");
      }
    }
    itsCurIter = 0;                      // Reset iterator
  }

  // The actual solve
  TRACER1("Solve for source = " << itsCurSource << " of " << itsNSources 
       << " iteration = " << itsCurIter << " of " << itsNIter);
  itsCal->clearPeelSources();
  itsCal->addPeelSource(itsCurSource);
  itsCal->clearPeelMasks();
  itsCal->commitPeelSourcesAndMasks();
  itsCal->Run(resultParmNames, resultParmValues, resultQuality);
  itsCal->SubtractOptimizedSources();
  itsCal->CommitOptimizedParameters();
  resultIterNo = itsCurIter;
  return true;
}
