//#  SC_Simple.cc:  The peeling calibration strategy
//#
//#  Copyright (C) 2002-2003
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

#include <lofar_config.h>

#include <BBS3/SC_Simple.h>
#include <Common/LofarLogger.h>
#include <Common/KeyValueMap.h>
#include <TransportPL/DH_PL.h>
#include <BBS3/DH_Solution.h>
#include <BBS3/DH_WOPrediff.h>
#include <BBS3/DH_WOSolve.h>

namespace LOFAR
{

SC_Simple::SC_Simple(int id, DH_Solution* inDH, DH_WOPrediff* woPD,
		     DH_WOSolve* woSolve, const KeyValueMap& args)
  : StrategyController(id, inDH, woPD, woSolve),
    itsFirstCall      (true),
    itsWOID           (0)
{
}

SC_Simple::~SC_Simple()
{}

bool SC_Simple::execute()
{
  if (itsFirstCall)
  {
    itsFirstCall = false;
    itsWOPD->clearData();
    itsWOSolve->clearData();
  }
  else
  {
    readSolution();
    itsWOPD->clearData();
    itsWOSolve->clearData();    
  }

  // Post new workorders
  itsWOID++;
//       // Define new simple work order
//       DH_WorkOrder* wo = dynamic_cast<DH_WorkOrder*>(getDataManager().getInHolder(0));
//       ASSERTSTR(wo != 0, "DataHolder cannot be cast to a DH_WorkOrder");
//       wo->setWorkOrderID(theirNextWorkOrderID++);
//       wo->setStatus(DH_WorkOrder::New);
//       wo->setKSType("KS");
//       wo->setStrategyNo(1);

      
//       // Set strategy arguments
//       KeyValueMap stratArgs = (const_cast<KeyValueMap&>(itsArgs))["STRATparams"].getValueMap();
//       wo->setVarData(stratArgs, pNames, itsStartSols);

//       //  wo->dump();

//       // Insert WorkOrder into database
//       DH_PL* woPtr = dynamic_cast<DH_PL*>(wo);
//       ASSERTSTR(woPtr != 0, "OutHolder cannot be cast to a DH_PL");
//       woPtr->insertDB();

  return true;
}

void SC_Simple::readSolution()
{
  DH_PL* solPtr = dynamic_cast<DH_PL*>(getSolution());

  // Wait for solution
  bool firstTime = true;
  while (solPtr->queryDB("woid=" + itsWOID) <= 0)
  {
    if (firstTime)
    {
      cout << "No solution found by SC_Simple " << getID() 
	   << ". Waiting for solution..." << endl;
      firstTime = false;
    }
  }

  getSolution()->dump();

}

} // namespace LOFAR
