//#  ObsClaimer.cc: Implementation of the MAC Scheduler task
//#
//#  Copyright (C) 2002-2004
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
#include <Common/SystemUtil.h>
#include <Common/StreamUtil.h>
#include <Common/Version.h>
#include <Common/ParameterSet.h>
#include <ApplCommon/StationConfig.h>
#include <ApplCommon/StationInfo.h>

#include <GCF/TM/GCF_Protocols.h>
#include <GCF/TM/GCF_PortInterface.h>
#include <MACIO/MACServiceInfo.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/ControllerDefines.h>
#include <GCF/RTDB/DP_Protocol.ph>
#include <APL/RTDBCommon/CM_Protocol.ph>
#include <APL/RTDBCommon/ClaimMgrTask.h>
#include <signal.h>

#include "ObsClaimer.h"
#include "PVSSDatapointDefs.h"
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace LOFAR::GCF::PVSS;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::RTDB;
using namespace LOFAR::APL::RTDBCommon;
using namespace boost::posix_time;
using namespace std;

namespace LOFAR {
	using namespace APLCommon;
	namespace MainCU {

//
// ObsClaimer()
//
ObsClaimer::ObsClaimer(GCFTask*		mainTask) :
	GCFTask 			((State)&ObsClaimer::idle_state, string("MS-ObsClaimer")),
	itsClaimMgrTask	(0),
	itsITCPort		(0),
   	itsHeartBeat	(0),
   	itsTimerPort	(0)
{
	LOG_TRACE_OBJ ("ObsClaimer construction");

	// construct the resources we need
	itsTimerPort = new GCFTimerPort(*this, "OStimerport");
	ASSERTSTR(itsTimerPort, "Can't construct a timer");

	itsHeartBeat = new GCFTimerPort(*this, "OSHeartBeat");
	ASSERTSTR(itsHeartBeat, "Can't construct a heatbeat timer");

	itsITCPort = new GCFITCPort(*mainTask, *this, "ITCPort", GCFPortInterface::SAP, CM_PROTOCOL);
	ASSERTSTR(itsITCPort, "Can't construct an ITC port");

	itsClaimMgrTask = ClaimMgrTask::instance();
	ASSERTSTR(itsClaimMgrTask, "Can't construct a claimMgrTask");

	registerProtocol(CM_PROTOCOL, CM_PROTOCOL_STRINGS);
}


//
// ~ObsClaimer()
//
ObsClaimer::~ObsClaimer()
{
	LOG_TRACE_OBJ ("~ObsClaimer");

	if (itsTimerPort) { delete itsTimerPort; }
	if (itsHeartBeat) { delete itsHeartBeat; }
	if (itsITCPort)   { delete itsITCPort; }

}

// -------------------- The only public function --------------------
//
// prepareObservation(const string&		observationName);
//
// Just add the observationname to our prepareList and trigger main-loop.
void ObsClaimer::prepareObservation(const string&		observationName)
{
	OMiter	iter = itsObsMap.find(observationName);
	if (iter == itsObsMap.end()) {	// new?
		obsInfo*	newObs = new obsInfo();
		newObs->obsName = observationName;
		newObs->state   = OS_NEW;
//		newObs->observation;
		itsObsMap["LOFAR_ObsSW_"+observationName] = newObs;
		LOG_DEBUG_STR("Added observation " << observationName << " to the prepareList");
	}	
	else {
		LOG_DEBUG_STR("Observation " << observationName << " already in the prepareList with state " << iter->second->state);
	}

	// Wake up state-machine asap.
	itsHeartBeat->cancelAllTimers();
	itsHeartBeat->setTimer(0.0);
}



//
// idle_state(event, port)
//
// Wait for new actions.
//
GCFEvent::TResult ObsClaimer::idle_state (GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("idle_state:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY:
		itsHeartBeat->setTimer(0.0);	// check if there is something to do.
		break;

	case F_TIMER: {
			// search the (first) observation that needs to be started; ask the claimMgr the name
			// of the real DP.
			OMiter		iter = itsObsMap.begin();
			OMiter		end  = itsObsMap.end();
			while (iter != end) {
				if (iter->second->state == OS_NEW) {
					iter->second->state = OS_CLAIMING;
					itsClaimMgrTask->claimObject("Observation", "LOFAR_ObsSW_"+iter->second->obsName, *itsITCPort);
					// will result in CM_CLAIM_RESULT event
					break;	// claim one at the time.
				}
				++iter;
			}
		}
		break;

	case CM_CLAIM_RESULT: {
			CMClaimResultEvent	cmEvent(event);
			LOG_INFO_STR(cmEvent.nameInAppl << " is mapped to " << cmEvent.DPname);
			OMiter		iter = itsObsMap.find(cmEvent.nameInAppl);
//			ASSERTSTR(iter != itsObsMap.end(), "Cannot find " << cmEvent.nameInAppl << " in admin");
// 			sometimes we receive a ghost message so we can't assert on it. Using an IF for the time being.
			if (iter == itsObsMap.end()) {
				LOG_ERROR_STR("Cannot find " << cmEvent.nameInAppl << " in admin");
				TRAN(ObsClaimer::idle_state);
			}
			else {
				iter->second->DPname = cmEvent.DPname;
				itsCurrentObs = iter;
				TRAN(ObsClaimer::preparePVSS_state);
			}
		}
		break;
	
	default:
		LOG_DEBUG ("ObsClaimer::initial, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}

//
// preparePVSS_state(event, port)
//
// Fill all fields of the Observation in PVSS.
//
GCFEvent::TResult ObsClaimer::preparePVSS_state (GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("preparePVSS_state:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
    case F_ENTRY: {
		// Create a PropSet for the Observation
		LOG_DEBUG_STR ("Connecting to DP(" << itsCurrentObs->second->DPname << ") from observation " 
						<< itsCurrentObs->second->obsName);
		itsCurrentObs->second->state = OS_FILLING;
		itsCurrentObs->second->propSet = new RTDBPropertySet(itsCurrentObs->second->DPname,
											 "Observation",
											 PSAT_RW,
											 this);
		}
		break;

	case DP_CREATED: {
			// NOTE: thsi function may be called DURING the construction of the PropertySet.
			// Always exit this event in a way that GCF can end the construction.
			DPCreatedEvent	dpEvent(event);
			LOG_DEBUG_STR("Result of creating " << dpEvent.DPname << " = " << dpEvent.result);
//			itsTimerPort->cancelAllTimers();
			itsTimerPort->setTimer(0.0);
        }
		break;
	  
	case F_TIMER: {		// must be timer that PropSet is enabled.
			// update PVSS.
			LOG_TRACE_FLOW ("Updateing observation-fields in PVSS");
			string				obsPSFilename(formatString("%s/%s", LOFAR_SHARE_LOCATION, 
															itsCurrentObs->second->obsName.c_str()));
			ParameterSet		obsPS(obsPSFilename);
			Observation			theObs(&obsPS); 
			RTDBPropertySet*	theObsPS = itsCurrentObs->second->propSet;
//			theObsPS->setValue(PN_OBS_CLAIM_PERIOD,		GCFPVInteger(itsClaimPeriod), 0.0, false);
//			theObsPS->setValue(PN_OBS_PREPARE_PERIOD,	GCFPVInteger(itsPreparePeriod), 0.0, false);
			theObsPS->setValue(PN_OBS_START_TIME,		GCFPVString (to_simple_string(from_time_t(theObs.startTime))), 0.0, false);
			theObsPS->setValue(PN_OBS_STOP_TIME,		GCFPVString (to_simple_string(from_time_t(theObs.stopTime))), 0.0, false);
			theObsPS->setValue(PN_OBS_BAND_FILTER, 		GCFPVString (theObs.filter), 		  0.0, false);
			theObsPS->setValue(PN_OBS_NYQUISTZONE, 		GCFPVInteger(theObs.nyquistZone), 	  0.0, false);
			theObsPS->setValue(PN_OBS_ANTENNA_ARRAY,	GCFPVString (theObs.antennaArray), 	  0.0, false);
			theObsPS->setValue(PN_OBS_RECEIVER_LIST, 	GCFPVString (theObs.receiverList), 	  0.0, false);
			theObsPS->setValue(PN_OBS_SAMPLE_CLOCK, 	GCFPVInteger(theObs.sampleClock), 	  0.0, false);
			theObsPS->setValue(PN_OBS_MEASUREMENT_SET, 	GCFPVString (theObs.MSNameMask), 	  0.0, false);
			theObsPS->setValue(PN_OBS_STATION_LIST, 	GCFPVString (theObs.stationList), 	  0.0, false);
			theObsPS->setValue(PN_OBS_BGL_NODE_LIST, 	GCFPVString (theObs.BGLNodeList), 	  0.0, false);
			theObsPS->setValue(PN_OBS_STORAGE_NODE_LIST,GCFPVString (theObs.storageNodeList), 0.0, false);

#if defined(THERE_IS_NO_REMOTE_STATION_CONFIG_ON_MCU001)
			// the receiver bitmap can be derived from the RCUset.
			StationConfig		config;
			bitset<MAX_RCUS>	theRCUs(theObs.getRCUbitset(config.nrLBAs, config.nrHBAs, config.nrRSPs, config.hasSplitters));
#endif
#if 0
			// TODO: this code should be moved to the StationController
			bitset<MAX_RCUS>	theRCUs(theObs.getRCUbitset(96, 48, 12, true));
			string	rbm;
			rbm.resize(MAX_RCUS, '0');
			for (int i = 0; i < MAX_RCUS; i++) {
				if (theRCUs[i]) {
					rbm[i] = '1';
				}
			}
			theObsPS->setValue(PN_OBS_RECEIVER_BITMAP,GCFPVString (rbm), 0.0, false);
#endif
			// for the beams we have to construct dyn arrays first.
			GCFPValueArray		subbandArr;
			GCFPValueArray		beamletArr;
			GCFPValueArray		angle1Arr;
			GCFPValueArray		angle2Arr;
			GCFPValueArray		dirTypesArr;
			for (uint32	i(0); i < theObs.beams.size(); i++) {
				stringstream		os;
				writeVector(os, theObs.beams[i].subbands);
				subbandArr.push_back  (new GCFPVString(os.str()));
				os.clear();
				writeVector(os, theObs.beams[i].beamlets);
				beamletArr.push_back  (new GCFPVString(os.str()));
				angle1Arr.push_back	  (new GCFPVDouble(theObs.beams[i].pointings[0].angle1));
				angle2Arr.push_back	  (new GCFPVDouble(theObs.beams[i].pointings[0].angle2));
				dirTypesArr.push_back (new GCFPVString(theObs.beams[i].pointings[0].directionType));
			}

			// Finally we can write those value to PVSS as well.
			theObsPS->setValue(PN_OBS_BEAMS_SUBBAND_LIST,	GCFPVDynArr(LPT_DYNSTRING, subbandArr),  0.0, false);
			theObsPS->setValue(PN_OBS_BEAMS_BEAMLET_LIST,	GCFPVDynArr(LPT_DYNSTRING, beamletArr),  0.0, false);
			theObsPS->setValue(PN_OBS_BEAMS_ANGLE1,			GCFPVDynArr(LPT_DYNDOUBLE, angle1Arr),   0.0, false);
			theObsPS->setValue(PN_OBS_BEAMS_ANGLE2,			GCFPVDynArr(LPT_DYNDOUBLE, angle2Arr),   0.0, false);
			theObsPS->setValue(PN_OBS_BEAMS_DIRECTION_TYPE,	GCFPVDynArr(LPT_DYNSTRING, dirTypesArr), 0.0, false);
			theObsPS->flush();

			// append DPname to the ParameterFile
			obsPS.add("_DPname", itsCurrentObs->second->DPname);
			obsPS.writeFile(obsPSFilename);

			// send Maintask a signal we are ready.
			LOG_DEBUG_STR("Sending Maintask ready signal for " << itsCurrentObs->second->obsName);
			CMClaimResultEvent	cmEvent;
			cmEvent.nameInAppl = itsCurrentObs->second->obsName;
			cmEvent.DPname	   = itsCurrentObs->second->DPname;
			itsITCPort->sendBack(cmEvent);

			// remove observation from list
			LOG_DEBUG_STR("Removing " << itsCurrentObs->second->obsName << " from my prepareList");
			delete itsCurrentObs->second;
			itsObsMap.erase(itsCurrentObs);
			itsCurrentObs = itsObsMap.end();	// reset iterator.

			// back to idle state.
			TRAN(ObsClaimer::idle_state);
		}
		break;
  
	default:
		LOG_DEBUG("updateObsInPVSS, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    

	return (status);
}


  }; // namespace MAINCU
}; // namespace LOFAR
