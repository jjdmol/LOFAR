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
#include <PSS3/CalibratorOld.h>

namespace LOFAR
{

SI_Peeling::SI_Peeling(CalibratorOld* cal, int argSize, char* args)
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

  vector<string> srcParams;   // Source specific parameters
  vector<string> genParams;   // Non-source specific (general) parameters
  // Split source specific and other parameters
  splitVector(parmNames, genParams, srcParams);

  if (itsFirstCall)
  {
    itsCal->Initialize();
    itsCal->ShowSettings();
    for (unsigned int i=0; i < srcParams.size(); i++)      // Add source specific
    {                                                      // parameters
      itsCal->addSolvableParm(srcParams[i], itsCurSource);
    }
    for (unsigned int j=0; j < genParams.size(); j++)      // Add all other parameters
    { 
      itsCal->addSolvableParm(genParams[j]);
    }
    itsCal->commitSolvableParms();

    itsCal->clearPeelSources();
    itsCal->addPeelSource(itsCurSource);
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
	for (unsigned int i=0; i < srcParams.size(); i++)    // Add source specific
	{                                                    // parameters
	  itsCal->addSolvableParm(srcParams[i], itsCurSource);
	}
	for (unsigned int j=0; j < genParams.size(); j++)   // Add all other parameters
	{ 
	  itsCal->addSolvableParm(genParams[j]);
	}
	itsCal->commitSolvableParms();
	//itsCal->clearPeelSources(); ///Temporary!!!!
	itsCal->addPeelSource(itsCurSource);
	itsCal->commitPeelSourcesAndMasks();

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
 
  itsCal->Run(resultParmNames, resultParmValues, resultQuality);

  // itsCal->SubtractOptimizedSources();  // N.B. This affects the state of the MS
  itsCal->CommitOptimizedParameters(); // Write to parmTable
  resultIterNo = itsCurIter;
  return true;
}

void SI_Peeling::splitVector(vector<string>& allParams, vector<string>& params, 
			     vector<string>& srcParams) const
{
  params.clear();
  srcParams.clear();
  for (unsigned int index=0; index < allParams.size(); index++)
  {
    string::size_type dotPos;
    string subStr;
    bool isGenParm = true;   // Is this a general parameter?
    dotPos = allParams[index].find_last_of(".");
    if (dotPos != string::npos)
    {
      string::size_type pos;
      pos = allParams[index].find("CP", dotPos);
      if (pos == dotPos+1)
      {                            // Source specific parameter
	isGenParm = false;
	subStr = allParams[index].substr(0, pos-1);
	bool srcFound = false;
	for (unsigned int nr = 0; nr<srcParams.size(); nr++) // Check for double entries in srcParams
	{
	  if (srcParams[nr] == subStr)
	  { 
	    srcFound = true; 
	    break;
	  }
	}
	if (srcFound == false)        // Add to source specific parameter vector
	{
	  srcParams.push_back(subStr);
	}
      }
    }

    if (isGenParm == true)           // Add to general parameter vector
    {
      params.push_back(allParams[index]);
    }

  } 
}


} // namespace LOFAR
