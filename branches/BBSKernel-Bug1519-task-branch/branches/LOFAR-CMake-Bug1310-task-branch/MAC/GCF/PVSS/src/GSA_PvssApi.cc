//#  GSA_PvssApi.cc: describes the SCADA handler for connection with the 
//#                      PVSS system
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
#include <Common/LofarLogger.h>

#include <unistd.h>
#include <GCF/TM/GCF_Task.h>
#include <CharString.hxx>
#include <StartDpInitSysMsg.hxx>
#include <GCF/PVSS/PVSSresult.h>
#include "GSA_PvssApi.h"

namespace LOFAR {
 namespace GCF {
  namespace PVSS {
// -------------------------------------------------------------------------
// Our manager class

// The constructor defines Manager type (API_MAN) and Manager number
GSAPvssApi::GSAPvssApi() : 
  Manager(ManagerIdentifier(API_MAN, Resources::getManNum()))
{
  init();
}

// Start our ApiTest manager
void GSAPvssApi::init()
{
  // First connect to Data manager.
  // We want Typecontainer and Identification so we can resolve names
  // This call succeeds or the manager will exit
	connectToData(StartDpInitSysMsg::TYPE_CONTAINER | StartDpInitSysMsg::DP_IDENTIFICATION);

  long sec, usec;
  
  // While we are in STATE_INIT  we are initialized by the Data manager
  while (getManagerState() == STATE_INIT) {
    // Wait max. 1 second in select to receive next message from data.
    // It won't take that long...
    sec = 1;
    usec = 0;
    dispatch(sec, usec);	// dispatch of the PVSS manager
  }

  // We are now in STATE_ADJUST and can connect to Event manager
  // This call will succeed or the manager will exit
  connectToEvent();

  if (getManagerState() == STATE_RUNNING) {
    LOG_INFO("Application connected to PVSS system");
  }
  
  // We are now hopefully in STATE_RUNNING. 
}

void GSAPvssApi::doWork()
{
  if (getManagerState() == STATE_RUNNING) {
   	long sec(0), usec(1);

    dispatch(sec, usec);	// dispatch of the PVSS manager
  }
  else {
    init();
  }
}

void GSAPvssApi::stop()
{

}
  } // namespace PVSS
 } // namespace GCF
} // namespace LOFAR
