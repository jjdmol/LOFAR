//#  WH_Solve.cc: predicts visibilities and determines the difference to
//#                 measured data
//#
//#  Copyright (C) 2002-2004
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <BBSControl/WH_Solve.h>
#include <Common/LofarLogger.h>
#include <BBSControl/DH_WOSolve.h>
#include <BBSControl/DH_Prediff.h>
#include <BBSControl/DH_Solution.h>
#include <BBS/Solver.h>
#include <BBS/Prediffer.h>
#include <BBS/ParmData.h>
#include <BBS/BBSTestLogger.h>

#include <iostream>
#include <Common/StreamUtil.h>

using namespace std;

namespace LOFAR
{
  namespace BBS
  {

    WH_Solve::WH_Solve(const string& name, int nPrediffInputs, 
		       bool writeIndivParms, const string& parmTableName)
      : WorkHolder        (nPrediffInputs+2, 2, name, "WH_Solve"),
	itsNPrediffers    (nPrediffInputs),
	itsWriteIndivParms(writeIndivParms),
	itsParmTableName  (parmTableName)
    {
      LOG_TRACE_FLOW("WH_Solve constructor");

      // Add workorder input
      getDataManager().addInDataHolder
	(0, new DH_WOSolve(name+"_in0"));
      getDataManager().addOutDataHolder    // dummy
	(0, new DH_WOSolve(name+"_out0"));
      // Add solution output
      getDataManager().addInDataHolder     // dummy
	(1, new DH_Solution(name+"_in1", writeIndivParms, parmTableName));
      getDataManager().addOutDataHolder
	(1, new DH_Solution(name+"_out1", writeIndivParms, parmTableName));

      // Switch input channel trigger off
      getDataManager().setAutoTriggerIn(0, false);
      getDataManager().setAutoTriggerOut(0, false);
      getDataManager().setAutoTriggerIn(1, false);
      getDataManager().setAutoTriggerOut(1, false);

      // Add prediffer inputs
      for (int i=0; i<itsNPrediffers; i++)
      {
	getDataManager().addInDataHolder(i+2, new DH_Prediff(name+"_in"));
	getDataManager().setAutoTriggerIn(i+2, false);
      }

    }

    WH_Solve::~WH_Solve()
    {
      LOG_TRACE_FLOW("WH_Solve destructor");
      // Clean up map with Solver objects
      SolverMap::iterator iter;
      for (iter=itsSolvers.begin(); iter!=itsSolvers.end(); iter++)
      {
	delete (*iter).second;
      }
      itsSolvers.clear();
    }

    WH_Solve* WH_Solve::make (const string& name)
    {
      return new WH_Solve (name, itsNPrediffers, itsWriteIndivParms, 
			   itsParmTableName);
    }

    void WH_Solve::preprocess()
    {
      LOG_TRACE_RTTI("WH_Solve preprocess()");
    }

    void WH_Solve::process()
    {
      LOG_TRACE_RTTI("WH_Solve process()");
      BBSTest::ScopedTimer st("S:total_WH_Solve::process");

      // Query the database for a work order
      DH_WOSolve* wo = dynamic_cast<DH_WOSolve*>
	(getDataManager().getInHolder(0));
      Connection* connWO = getDataManager().getInConnection(0);
      ASSERTSTR(connWO!=0, "Input 0 not connected");
 
      // Wait for workorder
      bool firstTime = true;
      ostringstream q;
      q << "SELECT * FROM bbs3wosolver WHERE STATUS=0 AND "
	<< "(KSTYPE='SOLVER' OR KSTYPE='" << getName() 
	<< "') order by KSTYPE DESC, WOID ASC";
      while ((wo->queryDB(q.str(), *connWO)) <= 0)
      {
	if (firstTime)
	{
	  cout << "No workorder found by " << getName() 
	       << ". Waiting for work order..." << endl;
	  firstTime = false;
	}
      }

      // Update workorder status
      wo->setStatus(DH_WOSolve::Assigned);
      BBSTest::ScopedTimer updateTimer("S:updateSolveWO");
      wo->updateDB(*connWO);
      updateTimer.end();

      if (wo->getDoNothing() == false)
      {
	int contrID = wo->getStrategyControllerID();
	Solver* solver = getSolver(contrID);
	DBGASSERTSTR(solver!=0, 
		     "The solver has not been created and initialized.");

	if (wo->getNewDomain())         // New domain 
	{
	  setParmData(solver);
	  readInputs(solver, false);   // Skip first read
	}
	else
	{
	  readInputs(solver, true);     // Read inputs first
	}

	// Get solution dataholder DH_Solution* sol;
	DH_Solution* sol = dynamic_cast<DH_Solution*>
	  (getDataManager().getOutHolder(1));
	sol->clearData();
	//>>>> For now: Assume all prediffer domains are equal!!
	DH_Prediff* predInp1 = 
	  dynamic_cast<DH_Prediff*>(getDataManager().getInHolder(2));
	sol->setDomain(predInp1->getStartFreq(), predInp1->getEndFreq(),
		       predInp1->getStartTime(), predInp1->getEndTime());
	sol->setWorkOrderID(wo->getWorkOrderID());

	// Loop over iterations
	for (int curIter=0; curIter < wo->getMaxIterations(); curIter++)
	{
	  if (curIter > 0)
	  {
	    readInputs(solver, true);   // Read inputs again
	  }

	  // Do the solve.
	  BBSTest::ScopedTimer solveTimer("S:solve");
	  solver->solve(wo->getUseSVD());
	  solveTimer.end();

	  // Determine if solution has converged
	  Quality resultQuality;    ////
	  bool converged = false;
	  double fit = resultQuality.itsFit;
	  if (fit<0)
	  {
	    fit = -fit;
	  }
	  if (fit < wo->getFitCriterion())
	  {
	    converged = true;
	  }
      
	  // Write result
	  Connection* connSol = getDataManager().getOutConnection(1);
	  ASSERTSTR(connSol != 0, "Output 1 not connected!");
	  sol->setSolution(solver->getSolvableParmData());
	  sol->setQuality(resultQuality);
	  sol->setIteration(wo->getIteration() + curIter);
	  sol->setConverged(converged);

	  // Add solution to database
	  sol->insertDB(*connSol);

	  if (converged) 
	  {
	    break;
	  }

	} // End loop iterations
 
      }

      // If Solver object is no longer needed: clean up  
      if (wo->getCleanUp())
      {
	int contrID = wo->getStrategyControllerID();
	deleteSolver(contrID);
      }
  
      // Update workorder status
      wo->setStatus(DH_WOSolve::Executed);
      wo->updateDB(*connWO);

    }

    void WH_Solve::dump() const
    {
      LOG_TRACE_RTTI("WH_Solve process()");
    }

    Solver* WH_Solve::getSolver(int id)
    {
      SolverMap::iterator iter;
      iter = itsSolvers.find(id);
      if (iter != itsSolvers.end())
      {
	return (*iter).second;
      }
      else
      {
	// Create a new Prediffer object
	Solver* slv = new Solver();
	// add to map
	itsSolvers.insert(SolverMap::value_type(id, slv));
	return slv;
      }
    }

    void WH_Solve::deleteSolver(int id)
    {
      SolverMap::iterator iter;
      iter = itsSolvers.find(id);
      if (iter != itsSolvers.end())
      {
	delete (iter->second);
	itsSolvers.erase(iter);
      }
    }

    void WH_Solve::readInputs(Solver* solver, bool firstRead)
    {
      BBSTest::ScopedTimer riTimer("S:ReadInputs");
      LOG_TRACE_FLOW("WH_Solve::readInputs");
      DH_Prediff* dh;
      for (int i=0; i<itsNPrediffers; i++)
      {
	if (firstRead)   // Do a read before merging fitter data
	{
	  dh = dynamic_cast<DH_Prediff*>(getDataManager().getInHolder(i+2));

	  // Force input to be read. Necessary because auto-triggering is
	  // switched off.
	  getDataManager().readyWithInHolder(i+2);
	}
	dh = dynamic_cast<DH_Prediff*>(getDataManager().getInHolder(i+2));
	vector<casa::LSQFit> fitters;
	dh->getFitters (fitters);
	///    solver->mergeFitter (fitter, i);
	  }
    }

    void WH_Solve::setParmData(Solver* solver)
    {
      LOG_TRACE_FLOW("WH_Solve::readInputsAndSetParmData");

      ////  solver->initSolvableParmData(itsNPrediffers); 
      DH_Prediff* dh;
      for (int i=0; i<itsNPrediffers; i++)
      {
	getDataManager().getInHolder(i+2);

	// Forces data to be read
	getDataManager().readyWithInHolder(i+2);

	dh = dynamic_cast<DH_Prediff*>(getDataManager().getInHolder(i+2));
	ParmDataInfo pData;
	dh->getParmData(pData);

	// id = i or from prediffer?
	solver->setSolvableParmData(pData, i);
      }
    }

  } // namespace BBS

} // namespace LOFAR
