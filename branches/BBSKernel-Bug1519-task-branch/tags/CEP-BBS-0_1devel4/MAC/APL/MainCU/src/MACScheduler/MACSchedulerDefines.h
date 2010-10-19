//#  MACScheduler_Defines.h: preprocessor definitions of various constants
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

#ifndef MACScheduler_DEFINES_H
#define MACScheduler_DEFINES_H

namespace LOFAR {
  namespace MCU {

#define MS_TASKNAME					"MACScheduler"

#define PSN_MAC_SCHEDULER   		"LOFAR_PermSW_MACScheduler"
#define PST_MAC_SCHEDULER   		"MACScheduler"
#define PN_MS_ACTIVE_OBSERVATIONS   "activeObservations"
#define PN_MS_OTDB_CONNECTED    	"OTDB.connected"
#define PN_MS_OTDB_LAST_POLL    	"OTDB.lastPoll"
#define PN_MS_OTDB_POLLINTERVAL 	"OTDB.pollinterval"

#define PVSSNAME_MS_QUEUEPERIOD		"QueuePeriod"
#define PVSSNAME_MS_CLAIMPERIOD		"ClaimPeriod"

// next lines should be defined somewhere in Common.
#define PVSSNAME_FSM_CURACT			"currentAction"
#define PVSSNAME_FSM_ERROR			"error"
#define PVSSNAME_FSM_LOGMSG			"logMsg"
#define PVSSNAME_FSM_STATE			"state"
#define PVSSNAME_FSM_CHILDSTATE		"childState"


}; // MCU
}; // LOFAR

#endif
