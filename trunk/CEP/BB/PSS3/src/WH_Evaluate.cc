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

using namespace LOFAR;

WH_Evaluate::WH_Evaluate (const string& name,
			  const int NrKS)
  : WorkHolder    (1, 1, name,"WH_Evaluate"),
    itsNrKS(NrKS),
    itsCurrentRun(0),
    itsEventCnt(0)
{
  getDataManager().addInDataHolder(0, new DH_Solution("in_0", "Control"), true); 
  getDataManager().addOutDataHolder(0, new DH_WorkOrder("out_0"), true);
  // switch output trigger of
  getDataManager().setAutoTriggerOut(0, false);
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

#define WATERCAL
   

  int iter=3;
#ifdef PEEL
  cout << "Strategy: PEEL" << endl;
  // Define new peeling work order
  DH_WorkOrder* outp = (DH_WorkOrder*)getDataManager().getOutHolder(0);
  outp->setStatus(DH_WorkOrder::New);
  outp->setKSType("KS");
  outp->setStrategyNo(2);
  // Set arguments for peeling
  int size = sizeof(SI_Peeling::Peeling_data);
  outp->setArgSize(size);
  SI_Peeling::Peeling_data* data = 
    (SI_Peeling::Peeling_data*)outp->getVarArgsPtr();
  data->nIter = 6;
  data->nSources = 3;
  data->startSource = 1;
  data->timeInterval = 3600.;
  // To be added: Set parameter names
  getDataManager().readyWithOutHolder(0);


#elif defined WATERCAL
  DbgAssert(itsNrKS==3);
  cout << "Strategy: WATERCAL with " << itsNrKS << " Knowledge Sources" <<endl;
  for (int step=0; step < itsNrKS; step++) {
    int sourceno = (step)%itsNrKS+1;
    // Define next WaterCal work order
    DH_WorkOrder* outp = (DH_WorkOrder*)getDataManager().getOutHolder(0);
    outp->setStatus(DH_WorkOrder::New);
    char ksname[4];
    sprintf(ksname,"KS%1i",sourceno);
    outp->setKSType(ksname);
    outp->setStrategyNo(3);
    if ((step==0) && (itsEventCnt==0)) {
      //the very first step
      outp->useSolutionNumber(-1);
    } else {
      int last_result_step = step        + (step==0 ? itsNrKS : 0);
      int last_result_Ecnt = itsEventCnt - (step==0 ? 1       : 0);
      outp->useSolutionNumber(   last_result_step   *10000 
				 + (last_result_Ecnt+1)*iter -1);
    }
    // Set arguments for peeling
    int size = sizeof(SI_WaterCal::WaterCal_data);
    outp->setArgSize(size);
    SI_WaterCal::WaterCal_data* data = 
      (SI_WaterCal::WaterCal_data*)outp->getVarArgsPtr();
    data->nIter = iter;
    data->sourceNo = sourceno;
    data->timeInterval = 3600.;
    // To be added: set parameter names
    getDataManager().readyWithOutHolder(0);
  }
  itsEventCnt++;

#elif defined RANDOM
  cout << "Strategy: RANDOM" << endl;
   for (int step=1; step <= 3; step++) {
    int rndSrc;
    rndSrc = (rand () % 3) + 1;
    cout << itsCurrentRun << " ========> Random source: " << rndSrc << endl;
    cerr << itsCurrentRun << ' ';
    DH_WorkOrder* outp = (DH_WorkOrder*)getDataManager().getOutHolder(0);
    outp->setStatus(DH_WorkOrder::New);
    char ksname[4];
    sprintf(ksname,"KS%1i",step);
    outp->setKSType(ksname);
    outp->setStrategyNo(3);
    if (step==1) {
      outp->useSolutionNumber(-1);
    } else {
      outp->useSolutionNumber((step-1)*10000+iter-1);
    }
    // Set arguments for peeling
    int size = sizeof(SI_Randomized::Randomized_data);
    outp->setArgSize(size);
    SI_Randomized::Randomized_data* data = 
      (SI_Randomized::Randomized_data*)outp->getVarArgsPtr();
    data->nIter = iter;
    data->sourceNo = rndSrc;
    data->timeInterval = 3600.;
    // To be added: set parameter names
    getDataManager().readyWithOutHolder(0);
  }

#else // default to SIMPLE
  cout << "Strategy: SIMPLE" << endl;
  // Define new simple work order
  DH_WorkOrder* outp = (DH_WorkOrder*)getDataManager().getOutHolder(0);
  outp->setStatus(DH_WorkOrder::New);
  outp->setKSType("KS");
  outp->setStrategyNo(1);
  // Set arguments for peeling
  int size = sizeof(SI_Simple::Simple_data);
  outp->setArgSize(size);
  SI_Simple::Simple_data* data = 
    (SI_Simple::Simple_data*)outp->getVarArgsPtr();
  data->nIter = iter;
  data->nSources = 3;
  data->timeInterval = 3600.;
  // To be added: Set parameter names
  getDataManager().readyWithOutHolder(0);

#endif
}

void WH_Evaluate::dump()
{
}

