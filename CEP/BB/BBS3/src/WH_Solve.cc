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
    itsNPrediffers(nPrediffInputs),
    itsSolver     (0)
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
    getDataManager().setAutoTriggerIn(i+2, false);
  }

}

WH_Solve::~WH_Solve()
{
  LOG_TRACE_FLOW("WH_Solve destructor");
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

  vector<string> resultParmNames;
  vector<double> resultParmValues;
  Quality resultQuality;

  // Read workorder
  if (wo->getInitialize())           // Initialize, next interval and solve
  {
    KeyValueMap msArgs;
    float timeInterval;
    vector<string> pNames;    // Parameter names
    wo->getVarData(msArgs, timeInterval, pNames);
    createSolver(msArgs);

    itsSolver->nextInterval();
    readInputsAndSetParmData();
    itsSolver->solve(wo->getUseSVD(), resultParmNames, resultParmValues,
		     resultQuality);
    itsSolver->saveParms();
  }
  else if (wo->getNextInterval())         // Next interval and solve
  {
    ASSERTSTR(itsSolver!=0, "The solver has not been created and initialized.");
    itsSolver->nextInterval();
    readInputsAndSetParmData();
    itsSolver->solve(wo->getUseSVD(), resultParmNames, resultParmValues,
		     resultQuality);
    itsSolver->saveParms();
  }
  else
  {                                       // just solve
    readInputs();
    itsSolver->solve(wo->getUseSVD(), resultParmNames, resultParmValues,
		     resultQuality);
    itsSolver->saveParms();
  }

  // Write result
  // Get solution dataholder DH_Solution* sol;
  DH_Solution* sol = dynamic_cast<DH_Solution*>(getDataManager().getOutHolder(1));
  sol->clearData();
  DH_PL* solPtr = dynamic_cast<DH_PL*>(sol);
  sol->setSolution(resultParmNames, resultParmValues);
  sol->setQuality(resultQuality);
  sol->setWorkOrderID(wo->getWorkOrderID());
  wo->setStatus(DH_WOSolve::Executed);
  // Add solution to database and update work order
  solPtr->insertDB();
  woPtr->updateDB();

}

void WH_Solve::dump()
{
  LOG_TRACE_RTTI("WH_Solve process()");
}

void WH_Solve::createSolver(const KeyValueMap& args)
{
  delete itsSolver;

  // Create a Prediffer object
  //  string msName = args.getString("MSName", "empty") + ".MS";
  string meqModel = args.getString("meqTableName", "meqmodel");
  string skyModel = args.getString("skyTableName", "skymodel");
  string dbType = args.getString("DBType", "postgres");
  string dbName = args.getString("DBName", "test");
  string dbHost = args.getString("DBHost", "dop50");
  string dbPwd = args.getString("DBPwd", "");

  itsSolver = new Solver(meqModel, skyModel, dbType, 
			 dbName, dbHost, dbPwd);
}

void WH_Solve::readInputs()
{
  LOG_TRACE_FLOW("WH_Solve::readInputs");
//   for (int i=1; i<=itsNPrediffers; i++)
//   {
//     DH_Prediff* dh = dynamic_cast<DH_Prediff*>(getDataManager().getInHolder(i));

//     itsSolver->setEquations(dh->getDataPtr(), dh->getNResults(), dh->getNspids(),
//                             dh->getNTimes(), dh->getNFreq(), i);     // id = i or from prediffer?
//   }
}

void WH_Solve::readInputsAndSetParmData()
{
  LOG_TRACE_FLOW("WH_Solve::readInputsAndSetParmData");
//   for (int i=1; i<=itsNPrediffers; i++)
//   {
//     DH_Prediff* dh = dynamic_cast<DH_Prediff*>(getDataManager().getInHolder(i));
//     vector<ParmData> pData;
//     dh->getParmData(pData);
//     vector<ParmData>::iterator iter;
//     for (iter=pData.begin(); iter!=pData.end(); iter++)
//     {
//       itsSolver->setSolvableParmData(*iter, i);           // id = i or from prediffer?
//     }

//     itsSolver->setEquations(dh->getDataPtr(), dh->getNResults(), dh->getNspids(),
//                             dh->getNTimes(), dh->getNFreq(), i);    // id = i or from prediffer?
//   }

}


} // namespace LOFAR
