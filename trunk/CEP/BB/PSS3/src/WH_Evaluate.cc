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

#include <stdio.h>             // for sprintf

#include <PSS3/WH_Evaluate.h>
#include <CEPFrame/Step.h>
#include <Common/Debug.h>
#include <PSS3/DH_WorkOrder.h>
#include <PSS3/DH_Solution.h>
#include <PSS3/SI_Peeling.h>
#include <PSS3/SI_Simple.h>
#include <PSS3/SI_WaterCal.h>
#include <PSS3/SI_Randomized.h>

namespace LOFAR
{

int WH_Evaluate::theirNextWorkOrderID = 1;

WH_Evaluate::WH_Evaluate (const string& name,
			  const int NrKS)
  : WorkHolder    (2, 2, name,"WH_Evaluate"),
    itsNrKS(NrKS),
    itsCurrentRun(0),
    itsEventCnt(0),
    itsOldestSolIdx(0)
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
				    const int NrKS)
{
  return new WH_Evaluate (name, NrKS);
}

WH_Evaluate* WH_Evaluate::make (const string& name)
{
  return new WH_Evaluate(name, itsNrKS);
}

void WH_Evaluate::process()
{
  TRACER3("WH_Evaluate process()");
//   if (itsEventCnt > 0)
//   {  readSolutions(); }

#define WATERCAL
  int iter=5;

#ifdef PEEL
  if (itsEventCnt == 0)
{
  cout << "Strategy: PEEL" << endl;
  // Define new peeling work order
  DH_WorkOrder* wo = dynamic_cast<DH_WorkOrder*>(getDataManager().getInHolder(0));
  AssertStr(wo != 0, "DataHolder cannot be cast to a DH_WorkOrder");
  wo->setWorkOrderID(theirNextWorkOrderID++);
  wo->setStatus(DH_WorkOrder::New);
  wo->setKSType("KS");
  wo->setStrategyNo(2);

  // Set strategy arguments
  int size = sizeof(SI_Peeling::Peeling_data);
  SI_Peeling::Peeling_data data;
  data.nIter = iter;
  data.nSources = 10;
  data.startSource = 1;
  data.timeInterval = 3600.;

  // Set parameter names
  vector<string> pNames(3);
  pNames[0] = "StokesI.CP";
  pNames[1] = "RA.CP";
  pNames[2] = "DEC.CP";
//   vector<string> pNames(30);
//   pNames[0] = "StokesI.CP1";
//   pNames[1] = "RA.CP1";
//   pNames[2] = "DEC.CP1";

//   pNames[3] = "StokesI.CP2";
//   pNames[4] = "RA.CP2";
//   pNames[5] = "DEC.CP2";

//   pNames[6] = "StokesI.CP3";
//   pNames[7] = "RA.CP3";
//   pNames[8] = "DEC.CP3";

//   pNames[9] = "StokesI.CP4";
//   pNames[10] = "RA.CP4";
//   pNames[11] = "DEC.CP4";

//   pNames[12] = "StokesI.CP5";
//   pNames[13] = "RA.CP5";
//   pNames[14] = "DEC.CP5";

//   pNames[15] = "StokesI.CP6";
//   pNames[16] = "RA.CP6";
//   pNames[17] = "DEC.CP6";

//   pNames[18] = "StokesI.CP7";
//   pNames[19] = "RA.CP7";
//   pNames[20] = "DEC.CP7";

//   pNames[21] = "StokesI.CP8";
//   pNames[22] = "RA.CP8";
//   pNames[23] = "DEC.CP8";

//   pNames[24] = "StokesI.CP9";
//   pNames[25] = "RA.CP9";
//   pNames[26] = "DEC.CP9";

//   pNames[27] = "StokesI.CP10";
//   pNames[28] = "RA.CP10";
//   pNames[29] = "DEC.CP10";
//   pNames[9] = "gain.11.SR1";
//   pNames[10] = "gain.22.SR1";
  wo->setVarData((char*)&data, size, pNames, itsStartSols);

  // Insert WorkOrder into database
  DH_PL* woPtr = dynamic_cast<DH_PL*>(wo);
  AssertStr(woPtr != 0, "OutHolder cannot be cast to a DH_PL");
  woPtr->insertDB(); 

  wo->dump();
}

#elif defined WATERCAL
  if (itsEventCnt < 30)
{
  //   DbgAssert(itsNrKS==3);
   cout << "Strategy: WATERCAL with " << itsNrKS << " Knowledge Sources" <<endl;
   for (int step=0; step < itsNrKS; step++) {
     int sourceno = (step)%itsNrKS+1;
     // Define next WaterCal work order
     DH_WorkOrder* wo = dynamic_cast<DH_WorkOrder*>(getDataManager().getOutHolder(0));
     AssertStr(wo != 0, "DataHolder cannot be cast to a DH_WorkOrder");
     wo->setWorkOrderID(theirNextWorkOrderID++);
     wo->setStatus(DH_WorkOrder::New);
     char ksname[4];
     sprintf(ksname,"KS%1i",sourceno);
     wo->setKSType(ksname);
     wo->setStrategyNo(3);
     if ((step==0) && (itsEventCnt==0)) {
       //the very first step
       itsStartSols.clear();
     } else {
       int last_result_step = step        + (step==0 ? itsNrKS : 0);
       int last_result_Ecnt = itsEventCnt - (step==0 ? 1       : 0);
       if (itsStartSols.size() >= 10)
	 {                                           // Replace oldest value
	 itsStartSols[itsOldestSolIdx] = last_result_step   *10000  + (last_result_Ecnt+1)*iter -1;
	   itsOldestSolIdx++;
	   if (itsOldestSolIdx >= 10)
	   { itsOldestSolIdx = 0; }
       }
       else
       {
	 itsStartSols.push_back( last_result_step   *10000 
			       + (last_result_Ecnt+1)*iter -1);
       }
     }						   
     // Determine arguments for watercal strategy
     int size = sizeof(SI_WaterCal::WaterCal_data);
     SI_WaterCal::WaterCal_data data;
     data.nIter = iter;
     int srcNr = itsEventCnt%10 + 1; 
     data.sourceNo = srcNr;
     data.timeInterval = 3600.;

     // Determine parameter names
     vector<string> pNames(3);
     char param[16];
     sprintf(param, "StokesI.CP%1i", srcNr);
     pNames[0] = param;
     sprintf(param, "RA.CP%1i", srcNr);
     pNames[1] = param;
     sprintf(param, "DEC.CP%1i", srcNr);
     pNames[2] = param;
//      pNames[3] = "gain.11.SR1";
//      pNames[4] = "gain.22.SR1";

     // Set variable size arguments
     wo->setVarData((char*)&data, size, pNames, itsStartSols);
     
     // Insert WorkOrder into database
     DH_PL* woPtr = dynamic_cast<DH_PL*>(wo);
     AssertStr(woPtr != 0, "OutHolder cannot be cast to a DH_PL");
     woPtr->insertDB(); 

   }
}

#elif defined RANDOM
   cout << "Strategy: RANDOM" << endl;
int nrOfSources = 6;
   for (int step=1; step <= nrOfSources; step++) {
     int rndSrc;
     rndSrc = (rand () % nrOfSources) + 1;
     cout << itsCurrentRun << " ========> Random source: " << rndSrc << endl;
     cerr << itsCurrentRun << ' ';
     // Define next WaterCal work order
     DH_WorkOrder* wo = dynamic_cast<DH_WorkOrder*>(getDataManager().getOutHolder(0));
     AssertStr(wo != 0, "DataHolder cannot be cast to a DH_WorkOrder");
	 wo->setWorkOrderID(theirNextWorkOrderID++);
     wo->setStatus(DH_WorkOrder::New);
     char ksname[4];
     sprintf(ksname,"KS%1i",step);
     wo->setKSType(ksname);
     wo->setStrategyNo(4);
     if (!((step==1) && (itsEventCnt==0))) {
       int last_result_step = step-1      + (step==1 ? itsNrKS : 0);
       int last_result_Ecnt = itsEventCnt + (step==1 ? 0       : 1);
       itsStartSols.push_back( last_result_step   *10000 
			       + last_result_Ecnt*iter -1);

      }
     // Set arguments for peeling
     int size = sizeof(SI_Randomized::Randomized_data);
     SI_Randomized::Randomized_data data;
     data.nIter = iter;
     data.sourceNo = rndSrc;
     data.timeInterval = 3600.;

     // Set parameter names
     vector<string> pNames(3);
     char param[16];
     sprintf(param, "StokesI.CP%1i", rndSrc);
     cout << param << endl;
     pNames[0] = param;
     sprintf(param, "RA.CP%1i", rndSrc);
     cout << param << endl;
     pNames[1] = param;
     sprintf(param, "DEC.CP%1i", rndSrc);
     cout << param << endl;
     pNames[2] = param;
     // Set variable size arguments.
     wo->setVarData((char*)&data, size, pNames, itsStartSols);
     wo->dump();

     // Insert WorkOrder into database
     DH_PL* woPtr = dynamic_cast<DH_PL*>(wo);
     AssertStr(woPtr != 0, "OutHolder cannot be cast to a DH_PL");
     woPtr->insertDB(); 
}

#elif defined TEST
   cout << "Strategy: TESTRANDOM" << endl;
   unsigned int nrOfSources = 3;
   for (unsigned int step=1; step <= nrOfSources; step++) {
     int rndSrc;
     rndSrc = (rand () % nrOfSources) + 1;
     cout << itsCurrentRun << " ========> Random source: " << rndSrc << endl;
     cerr << itsCurrentRun << ' ';
     // Define next WaterCal work order
     DH_WorkOrder* wo = dynamic_cast<DH_WorkOrder*>(getDataManager().getOutHolder(0));
     AssertStr(wo != 0, "DataHolder cannot be cast to a DH_WorkOrder");
     wo->setWorkOrderID(theirNextWorkOrderID++);
     wo->setStatus(DH_WorkOrder::New);
     char ksname[4];
     sprintf(ksname,"KS%1i",step);
     wo->setKSType(ksname);
     wo->setStrategyNo(4);
     // Determine start solutions
     if (itsEventCnt==0) {
       itsStartSols.clear();
     } 
     else if (step==1) {
       for (unsigned int k = 1; k <= nrOfSources; k++)
       {
	 if ((itsStartSols.size()) >= (2*nrOfSources))  // Make sure itsStartSols does not become
	 {                                             // unnecessarily large.
           itsStartSols[itsOldestSolIdx] = k*10000 + itsEventCnt*iter-1; // Replace oldest solution id
	   itsOldestSolIdx++;
	   if (itsOldestSolIdx >= 2*nrOfSources)
	   { itsOldestSolIdx = 0; }
         }
	 else
	 {
	   itsStartSols.push_back(k*10000 + itsEventCnt*iter-1); // Add to vector
	 }
       }
     }

     // Set arguments for peeling
     int size = sizeof(SI_Randomized::Randomized_data);
     SI_Randomized::Randomized_data data;
     data.nIter = iter;
     data.sourceNo = rndSrc;
     data.timeInterval = 3600.;

     // Set parameter names
     vector<string> pNames(3);
     char param[16];
     sprintf(param, "StokesI.CP%1i", rndSrc);
     cout << param << endl;
     pNames[0] = param;
     sprintf(param, "RA.CP%1i", rndSrc);
     cout << param << endl;
     pNames[1] = param;
     sprintf(param, "DEC.CP%1i", rndSrc);
     cout << param << endl;
     pNames[2] = param;

     wo->setVarData((char*)&data, size, pNames, itsStartSols);

      // Insert WorkOrder into database
     DH_PL* woPtr = dynamic_cast<DH_PL*>(wo);
     AssertStr(woPtr != 0, "OutHolder cannot be cast to a DH_PL");
     woPtr->insertDB(); 

     wo->dump();
}

#else // default to SIMPLE

  if (itsEventCnt == 0)
{
  cout << "Strategy: SIMPLE" << endl;

  // Define new simple work order
  DH_WorkOrder* wo = dynamic_cast<DH_WorkOrder*>(getDataManager().getInHolder(0));
  AssertStr(wo != 0, "DataHolder cannot be cast to a DH_WorkOrder");
  wo->setWorkOrderID(theirNextWorkOrderID++);
  wo->setStatus(DH_WorkOrder::New);
  wo->setKSType("KS");
  wo->setStrategyNo(1);

  // Set strategy arguments
  int size = sizeof(SI_Simple::Simple_data);
  SI_Simple::Simple_data data;
  data.nIter = iter;
  data.nSources = 10;
  data.timeInterval = 3600.;

  // Set parameter names
  vector<string> pNames(30);
  pNames[0] = "StokesI.CP1";
  pNames[1] = "RA.CP1";
  pNames[2] = "DEC.CP1";

  pNames[3] = "StokesI.CP2";
  pNames[4] = "RA.CP2";
  pNames[5] = "DEC.CP2";

  pNames[6] = "StokesI.CP3";
  pNames[7] = "RA.CP3";
  pNames[8] = "DEC.CP3";

  pNames[9] = "StokesI.CP4";
  pNames[10] = "RA.CP4";
  pNames[11] = "DEC.CP4";

  pNames[12] = "StokesI.CP5";
  pNames[13] = "RA.CP5";
  pNames[14] = "DEC.CP5";

  pNames[15] = "StokesI.CP6";
  pNames[16] = "RA.CP6";
  pNames[17] = "DEC.CP6";

  pNames[18] = "StokesI.CP7";
  pNames[19] = "RA.CP7";
  pNames[20] = "DEC.CP7";

  pNames[21] = "StokesI.CP8";
  pNames[22] = "RA.CP8";
  pNames[23] = "DEC.CP8";

  pNames[24] = "StokesI.CP9";
  pNames[25] = "RA.CP9";
  pNames[26] = "DEC.CP9";

  pNames[27] = "StokesI.CP10";
  pNames[28] = "RA.CP10";
  pNames[29] = "DEC.CP10";

//  pNames[9] = "gain.11.SR1";
//  pNames[10] = "gain.22.SR1";

  wo->setVarData((char*)&data, size, pNames, itsStartSols);

  // Insert WorkOrder into database
  DH_PL* woPtr = dynamic_cast<DH_PL*>(wo);
  AssertStr(woPtr != 0, "OutHolder cannot be cast to a DH_PL");
  woPtr->insertDB();

  //  wo->dump();

}

#endif

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
