//#  SI_Simple.cc:  The peeling calibration strategy
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

#include <PSS3/SI_Simple.h>
#include <Common/Debug.h>
#include <PSS3/CalibratorOld.h>

namespace LOFAR
{

SI_Simple::SI_Simple(CalibratorOld* cal, int argSize, char* args)
  : StrategyImpl(),
    itsCal(cal),
    itsCurIter(-1),
    itsFirstCall(true)
{
  AssertStr(argSize == sizeof(Simple_data), "Incorrect argument list");
  SI_Simple::Simple_data* sData = (SI_Simple::Simple_data*)args;
  itsNIter = sData->nIter;
  itsNSources = sData->nSources;
  itsTimeInterval = sData->timeInterval;
  TRACER1("Creating Simple strategy implementation with " 
	  << "number of iterations = " << itsNIter << ", "
	  << "number of sources = " << itsNSources << ", "
	  << " time interval = " << itsTimeInterval);

}

SI_Simple::~SI_Simple()
{}

bool SI_Simple::execute(vector<string>& parmNames, 
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
    itsCal->ShowSettings();

    for (unsigned int i=0; i < parmNames.size(); i++)      // Add all parms
    { 
      itsCal->addSolvableParm(parmNames[i]);
      TRACER1("Adding Parameter " << parmNames[i] );
    }
    itsCal->commitSolvableParms();

    itsCal->clearPeelSources();
    for (int srcNo = 1; srcNo <= itsNSources; srcNo++)
    {
      itsCal->addPeelSource(srcNo);
      TRACER1("Adding peel (predict) source no: " << srcNo);
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
    //  itsCal->SubtractOptimizedSources();
    itsCal->CommitOptimizedParameters(); // Write to parmTable

    TRACER1("Next interval");
    if (itsCal->advanceTimeIntervalIterator() == false) // Next time interval
    {                                    // Finished with all time intervals
      itsCurIter = -1;
      itsFirstCall = true;
      return false;       
    }
    itsCurIter = 0;                      // Reset iterator
  }

  // The actual solve
  TRACER1("Solve for " << itsNSources <<" sources, " 
       << " iteration = " << itsCurIter << " of " << itsNIter);
 
  itsCal->Run(resultParmNames, resultParmValues, resultQuality);

  resultIterNo = itsCurIter;
  return true;
}

} // namespace LOFAR
