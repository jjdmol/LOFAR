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
#include <BBS3/Solver.h>
#include <BBS3/ParmData.h>

namespace LOFAR
{

WH_Solve::WH_Solve(const string& name, int nPrediffInputs)
  : WorkHolder    (nPrediffInputs+2, 2, name, "WH_Solve"),
    itsNPrediffers(nPrediffInputs)
{
  LOG_TRACE_FLOW("WH_Solve constructor");
  // Add workorder input
  getDataManager().addInDataHolder(0, new DH_WOSolve(name+"_in0"));
  getDataManager().addOutDataHolder(0, new DH_WOSolve(name+"_out0")); // dummy
  // Add solution output
  getDataManager().addInDataHolder(1, new DH_Solution(name+"_in1"));  // dummy
  getDataManager().addOutDataHolder(1, new DH_Solution(name+"_out1"));
  // Switch input channel trigger off
  getDataManager().setAutoTriggerIn(0, false);
  getDataManager().setAutoTriggerOut(0, false);
  getDataManager().setAutoTriggerIn(1, false);
  getDataManager().setAutoTriggerOut(1, false);

  // Add prediffer inputs
  for (int i=0; i<itsNPrediffers; i++)
  {
    getDataManager().addInDataHolder(i+2, new DH_Prediff(name+"_in"));
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
  return new WH_Solve (name, itsNPrediffers);
}

void WH_Solve::preprocess()
{
  LOG_TRACE_RTTI("WH_Solve preprocess()");
}

void WH_Solve::process()
{
  LOG_TRACE_RTTI("WH_Solve process()");

  // Query the database for a work order
  DH_WOSolve* wo =  dynamic_cast<DH_WOSolve*>(getDataManager().getInHolder(0));
  DH_PL* woPtr = dynamic_cast<DH_PL*>(wo);
 
  // Wait for workorder
  bool firstTime = true;
  while ((woPtr->queryDB("status=0 and (kstype='SOLVER' or kstype='" + getName()
                         + "') order by kstype desc, woid asc")) <= 0)
  {
    if (firstTime)
    {
      cout << "No workorder found by " << getName() << ". Waiting for work order..." << endl;
      firstTime = false;
    }
  }

  cout << "!!!!!! Solver read workorder: " << endl;
  wo->dump();
  cout << "!!!!!! " << endl;


  // Update workorder status
  wo->setStatus(DH_WOSolve::Assigned);
  woPtr->updateDB();

  int contrID = wo->getStrategyControllerID();
  Solver* solver = getSolver(contrID);
  DBGASSERTSTR(solver!=0, "The solver has not been created and initialized.");

  if (wo->getNewDomain())         // New domain
  {
    setParmData(solver);
  }

  readInputs(solver);

  Quality resultQuality;
  // Do the solve
  solver->solve(wo->getUseSVD(), resultQuality);

  // Write result
  // Get solution dataholder DH_Solution* sol;
  DH_Solution* sol = dynamic_cast<DH_Solution*>(getDataManager().getOutHolder(1));
  sol->clearData();
  DH_PL* solPtr = dynamic_cast<DH_PL*>(sol);
  sol->setSolution(solver->getSolvableParmData());
  sol->setQuality(resultQuality);
  sol->setWorkOrderID(wo->getWorkOrderID());
  wo->setStatus(DH_WOSolve::Executed);
  // Add solution to database and update work order
  solPtr->insertDB();
  woPtr->updateDB();
  
  if (wo->getCleanUp())   // If Solver (cache) is no longer needed: clean up  
  {
    itsSolvers.erase(contrID);
  }
}

void WH_Solve::dump()
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

void WH_Solve::readInputs(Solver* solver)
{
  LOG_TRACE_FLOW("WH_Solve::readInputs");
  for (int i=1; i<=itsNPrediffers; i++)
  {
    DH_Prediff* dh = dynamic_cast<DH_Prediff*>(getDataManager().getInHolder(i));
    vector<uint32> shape = dh->getDataSize();
    solver->setEquations(dh->getDataPtr(), shape[0],
                         shape[1], shape[2], i);     // id = i or from prediffer?
  }
}

void WH_Solve::setParmData(Solver* solver)
{
  LOG_TRACE_FLOW("WH_Solve::readInputsAndSetParmData");

  solver->initSolvableParmData(itsNPrediffers);

  for (int i=1; i<=itsNPrediffers; i++)
  {
    DH_Prediff* dh = dynamic_cast<DH_Prediff*>(getDataManager().getInHolder(i));
    vector<ParmData> pData;
    dh->getParmData(pData);
    solver->setSolvableParmData(pData, i);           // id = i or from prediffer?
  }
}

} // namespace LOFAR
