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
    itsCurSource(1),
    itsFirstCall(true)
{
  AssertStr(argSize == sizeof(Peeling_data), "Incorrect argument list");
  SI_Peeling::Peeling_data* pData = (SI_Peeling::Peeling_data*)args;
  itsNIter = pData->nIter;
  itsNSources = pData->nSources;
  itsTimeInterval = pData->timeInterval;

//   mc->setTimeInterval(itsTimeInterval);
//   mc->clearSolvableParms();
}

SI_Peeling::~SI_Peeling()
{}

bool SI_Peeling::execute(Vector<String>& paramNames, 
			 Vector<float>& paramValues,
			 Solution& solutionQuality)
{
  AssertStr(itsCal != 0, 
	    "Calibrator pointer not set for this peeling strategy");
  if (itsCurSource > itsNSources)
  {
    return false;                   // Finished with all sources.
  }
  else
  {
    itsCal->Initialize();
    itsCal->OptimizeSource(itsCurSource, itsNIter);
    itsCurSource++;
    return true;
  }

//   if (itsFirstCall)
//   {
//     Vector<String> pp(3);
//     Vector<String> ep(3);
//     ostringstream oss[3];
//     oss[0] << paramNames[0] << itsCurSource;        pp[0] = oss[0].str();
//     oss[1] << paramNames[1] << itsCurSource;        pp[1] = oss[1].str();
//     oss[2] << paramNames[2] << itsCurSource;        pp[2] = oss[2].str();
//     itsMC->setSolvableParms (pp,ep,true);
//     itsMC->resetIterator();
//     itsMC->nextInterval();
//     cout << "Next interval" << endl;
//     itsFirstCall = false;
//   }

//   // Temporary code to get an idea:
//   if (++itsCurIter >= itsNIter)          // Peeling strategy: 
//   {                                     // inner loop over iterations, outer
//     cout << "Next interval" << endl;    // over sources.
//     if (itsMC->nextInterval() == false)    
//     {
//       if (++itsCurSource > itsNSources)
//       {
// 	return false;                   // Finished with all iterations,
//       }                                 // intervals and sources
//       else
//       {
// 	cout << "Next source" << endl;
// 	itsMC->clearSolvableParms();
// 	Vector<String> pp(3);
// 	Vector<String> ep(3);
// 	ostringstream oss[3];
// 	oss[0] << paramNames[0] << itsCurSource;        pp[0] = oss[0].str();
// 	oss[1] << paramNames[1] << itsCurSource;        pp[1] = oss[1].str();
// 	oss[2] << paramNames[2] << itsCurSource;        pp[2] = oss[2].str();
// 	itsMC->setSolvableParms (pp,ep,true);
// 	itsMC->resetIterator();
// 	itsMC->nextInterval();
// 	cout << "Next interval" << endl;
//       }
//     }
//     itsCurIter = 0;                   // Reset iterator
//   }
//   cout << "Next iteration: " << itsCurIter << endl;    
//   itsMC->solve(false);                      
//   itsMC->updateParms();
//   itsMC->saveResidualData();
//   itsMC->saveParms();
      
//   Vector<String> solution(3*itsNSources + 6);  // Output from the solve call
 
//   return true;
}
