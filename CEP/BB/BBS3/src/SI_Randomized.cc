//#  SI_Randomized.cc:  A random calibration strategy
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

#include <lofar_config.h>

#include <BBS3/SI_Randomized.h>
#include <Common/LofarLogger.h>
#include <Common/KeyValueMap.h>
#include <BBS3/MeqCalibraterImpl.h>
#include <unistd.h>

namespace LOFAR
{

SI_Randomized::SI_Randomized(MeqCalibrater* cal, const KeyValueMap& args)
  : StrategyImpl(),
    itsCal(cal),
    itsCurIter(-1),
    itsInitialized(false),
    itsFirstCall(true)
{
  itsNIter = args.getInt("nrIterations", 1);
  itsSourceNo = args.getInt("source", 1);
  itsTimeInterval = args.getFloat("timeInterval", 10.0);
  itsStartChan = args.getInt("startChan", 0);
  itsEndChan = args.getInt("endChan", 0);
  itsAnt = (const_cast<KeyValueMap&>(args))["antennas"].getVecInt();

  LOG_INFO_STR("Creating Randomized strategy implementation with " 
		     << "number of iterations = " << itsNIter << ", "
		     << "source number = " << itsSourceNo << ", "
		     << " time interval = " << itsTimeInterval);

}

SI_Randomized::~SI_Randomized()
{}

bool SI_Randomized::execute(vector<string>& parmNames, 
			 vector<string>& resultParmNames,
			 vector<double>& resultParmValues,
			 Quality& resultQuality,
			 int& resultIterNo)
{
  ASSERTSTR(itsCal != 0, 
	    "Calibrator pointer not set for this random strategy");

  if (itsFirstCall)
  {
    if (!itsInitialized)
    {
      itsCal->select(itsAnt, itsAnt, itsStartChan, itsEndChan);
      itsCal->setTimeInterval(itsTimeInterval);
      itsCal->clearSolvableParms();
      itsCal->showSettings();
      itsInitialized = true;
    }

    vector<string> emptyP(0);
    itsCal->setSolvableParms(parmNames, emptyP, true);

    vector<int> emptyS;
    vector<int> sources;
    sources.push_back(itsSourceNo);
    vector<int>::const_iterator iter;
    for (iter = itsExtraPeelSrcs.begin(); iter!= itsExtraPeelSrcs.end(); iter++)
    {
      if (*iter != itsSourceNo)
      { 
	sources.push_back(*iter);
	LOG_TRACE_RTTI_STR("SI_Randomized::execute : Adding an extra peel source " << *iter);
      }
    }
    itsCal->peel(sources, emptyS);    
  
    itsCal->resetIterator();
    itsCal->nextInterval();
    LOG_TRACE_RTTI("Next interval");
   
    itsFirstCall = false;
  }

  LOG_TRACE_RTTI_STR("Next iteration: " << itsCurIter+1);
  if (++itsCurIter >= itsNIter)          // Next iteration
  {                                      // Finished with all iterations
    LOG_TRACE_RTTI("Next interval");
    if (itsCal->nextInterval() == false) // Next time interval
    {                                    // Finished with all time intervals
      itsCurIter = -1;
      itsInitialized = false;
      itsFirstCall = true;
      return false;                      // Finished with all sources
    }
    itsCurIter = 0;                      // Reset iterator
  }

  // The actual solve
  LOG_TRACE_RTTI_STR("Solve for source = " << itsSourceNo  
		     << " iteration = " << itsCurIter << " of " << itsNIter);

  itsCal->solve(false, resultParmNames, resultParmValues, resultQuality);
  //  itsCal->SubtractOptimizedSources();
  itsCal->saveParms();
  resultIterNo = itsCurIter;
  return true;
}

bool SI_Randomized::useParms (const vector<string>& parmNames, 
			    const vector<double>& parmValues, 
			    const vector<int>& srcNumbers)
{
  ASSERTSTR(itsCal != 0, 
	    "Calibrator pointer not set for this random strategy");

  itsCal->select(itsAnt, itsAnt, itsStartChan, itsEndChan);
  itsCal->setTimeInterval(itsTimeInterval);
  itsCal->clearSolvableParms();
  itsInitialized = true;
  itsCal->showSettings();
  itsCal->resetIterator();
  itsCal->nextInterval();

  itsCal->setParmValues(parmNames, parmValues);

  vector<string> emptyP(0);
  itsCal->setSolvableParms(const_cast<vector<string>&>(parmNames), 
                           emptyP, true); // Add all parms

  itsCal->saveParms();                    // save values in parm table

  itsCal->clearSolvableParms();

  for (unsigned int i=0; i < srcNumbers.size(); i++)      
  { 
    itsExtraPeelSrcs.push_back(srcNumbers[i]);
    LOG_TRACE_RTTI_STR("SI_Randomized::useParms : Using a start solution for source " 
	    << srcNumbers[i]);
  }
  return true;
}

} // namespace LOFAR
