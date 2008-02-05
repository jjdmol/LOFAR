//#  DigitalBoardControlDefines.h: preprocessor definitions of various constants
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

#ifndef DIGITALBOARDCONTROLDEFINES_H
#define DIGITALBOARDCONTROLDEFINES_H

namespace LOFAR {
  namespace StationCU {

#define PSN_DIG_BOARD_CTRL	"LOFAR_PermSW_DigBoardCtrl@instance@"
#define PST_DIG_BOARD_CTRL	"DigBoardCtrl"
#define PN_DBC_CONNECTED	"connected"
#define PN_DBC_CLOCK		"clock"

// next three line should be defined elsewhere because we are not the owner.
#define	PSN_STATION_CLOCK	"LOFAR_PIC_StationClock"
#define PST_STATION_CLOCK	"StationClock"
#define PN_SC_CLOCK			"clock"

// next lines should be defined somewhere in Common.
#define PVSSNAME_FSM_CURACT			"currentAction"
#define PVSSNAME_FSM_ERROR			"error"
#define PVSSNAME_FSM_LOGMSG			"logMsg"
#define PVSSNAME_FSM_STATE			"state"
#define PVSSNAME_FSM_CHILDSTATE		"childState"


}; // StationCU
}; // LOFAR

#endif
