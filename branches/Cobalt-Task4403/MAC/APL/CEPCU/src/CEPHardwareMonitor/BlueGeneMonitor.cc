//#  BlueGeneMonitor.cc: Monitors if the BGP hardware is available
//#
//#  Copyright (C) 2011
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
//#  $Id: BlueGeneMonitor.cc 10505 2007-09-07 17:14:57Z overeem $
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarConstants.h>
#include <Common/LofarLocators.h>
#include <Common/StringUtil.h>
#include <Common/ParameterSet.h>

#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/PVSS/PVSSinfo.h>
#include <MACIO/MACServiceInfo.h>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/RTDBCommon/RTDButilities.h>
#include <GCF/RTDB/DP_Protocol.ph>
//#include <APL/APLCommon/StationInfo.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>	// usleep
#include <netdb.h>	// gethostbyname

#include "BlueGeneMonitor.h"
#include "PVSSDatapointDefs.h"

#define	IONODES_PER_BGP_PARTITION	64
#define MAX2(a,b)	((a) > (b)) ? (a) : (b)

namespace LOFAR {
	using namespace APLCommon;
	using namespace APL::RTDBCommon;
	using namespace GCF::TM;
	using namespace GCF::PVSS;
	using namespace GCF::RTDB;
	namespace CEPCU {
	
//
// BlueGeneMonitor()
//
BlueGeneMonitor::BlueGeneMonitor(const string&	cntlrName) :
	GCFTask 			((State)&BlueGeneMonitor::initial_state,cntlrName),
	itsOwnPropertySet	(0),
	itsTimerPort		(0),
	itsDPservice		(0),
	itsPollInterval		(60),
	itsLastBGPState		(-1)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");

	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "BGPTimerPort");

	itsDPservice = new DPservice(this);
	ASSERTSTR(itsDPservice, "Can't allocate DPservice");

	itsBlueGeneFrontEnd = globalParameterSet()->getString("BlueGeneFrontEnd", "");
	ASSERTSTR(!itsBlueGeneFrontEnd.empty(), "Name of BlueGene FrontEnd node not specified");

	registerProtocol(DP_PROTOCOL, DP_PROTOCOL_STRINGS);
}


//
// ~BlueGeneMonitor()
//
BlueGeneMonitor::~BlueGeneMonitor()
{
	LOG_TRACE_OBJ_STR (getName() << " destruction");

	if (itsDPservice)	delete itsDPservice;

	if (itsTimerPort)	delete itsTimerPort;
}


//
// initial_state(event, port)
//
// Setup connection with PVSS
//
GCFEvent::TResult BlueGeneMonitor::initial_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("initial:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
    case F_INIT:
   		break;

	case F_ENTRY: {
		// Get access to my own propertyset.
		LOG_DEBUG_STR ("Activating PropertySet " << PSN_CEP_HARDWARE_MONITOR);
		itsTimerPort->setTimer(2.0);
		itsOwnPropertySet = new RTDBPropertySet(PSN_CEP_HARDWARE_MONITOR,
												PST_CEP_HARDWARE_MONITOR,
												PSAT_WO,
												this);

		}
		break;

	case DP_CREATED: {
		// NOTE: this function may be called DURING the construction of the PropertySet.
		// Always exit this event in a way that GCF can end the construction.
		DPCreatedEvent		dpEvent(event);
		LOG_DEBUG_STR("Result of creating " << dpEvent.DPname << " = " << dpEvent.result);
		itsTimerPort->cancelAllTimers();
		itsTimerPort->setTimer(0.0);
		}
		break;

	case F_TIMER: {
		// PropertySet must exist by now
		ASSERTSTR(itsOwnPropertySet, "Could not create the PVSS datapoint " << PSN_CEP_HARDWARE_MONITOR);

		// update PVSS.
		LOG_TRACE_FLOW ("Updateing state to PVSS");
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("BlueGene:initialising"));
		itsOwnPropertySet->setValue(PN_CHM_BGP_CONNECTED,  GCFPVBool(false));

		// is name resolvable?
		struct hostent*		hostinfo = gethostbyname(itsBlueGeneFrontEnd.c_str());
		if (!hostinfo) {
			itsOwnPropertySet->setValue(PN_FSM_ERROR,  GCFPVString("Name of BG frontend node is unresolvable"));
			TRAN(BlueGeneMonitor::finish_state);		// go to final state.
		}
		itsOwnPropertySet->setValue(PN_FSM_ERROR,  GCFPVString(""));
		TRAN(BlueGeneMonitor::getBlueGeneState);		// do inital check
	}
	break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (BlueGeneMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("initial, DEFAULT: " << eventName(event));
		break;
	}    

	return (GCFEvent::HANDLED);
}



//
// getBlueGeneState(event, port)
//
// Ask the information of the BlueGene
//
GCFEvent::TResult BlueGeneMonitor::getBlueGeneState(GCFEvent& event, 
													GCFPortInterface& port)
{
	if (eventName(event) != "DP_SET") {
		LOG_DEBUG_STR ("getBlueGeneState:" << eventName(event) << "@" << port.getName());
	}

	switch (event.signal) {
	case F_ENTRY: {
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("BlueGene:requesting BlueGene info"));
		itsTimerPort->setTimer(15.0);		// in case the answer never comes

		string	command(formatString("ssh %s 'bgpartstatus R00' 2>&1", itsBlueGeneFrontEnd.c_str()));
		FILE*	pipe(popen(command.c_str(), "r"));
		char	line[1024];
		line[0] = '\0';
		if (!pipe || !fgets (line, sizeof (line), pipe)) {
			LOG_ERROR_STR ("BlueGene:Unable to read pipe: " << command);
			itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("BlueGene:pipe failure"));
			if (pipe) {
				LOG_ERROR_STR("Pipe error: " << strerror(errno));
				fclose(pipe);
			}
			TRAN(BlueGeneMonitor::waitForNextCycle);				// go to next state.
			break;
		}
		fclose(pipe);

		// possible answers:
		// free         - partition is available.
        // initializing - partition is booting.
        // rebooting    - partition is rebooting.
        // busy         - partition is running a job.
		// unavailable  - partition is partly used by other processes
        // deallocating - partition is cleaning up.
        // error        - partition is in error state
		bool	inError(false);
		if (!strcmp(line, "error")) {
			LOG_ERROR_STR ("BlueGene:Partition R00 in error state: " << line);
			itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("BlueGene:unknown partitionstate"));
			inError = true;
		}

		int		newState(inError ? RTDB_OBJ_STATE_BROKEN : RTDB_OBJ_STATE_OPERATIONAL);
		if (newState != itsLastBGPState) {
			string	pvssDBname(PVSSinfo::getLocalSystemName());
			for (int i = 0;  i < IONODES_PER_BGP_PARTITION; i++) {
				LOG_INFO_STR("setObjectState(" << getName() << "," <<  formatString("%s:%s", pvssDBname.c_str(), _IOnodeName(i).c_str()) << "," <<  newState << ")");
				setObjectState(getName(), formatString("%s:%s", pvssDBname.c_str(), _IOnodeName(i).c_str()), newState);
			}
		}
		itsLastBGPState = newState;
		TRAN(BlueGeneMonitor::waitForNextCycle);				// go to next state.
		break;
	}

	case DP_SET:
	case F_EXIT:
		break;

	case F_QUIT:
		TRAN (BlueGeneMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("getBlueGeneState, DEFAULT: " << eventName(event));
		break;
	}    

	return (GCFEvent::HANDLED);
}

//
// IOnodeName(nodeNr)
//
string BlueGeneMonitor::_IOnodeName(int	nodeNr)
{
	string	IONodeMask(createPropertySetName(PSN_IO_NODE,""));
	return (formatString(IONodeMask.c_str(), nodeNr/32, nodeNr));
}

//
// waitForNextCycle(event, port)
//
// Wait for our next cycle.
//
GCFEvent::TResult BlueGeneMonitor::waitForNextCycle(GCFEvent& event, 
													GCFPortInterface& port)
{
	if (eventName(event) != "DP_SET") {
		LOG_DEBUG_STR ("waitForNextCycle:" << eventName(event) << "@" << port.getName());	
	}

	switch (event.signal) {
	case F_ENTRY: {
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("BlueGene:wait for next cycle"));
		int		waitTime = itsPollInterval - (time(0) % itsPollInterval);
		if (waitTime == 0) {
			waitTime = itsPollInterval;
		}
		itsTimerPort->cancelAllTimers();
		itsTimerPort->setTimer(double(waitTime));
		LOG_INFO_STR("BlueGene:Waiting " << waitTime << " seconds for next cycle");
	}
	break;

	case F_TIMER: {
		itsOwnPropertySet->setValue(string(PN_FSM_ERROR),GCFPVString(""));
		TRAN(BlueGeneMonitor::getBlueGeneState);
	}
	break;

	case DP_SET:
	case F_EXIT:
		break;

	case F_QUIT:
		TRAN (BlueGeneMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("waitForNextCycle, DEFAULT: " << eventName(event));
		break;
	}    

	return (GCFEvent::HANDLED);
}


//
// finish_state(event, port)
//
// Write controller state to PVSS
//
GCFEvent::TResult BlueGeneMonitor::finish_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("finish_state:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		// update PVSS
		itsOwnPropertySet->setValue(string(PN_FSM_CURRENT_ACTION),GCFPVString("BlueGene:finished"));
		break;
	}
  
	case DP_SET:
	case F_EXIT:
		break;

	default:
		LOG_DEBUG("finishing_state, DEFAULT");
		break;
	}    
	return (GCFEvent::HANDLED);
}


}; // CEPCU
}; // LOFAR
