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

#include <lofar_config.h>

#include <PSS3/SI_Simple.h>
#include <Common/Debug.h>
#include <Common/KeyValueMap.h>
#include <PSS3/MeqCalibraterImpl.h>

namespace LOFAR
{

SI_Simple::SI_Simple(MeqCalibrater* cal, const KeyValueMap& args)
  : StrategyImpl(),
    itsCal(cal),
    itsCurIter(-1),
    itsFirstCall(true)
{
  itsNIter = args.getInt("nrIterations", 1);
  itsTimeInterval = args.getFloat("timeInterval", 10.0);
  itsStartChan = args.getInt("startChan", 0);
  itsEndChan = args.getInt("endChan", 0);
  itsAnt = (const_cast<KeyValueMap&>(args))["antennas"].getVecInt();
  itsSources = (const_cast<KeyValueMap&>(args))["sources"].getVecInt();
  itsNSources = itsSources.size();
  itsUseSVD = args.getBool("useSVD", false);
  itsSaveAllIter = args.getBool("saveAllIter", false);

  TRACER1("Creating Simple strategy implementation with " 
	  << "number of iterations = " << itsNIter << ", "
	  << "number of sources = " << itsNSources << ", "
	  << "time interval = " << itsTimeInterval << ", "
	  << "start channel = " << itsStartChan << ", "
	  << "end channel = " << itsEndChan << ", "
	  << "number of antennas = " << itsAnt.size() << ", "
	  << "use SVD = " << itsUseSVD << ", "
	  << "save parms at all iterations = " << itsSaveAllIter);

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
    itsCal->select(itsAnt, itsAnt, itsStartChan, itsEndChan);
    itsCal->setTimeInterval(itsTimeInterval);
    itsCal->clearSolvableParms();

    vector<string> emptyP(0);
    itsCal->setSolvableParms(parmNames, emptyP, true);

    vector<int> emptyS(0);
    itsCal->peel(itsSources, emptyS);           // add peel sources

    itsCal->showSettings();
    itsCal->resetIterator();
    itsCal->nextInterval();
    TRACER1("Next interval");

    itsCal->showParmValues();
    itsFirstCall = false;
  }

  TRACER1("Next iteration: " << itsCurIter+1);
  if (++itsCurIter >= itsNIter)          // Next iteration
  {                                      // Finished with all iterations
    itsCal->saveParms(); // Write to parmTable

    TRACER1("Next interval");
    if (itsCal->nextInterval() == false) // Next time interval
    {                                    // Finished with all time intervals
      itsCurIter = -1;
      itsFirstCall = true;
      return false;       
    }
    cout << "BBSTest: BeginOfIteration " << itsCurIter << endl;
    itsCurIter = 0;                      // Reset iterator
  }
  else
  {
    cout << "BBSTest: BeginOfIteration " << itsCurIter << endl;
  }

  // The actual solve
  TRACER1("Solve for " << itsNSources <<" sources, " 
       << " iteration = " << itsCurIter << " of " << itsNIter);
 
  itsCal->solve(itsUseSVD, resultParmNames, resultParmValues, resultQuality);

  itsCal->showParmValues();

  if (itsSaveAllIter)
  {
    itsCal->saveParms();
  }
  
  cout << "BBSTest: EndOfIteration " << itsCurIter << endl;

  resultIterNo = itsCurIter;
  return true;
}

} // namespace LOFAR
