//  WH_PSS3.cc:
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>             // for sprintf

#include <PSS3/WH_PSS3.h>
#include <CEPFrame/Step.h>
#include <Common/Debug.h>
#include <PSS3/DH_WorkOrder.h>
#include <PSS3/DH_Solution.h>
#include <PSS3/CalibratorOld.h>
#include <PSS3/Strategy.h>

using namespace LOFAR;

const int DefaultAntennaCount = 21;

const int MaxNumberOfParms = 30;

WH_PSS3::WH_PSS3 (const string& name, const string & msName, 
		  const string& meqModel, const string& skyModel,
		  const string& dbType, const string& dbName,
		  const string& dbPwd, unsigned int ddid, 
		  const string& modelType, bool calcUVW, 
		  const string& dataColName, 
		  const string& residualColName, bool outputAllIter, 
		  int number)
  : WorkHolder        (2, 1, name, "WH_PSS3"),
    itsCal            (0),
    itsMSName         (msName),
    itsMeqModel       (meqModel),
    itsSkyModel       (skyModel),
    itsDbType         (dbType),
    itsDbName         (dbName),
    itsDbPwd          (dbPwd),
    itsDDID           (ddid),
    itsModelType      (modelType),
    itsCalcUVW        (calcUVW),
    itsDataColName    (dataColName),
    itsResidualColName(residualColName),
    itsOutputAllIter  (outputAllIter),
    itsNumber         (number)

{
  TRACER4("WH_PSS3 construction");
  getDataManager().addInDataHolder(0, new DH_WorkOrder(name));
  getDataManager().addInDataHolder(1, new DH_Solution("in_1", "KS"));
  getDataManager().addOutDataHolder(0, new DH_Solution("out_0", "KS"),true);
  // switch input and output channel trigger off
  getDataManager().setAutoTriggerIn(1, false);
  getDataManager().setAutoTriggerOut(0, false);

  for (int i = 0; i < DefaultAntennaCount; i ++)
  {
    itsAnt1.push_back (4 * i);
    itsAnt2.push_back (4 * i);
  }
}

WH_PSS3::~WH_PSS3()
{
  TRACER4("WH_PSS3 destructor");
  if (itsCal != 0)    
  {                  
    delete itsCal;   
  }                  
}


WH_PSS3* WH_PSS3::make (const string& name)
{
  return new WH_PSS3 (name, itsMSName, itsMeqModel, itsSkyModel, itsDbType,
		      itsDbName, itsDbPwd,  itsDDID, itsModelType, 
		      itsCalcUVW, itsDataColName, itsResidualColName, 
		      itsOutputAllIter, itsNumber);
}

void WH_PSS3::preprocess()
{
  TRACER4("WH_PSS3 preprocess()");
}

void WH_PSS3::process()
{
  TRACER4("WH_PSS3 process()");
  itsCal = new CalibratorOld(itsMSName, itsMeqModel, itsSkyModel, itsDbType, 
			     itsDbName, itsDbPwd);

  DH_WorkOrder* inp = (DH_WorkOrder*)getDataManager().getInHolder(0);
  DH_Solution* startSol;
  TRACER1("Strategy number: " << inp->getStrategyNo());
  Strategy strat(inp->getStrategyNo(), itsCal, inp->getArgSize(), 
		inp->getVarArgsPtr());
  TRACER1("Parameter names: " << inp->getParam1Name() << ", " 
	  << inp->getParam2Name() << ", " 
	  << inp->getParam3Name());

  vector<string> pNames(3);
  pNames[0] = inp->getParam1Name();
  pNames[1] = inp->getParam2Name();
  pNames[2] = inp->getParam3Name();

  DH_Solution* outp;
  outp = (DH_Solution*)getDataManager().getOutHolder(0);
  outp->clearData();

  if (inp->getSolutionNumber() != -1)
  {
    TRACER1("Use start solution number " << inp->getSolutionNumber());
    DH_Solution* solObj = (DH_Solution*)getDataManager().getInHolder(1);
    solObj->setSolutionID(inp->getSolutionNumber());
    getDataManager().readyWithInHolder(1);
    vector<string> startNames;
    vector<double> startValues;
    vector<int> startSource;
    startSol = (DH_Solution*)getDataManager().getInHolder(1);
    putSolutionIntoVectors(startSol, startNames, startValues, startSource);
    strat.useParms(startNames, startValues,startSource); 
    putVectorsIntoSolution(outp, startNames, startValues);
    getDataManager().readyWithInHolder(1);
  }

  vector<string> resPNames(MaxNumberOfParms);
  vector<double> resPValues(MaxNumberOfParms);
  int iterNo = -1;

  // Execute the strategy until finished with all iterations, intervals 
  // and sources.
  int count = 0;
  Quality resQuality;
  resQuality.init();

  while (strat.execute(pNames, resPNames, resPValues, resQuality, iterNo))
  {
    TRACER1("Executed strategy");
    outp = (DH_Solution*)getDataManager().getOutHolder(0);
    outp->setWorkOrderID(inp->getWorkOrderID());

    putVectorsIntoSolution(outp, resPNames, resPValues);

    outp->setIterationNo(iterNo);
    outp->getQuality()->itsFit = resQuality.itsFit;
    outp->getQuality()->itsMu = resQuality.itsMu;
    outp->getQuality()->itsStddev = resQuality.itsStddev;
    outp->getQuality()->itsChi = resQuality.itsChi;
    if (itsOutputAllIter)   // Write output on every iteration
    {
      outp->setID(itsNumber++);
      getDataManager().readyWithOutHolder(0);
    }
    count++;
    
  }

  if (!itsOutputAllIter) // Write only the resulting output
  {  
    outp->setID(itsNumber++);
    getDataManager().readyWithOutHolder(0);
  }

  delete itsCal;
}

void WH_PSS3::dump()
{
}

void WH_PSS3::putVectorsIntoSolution(DH_Solution* dh, const vector<string>& pNames, 
				const vector<double>& pValues)
{
  // Warning: this method assumes there are always 3 parameters solved for
  //          each source!
  DbgAssertStr(pNames.size() == pValues.size(), 
	       "Sizes of resulting name and values vectors are not equal");
  int numberOfSources = pNames.size()/3;
  int index;
  int strSize;
  int sourceNo;
  for (int count = 0; count < numberOfSources; count++)
  {
    index = count * 3;
    strSize = pNames[index].size();
    sourceNo = atoi(&pNames[index][strSize-1]);
    if (sourceNo == 0)          // If source number is 10
    {
      sourceNo = 10;
    }
    dh->setRAValue(sourceNo, pValues[index]);
    dh->setDECValue(sourceNo, pValues[index+1]);
    dh->setStokesIValue(sourceNo, pValues[index+2]);
  }
}

void WH_PSS3::putSolutionIntoVectors(DH_Solution* dh, vector<string>& pNames, 
				     vector<double>& pValues, vector<int>& pSources)
{
  // Warning: this method assumes there are always 3 parameters solved for
  //          each source and maximum number of sources is 10

  TRACER1("WH_PSS3::putSolutionIntoVectors");

  string RAname = "RA.CP";
  string DECname = "DEC.CP";
  string StokesIname = "StokesI.CP";

  for (int srcNo = 1; srcNo <= 10; srcNo++)
  {
    if (dh->getRAValue(srcNo) != 0)
    {
      char* src;
      sprintf(src, "%d", srcNo);
      pNames.push_back(RAname+src);
      pValues.push_back(dh->getRAValue(srcNo));
      pSources.push_back(srcNo);
      pNames.push_back(DECname+src);
      pValues.push_back(dh->getDECValue(srcNo));
      pNames.push_back(StokesIname+src);
      pValues.push_back(dh->getStokesIValue(srcNo));
    }
  }
  
}
