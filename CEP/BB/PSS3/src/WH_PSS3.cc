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
#include <PSS3/Calibrator.h>
#include <PSS3/Strategy.h>

using namespace LOFAR;

const int DefaultAntennaCount = 21;

// WH_PSS3::WH_PSS3 (const string& name,  bool outputAllIter, int number)
//   : WorkHolder        (1, 1, name, "WH_PSS3"),
//     itsCal            (0),
//     itsOutputAllIter  (outputAllIter),
//     itsNumber         (number)

// {
//   TRACER4("WH_PSS3 construction");
//   getDataManager().addInDataHolder(0, new DH_WorkOrder("in_0", "KS")); 
//   getDataManager().addOutDataHolder(0, new DH_WorkOrder("out_0", "KS"),
//    				    true);
//   // switch output channel trigger off
//   getDataManager().setAutoTriggerOut(0, false);
// }


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
    itsDDID           (ddid),
    itsModelType      (modelType),
    itsCalcUVW        (calcUVW),
    itsDataColName    (dataColName),
    itsResidualColName(residualColName),
    itsOutputAllIter  (outputAllIter),
    itsNumber         (number)

{
  TRACER4("WH_PSS3 construction");
  getDataManager().addInDataHolder(0, new DH_WorkOrder("in_0"));
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
// WH_PSS3* WH_PSS3::make (const string& name)
// {
//   return new WH_PSS3 (name, itsOutputAllIter, itsNumber);
// }

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
  itsCal = new Calibrator(itsMSName, itsMeqModel, itsSkyModel, itsDbType, 
			  itsDbName, itsDbPwd, itsDDID, itsAnt1, itsAnt2, 
			  itsModelType, itsCalcUVW, itsDataColName, itsResidualColName);
}

void WH_PSS3::process()
{
  TRACER4("WH_PSS3 process()");
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

  if (inp->getSolutionNumber() != -1)
  {
    TRACER1("Use start solution number " << inp->getSolutionNumber());
    startSol = (DH_Solution*)getDataManager().getInHolder(1);
  // To do: Use a previous solution    
    getDataManager().readyWithInHolder(1);
  }

  vector<string> resPNames(3);
  vector<double> resPValues(3);
  int iterNo = -1;

  DH_Solution* outp;

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
    int size = resPNames[0].size();
    int sourceNo = atoi(&resPNames[0][size-1]);
    if (sourceNo == 0)          // If source number is 10
    {
      outp->setRAValue(10, resPValues[0]);
      outp->setDECValue(10, resPValues[1]);
      outp->setStokesIValue(10, resPValues[2]);
    }
    else 
    {
      outp->setRAValue(sourceNo, resPValues[0]);
      outp->setDECValue(sourceNo, resPValues[1]);
      outp->setStokesIValue(sourceNo, resPValues[2]);
    }

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
}

void WH_PSS3::dump()
{
}

