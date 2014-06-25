//
//  disabledRCUlist.cc : Small utility to allow scripts to set an objectstate.
//
//  Copyright (C) 2011
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id: tDPservice.cc 10538 2007-10-03 15:04:43Z overeem $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/Exception.h>
#include <Common/lofar_fstream.h>
#include <Common/SystemUtil.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/RTDB/DPservice.h>
#include <GCF/RTDB/DP_Protocol.ph>
#include <APL/RTDBCommon/RTDButilities.h>
#include "disabledRCUlist.h"

string	gResult;

namespace LOFAR {
  using namespace GCF;
  using namespace GCF::TM;
  using namespace GCF::PVSS;
  using namespace RTDB;
  namespace APL {
    namespace RTDBCommon {


mainTask::mainTask(const string& name, int	rcuMode) : 
	GCFTask((State)&mainTask::takeSubscription, name),
	itsTimerPort(0),
	itsDPservice(0),
	itsRCUmode(rcuMode)
{
	LOG_DEBUG_STR("mainTask(" << name << ")");

	itsTimerPort = new GCFTimerPort(*this, "timerPort");
	ASSERTSTR(itsTimerPort, "Can't allocate GCFTimerPort");

	itsDPservice = new DPservice(this);
	ASSERTSTR(itsDPservice, "Can't allocate DPservice");

	itsRCUmask.reset();

	registerProtocol(F_FSM_PROTOCOL, F_FSM_PROTOCOL_STRINGS);
	registerProtocol(DP_PROTOCOL, DP_PROTOCOL_STRINGS);
}

//
// destructor
//
mainTask::~mainTask()
{
	if (itsDPservice) {
		delete itsDPservice;
	}
	if (itsTimerPort) {
		delete itsTimerPort;
	}
}

//
// takeSubscription(event, port)
//
// Take a subscribtion to the states of the hardware so we can construct correct
// masks that reflect the availability of the LBA and HBA antenna's.
//
GCFEvent::TResult mainTask::takeSubscription(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("takeSubscription:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// take subscribtion on *.state
		LOG_DEBUG("Taking subscription on all state fields");
		PVSSresult  result = itsDPservice->query("'LOFAR_PIC_*.status.state'", "");
		if (result != SA_NO_ERROR) {
			LOG_ERROR ("Taking subscription on PVSS-states failed, returning error");
			gResult="DatabaseError";
			GCFScheduler::instance()->stop();
			break;
		}
		// wait for DP event
		LOG_DEBUG ("Waiting for subscription answer");
		itsTimerPort->setTimer(3.0);
	} break;

	case F_TIMER:
		gResult = "TimeoutError";
		GCFScheduler::instance()->stop();
		break;

	case DP_QUERY_CHANGED: {
		// don't expect this event here right now, but you never know.
 		// in case DP_QUERY_SUBSCRIBED is skipped.
		itsTimerPort->cancelAllTimers();
		_handleQueryEvent(event);
		gResult="";
		for (uint a = 0; a < itsRCUmask.size(); a++) {
			if (itsRCUmask[a]) {
				if (gResult == "") {
					gResult.append(formatString("%d",a));
				}
				else {
					gResult.append(formatString(",%d",a));
				}
			}
		}
		GCFScheduler::instance()->stop();
	} break;
	
	default:
		status = GCFEvent::NOT_HANDLED;
	break;
	}

	return (status);
}
//
// _handleQueryEvent(event)
//
void mainTask::_handleQueryEvent(GCFEvent& event)
{
	LOG_TRACE_FLOW_STR ("_handleQueryEvent:" << eventName(event));
	
	// Check for errors
	DPQueryChangedEvent		DPevent(event);
	if (DPevent.result != SA_NO_ERROR) {
		LOG_ERROR_STR("PVSS reported error " << DPevent.result << " for a query " << 
				", cannot determine actual state of the hardware! Assuming everything is available.");
		gResult="PVSS_Error";
		return;
	}

	// The selected datapoints are delivered with full PVSS names, like:
	// CS001:LOFAR_PIC_Cabinet0_Subrack0_RSPBoard0_RCU5.status.state
	// CS001:LOFAR_PIC_LBA000.status.state
	// Each event may contain more than one DP.
	int     nrDPs = ((GCFPVDynArr*)(DPevent.DPnames._pValue))->getValue().size();
	GCFPVDynArr*    DPnames  = (GCFPVDynArr*)(DPevent.DPnames._pValue);
	GCFPVDynArr*    DPvalues = (GCFPVDynArr*)(DPevent.DPvalues._pValue);
	// GCFPVDynArr*    DPtimes  = (GCFPVDynArr*)(DPevent.DPtimes._pValue);

	uint32	modeOff(objectStateIndex2Value(RTDB_OBJ_STATE_OFF));
	uint32	modeOperational(objectStateIndex2Value(RTDB_OBJ_STATE_OPERATIONAL));
	string	sysID(toUpper(myHostname(false).substr(0,2)));
	bool	dutchStn(sysID == "CS" || sysID == "RS");
	for (int    idx = 0; idx < nrDPs; ++idx) {
		// Get the name and figure out what circuitboard we are talking about
		string  nameStr(DPnames->getValue()[idx]->getValueAsString());				// DP name
		uint32	newState(((GCFPVInteger*)(DPvalues->getValue()[idx]))->getValue());	// value
		string::size_type	pos;

		LOG_DEBUG_STR("QryUpdate: DP=" << nameStr << ", value=" << newState);

		// mapping antenna's to RCUs
		//
		//  RcuMode | StationType : Ant | Mapping
		//   1,2    |   Dutch     : LBA | Ant 48-95 --> RCU 0-95
		//   1,2    |   Europe    : ---
		//   3,4    |   Dutch     : LBA | Ant 0-47 --> RCU 0-95
		//   3,4    |   Europe    : LBA | Ant 0-95 --> RCU 0-191
		//   5,6,7  |   Dutch     : HBA | Ant 0-47 --> RCU 0-95
		//   5,6,7  |   Europe    : HBA | Ant 0-95 --> RCU 0-191

		// test for LBA
		if ((itsRCUmode < 5) && (pos = nameStr.find("PIC_LBA")) != string::npos) {
			uint		antNr;
			if (sscanf(nameStr.substr(pos).c_str(), "PIC_LBA%u.status.state", &antNr) != 1) {
				LOG_ERROR_STR("Cannot determine address of " << nameStr << 
								". AVAILABILITY OF ANTENNA'S MIGHT NOT BE UP TO DATE ANYMORE");
				continue;
			}
			// LBA's in de mode OFF and OPERATIONAL are not disabled
			if (newState != modeOff && newState != modeOperational) {
				if ((itsRCUmode == 1 || itsRCUmode == 2) && dutchStn && (antNr >= 48)) {
					itsRCUmask.set((antNr-48)*2);
					itsRCUmask.set((antNr-48)*2 + 1);
					LOG_INFO_STR("LBA 1,2:" << (antNr-48)*2 << ", " << (antNr-48)*2 + 1);
				}
				else if ((itsRCUmode == 3 || itsRCUmode == 4) && (antNr < ((dutchStn ? 1 : 2)*48))) {
					itsRCUmask.set(antNr*2);
					itsRCUmask.set(antNr*2 + 1);
					LOG_INFO_STR("LBA 3,4:" << antNr*2 << ", " << antNr*2 + 1);
				}
			}
		} // PIC_LBA

		// test for HBA
		if ((itsRCUmode >= 5) && (pos = nameStr.find("PIC_HBA")) != string::npos) {
			uint		antNr;
			if (sscanf(nameStr.substr(pos).c_str(), "PIC_HBA%u.status.state", &antNr) != 1) {
				LOG_ERROR_STR("Cannot determine address of " << nameStr << 
								". AVAILABILITY OF ANTENNA'S MIGHT NOT BE UP TO DATE ANYMORE");
				continue;
			}
			// HBA's in de mode OFF and OPERATIONAL are not disabled
			if (newState != modeOff && newState != modeOperational) {
				itsRCUmask.set(antNr*2);
				itsRCUmask.set(antNr*2 + 1);
				LOG_INFO_STR("HBA 567:" << antNr*2 << ", " << antNr*2 + 1);
			}
		} // PIC_HBA

		// test for RCU
		if ((pos = nameStr.find("_RCU")) != string::npos) {
			uint		rcu;
			if (sscanf(nameStr.substr(pos).c_str(), "_RCU%u.status.state", &rcu) != 1) {
				LOG_ERROR_STR("Cannot determine address of " << nameStr << 
								". AVAILABILITY OF ANTENNA'S MIGHT NOT BE UP TO DATE ANYMORE");
				continue;
			}
			// RCU's in de mode OFF and OPERATIONAL are not disabled
			if (newState != modeOff && newState != modeOperational) {
				itsRCUmask.set(rcu);
				LOG_INFO_STR("RCU:" << rcu);
			}
		} // _RCU

		// test for RSPBoard
		else if ((pos = nameStr.find("_RSPBoard")) != string::npos) {
			uint		rsp;
			if (nameStr.find(".status.state") != string::npos) {
				if (sscanf(nameStr.substr(pos).c_str(), "_RSPBoard%u.status.state", &rsp) == 1) {
					LOG_DEBUG_STR("State of RSPBoard " << rsp << " is " << newState);
					int rcubase = rsp * NR_RCUS_PER_RSPBOARD;
					for (int i = 0; i < NR_RCUS_PER_RSPBOARD; i++) {
						if (newState != modeOperational) {
							itsRCUmask.set(rcubase + i);
							LOG_INFO_STR("RSP " << rsp << ",RCU=" << rcubase+i);
						}
					} // for
				}
			}
		} // _RSPBoard
	} // for
}

    } // namepsace RTDBCommon
  } // namspace APL
} // namespace LOFAR

// -------------------- MAIN --------------------
using namespace LOFAR;
using namespace LOFAR::GCF;
using namespace LOFAR::APL::RTDBCommon;

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler t(Exception::terminate);

int main(int argc, char* argv[])
{
	if (argc != 2) {
		cout << "Syntax: " << argv[0] << " rcumode (1..7)" << endl;
		return (1);
	}

	int	rcuMode = atoi(argv[1]);
	if ((rcuMode < 1) || (rcuMode > 7)) {
		cout << "Value of rcumode should match integer range 1..7, not " << rcuMode << endl;
		return (2);
	}

	TM::GCFScheduler::instance()->init(argc, argv);

	mainTask getInfo("disabledRCUlistTask", rcuMode);  
	getInfo.start(); // make initial transition

	TM::GCFScheduler::instance()->run();

	cout << gResult << endl;
	return (0);
}
