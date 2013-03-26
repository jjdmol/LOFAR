//#  ClusterMonitor.cc: Monitors if the BGP hardware is available
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
//#  $Id: ClusterMonitor.cc 10505 2007-09-07 17:14:57Z overeem $
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarConstants.h>
#include <Common/LofarLocators.h>
#include <Common/lofar_vector.h>
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

#include "ClusterMonitor.h"
#include "PVSSDatapointDefs.h"

#define	MAX_CLUSTER_NODE	100
#define MAX2(a,b)	((a) > (b)) ? (a) : (b)

namespace LOFAR {
	using namespace APLCommon;
	using namespace APL::RTDBCommon;
	using namespace GCF::TM;
	using namespace GCF::PVSS;
	using namespace GCF::RTDB;
	namespace CEPCU {
	
//
// ClusterMonitor()
//
ClusterMonitor::ClusterMonitor(const string&	cntlrName) :
	GCFTask 			((State)&ClusterMonitor::initial_state,cntlrName),
	itsOwnPropertySet	(0),
	itsTimerPort		(0),
	itsDPservice		(0),
	itsPollInterval		(60)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");

	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "ClusterTimerPort");

	itsDPservice = new DPservice(this);
	ASSERTSTR(itsDPservice, "Can't allocate DPservice");

	itsClusterNameMask  = globalParameterSet()->getString("ClusterNameMask", "locus%03d");
	itsClusterNetwork   = globalParameterSet()->getString("ClusterNetwork",  "cep2.lofar");
	itsFirstClusterNode = globalParameterSet()->getUint("FirstClusterNode", 1);
	itsLastClusterNode  = globalParameterSet()->getUint("LastClusterNode", MAX_CLUSTER_NODE);
	ASSERTSTR(!itsClusterNameMask.empty(), "NameMask of Cluster not specified");
	ASSERTSTR(!itsClusterNetwork.empty(),  "Network name of Cluster not specified");
	ASSERTSTR(itsLastClusterNode <= MAX_CLUSTER_NODE, "Supporting only " << MAX_CLUSTER_NODE << " nodes");

	itsLastState.resize(itsLastClusterNode+1, -1);
}


//
// ~ClusterMonitor()
//
ClusterMonitor::~ClusterMonitor()
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
GCFEvent::TResult ClusterMonitor::initial_state(GCFEvent& event, 
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
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("Cluster:initialising"));
		itsOwnPropertySet->setValue(PN_CHM_CLUSTER_CONNECTED,  GCFPVBool(false));

		TRAN(ClusterMonitor::getClusterState);		// do inital check
	}
	break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (ClusterMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("initial, DEFAULT: " << eventName(event));
		break;
	}    

	return (GCFEvent::HANDLED);
}



//
// getClusterState(event, port)
//
// Ask the information of the Cluster
//
GCFEvent::TResult ClusterMonitor::getClusterState(GCFEvent& event, 
													GCFPortInterface& port)
{
	if (eventName(event) != "DP_SET") {
		LOG_DEBUG_STR ("getClusterState:" << eventName(event) << "@" << port.getName());
	}

	switch (event.signal) {
	case F_ENTRY: {
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("Cluster:requesting Cluster info"));
		itsTimerPort->setTimer(15.0);		// in case the answer never comes

		string	command(formatString("for i in `echo \".\" | awk '{ for (i=%d; i<=%d;i++) { printf \"%s.%s\\n\",i } }'`; do ../sbin/zabbix_get -s $i -k system.hostname ; done", 
				itsFirstClusterNode, itsLastClusterNode, itsClusterNameMask.c_str(), itsClusterNetwork.c_str()));
		FILE*	pipe(popen(command.c_str(), "r"));
		if (!pipe) {
			LOG_ERROR_STR ("Cluster:Unable to read pipe: " << command);
			TRAN(ClusterMonitor::waitForNextCycle);
			break;
		}

		int		lineLength((itsLastClusterNode-itsFirstClusterNode)*256);
		vector<char>	line(lineLength);
		line[0] = '\0';
		size_t	btsRead = fread(&line[0], 1, lineLength-1, pipe);
		if (!btsRead) {
			itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("Cluster:pipe failure"));
			LOG_WARN_STR("Could not determine the state of the clusternodes!");
		}
		else {
			line[btsRead] = '\0';
			_analyzeClusterResult(line, btsRead);
		}
		fclose(pipe);
		TRAN(ClusterMonitor::waitForNextCycle);				// go to next state.
		break;
	}

	case DP_SET:
	case F_EXIT:
		break;

	case F_QUIT:
		TRAN (ClusterMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("getClusterState, DEFAULT: " << eventName(event));
		break;
	}    

	return (GCFEvent::HANDLED);
}


//
// _analyzeClusterResult(result, length);
//
void ClusterMonitor::_analyzeClusterResult(vector<char>	result, size_t	length)
{
	vector<bool> 	online(itsLastClusterNode+1, false);
	size_t	end(length);
	size_t	begin(end);
	while (begin && end) {
		while (end && (result[end]=='\0' || result[end]=='\n')) {
			result[end--]='\0';
		}
		begin = end;
		while (begin && result[begin-1]!='\n') {
			begin--;
		}

		// possible answers:
		// locus999
		// zabbix_get [4624]: Get value error: cannot connect to [[locus002.cep2.lofar]:10050]: [113] No route to host
		int		nodeNr = -1;
		if (sscanf(&result[begin], itsClusterNameMask.c_str(), &nodeNr) != 1) {
			LOG_INFO_STR("Received error: " << (char*)&result[begin]);
		}
		else if (nodeNr < itsFirstClusterNode || nodeNr > itsLastClusterNode) {
			LOG_WARN_STR("Received info about node " << nodeNr << " which is not in my monitor range!");
		}
		else {
			online[nodeNr] = true;
		}
		end = begin - 1;
	}

	// Finally update the statusfields of all the nodes
	for (int i = itsFirstClusterNode; i <= itsLastClusterNode; i++) {
		int		newState = online[i] ? RTDB_OBJ_STATE_BROKEN : RTDB_OBJ_STATE_OPERATIONAL;
		if (itsLastState[i] != newState) {
			LOG_INFO_STR("Node " << _clusterNodeName(i) << ": " << (online[i] ? "ON" : "OFF"));
			itsLastState[i] = newState;
		}
	}
}

//
// _clusterNodeName(nodeNr)
//
string ClusterMonitor::_clusterNodeName(int nodeNr)
{
	// NOTE: THIS IS INSIDE INFORMATION ABOUT THE SETUP OF THE CLUSTER!!!
	int     rackMax[] = {12, 24, 36, 48, 52, 64, 76, 88, 100 };
	uint	rack;
	for (rack = 0; rack < sizeof(rackMax)-1; rack++) {
		if (nodeNr <= rackMax[rack])
			break;
	}

	string	locusNodeMask (createPropertySetName(PSN_LOCUS_NODE, ""));
	return (formatString(locusNodeMask.c_str(), rack, nodeNr));
}


//
// waitForNextCycle(event, port)
//
// Wait for our next cycle.
//
GCFEvent::TResult ClusterMonitor::waitForNextCycle(GCFEvent& event, 
													GCFPortInterface& port)
{
	if (eventName(event) != "DP_SET") {
		LOG_DEBUG_STR ("waitForNextCycle:" << eventName(event) << "@" << port.getName());	
	}

	switch (event.signal) {
	case F_ENTRY: {
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("Cluster:wait for next cycle"));
		int		waitTime = itsPollInterval - (time(0) % itsPollInterval);
		if (waitTime == 0) {
			waitTime = itsPollInterval;
		}
		itsTimerPort->cancelAllTimers();
		itsTimerPort->setTimer(double(waitTime));
		LOG_INFO_STR("Cluster:Waiting " << waitTime << " seconds for next cycle");
	}
	break;

	case F_TIMER: {
		itsOwnPropertySet->setValue(string(PN_FSM_ERROR),GCFPVString(""));
		TRAN(ClusterMonitor::getClusterState);
	}
	break;

	case DP_SET:
	case F_EXIT:
		break;

	case F_QUIT:
		TRAN (ClusterMonitor::finish_state);
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
GCFEvent::TResult ClusterMonitor::finish_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("finish_state:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		// update PVSS
		itsOwnPropertySet->setValue(string(PN_FSM_CURRENT_ACTION),GCFPVString("Cluster:finished"));
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
