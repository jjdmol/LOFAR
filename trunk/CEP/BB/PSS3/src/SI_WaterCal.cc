//#  SI_WaterCal.cc:  The peeling calibration strategy
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

#include <PSS3/SI_WaterCal.h>
#include <Common/Debug.h>
#include <PSS3/CalibratorOld.h>
#include <unistd.h>

SI_WaterCal::SI_WaterCal(CalibratorOld* cal, int argSize, char* args)
  : StrategyImpl(),
    itsCal(cal),
    itsCurIter(-1),
    itsInitialized(false),
    itsFirstCall(true)
{
  AssertStr(argSize == sizeof(WaterCal_data), "Incorrect argument list");
  SI_WaterCal::WaterCal_data* pData = (SI_WaterCal::WaterCal_data*)args;
  itsNIter = pData->nIter;
  itsSourceNo = pData->sourceNo;
  itsTimeInterval = pData->timeInterval;
  TRACER1("Creating WaterCal strategy implementation with " 
	  << "number of iterations = " << itsNIter << ", "
	  << "source number = " << itsSourceNo << ", "
	  << " time interval = " << itsTimeInterval);

}

SI_WaterCal::~SI_WaterCal()
{}

bool SI_WaterCal::execute(vector<string>& parmNames, 
			 vector<string>& resultParmNames,
			 vector<double>& resultParmValues,
			 Quality& resultQuality,
			 int& resultIterNo)
{
  AssertStr(itsCal != 0, 
	    "Calibrator pointer not set for this peeling strategy");

  if (itsFirstCall)
  {
    if (!itsInitialized)
    {
      itsCal->Initialize();
      itsCal->ShowSettings();
      itsInitialized = true;
    }
    for (unsigned int i=0; i < parmNames.size(); i++)      // Add all parms
    { 
      itsCal->addSolvableParm(parmNames[i], itsSourceNo);
      TRACER1("SI_WaterCal::execute  Addding solvable parm " << parmNames[i]
	      << " for source " << itsSourceNo);
    }
    itsCal->commitSolvableParms();

    itsCal->clearPeelSources();
    itsCal->addPeelSource(itsSourceNo);
    vector<int>::const_iterator iter;
    for (iter = itsExtraPeelSrcs.begin(); iter!= itsExtraPeelSrcs.end(); iter++)
    {
      if (*iter != itsSourceNo)
      { 
	itsCal->addPeelSource(*iter);
	TRACER1("SI_WaterCal::execute : Adding an extra peel source " << *iter);
      }
    }
    itsCal->commitPeelSourcesAndMasks();
    
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
      itsCurIter = -1;
      itsInitialized = false;
      itsFirstCall = true;
      return false;                      // Finished with all sources
    }
    itsCurIter = 0;                      // Reset iterator
  }

  // The actual solve
  TRACER1("Solve for source = " << itsSourceNo  
	  << " iteration = " << itsCurIter << " of " << itsNIter);

  itsCal->Run(resultParmNames, resultParmValues, resultQuality);
  //  itsCal->SubtractOptimizedSources();
  itsCal->showCurrentParms ();

  itsCal->CommitOptimizedParameters();
  resultIterNo = itsCurIter;
  return true;
}

bool SI_WaterCal::useParms (const vector<string>& parmNames, 
			    const vector<double>& parmValues, 
			    const vector<int>& srcNumbers)
{
  AssertStr(itsCal != 0, 
	    "Calibrator pointer not set for this peeling strategy");

  itsCal->Initialize();
  itsInitialized = true;
  //itsCal->ShowSettings();
  itsCal->resetTimeIntervalIterator();
  itsCal->advanceTimeIntervalIterator();

  itsCal->setParmValues(parmNames, parmValues);

  for (vector<string>::const_iterator iter = parmNames.begin();
       iter != parmNames.end(); iter++)        // Add all parms
  { 
    itsCal->addSolvableParm(*iter);
    TRACER1("SI_WaterCal::useParms  Addding solvable parm " << *iter);
  }
  itsCal->commitSolvableParms();

  itsCal->commitAllSolvableParameters();         // save values in parm table

  itsCal->clearSolvableParms();

  for (unsigned int i=0; i < srcNumbers.size(); i++)      
  { 
    itsExtraPeelSrcs.push_back(srcNumbers[i]);
    TRACER1("SI_WaterCal::useParms : Using a start solution for source " 
	    << srcNumbers[i]);
  }

  return true;
}
