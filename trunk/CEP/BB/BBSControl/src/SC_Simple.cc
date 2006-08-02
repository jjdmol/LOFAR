//#  SC_Simple.cc:  A simple calibration strategy
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

#include <BBSControl/SC_Simple.h>
#include <BBSControl/DH_Solution.h>
#include <BBSControl/DH_WOPrediff.h>
#include <BBSControl/DH_WOSolve.h>
#include <BBSControl/BBSStrategy.h>
#include <BBSControl/BBSStep.h>
#include <BBS/BBSTestLogger.h>
#include <Common/LofarLogger.h>
#include <unistd.h>                // for usleep()

namespace LOFAR
{
  namespace BBS
  {
    SC_Simple::SC_Simple(Connection* inSolConn, Connection* outWOPDConn, 
			 Connection* outWOSolveConn, int nrPrediffers,
			 const ParameterSet& args)
      : StrategyController(inSolConn, outWOPDConn, outWOSolveConn, 
			   nrPrediffers), 
	itsPrevWOID       (0),
	itsArgs           (args),
	itsCurIter        (-1),
	itsCurStartTime   (0),
	itsControlParmUpd (false),
	itsStartTime      (0),
	itsTimeLength     (0),
	itsStartChannel   (0),
	itsEndChannel     (0),
	itsSendDoNothingWO(false)
    {
      itsNrIterations = itsArgs.getInt32("maxNrIterations");
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
      itsStartTime = itsArgs.getDouble ("startTimeSec");
      itsEndTime = itsArgs.getDouble ("endTimeSec");
      itsTimeLength = itsArgs.getDouble ("timeInterval");
      itsStartChannel = itsArgs.getInt32 ("startChan");
      itsEndChannel = itsArgs.getInt32 ("endChan");
    }

    SC_Simple::~SC_Simple()
    {}

    void SC_Simple::preprocess()
    {
      DH_WOPrediff* WOPD = getPrediffWorkOrder();
      DH_WOSolve* WOSolve = getSolveWorkOrder();
      BBSTest::Logger::log("Start of testrun");
      WOPD->setNewBaselines(true);
      WOPD->setNewPeelSources(true);
      WOPD->setSubtractSources(false);
      WOPD->setUpdateParms(false);
      WOPD->setCleanUp(false);
      WOSolve->setNewDomain(true);
      WOSolve->setCleanUp(false);
      itsCurStartTime = itsArgs.getDouble ("startTimeSec");

     // Retrieve the parameter set file describing the BBSStrategy to be
     // used. The name of this file must be specified by the key
     // "strategyParset".
     string strategyParset = itsArgs.getString("strategyParset");
     BBSStrategy strategy(ParameterSet("strategyParset"));

     // Put all the steps that comprise this strategy into our private vector
     // of BBSSteps.
     itsSteps = strategy.getAllSteps();

     // Set the current step to the first step of the strategy
     itsCurrentStep = *itsSteps.begin();
    }

    bool SC_Simple::execute()
    {
      // IF workdomain done THEN
      //   Set new workdomain. (Re)initialize data structures.
      //   Make first BBSStep in BBSStrategy current
      // ENDIF
      // Call execute() on current BBSStep
      // IF stop-criterion met THEN
      //   Goto next BBSStep
      //   IF all BBSSteps done THEN
      //     Set workdomain done
      //   ENDIF
      // ENDIF
      
#if 0
      itsCurrentStep->execute(this);
#endif
      
      BBSTest::ScopedTimer si_exec("C:strategycontroller_execute");
      BBSTest::ScopedTimer getWOTimer("C:getWorkOrders");
      bool finished = false;   // Has this strategy completed?
      DH_WOPrediff* WOPD = getPrediffWorkOrder();
      DH_WOSolve* WOSolve = getSolveWorkOrder();
      getWOTimer.end();

      itsCurIter++;
      bool nextInter = false;
      {
	
	WOPD->setNewBaselines(false);
	WOPD->setNewPeelSources(false);
	WOPD->setSubtractSources(false);
	WOPD->setUpdateParms(true);
	WOPD->setCleanUp(false);         //Reset
	WOSolve->setNewDomain(false);
	WOSolve->setCleanUp(false);
	
	// If previous sent WorkOrder was a "do nothing", do not read solution
	if (itsSendDoNothingWO==false)
	{
	  // Read solution of previously issued workorders
	  BBSTest::ScopedTimer readSolTimer("C:read solutions");
	  readSolution();
	  readSolTimer.end();

	  // If Controller handles parameter writing, then write new parameter
	  // values directly to the tables; else send the (reference to)
	  // parameter values to Prediffers.
	  if (itsControlParmUpd) writeParmData();
	  else WOPD->setSolutionID(itsPrevWOID);
	}
     
	// Take absolute value of fit
	double fit = abs(getSolution()->getQuality().itsFit);

	itsSendDoNothingWO = false;

	// If max number of iterations reached, go to next interval. If
	// solution for this interval is good enough, send "do nothing"
	// workorders until max number of iterations reached.
	if (itsCurIter == itsNrIterations)
	{
	  nextInter = true;
	  itsCurIter = 0;

	  // New time interval, so do not reread parameters
	  WOPD->setUpdateParms(false);

	  // New time interval, so do not use solution from previous interval
	  WOPD->setSolutionID(-1);

	  itsCurStartTime += itsArgs.getDouble ("timeInterval");

	  // If all time intervals handled, send workorders to clean up.
	  if (itsCurStartTime >= itsEndTime)
	  {
	    itsSendDoNothingWO = true;
	    WOPD->setCleanUp(true);
	    WOSolve->setCleanUp(true);
	    finished = true;               // This strategy has finished!
	  }

	  // If controller should write params at end of each interval and has
	  // not already done so...
	  if (itsWriteParms && (!itsControlParmUpd)) writeParmData();

	  BBSTest::Logger::log("NextInterval");
	}
	else if (fit < itsFitCriterion)
	{
	  LOG_INFO_STR("Fit criterion met after " << itsCurIter 
		       << " iterations");
	  itsSendDoNothingWO = true;
	}
      }

      // Get a new work order id
      int woid = getNewWorkOrderID();

      // Set prediffer workorder data
      setWOPrediff(woid);

      // Set solver workorder data  
      setWOSolve(woid);

      return (!finished);
    }


    void SC_Simple::postprocess()
    {
      // Only read solution if previous workorder was not a "do nothing"
      if (itsSendDoNothingWO == false)
      {
	// write solution in parmtable
	if (itsWriteParms || itsControlParmUpd) {
	  readSolution();
	  writeParmData();
	}
      }
      BBSTest::Logger::log("End of TestRun");
    }


    void SC_Simple::readSolution()
    {
      LOG_TRACE_FLOW("SC_Simple reading solution");

      DH_DB* solPtr = getSolution();

      // Wait for solution
      bool firstTime = true;
      ostringstream oss;
      oss << "SELECT * FROM bbs3solutions WHERE WOID=" << itsPrevWOID;
      string query(oss.str());

      while (solPtr->queryDB(query, *itsInSolConn) <= 0) {
	if (firstTime) {
	  cout << "No solution found by SC_Simple " << getID() 
	       << ". Waiting for solution..." << endl;
	  firstTime = false;
	  // Sleep for 50 ms, in order not to hammer the database.
	  usleep(50000);
	}
      }
      // getSolution()->dump();
    }


    void SC_Simple::writeParmData() const
    {
      ParmDataInfo pData;
      getSolution()->getSolution(pData);
      double fStart = getSolution()->getStartFreq();
      double fEnd = getSolution()->getEndFreq();
      double tStart = getSolution()->getStartTime();
      double tEnd = getSolution()->getEndTime();
      BBSTest::ScopedTimer st("C:parmwriter");
      getParmWriter().write(pData, fStart, fEnd, tStart, tEnd);
    }

    void SC_Simple::setWOPrediff(int woid)
    {
      DH_WOPrediff* WOPD = getPrediffWorkOrder();
      WOPD->setDoNothing(itsSendDoNothingWO);
      if (itsSendDoNothingWO==false)
      {
	WOPD->setStatus(DH_WOPrediff::New);
	WOPD->setKSType("Prediff1");
	WOPD->setStartChannel (itsStartChannel);
	WOPD->setEndChannel (itsEndChannel);
	WOPD->setStartTime (itsCurStartTime);
	double timeLength = itsArgs.getDouble ("timeInterval");
	WOPD->setTimeLength (timeLength);
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
	msParams.add ("modelType", itsArgs.getString("modelType"));
	msParams.add ("calcUVW", itsArgs.getString("calcUVW"));
	WOPD->setVarData (msParams, ant, pNames, exPNames, srcs, corrs);
      }
      WOPD->setWorkOrderID(woid);
      WOPD->setStrategyControllerID(getID());
      WOPD->setNewDomain(itsWorkDomainDone);

      // Insert WorkOrders into database; 
      WOPD->insertDB(*itsOutWOPDConn);

      // Send the same workorder to other prediffers (if there's more than 1)
      int nrPred = getNumberOfPrediffers();
      for (int i = 2; i <= nrPred; i++)
      {
	WOPD->setWorkOrderID(getNewWorkOrderID());
	ostringstream oss;
	oss << "Prediff" << i;
	WOPD->setKSType(oss.str());
	WOPD->insertDB(*itsOutWOPDConn);
      }

    }


    void SC_Simple::setWOSolve(int woid)
    {
      DH_WOSolve* WOSolve = getSolveWorkOrder();
      WOSolve->setDoNothing(itsSendDoNothingWO);
      WOSolve->setStatus(DH_WOSolve::New);
      WOSolve->setKSType("Solver");
      WOSolve->setUseSVD (itsArgs.getBool ("useSVD"));
      WOSolve->setIteration(itsCurIter);

      // We need to set the work-id of the solver equal to that of the
      // prediffer (for reasons that elude me (GML)).
      WOSolve->setWorkOrderID(woid);

      // Remember the issued workorder id
      itsPrevWOID = WOSolve->getWorkOrderID();

      WOSolve->setStrategyControllerID(getID());
      WOSolve->setNewDomain(itsWorkDomainDone);

      // Insert workorder into the database
      WOSolve->insertDB(*itsOutWOSolveConn);
    }


  } // namespace BBS

} // namespace LOFAR
