//#  SC_WritePredData.cc:  A simple calibration strategy
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

#include <BBS3/SC_WritePredData.h>
#include <Common/LofarLogger.h>
#include <BBS3/DH_Solution.h>
#include <BBS3/DH_WOPrediff.h>
#include <BBS3/DH_WOSolve.h>
#include <BBS3/BBSTestLogger.h>

namespace LOFAR
{

SC_WritePredData::SC_WritePredData(Connection* inSolConn, Connection* outWOPDConn, 
		     Connection* outWOSolveConn, int nrPrediffers,
		     const ParameterSet& args)
  : StrategyController(inSolConn, outWOPDConn, outWOSolveConn, 
		       nrPrediffers), 
    itsFirstCall      (true),
    itsArgs           (args),
    itsCurStartTime   (0),
    itsStartTime      (0),
    itsEndTime        (0),
    itsTimeLength     (0),
    itsStartChannel   (0),
    itsEndChannel     (0)
{
  itsWriteInDataCol = itsArgs.getBool ("writeInDataCol");
  itsStartTime = itsArgs.getDouble ("startTimeSec");
  itsEndTime = itsArgs.getDouble ("endTimeSec");
  itsTimeLength = itsArgs.getDouble ("timeInterval");
  itsStartChannel = itsArgs.getInt32 ("startChan");
  itsEndChannel = itsArgs.getInt32 ("endChan");
}

SC_WritePredData::~SC_WritePredData()
{}

bool SC_WritePredData::execute()
{
  BBSTest::ScopedTimer si_exec("C:strategycontroller_execute");
  BBSTest::ScopedTimer getWOTimer("C:getWorkOrders");
  bool finished = false; // Has this strategy completed?
  DH_WOPrediff* WOPD = getPrediffWorkOrder();
  DH_WOSolve* WOSolve = getSolveWorkOrder();
  getWOTimer.end();

  WOPD->setCleanUp(false);
  WOPD->setDoNothing(false);
  
  if (itsFirstCall)
  {
    BBSTest::Logger::log("Start of testrun");
    itsFirstCall = false;
    itsCurStartTime = itsStartTime;
  }
  else
  {
    itsCurStartTime += itsTimeLength;
    BBSTest::Logger::log("NextInterval");
    
    if (itsCurStartTime >= itsEndTime)  // If all time intervals handled, send workorder to
    {                                  // clean up.
      WOPD->setDoNothing(true);
      WOPD->setCleanUp(true);
      finished = true;               // This strategy has finished!
    }

  }

  // Set prediffer workorder data
  WOPD->setStatus(DH_WOPrediff::New);
  WOPD->setKSType("Prediff1");
  WOPD->setWritePredData(true);
  WOPD->setWriteInDataCol(itsWriteInDataCol);
  WOPD->setStartChannel (itsStartChannel);
  WOPD->setEndChannel (itsEndChannel);
  WOPD->setStartTime (itsCurStartTime);
  WOPD->setTimeLength (itsTimeLength);
  WOPD->setModelType (itsArgs.getString ("modelType"));
  WOPD->setCalcUVW (itsArgs.getBool ("calcUVW"));
  ParameterSet msParams = itsArgs.makeSubset("MSDBparams.");
  vector<int> ant = itsArgs.getInt32Vector("antennas");

  vector<int> srcs = itsArgs.getInt32Vector("sources");
  vector<int> corrs;
  if (itsArgs.isDefined("correlations"))
  {
    corrs = itsArgs.getInt32Vector("correlations");
  }
  // the prediffer needs to know the modelType too
  msParams["modelType"] = itsArgs.getString("modelType");
  msParams["calcUVW"] = itsArgs.getString("calcUVW");
  vector<string> pNames;    // Empty vector
  vector<string> exPNames;    // Empty vector
  WOPD->setVarData (msParams, ant, pNames, exPNames, srcs, corrs);
 
  int woid = getNewWorkOrderID();  
  WOPD->setWorkOrderID(woid);
  WOPD->setStrategyControllerID(getID());
  WOPD->setNewDomain(true);

  // Set solver workorder data  
  WOSolve->setStatus(DH_WOSolve::New);
  WOSolve->setKSType("Solver");
  WOSolve->setDoNothing(true);  // Solver is not used in this strategy
  WOSolve->setWorkOrderID(woid);
  WOSolve->setStrategyControllerID(getID());

  // Temporarily show on cout
  cout << "!!!!!!! Sent workorders: " << endl;
  //WOPD->dump();
  //WOSolve->dump();

  cout << "!!!!!!! " << endl;

  // Insert WorkOrders into database
  BBSTest::ScopedTimer st("C:putWOinDB");
  WOPD->insertDB(*itsOutWOPDConn);

  // Send workorders the same workorders to other prediffers (if there are more than 1)
  int nrPred = getNumberOfPrediffers();
  for (int i = 2; i <= nrPred; i++)
  {
    WOPD->setWorkOrderID(getNewWorkOrderID());
    char str[32];
    sprintf(str, "%i", i);
    WOPD->setKSType("Prediff"+string(str));
    WOPD->insertDB(*itsOutWOPDConn);
  }

  WOSolve->insertDB(*itsOutWOSolveConn);

  return (!finished);
}

void SC_WritePredData::postprocess()
{
}


} // namespace LOFAR
