//#  BeamControlDefines.h: preprocessor definitions of various constants
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

#ifndef BEAMCONTROLDEFINES_H
#define BEAMCONTROLDEFINES_H

namespace LOFAR {
  namespace StationCU {

#define BC_TASKNAME					"BeamCtrl"

#define PSN_BEAM_CTRL   			"LOFAR_ObsSW_@observation@_BeamCtrl"
#define PST_BEAM_CTRL   			"BeamCtrl"
#define PN_BC_CONNECTED 			"connected"
#define PN_BC_SUBBANDLIST 			"subbandList"
#define PN_BC_BEAMLETLIST 			"beamletList"
#define PN_BC_SUBARRAY 				"subArray"
#define PN_BC_ANGLE1 				"angle1"
#define PN_BC_ANGLE2	 			"angle2"
#define PN_BC_ANGLETIMES 			"angleTimes"
#define PN_BC_BEAMID 				"beamID"

// next lines should be defined somewhere in Common.
#define PVSSNAME_FSM_CURACT			"currentAction"
#define PVSSNAME_FSM_ERROR			"error"
#define PVSSNAME_FSM_LOGMSG			"logMsg"
#define PVSSNAME_FSM_STATE			"state"
#define PVSSNAME_FSM_CHILDSTATE		"childState"

}; // MCU
}; // LOFAR

#endif
