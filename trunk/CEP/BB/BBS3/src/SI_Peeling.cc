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

#include <lofar_config.h>

#include <BBS3/SI_Peeling.h>
#include <Common/LofarLogger.h>
#include <Common/KeyValueMap.h>
#include <BBS3/MeqCalibraterImpl.h>

namespace LOFAR
{

SI_Peeling::SI_Peeling(MeqCalibrater* cal, const KeyValueMap& args)
  : StrategyImpl(),
    itsCal(cal),
    itsCurIter(-1),
    itsFirstCall(true)
{
  itsNIter = args.getInt("nrIterations", 0);
  itsNSources = args.getInt("nrSources", 1);
  itsStartSource = args.getInt("startSource", 1);
  itsCurSource = itsStartSource;
  itsTimeInterval = args.getFloat("timeInterval", 10.0);
  itsStartChan = args.getInt("startChan", 0);
  itsEndChan = args.getInt("endChan", 0);
  itsAnt = (const_cast<KeyValueMap&>(args))["antennas"].getVecInt();
 
  LOG_INFO_STR("Creating Peeling strategy implementation with " 
		     << "number of iterations = " << itsNIter << ", "
		     << "number of sources = " << itsNSources << ", "
		     << " time interval = " << itsTimeInterval<< ", "
		     << "start channel = " << itsStartChan << ", "
		     << "end channel = " << itsEndChan << ", "
		     << "number of antennas = " << itsAnt.size());

}

SI_Peeling::~SI_Peeling()
{}

bool SI_Peeling::execute(vector<string>& parmNames, 
			 vector<string>& resultParmNames,
			 vector<double>& resultParmValues,
			 Quality& resultQuality,
			 int& resultIterNo)
{
  ASSERTSTR(itsCal != 0, 
	    "Calibrator pointer not set for this peeling strategy");

  vector<string> srcParams;   // Source specific parameters
  vector<string> genParams;   // Non-source specific (general) parameters
  // Split source specific and other parameters
  splitVector(parmNames, genParams, srcParams);

  if (itsFirstCall)
  {
    itsCal->select(itsAnt, itsAnt, itsStartChan, itsEndChan);
    itsCal->setTimeInterval(itsTimeInterval);
    itsCal->clearSolvableParms();
    itsCal->showSettings();

    vector<string> emptyP;
    vector<string> params(genParams);
    for (unsigned int i=0; i < srcParams.size(); i++)      // Add source specific
    {                                                      // parameters
      ostringstream parm;
      parm << srcParams[i] << ".CP" << itsCurSource;
      params.push_back(parm.str());
    }
    itsCal->setSolvableParms(params, emptyP, true); 

    vector<int> emptyS;
    vector<int> sources(1);
    sources[1] = itsCurSource;
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
      LOG_TRACE_RTTI_STR("Next source: " << itsCurSource+1);
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

	vector<string> emptyP;
	vector<string> params(genParams);
	for (unsigned int i=0; i < srcParams.size(); i++)      // Add source specific
	{                                                      // parameters
	  ostringstream parm;
	  parm << srcParams[i] << ".CP" << itsCurSource;
	  params.push_back(parm.str());
	}
	itsCal->setSolvableParms(params, emptyP, true); 

	vector<int> emptyS;
	vector<int> sources(1);
	sources[1] = itsCurSource;
	itsCal->peel(sources, emptyS);

	itsCal->resetIterator();
	itsCal->nextInterval();
	LOG_TRACE_RTTI("Next interval");
      }
    }
    itsCurIter = 0;                      // Reset iterator
  }

  // The actual solve
  LOG_TRACE_RTTI_STR("Solve for source = " << itsCurSource << " of " << itsNSources 
		     << " iteration = " << itsCurIter << " of " << itsNIter);
 
  itsCal->solve(false, resultParmNames, resultParmValues, resultQuality);

  // itsCal->saveResidualData();  // N.B. This affects the state of the MS

  itsCal->saveParms(); // Write to parmTable
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
