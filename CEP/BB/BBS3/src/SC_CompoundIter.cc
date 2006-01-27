//#  SC_CompoundIter.cc:  A calibration strategy which sends compound workorders
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

#include <BBS3/SC_CompoundIter.h>
#include <Common/LofarLogger.h>
#include <BBS3/DH_Solution.h>
#include <BBS3/DH_WOPrediff.h>
#include <BBS3/DH_WOSolve.h>
#include <BBS3/BBSTestLogger.h>

namespace LOFAR
{

SC_CompoundIter::SC_CompoundIter(Connection* inSolConn, Connection* outWOPDConn, 
		     Connection* outWOSolveConn, int nrPrediffers,
		     const ParameterSet& args)
  : StrategyController(inSolConn, outWOPDConn, outWOSolveConn, 
		       nrPrediffers, args.getInt32("MSDBparams.DBMasterPort")), 
    itsFirstCall      (true),
    itsPrevWOID       (0),
    itsArgs           (args),
    itsCurIter        (0),
    itsCurStartTime   (0),
    itsControlParmUpd (false),
    itsStartTime      (0),
    itsEndTime        (0),
    itsTimeLength     (0),
    itsStartFreq      (0),
    itsFreqLength     (0),
    itsSendDoNothingWO(false)
{
  itsMaxIterations = itsArgs.getInt32("maxNrIterations");
  if (itsArgs.isDefined("fitCriterion"))
  {
    itsFitCriterion = itsArgs.getDouble("fitCriterion");
  }
  else
  {
    itsFitCriterion = -1;
  }
  itsControlParmUpd = itsArgs.getBool ("controlParmUpdate");
  itsWriteParms = itsArgs.getBool("writeParms");
  itsStartTime = itsArgs.getDouble ("startTime");
  itsEndTime = itsArgs.getDouble ("endTime");
  itsTimeLength = itsArgs.getDouble ("timeInterval");
  itsStartFreq = itsArgs.getDouble ("startFreq");
  itsFreqLength = itsArgs.getDouble ("freqLength");
}

SC_CompoundIter::~SC_CompoundIter()
{}

bool SC_CompoundIter::execute()
{
  BBSTest::ScopedTimer si_exec("C:strategycontroller_execute");
  BBSTest::ScopedTimer getWOTimer("C:getWorkOrders");
  bool finished = false;  // Has this strategy completed?
  DH_WOPrediff* WOPD = getPrediffWorkOrder();
  DH_WOSolve* WOSolve = getSolveWorkOrder();
  getWOTimer.end();

  if (itsFirstCall)
  {
    BBSTest::Logger::log("Start of testrun");
    itsFirstCall = false;

    itsCurStartTime = itsArgs.getDouble ("startTime");

    WOPD->setNewBaselines(true);
    WOPD->setNewPeelSources(true);
    WOPD->setDoNothing(false);
    WOPD->setCleanUp(false);
    WOSolve->setDoNothing(false);
    WOSolve->setCleanUp(false);
  }
  else
  {
    WOPD->setNewBaselines(false);
    WOPD->setNewPeelSources(false);
    WOPD->setCleanUp(false);            // Reset
    WOSolve->setCleanUp(false);
  
    if (itsWriteParms) // If Controller writes parameters at end of interval
    {
      // Read final solution of previously issued workorders
      BBSTest::ScopedTimer readSolTimer("C:read solutions");
      readFinalSolution();
      readSolTimer.end();

      // Controller writes new parameter values directly to the tables
      vector<ParmData> pData;
      getSolution()->getSolution(pData);
      double fStart = getSolution()->getStartFreq();
      double fEnd = getSolution()->getEndFreq();
      double tStart = getSolution()->getStartTime();
      double tEnd = getSolution()->getEndTime();
      BBSTest::ScopedTimer st("C:parmwriter");
      getParmWriter().write(pData, fStart, fEnd, tStart, tEnd);
    }

    itsCurStartTime += itsTimeLength;

    if (itsCurStartTime >= itsEndTime) // If all time intervals handled, send workorders to
    {                                  // clean up.
      itsSendDoNothingWO = true;
      WOPD->setCleanUp(true);
      WOSolve->setCleanUp(true);
      finished = true;                 // This strategy has finished!
    }
    BBSTest::Logger::log("NextInterval");
  }

  // Set prediffer workorder data
  WOPD->setStatus(DH_WOPrediff::New);
  WOPD->setKSType("Prediff1");

  // The following settings remain the same for each workorder:
  {
    WOPD->setUpdateParms(false);
    WOPD->setMaxIterations(itsMaxIterations);
    WOPD->setDoNothing(itsSendDoNothingWO);
    WOPD->setNewDomain(true);
    WOPD->setSubtractSources(false);
    WOPD->setStartFreq (itsArgs.getDouble ("startFreq"));
    WOPD->setFreqLength (itsArgs.getDouble ("freqLength")); 
    WOPD->setTimeLength (itsTimeLength); 
    WOPD->setModelType (itsArgs.getString ("modelType"));
    WOPD->setUseAutoCorrelations(itsArgs.getBool ("useAutoCorr"));
    WOPD->setCalcUVW (itsArgs.getBool ("calcUVW"));
    ParameterSet msParams = itsArgs.makeSubset("MSDBparams.");
    vector<int> ant = itsArgs.getInt32Vector("antennas");
    vector<string> pNames = itsArgs.getStringVector("solvableParams");
    vector<string> exPNames;
    if (itsArgs.isDefined("excludeParams"))
    {
      exPNames = itsArgs.getStringVector("excludeParams");
    }
    vector<int> srcs = itsArgs.getInt32Vector("sources");
    vector<int> corrs;
    if (itsArgs.isDefined("correlations"))
    {
      corrs = itsArgs.getInt32Vector("correlations");
    }
    // the prediffer needs to know the modelType too
    msParams["modelType"] = itsArgs.getString("modelType");
    msParams["calcUVW"] = itsArgs.getString("calcUVW");
    WOPD->setVarData (msParams, ant, pNames, exPNames, srcs, corrs);
    WOPD->setStrategyControllerID(getID());
 
    WOSolve->setDoNothing(itsSendDoNothingWO); 
    WOSolve->setNewDomain(true);
    WOSolve->setMaxIterations(itsMaxIterations);
    WOSolve->setFitCriterion(itsFitCriterion);
    WOSolve->setUseSVD (itsArgs.getBool ("useSVD"));
    WOSolve->setIteration(itsCurIter);
    WOSolve->setStrategyControllerID(getID());
  }

  WOPD->setStartTime (itsCurStartTime);

  int woid = getNewWorkOrderID();
  WOPD->setWorkOrderID(woid);
  WOPD->setSolutionID(woid);  // WOID of solution (the same for each prediffer)

  // Set solver workorder data  
  WOSolve->setStatus(DH_WOSolve::New);
  WOSolve->setKSType("Solver");
  WOSolve->setWorkOrderID(woid);  
  itsPrevWOID = woid;        // Remember the issued workorder id

  // Temporarily show on cout
  //  cout << "!!!!!!! Sent workorders: " << endl;
  //WOPD->dump();
  //WOSolve->dump();

  //  cout << "!!!!!!! " << endl;

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

void SC_CompoundIter::postprocess()
{
  if ((!itsSendDoNothingWO) && itsWriteParms)   // If Controller writes parameters at end of interval
  {
    // Read final solution of previously issued workorders
    readFinalSolution();

    // Controller writes found parameter values in the tables
    vector<ParmData> pData;
    getSolution()->getSolution(pData);
    double fStart = getSolution()->getStartFreq();
    double fEnd = getSolution()->getEndFreq();
    double tStart = getSolution()->getStartTime();
    double tEnd = getSolution()->getEndTime();
    BBSTest::ScopedTimer st("C:parmwriter");
    getParmWriter().write(pData, fStart, fEnd, tStart, tEnd);
  }
  BBSTest::Logger::log("End of TestRun");
}

void SC_CompoundIter::readFinalSolution()
{
  LOG_TRACE_FLOW("SC_CompoundIter reading solution");

  DH_DB* solPtr = getSolution();

  // Wait for solution
  bool firstTime = true;
  int id = itsPrevWOID;
  char str[128];
  sprintf(str, "SELECT * FROM bbs3solutions WHERE WOID=%i AND (HASCONVERGED=1 OR ITERATION=%i)", 
	  id, itsMaxIterations-1);
  string query(str);

  while (solPtr->queryDB(query, *itsInSolConn) <= 0)
  {
    if (firstTime)
    {
      cout << "No solution found by SC_CompoundIter " << getID() 
	   << ". Waiting for solution..." << endl;
      firstTime = false;
    }
  }

  //getSolution()->dump();

}


} // namespace LOFAR
