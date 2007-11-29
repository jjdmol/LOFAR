//#  ObservationControlDefines.h: preprocessor definitions of various constants
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

#ifndef OBSERVATIONDEFINES_H
#define OBSERVATIONDEFINES_H

namespace LOFAR {
  namespace MCU {

#define OC_TASKNAME					"ObsCtrl"
#define OC_OBSERVATIONSTATE			"observationState"

// ObsCtrl
#define PSN_OBS_CTRL    		"LOFAR_ObsSW_@observation@_ObsCtrl"
#define PST_OBS_CTRL    		"ObsCtrl"
#define PN_OC_CLAIM_PERIOD  	"claimPeriod"
#define PN_OC_PREPARE_PERIOD    "preparePeriod"
#define PN_OC_START_TIME    	"startTime"
#define PN_OC_STOP_TIME	    	"stopTime"
#define PN_OC_SUBBAND_LIST 		"subbandList"
#define PN_OC_BEAMLET_LIST 		"beamletList"
#define PN_OC_BAND_FILTER 		"bandFilter"
#define PN_OC_NYQUISTZONE 		"nyquistzone"
#define PN_OC_ANTENNA_ARRAY 	"antennaArray"
#define PN_OC_RECEIVER_LIST 	"receiverList"
#define PN_OC_SAMPLE_CLOCK 		"sampleClock"
#define PN_OC_MEASUREMENT_SET 	"measurementSet"
#define PN_OC_STATION_LIST 		"stationList"
#define PN_OC_INPUT_NODE_LIST 	"inputNodeList"
#define PN_OC_BGL_NODE_LIST 	"BGLNodeList"
#define PN_OC_STORAGE_NODE_LIST	"storageNodeList"


// next lines should be defined somewhere in Common.
#define PVSSNAME_FSM_CURACT			"currentAction"
#define PVSSNAME_FSM_ERROR			"error"
#define PVSSNAME_FSM_LOGMSG			"logMsg"
#define PVSSNAME_FSM_STATE			"state"
#define PVSSNAME_FSM_CHILDSTATE		"childState"

// Observation
#define PSN_OBSERVATION	"LOFAR_ObsSW_@observation@"
#define PST_OBSERVATION	"Observation"

}; // MCU
}; // LOFAR

#endif
