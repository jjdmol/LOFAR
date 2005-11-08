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
#include <BBS3/WH_Solve.h>
#include <Common/LofarLogger.h>
#include <BBS3/DH_WOSolve.h>
#include <BBS3/DH_Prediff.h>
#include <BBS3/DH_Solution.h>
#include <BBS3/DH_ParmSol.h>
#include <BBS3/Solver.h>
#include <BBS3/Prediffer.h>
#include <BBS3/ParmData.h>
#include <BBS3/BBSTestLogger.h>

#include <iostream>
#include <Common/VectorUtil.h>

using namespace std;

namespace LOFAR
{

WH_Solve::WH_Solve(const string& name, int nPrediffInputs, bool writeIndivParms)
  : WorkHolder        (nPrediffInputs+3, 3, name, "WH_Solve"),
    itsNPrediffers    (nPrediffInputs),
    itsWriteIndivParms(writeIndivParms)
{
  LOG_TRACE_FLOW("WH_Solve constructor");
  // Add workorder input
  getDataManager().addInDataHolder(0, new DH_WOSolve(name+"_in0"));
  getDataManager().addOutDataHolder(0, new DH_WOSolve(name+"_out0")); // dummy
  // Add solution output
  getDataManager().addInDataHolder(1, new DH_Solution(name+"_in1"));  // dummy
  getDataManager().addOutDataHolder(1, new DH_Solution(name+"_out1"));
  // Add parmsolution output
  getDataManager().addInDataHolder(2, new DH_ParmSol(name+"_in2"));  // dummy
  getDataManager().addOutDataHolder(2, new DH_ParmSol(name+"_out2"));
  // Switch input channel trigger off
  getDataManager().setAutoTriggerIn(0, false);
  getDataManager().setAutoTriggerOut(0, false);
  getDataManager().setAutoTriggerIn(1, false);
  getDataManager().setAutoTriggerOut(1, false);
  getDataManager().setAutoTriggerIn(2, false);
  getDataManager().setAutoTriggerOut(2, false);

  // Add prediffer inputs
  for (int i=0; i<itsNPrediffers; i++)
  {
    getDataManager().addInDataHolder(i+3, new DH_Prediff(name+"_in"));
    getDataManager().setAutoTriggerIn(i+3, false);
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
  return new WH_Solve (name, itsNPrediffers, itsWriteIndivParms);
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
  DH_WOSolve* wo =  dynamic_cast<DH_WOSolve*>(getDataManager().getInHolder(0));
  DH_PL* woPtr = dynamic_cast<DH_PL*>(wo);
  Connection* connWO = getDataManager().getInConnection(0);
  ASSERTSTR(connWO!=0, "Input 0 not connected");
 
  // Wait for workorder
  bool firstTime = true;
  while ((woPtr->queryDB("status=0 and (kstype='SOLVER' or kstype='" + getName()
                         + "') order by kstype desc, woid asc", *connWO)) <= 0)
  {
    if (firstTime)
    {
      cout << "No workorder found by " << getName() << ". Waiting for work order..." << endl;
      firstTime = false;
    }
  }

  cout << "!!!!!! Solver read workorder: " << endl;
  //wo->dump();
  cout << "!!!!!! " << endl;


  // Update workorder status
  wo->setStatus(DH_WOSolve::Assigned);
  BBSTest::ScopedTimer updateTimer("S:updateSolveWO");
  woPtr->updateDB(*connWO);
  updateTimer.end();

  if (wo->getDoNothing() == false)
  {
    int contrID = wo->getStrategyControllerID();
    Solver* solver = getSolver(contrID);
    DBGASSERTSTR(solver!=0, "The solver has not been created and initialized.");

    if (wo->getNewDomain())         // New domain 
    {
      setParmData(solver);
      readInputs(solver, false);   // Skip first read
    }
    else
    {
      readInputs(solver, true);     // Read inputs first
    }

    Quality resultQuality;
    // Do the solve
    // Do the solve.
    vector<double> res = solver->getSolvableValues();
    char strVal[20];
    cout << "Before: [ " ;
    for (unsigned int i = 0; i < res.size(); i++)
    {
      sprintf(strVal, "%1.10f ", res[i]);
      cout << strVal << " ";
    }
    cout << " ]" << endl;

    BBSTest::ScopedTimer solveTimer("S:solve");
    solver->solve(wo->getUseSVD(), resultQuality);
    solveTimer.end();

    // Do the solve.
    res = solver->getSolvableValues();
    cout << "After: [ ";
    for (unsigned int i = 0; i < res.size(); i++)
    {
      sprintf(strVal, "%1.14f ", res[i]);
      cout << strVal << " ";
    }
    cout << " ]" << endl;

    //>>>Temporary:
    streamsize prec = cout.precision();
    cout.precision(10);
    cout << "Per prediffer: " << solver->getSolutions() << endl;
    cout.precision (prec);


    // Write result
    // Get solution dataholder DH_Solution* sol;
    DH_Solution* sol = dynamic_cast<DH_Solution*>(getDataManager().getOutHolder(1));
    sol->clearData();
    DH_PL* solPtr = dynamic_cast<DH_PL*>(sol);
    Connection* connSol = getDataManager().getOutConnection(1);
    ASSERTSTR(connSol != 0, "Output 1 not connected!");
    sol->setSolution(solver->getSolvableParmData());
    sol->setQuality(resultQuality);
    sol->setWorkOrderID(wo->getWorkOrderID());
    sol->setIteration(wo->getIteration());

    //>>>> For now: Assume all prediffer domains are equal!!
    DH_Prediff* predInp1 = 
      dynamic_cast<DH_Prediff*>(getDataManager().getInHolder(3));
    sol->setDomain(predInp1->getStartFreq(), predInp1->getEndFreq(),
		   predInp1->getStartTime(), predInp1->getEndTime());

    // Add solution to database
    solPtr->insertDB(*connSol);
  
    if (itsWriteIndivParms) // Write individual parameter solutions to separate table 
    {                       // for easy "querying"
      DH_ParmSol* parmSol = dynamic_cast<DH_ParmSol*>(getDataManager().getOutHolder(2)); 
      parmSol->setWorkOrderID(wo->getWorkOrderID());
      parmSol->setIteration(wo->getIteration());
      parmSol->setQuality(resultQuality);
      parmSol->setDomain(predInp1->getStartFreq(), predInp1->getEndFreq(),
			 predInp1->getStartTime(), predInp1->getEndTime());
      Connection* connParmSol = getDataManager().getOutConnection(2);
      // loop over all solvable parms
      vector<ParmData> pSols = solver->getSolvableParmData();
      vector<ParmData>::iterator iter;
      for (iter = pSols.begin(); iter != pSols.end(); iter++)
      {
	parmSol->setParmName(iter->getName());
	parmSol->setCoefficients(iter->getValue(0), iter->getValue(1),
				 iter->getValue(2), iter->getValue(3));
	parmSol->insertDB(*connParmSol);
      }
    
    }

    if (wo->getCleanUp())   // If Solver (cache) is no longer needed: clean up  
    {
      itsSolvers.erase(contrID);
    }
  }
  
  // Update workorder status
  wo->setStatus(DH_WOSolve::Executed);
  woPtr->updateDB(*connWO);

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

void WH_Solve::readInputs(Solver* solver, bool firstRead)
{
  BBSTest::ScopedTimer riTimer("S:ReadInputs");
  LOG_TRACE_FLOW("WH_Solve::readInputs");
  DH_Prediff* dh;
  for (int i=0; i<itsNPrediffers; i++)
  {
    if (firstRead)
    {
      dh = dynamic_cast<DH_Prediff*>(getDataManager().getInHolder(i+3));
      getDataManager().readyWithInHolder(i+3);  // Cause input to be read
    }
    dh = dynamic_cast<DH_Prediff*>(getDataManager().getInHolder(i+3));
    casa::LSQFit fitter;
    Prediffer::demarshall (fitter, dh->getDataBuffer(), dh->getBufferSize());
    solver->mergeFitter (fitter, i);
  }
}

void WH_Solve::setParmData(Solver* solver)
{
  LOG_TRACE_FLOW("WH_Solve::readInputsAndSetParmData");

  solver->initSolvableParmData(itsNPrediffers); 
  DH_Prediff* dh;
  for (int i=0; i<itsNPrediffers; i++)
  {
    getDataManager().getInHolder(i+3);
    getDataManager().readyWithInHolder(i+3);  // Causes data to be read
    dh = dynamic_cast<DH_Prediff*>(getDataManager().getInHolder(i+3));
    vector<ParmData> pData;
    dh->getParmData(pData);
    solver->setSolvableParmData(pData, i);           // id = i or from prediffer?
  }
}

} // namespace LOFAR
