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

using namespace LOFAR;

WH_Evaluate::WH_Evaluate (const string& name)
: WorkHolder    (1, 1, name,"WH_Evaluate")
{
  getDataManager().addInDataHolder(0, new DH_Solution("in_0", "Control"), true); 
  getDataManager().addOutDataHolder(0, new DH_WorkOrder("out_0"), true);
  // switch output trigger of
  getDataManager().setAutoTriggerOut(0, false);
}

WH_Evaluate::~WH_Evaluate()
{
}

WH_Evaluate* WH_Evaluate::make (const string& name)
{
  return new WH_Evaluate(name);
}

void WH_Evaluate::process()
{
  TRACER3("WH_Evaluate process()");

//   // Define new simple work order
//   DH_WorkOrder* outp = (DH_WorkOrder*)getDataManager().getOutHolder(0);
//   outp->setStatus(DH_WorkOrder::New);
//   outp->setKSType("KS");
//   outp->setStrategyNo(1);
//   // Set arguments for peeling
//   int size = sizeof(SI_Simple::Simple_data);
//   outp->setArgSize(size);
//   SI_Simple::Simple_data* data = 
//     (SI_Simple::Simple_data*)outp->getVarArgsPtr();
//   data->nIter = 20;
//   data->nSources = 2;
//   data->timeInterval = 3600.;
//   // To be added: Set parameter names
//   getDataManager().readyWithOutHolder(0);

//   // Define new peeling work order
//   DH_WorkOrder* outp = (DH_WorkOrder*)getDataManager().getOutHolder(0);
//   outp->setStatus(DH_WorkOrder::New);
//   outp->setKSType("KS");
//   outp->setStrategyNo(2);
//   // Set arguments for peeling
//   int size = sizeof(SI_Peeling::Peeling_data);
//   outp->setArgSize(size);
//   SI_Peeling::Peeling_data* data = 
//     (SI_Peeling::Peeling_data*)outp->getVarArgsPtr();
//   data->nIter = 4;
//   data->nSources = 1;
//   data->startSource = 1;
//   data->timeInterval = 3600.;
//   // To be added: Set parameter names
//   getDataManager().readyWithOutHolder(0);

//   // Define next peeling work order
//   DH_WorkOrder* outp = (DH_WorkOrder*)getDataManager().getOutHolder(0);
//   outp->setStatus(DH_WorkOrder::New);
//   outp->setKSType("KS");
//   outp->setStrategyNo(2);
//   // Set arguments for peeling
//   int size = sizeof(SI_Peeling::Peeling_data);
//   outp->setArgSize(size);
//   SI_Peeling::Peeling_data* data = (SI_Peeling::Peeling_data*)outp->getVarArgsPtr();
//   data->nIter = 5;
//   data->nSources = 2;
//   data->startSource = 1;
//   data->timeInterval = 3600.;
//   // To be added: set parameter names
//   getDataManager().readyWithOutHolder(0);

  // Define next WaterCal work order
   DH_WorkOrder* outp3 = (DH_WorkOrder*)getDataManager().getOutHolder(0);
  outp3->setStatus(DH_WorkOrder::New);
  outp3->setKSType("KS1");
  outp3->setStrategyNo(3);
  //outp3->useSolutionNumber(10005);
  // Set arguments for peeling
  int size3 = sizeof(SI_WaterCal::WaterCal_data);
  outp3->setArgSize(size3);
  SI_WaterCal::WaterCal_data* data3 = 
    (SI_WaterCal::WaterCal_data*)outp3->getVarArgsPtr();
  data3->nIter = 5;
  data3->sourceNo = 1;
  data3->timeInterval = 3600.;
  // To be added: set parameter names
  getDataManager().readyWithOutHolder(0);

  // Define next WaterCal work order
   DH_WorkOrder* outp4 = (DH_WorkOrder*)getDataManager().getOutHolder(0);
  outp4->setStatus(DH_WorkOrder::New);
  outp4->setKSType("KS2");
  outp4->setStrategyNo(3);
  outp4->useSolutionNumber(10004);
  // Set arguments for peeling
  int size4 = sizeof(SI_WaterCal::WaterCal_data);
  outp4->setArgSize(size4);
  SI_WaterCal::WaterCal_data* data4 = 
    (SI_WaterCal::WaterCal_data*)outp4->getVarArgsPtr();
  data4->nIter = 5;
  data4->sourceNo = 2;
  data4->timeInterval = 3600.;
  // To be added: set parameter names
  getDataManager().readyWithOutHolder(0);
}

void WH_Evaluate::dump()
{
}

