//#  StrategyController.cc:  A base class for all calibration strategy controllers
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

#include <BBS3/StrategyController.h>
#include <BBS3/DH_WOPrediff.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{

int StrategyController::theirNextSCID=-1;
int StrategyController::theirNextWOID=-1;

StrategyController::StrategyController(Connection* inSolConn, 
				       Connection* outWOPDConn, 
				       Connection* outWOSolveConn,
				       int nrPrediffers)
  : itsInSolConn     (inSolConn),
    itsOutWOPDConn   (outWOPDConn),
    itsOutWOSolveConn(outWOSolveConn),
    itsNrPrediffers(nrPrediffers)
    
{
  LOG_TRACE_FLOW("StrategyController constructor");
  ASSERTSTR(itsInSolConn != 0, "Solution input has not been connected!");
  ASSERTSTR(itsOutWOPDConn != 0, 
	    "Output to Prediffer has not been connected!");
  ASSERTSTR(itsOutWOSolveConn != 0, 
	    "Output to Solver has not been connected!");

  if (theirNextSCID == -1)       // If first instance of StrategyController
  {                              // Determine unique scid.
    TH_DB* thDB = (TH_DB*)(itsOutWOPDConn->getTransportHolder());
    ASSERT(thDB!=0);
    theirNextSCID = getPrediffWorkOrder()->getMaxSCID(thDB) + 1;
    theirNextWOID = getPrediffWorkOrder()->getMaxWOID(thDB) + 1;
  }
  itsID = theirNextSCID++;
}

StrategyController::~StrategyController()
{
  LOG_TRACE_FLOW("StrategyController destructor");
}


} // namespace LOFAR
