//  WH_Control.cc:
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

#include <BBS3/WH_Control.h>
#include <CEPFrame/Step.h>
#include <BBS3/DH_Solution.h>
#include <BBS3/DH_WOPrediff.h>
#include <BBS3/DH_WOSolve.h>
#include <BBS3/SC_Simple.h>
#include <BBS3/StrategyController.h>

namespace LOFAR
{

int WH_Control::theirNextWorkOrderID = 1;

WH_Control::WH_Control (const string& name,
			const KeyValueMap& args)
  : WorkHolder    (2, 2, name,"WH_Control"),
    itsCurrentRun(0),
    itsEventCnt(0),
    itsOldestSolIdx(0),
    itsArgs(args)
{
  getDataManager().addInDataHolder(0, new DH_Solution("in_1")); 
  getDataManager().addOutDataHolder(0, new DH_WOPrediff("out_0"));
  getDataManager().addOutDataHolder(1, new DH_WOSolve("out_1")); 
  // switch output trigger of
  getDataManager().setAutoTriggerIn(0, false);
  getDataManager().setAutoTriggerOut(0, false);
  getDataManager().setAutoTriggerOut(1, false);
}

WH_Control::~WH_Control()
{
  list<StrategyController*>::iterator iter;
  for (iter=itsControllers.begin(); iter!=itsControllers.end(); iter++)
  {
    delete (*iter);
  }
  itsControllers.clear();
}

WorkHolder* WH_Control::construct (const string& name, 
				    const KeyValueMap& args)
{
  return new WH_Control (name, args);
}

void WH_Control::preprocess()
{
  // >>> Create StrategyController(s) and add to list
  DH_Solution* inp = dynamic_cast<DH_Solution*>(getDataManager().getInHolder(0));
  DH_WOPrediff* pd = dynamic_cast<DH_WOPrediff*>(getDataManager().getOutHolder(0));
  DH_WOSolve* sv = dynamic_cast<DH_WOSolve*>(getDataManager().getOutHolder(1));
  SC_Simple* sc = new SC_Simple(1, inp, pd, sv, itsArgs);
  itsControllers.push_back(sc);
}

WH_Control* WH_Control::make (const string& name)
{
  return new WH_Control(name, itsArgs);
}

void WH_Control::process()
{
  list<StrategyController*>::iterator iter;
  for (iter=itsControllers.begin(); iter!=itsControllers.end(); iter++)
  {
    if ((*iter)->execute() == false)  // Remove from list
    {
      delete (*iter);
      iter = itsControllers.erase(iter); // Returns an iterator that designates 
                                         // the first element remaining beyond 
                                         // any elements removed, or end()
      iter--;
    }
  }
}

void WH_Control::dump()
{
}

void WH_Control::postprocess()
{
}



} // namespace LOFAR
