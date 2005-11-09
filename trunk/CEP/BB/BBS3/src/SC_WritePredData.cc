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
#include <TransportPL/DH_PL.h>
#include <BBS3/DH_Solution.h>
#include <BBS3/DH_WOPrediff.h>
#include <BBS3/DH_WOSolve.h>
#include <BBS3/BBSTestLogger.h>
#include <BBS3/MNS/ParmTable.h>

namespace LOFAR
{

SC_WritePredData::SC_WritePredData(int id, Connection* inSolConn, Connection* outWOPDConn, 
		     Connection* outWOSolveConn, int nrPrediffers,
		     const ParameterSet& args)
  : StrategyController(id, inSolConn, outWOPDConn, outWOSolveConn, 
		       nrPrediffers, args.getInt32("MSDBparams.DBMasterPort")), 
    itsFirstCall      (true),
    itsArgs           (args),
    itsCurStartTime   (0),
    itsStartTime      (0),
    itsTimeLength     (0),
    itsStartFreq      (0),
    itsFreqLength     (0)
{
  itsWriteInDataCol = itsArgs.getBool ("writeInDataCol");
  itsStartTime = itsArgs.getDouble ("startTime");
  itsTimeLength = itsArgs.getDouble ("timeInterval");
  itsStartFreq = itsArgs.getDouble ("startFreq");
  itsFreqLength = itsArgs.getDouble ("freqLength");
}

SC_WritePredData::~SC_WritePredData()
{}

bool SC_WritePredData::execute()
{
  BBSTest::ScopedTimer si_exec("C:strategycontroller_execute");
  BBSTest::ScopedTimer getWOTimer("C:getWorkOrders");
  DH_WOPrediff* WOPD = getPrediffWorkOrder();
  DH_WOSolve* WOSolve = getSolveWorkOrder();
  getWOTimer.end();

  if (itsFirstCall)
  {
    BBSTest::Logger::log("Start of testrun");
    itsFirstCall = false;
    itsCurStartTime = itsArgs.getDouble ("startTime");
  }
  else
  {
    itsCurStartTime += itsArgs.getDouble ("timeInterval");
    BBSTest::Logger::log("NextInterval");
  }

  // Set prediffer workorder data
  WOPD->setStatus(DH_WOPrediff::New);
  WOPD->setKSType("Prediff1");
  WOPD->setWritePredData(true);
  WOPD->setWriteInDataCol(itsWriteInDataCol);
  WOPD->setStartFreq (itsArgs.getDouble ("startFreq"));
  WOPD->setFreqLength (itsArgs.getDouble ("freqLength"));
  WOPD->setStartTime (itsCurStartTime);
  double timeLength = itsArgs.getDouble ("timeInterval");
  WOPD->setTimeLength (timeLength);
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
 
  WOPD->setNewWorkOrderID();
  WOPD->setStrategyControllerID(getID());
  WOPD->setNewDomain(true);

  // Set solver workorder data  
  WOSolve->setStatus(DH_WOSolve::New);
  WOSolve->setKSType("Solver");
  WOSolve->setDoNothing(true);  // No solve
  WOSolve->setNewWorkOrderID();
  WOSolve->setStrategyControllerID(getID());

  // Temporarily show on cout
  cout << "!!!!!!! Sent workorders: " << endl;
  //WOPD->dump();
  //WOSolve->dump();

  cout << "!!!!!!! " << endl;

  {
    BBSTest::ScopedTimer syncTimer("C:SyncTimer");
    // Sync the parmtables so the prediffers read the newest values of the parms
    vector<ParmData> pData;
    getSolution()->getSolution(pData);
    if (pData.size() > 0) {
      ParmTable ptab(pData.back().getParmTableData());
      // sync is now only implemented by ParmTableBDBRepl
      // if a sync is called on a table, all tables in that replication group are synced
      // right now all our tables are in the same replication group
      ptab.sync();
    }
  }

  // Insert WorkOrders into database
  BBSTest::ScopedTimer st("C:putWOinDB");
  WOPD->insertDB(*itsOutWOPDConn);

  // Send workorders the same workorders to other prediffers (if there are more than 1)
  int nrPred = getNumberOfPrediffers();
  for (int i = 2; i <= nrPred; i++)
  {
    WOPD->setNewWorkOrderID();
    char str[32];
    sprintf(str, "%i", i);
    WOPD->setKSType("Prediff"+string(str));
    WOPD->insertDB(*itsOutWOPDConn);
  }

  WOSolve->insertDB(*itsOutWOSolveConn);

  return true;
}

void SC_WritePredData::postprocess()
{
}


} // namespace LOFAR
