//  WH_Evaluate.cc:
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

#include <lofar_config.h>

#include <stdio.h>             // for sprintf

#include <BBS3/WH_Evaluate.h>
#include <CEPFrame/Step.h>
#include <Common/Debug.h>
#include <BBS3/DH_WorkOrder.h>
#include <BBS3/DH_Solution.h>
#include <BBS3/SI_Peeling.h>
#include <BBS3/SI_Simple.h>
#include <BBS3/SI_WaterCal.h>
#include <BBS3/SI_Randomized.h>

namespace LOFAR
{

int WH_Evaluate::theirNextWorkOrderID = 1;

WH_Evaluate::WH_Evaluate (const string& name,
			  const KeyValueMap& args)
  : WorkHolder    (2, 2, name,"WH_Evaluate"),
    itsCurrentRun(0),
    itsEventCnt(0),
    itsOldestSolIdx(0),
    itsArgs(args)
{
  getDataManager().addInDataHolder(0, new DH_WorkOrder("in_0"));
  getDataManager().addInDataHolder(1, new DH_Solution("in_1")); 
  getDataManager().addOutDataHolder(0, new DH_WorkOrder("out_0"));
  getDataManager().addOutDataHolder(1, new DH_Solution("out_1")); 
  // switch output trigger of
  getDataManager().setAutoTriggerIn(0, false);
  getDataManager().setAutoTriggerIn(1, false);
  getDataManager().setAutoTriggerOut(0, false);
  getDataManager().setAutoTriggerOut(1, false);
}

WH_Evaluate::~WH_Evaluate()
{
}

WorkHolder* WH_Evaluate::construct (const string& name, 
				    const KeyValueMap& args)
{
  return new WH_Evaluate (name, args);
}

WH_Evaluate* WH_Evaluate::make (const string& name)
{
  return new WH_Evaluate(name, itsArgs);
}

void WH_Evaluate::process()
{
  TRACER3("WH_Evaluate process()");
//   if (itsEventCnt > 0)
//   {  readSolutions(); }

//  int numberKS = itsArgs.getInt("nrKS", 1);
  string strategy = itsArgs.getString("strategy", "Simple");
 
  vector<string> pNames = (const_cast<KeyValueMap&>(itsArgs))["solvableParams"].getVecString();

  if (strategy == "Simple")
  {
    if (itsEventCnt == 0)
    {
      TRACER1( "Strategy: SIMPLE");

      // Define new simple work order
      DH_WorkOrder* wo = dynamic_cast<DH_WorkOrder*>(getDataManager().getInHolder(0));
      AssertStr(wo != 0, "DataHolder cannot be cast to a DH_WorkOrder");
      wo->setWorkOrderID(theirNextWorkOrderID++);
      wo->setStatus(DH_WorkOrder::New);
      wo->setKSType("KS");
      wo->setStrategyNo(1);

      
      // Set strategy arguments
      KeyValueMap stratArgs = (const_cast<KeyValueMap&>(itsArgs))["STRATparams"].getValueMap();
      wo->setVarData(stratArgs, pNames, itsStartSols);

      //  wo->dump();

      // Insert WorkOrder into database
      DH_PL* woPtr = dynamic_cast<DH_PL*>(wo);
      AssertStr(woPtr != 0, "OutHolder cannot be cast to a DH_PL");
      woPtr->insertDB();
    }
  }

  else if (strategy == "Peel")
  {
    if (itsEventCnt == 0)
    {
      TRACER1("Strategy: PEEL");
      // Define new peeling work order
      DH_WorkOrder* wo = dynamic_cast<DH_WorkOrder*>(getDataManager().getInHolder(0));
      AssertStr(wo != 0, "DataHolder cannot be cast to a DH_WorkOrder");
      wo->setWorkOrderID(theirNextWorkOrderID++);
      wo->setStatus(DH_WorkOrder::New);
      wo->setKSType("KS");
      wo->setStrategyNo(2);

      // Set strategy arguments
      KeyValueMap stratArgs = (const_cast<KeyValueMap&>(itsArgs))["STRATargs"].getValueMap();
      wo->setVarData(stratArgs, pNames, itsStartSols);

      // Insert WorkOrder into database
      DH_PL* woPtr = dynamic_cast<DH_PL*>(wo);
      AssertStr(woPtr != 0, "OutHolder cannot be cast to a DH_PL");
      woPtr->insertDB(); 
      
      wo->dump();
    }
  }

  itsEventCnt++;

}

void WH_Evaluate::dump()
{
}

void WH_Evaluate::postprocess()
{
}

void WH_Evaluate::readSolutions()
{
  DH_Solution* sol = dynamic_cast<DH_Solution*>(getDataManager().getInHolder(1));
  AssertStr(sol != 0,  "DataHolder* can not be cast to a DH_Solution*");
  DH_PL* solPtr = dynamic_cast<DH_PL*>(sol);
  AssertStr(solPtr != 0,  "DH_Solution* can not be cast to a DH_PL*");


  // Wait for solution
  bool firstTime = true;
  while (solPtr->queryDB("") <= 0)
  {
    if (firstTime)
    {
      cout << "No solution found by " << getName() << ". Waiting for solution..." << endl;
      firstTime = false;
    }
  }

  sol->dump();
  vector<int> readIDs;
  readIDs.push_back(sol->getID());
  string query;
  query = createQuery(readIDs);
  cout << "Solution query : " << query << endl;
  //baseQuery << "bbid not in (" << sol->getID();
  while ((solPtr->queryDB(query)) > 0)
  {
    sol->dump();
    readIDs.push_back(sol->getID());
    query = createQuery(readIDs);
    cout << "Solution query : " << query << endl;
  }
}

string WH_Evaluate::createQuery(vector<int>& solIDs) const
{
  int nr = solIDs.size();
  std::ostringstream q;
  if (nr > 0)
  {
    q << "bbid not in (" << solIDs[0];
    for (int i = 1; i < nr; i++)
    {
      q << "," << solIDs[i];
    }
    q << ")";
  }
  return q.str();
}

} // namespace LOFAR
