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

#include "PSS3/WH_Evaluate.h"
#include "CEPFrame/Step.h"
#include "CEPFrame/ParamBlock.h"
#include "Common/Debug.h"
#include "PSS3/DH_WorkOrder.h"
#include "PSS3/SI_Peeling.h"

using namespace LOFAR;

WH_Evaluate::WH_Evaluate (const string& name)
: WorkHolder    (1, 1, name,"WH_Evaluate")
{
  getDataManager().addInDataHolder(0, new DH_WorkOrder("in_0", "Control"), 
 				   true); 
  getDataManager().addOutDataHolder(0, new DH_WorkOrder("out_0", "Control"), 
				    true);
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
  DH_WorkOrder* inp = (DH_WorkOrder*)getDataManager().getInHolder(0);
  cout << "Controller reads results : [" 
       << inp->getParam1Value() <<", "
       << inp->getParam2Value() <<", "
       << inp->getParam3Value() << "]" << endl;

  // Define new work order
  DH_WorkOrder* outp = (DH_WorkOrder*)getDataManager().getOutHolder(0);
  outp->setStatus(DH_WorkOrder::New);
  outp->setKSType("PSS3");
  outp->setStrategyNo(1);
  // Set arguments for peeling
  int size = sizeof(SI_Peeling::Peeling_data);
  outp->setArgSize(size);
  SI_Peeling::Peeling_data* data = 
    (SI_Peeling::Peeling_data*)outp->getVarArgsPtr();
  data->nIter = 2;
  data->nSources = 2;
  data->timeInterval = 3600.;
  // To be added: Set parameter names
  getDataManager().readyWithOutHolder(0);

  // Define next work order
  outp = (DH_WorkOrder*)getDataManager().getOutHolder(0);
  outp->setStatus(DH_WorkOrder::New);
  outp->setKSType("PSS3");
  outp->setStrategyNo(1);
  // Set arguments for peeling
  size = sizeof(SI_Peeling::Peeling_data);
  outp->setArgSize(size);
  data = (SI_Peeling::Peeling_data*)outp->getVarArgsPtr();
  data->nIter = 1;
  data->nSources = 3;
  data->timeInterval = 3600.;
  // To be added: set parameter names
  getDataManager().readyWithOutHolder(0);
}

void WH_Evaluate::dump()
{
}

