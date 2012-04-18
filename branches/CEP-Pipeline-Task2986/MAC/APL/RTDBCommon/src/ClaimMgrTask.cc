//#  ClaimMgrTask.cc: Task for communicating with the ClaimManager in PVSS.
//#
//#  Copyright (C) 2008
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
#include <GCF/PVSS/GCF_PVString.h>
#include <GCF/RTDB/DP_Protocol.ph>

#include <APL/RTDBCommon/ClaimMgrTask.h>
#include "CM_Protocol.ph"

namespace LOFAR {
  using namespace GCF::TM;
  using namespace GCF::PVSS;
  using namespace GCF::RTDB;
  namespace APL {
    namespace RTDBCommon {

//
// Initialize static elements
CMHandler* CMHandler::itsInstance = 0;

//
// CMHandler()
//
CMHandler::CMHandler()
{
}

// --------------- Construction and destruction -----------
//
// ClaimMgrTask()
//
ClaimMgrTask::ClaimMgrTask() :
	GCFTask((State)&ClaimMgrTask::operational, "ClaimManager"),
	itsReplyPort	(0),
	itsTimerPort	(new GCFTimerPort(*this, "CMtimerPort")),
	itsClaimMgrPS	(0),
	itsResolveState(RO_UNDEFINED)
{
	LOG_DEBUG_STR("ClaimMgrTask()");
	registerProtocol(DP_PROTOCOL, DP_PROTOCOL_STRINGS);
}

//
// ~ClaimMgrTask()
//
ClaimMgrTask::~ClaimMgrTask()
{
}

// -------------------- static functions --------------------
//
// [static] instance(temp)
//
ClaimMgrTask* ClaimMgrTask::instance(bool temporary)
{
	// if there is not taskHandler yet start one up.
	if (!CMHandler::itsInstance) {    
		CMHandler::itsInstance = new CMHandler();
		// note: the CMHandler constructor also created an instance of theCMTask.
		ASSERT(!CMHandler::itsInstance->mayDeleted());
		CMHandler::itsInstance->itsCMTask.start();
	}

	if (!temporary) { 
		CMHandler::itsInstance->use();
	}

	return (&CMHandler::itsInstance->itsCMTask);
}

//
// [static] release()
//
void ClaimMgrTask::release()
{
	ASSERT(CMHandler::itsInstance);
	ASSERT(!CMHandler::itsInstance->mayDeleted());
	// disconnect handler from the GCF engine.
	CMHandler::itsInstance->leave(); 
	if (CMHandler::itsInstance->mayDeleted()) {
		delete CMHandler::itsInstance;
		ASSERT(!CMHandler::itsInstance);
	}
}

// -------------------- USER FUNCTIONS --------------------
//
// claimObject(objectype, nameInAppl)
//
void ClaimMgrTask::claimObject(const string&		objectType,
							   const string&		nameInAppl,
							   GCFPortInterface&	replyPort)	// ???
{
	// not yet started yet? (user called us directly after creation)
	if (itsResolveState==RO_UNDEFINED) {
		itsRequestPool.push(cmRequest_t(objectType,nameInAppl,&replyPort));
		LOG_INFO_STR("claimrequest '" << nameInAppl << "' queued because manager is still starting up");
		return;
	}

	// are we in an idle state?
	if (!itsObjectType.empty() || !itsNameInAppl.empty()) {
		itsRequestPool.push(cmRequest_t(objectType,nameInAppl,&replyPort));
		LOG_INFO_STR("claimrequest '" << nameInAppl << "' queued because manager is still busy with " << itsNameInAppl);
		return;
	}

	// save info
	itsObjectType = objectType;
	itsNameInAppl = nameInAppl;
	itsReplyPort  = &replyPort;
	LOG_INFO_STR("claimrequest '" << nameInAppl << "' will be handled immediately");
	if (itsResolveState == RO_READY) {
		itsTimerPort->setTimer(0.1);	// wake up FSM
	}
	// else: some other timer must be active.
}


// -------------------- INTERNAL FUNCTIONS --------------------


//
// operational(event, port)
//
GCFEvent::TResult ClaimMgrTask::operational(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("ClaimMgrTask:" << eventName(event) << "@" << port.getName() << ", itsResolveState=" << itsResolveState);
	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY: 
		itsResolveState = RO_UNDEFINED;
	break;

	case F_INIT: {
		LOG_DEBUG("Create propertySet for accessing the ClaimManager");
		itsResolveState = RO_CREATING;	// 1
		itsClaimMgrPS = new RTDBPropertySet("ClaimManager", "ClaimManager", PSAT_RW, this);
		itsTimerPort->setTimer(5.0);
	}
	break;

	case DP_CREATED: {
		DPCreatedEvent	dpEvent(event);
		LOG_DEBUG_STR("Result of creating '" << dpEvent.DPname << "' = " << dpEvent.result);
		itsResolveState = RO_CREATED; // 2
		itsTimerPort->cancelAllTimers();
		itsTimerPort->setTimer(0.5); // give RTDB time to get the original value.
	}
	break;
	
	// Note: we finish each resolve state with a timer setting so each state always ends here.
	//       This way we only have to implement 1 statemachine in stead of 7.
	case F_TIMER: {
		LOG_DEBUG_STR("itsResolveState=" << itsResolveState);

		switch (itsResolveState) {
		case RO_CREATED:		// 2
			itsResolveState = RO_READY;
			itsTimerPort->cancelAllTimers();
			itsTimerPort->setTimer(0.1);
			break;

		case RO_READY:			// 4
			if (itsObjectType.empty() || itsNameInAppl.empty()) {
				if (itsRequestPool.empty()) {
					LOG_DEBUG_STR("Nothing to claim");
					break;
				}
				// continue with requests on the stack
				cmRequest_t		newRequest = itsRequestPool.front();
				itsObjectType = newRequest.objectType;
				itsNameInAppl = newRequest.objectName;
				itsReplyPort  = newRequest.replyPort;
				itsRequestPool.pop();
			}
			// request a DPname
			LOG_INFO_STR("ClaimObject(" << itsObjectType << "," << itsNameInAppl << ")");
			itsClaimMgrPS->setValue("request.typeName",       GCFPVString(itsObjectType), 0.0, false);
			itsClaimMgrPS->setValue("request.newObjectName",  GCFPVString(itsNameInAppl), 0.0, false);
			// clear the answer also otherwise we will not be notified when asking the same question twice.
			itsClaimMgrPS->setValue("response.DPName",        GCFPVString(""), 0.0, false);
			itsClaimMgrPS->setValue("response.newObjectName", GCFPVString(""), 0.0, false);
			itsClaimMgrPS->flush();
			itsResolveState = RO_ASKED; // 3
			// clear result fields
			itsFieldsReceived = 0;
			itsResultDPname.clear();
			itsTimerPort->cancelAllTimers();
			itsTimerPort->setTimer(3.0);		// don't wait forever.
		break;

		case RO_ASKED:		// 3
			LOG_ERROR_STR("No response from ClaimManager in 3 seconds, for DP:" << itsNameInAppl);
			// Report claimfailure back to the user
			CMClaimResultEvent	cmEvent;
			cmEvent.typeName	= itsObjectType;
			cmEvent.nameInAppl	= itsNameInAppl;
			cmEvent.DPname		= "";
			itsReplyPort->send(cmEvent);
			// clear admin to receive a new claim request.
			itsObjectType.clear();
			itsNameInAppl.clear();
			itsResultDPname.clear();
			itsResolveState = RO_READY;
			itsTimerPort->cancelAllTimers();
			itsTimerPort->setTimer(0.0);
		break;
		}
	}
	break;

	case DP_CHANGED: {
// NOTE: we are called here for every field!
// CS001:ClaimManager.Request.NewObjectName
// CS001:ClaimManager.Request.TypeName
		if (itsObjectType.empty() || itsNameInAppl.empty()) {		// Data change must be ours
			break;
		}

		DPChangedEvent	dpEvent(event);
		LOG_DEBUG_STR("DP " << dpEvent.DPname << " changed");
		if (dpEvent.DPname.find("response.newObjectName") != string::npos) {
			string	fldContents(((GCFPVString*)(dpEvent.value._pValue))->getValue());
			if (!fldContents.empty()) {
				ASSERTSTR(fldContents == itsNameInAppl, "CM returned answer for request '" 
							<< fldContents <<"' iso " << itsNameInAppl);
				itsFieldsReceived++;
			}
		}
		else if (dpEvent.DPname.find("response.DPName") != string::npos) {
			itsResultDPname = ((GCFPVString*)(dpEvent.value._pValue))->getValue();
			if (!itsResultDPname.empty()) {
				itsFieldsReceived++;
			}
		}
		if (itsFieldsReceived >= 2) {
			LOG_DEBUG_STR("ClaimMgr:" << itsNameInAppl << "=" << itsResultDPname);
			// Report claimresult back to the user
			CMClaimResultEvent	cmEvent;
			cmEvent.typeName	= itsObjectType;
			cmEvent.nameInAppl	= itsNameInAppl;
			cmEvent.DPname		= itsResultDPname;
			itsReplyPort->send(cmEvent);
			// clear admin to receive a new claim request.
			itsObjectType.clear();
			itsNameInAppl.clear();
			itsResultDPname.clear();
			itsResolveState = RO_READY;
			itsTimerPort->cancelAllTimers();
			itsTimerPort->setTimer(0.0);
		}
	}
	break;

	default:
		LOG_DEBUG_STR ("claimManager: default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}

    } // namespace RTDBCommon
  } // namespace APL
} // namespace LOFAR
