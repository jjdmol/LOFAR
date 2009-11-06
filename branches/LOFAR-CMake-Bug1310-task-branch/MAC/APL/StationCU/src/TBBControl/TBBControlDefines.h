//#  TBBControlDefines.h: preprocessor definitions of various constants
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
//#  $Id: TBBControlDefines.cc,v 1.1 2007/11/03 16:29:07 overeem Exp $

#ifndef TBBCONTROLDEFINES_H
#define TBBCONTROLDEFINES_H

namespace LOFAR {
  namespace StationCU {

#define TBC_TASKNAME		"TBBCtrl"

// TBBCtrl
#define PSN_TBB_CTRL	"LOFAR_ObsSW_@observation@_TBBCtrl"
#define PST_TBB_CTRL	"TBBCtrl"
#define PN_TBC_CONNECTED	"connected"
#define PN_TBC_TRIGGER_RCU_NR	"trigger.rcuNr"
#define PN_TBC_TRIGGER_SEQUENCE_NR	"trigger.sequenceNr"
#define PN_TBC_TRIGGER_TIME	"trigger.time"
#define PN_TBC_TRIGGER_SAMPLE_NR	"trigger.sampleNr"
#define PN_TBC_TRIGGER_SUM	"trigger.sum"
#define PN_TBC_TRIGGER_NR_SAMPLES	"trigger.nrSamples"
#define PN_TBC_TRIGGER_PEAK_VALUE	"trigger.peakValue"
#define PN_TBC_TRIGGER_FLAGS	"trigger.flags"
#define PN_TBC_TRIGGER_TABLE	"trigger.table"

// next lines should be defined somewhere in Common.
#define PVSSNAME_FSM_CURACT			"currentAction"
#define PVSSNAME_FSM_ERROR			"error"
#define PVSSNAME_FSM_LOGMSG			"logMsg"
#define PVSSNAME_FSM_STATE			"state"
#define PVSSNAME_FSM_CHILDSTATE		"childState"

}; // StationCU
}; // LOFAR

#endif
