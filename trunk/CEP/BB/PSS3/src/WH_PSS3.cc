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

#include "PSS3/WH_PSS3.h"
#include "CEPFrame/Step.h"
#include "Common/Debug.h"
#include "PSS3/DH_WorkOrder.h"
#include "PSS3/DH_Parms.h"
#include "PSS3/Calibrator.h"
#include "PSS3/Strategy.h"

using namespace LOFAR;

WH_PSS3::WH_PSS3 (const string& name,  bool outputAllIter, int number)
  : WorkHolder        (1, 1, name, "WH_PSS3"),
    itsCal            (0),
    itsOutputAllIter  (outputAllIter),
    itsNumber         (number)

{
  TRACER4("WH_PSS3 construction");
  getDataManager().addInDataHolder(0, new DH_WorkOrder("in_0", "KS")); 
  getDataManager().addOutDataHolder(0, new DH_WorkOrder("out_0", "KS"),
   				    true);
  // switch output channel trigger off
  getDataManager().setAutoTriggerOut(0, false);
}


// WH_PSS3::WH_PSS3 (const string& name, const String & msName, 
// 		  const String& meqModel, const String& skyModel,
// 		  uInt ddid, const Vector<int>& ant1, 
// 		  const Vector<int>& ant2, const String& modelType, 
// 		  bool calcUVW, const String& dataColName, 
// 		  const String& residualColName, bool outputAllIter)
//   : WorkHolder        (1, 1, name, "WH_PSS3"),
//     itsCal            (0),
//     itsMSName         (msName),
//     itsMeqModel       (meqModel),
//     itsSkyModel       (skyModel),
//     itsDDID           (ddid),
//     itsAnt1           (ant1),
//     itsAnt2           (ant2),
//     itsModelType      (modelType),
//     itsCalcUVW        (calcUVW),
//     itsDataColName    (dataColName),
//     itsResidualColName(residualColName),
//     itsOutputAllIter  (outputAllIter)

// {
//   TRACER4("WH_PSS3 construction");
//   getDataManager().addInDataHolder(0, new DH_WorkOrder("in_0", "KS")); 
//   getDataManager().addOutDataHolder(0, new DH_WorkOrder("out_0", "KS"),
//    				    true);
//   // switch output channel trigger off
//   getDataManager().setAutoTriggerOut(0, false);
// }

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
  return new WH_PSS3 (name, itsOutputAllIter, itsNumber);
}

// WH_PSS3* WH_PSS3::make (const string& name)
// {
//   return new WH_PSS3 (name, itsMSName, itsMeqModel, itsSkyModel, itsDDID,
// 		      itsAnt1, itsAnt2, itsModelType, itsCalcUVW, 
// 		      itsDataColName, itsResidualColName, itsOutputAllIter);
// }

void WH_PSS3::preprocess()
{
  TRACER4("WH_PSS3 preprocess()");
   itsCal = new Calibrator();
//   itsCal = new Calibrator(itsMSName, itsMeqModel, itsSkyModel, itsDDID, itsAnt1, 
// 			    itsAnt2, itsModelType, itsCalcUVW, itsDataColName, 
// 			    itsResidualColName);
}

void WH_PSS3::process()
{
  TRACER4("WH_PSS3 process()");
  DH_WorkOrder* inp = (DH_WorkOrder*)getDataManager().getInHolder(0);
  cout << "Strategy number: " << inp->getStrategyNo() << endl;
 Strategy strat(inp->getStrategyNo(), itsCal, inp->getArgSize(), 
		inp->getVarArgsPtr());
  cout << "Parameter names: " << inp->getParam1Name() << ", " 
       << inp->getParam2Name() << ", " 
       << inp->getParam3Name() << endl;

  vector<string> pNames(3);
  pNames[0] = inp->getParam1Name();
  pNames[1] = inp->getParam2Name();
  pNames[2] = inp->getParam3Name();
  vector<float> pValues(3);
  int srcNumber = 0;

  DH_WorkOrder* outp = (DH_WorkOrder*)getDataManager().getOutHolder(0);

  // Execute the strategy until finished with all iterations, intervals 
  // and sources.
  int count = 0;
  while (strat.execute(pNames, pValues, *inp->getSolution(), srcNumber))
  {
    cout << "Executed strategy" << endl;
    if (itsOutputAllIter)   // Write output on every iteration
    {
      DH_WorkOrder* outp = (DH_WorkOrder*)getDataManager().getOutHolder(0);
      outp->setID(inp->getID());
      outp->setStatus(DH_WorkOrder::Executed);
      outp->setParam1Value(itsNumber*10+count);   // Temporary 
      outp->setParam2Value(itsNumber*10+1+count);
      outp->setParam3Value(itsNumber*10+2+count);
      outp->setSourceNo(srcNumber);
      outp->getSolution()->itsMu = (itsNumber - 0.01)-(0.01*count);
      outp->setStatus(DH_WorkOrder::Executed);
      getDataManager().readyWithOutHolder(0);
      count++;
    }
  }

  if (!itsOutputAllIter) // Write only the resulting output
  {  
    DH_WorkOrder* outp = (DH_WorkOrder*)getDataManager().getOutHolder(0);
    outp->setID(inp->getID());
    outp->setStatus(DH_WorkOrder::Executed);
    outp->setParam1Value(itsNumber*10);   // Temporary 
    outp->setParam2Value(itsNumber*10+1);
    outp->setParam3Value(itsNumber*10+2);
    outp->setSourceNo(srcNumber);
    outp->getSolution()->itsMu = (itsNumber - 0.01);
    outp->setStatus(DH_WorkOrder::Executed);
    getDataManager().readyWithOutHolder(0);
  }
}

void WH_PSS3::dump()
{
}

