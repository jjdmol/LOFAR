//#  SoftwareMonitor.cc: Implementation of the MAC Scheduler task
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
#include <Common/lofar_fstream.h>
#include <Common/LofarLogger.h>
#include <Common/LofarConstants.h>
#include <Common/LofarLocators.h>
#include <Common/StringUtil.h>
#include <Common/ParameterSet.h>

#include <GCF/PVSS/GCF_PVTypes.h>
#include <MACIO/MACServiceInfo.h>
#include <APL/RTDBCommon/RTDButilities.h>
#include <GCF/RTDB/DP_Protocol.ph>
#include <APL/APLCommon/StationInfo.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>

#include "SoftwareMonitor.h"
#include "PVSSDatapointDefs.h"

enum {
	SW_FLD_LEVEL = 0,
	SW_FLD_UP,
	SW_FLD_DOWN,
	SW_FLD_ROOT,
	SW_FLD_MPI,
	SW_FLD_NAME,
	SW_FLD_NR_OF_FIELDS
};

#define MAX2(a,b)	((a) > (b)) ? (a) : (b)

namespace LOFAR {
	using namespace Deployment;
	using namespace APL::RTDBCommon;
	using namespace GCF::TM;
	using namespace GCF::PVSS;
	using namespace GCF::RTDB;
	namespace RTDBDaemons {
	
//
// SoftwareMonitor()
//
SoftwareMonitor::SoftwareMonitor(const string&	cntlrName) :
	GCFTask 			((State)&SoftwareMonitor::initial_state,cntlrName),
	itsOwnPropertySet	(0),
	itsTimerPort		(0),
	itsDPservice		(0)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");

	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "TimerPort");

	itsDPservice = new DPservice(this);
	ASSERTSTR(itsDPservice, "Can't allocate DPservice");

	itsPollInterval      = globalParameterSet()->getInt("pollInterval", 		15);
	itsSuspThreshold     = globalParameterSet()->getInt("suspisciousThreshold", 2);
	itsBrokenThreshold   = globalParameterSet()->getInt("brokenThreshold", 		4);
	itsMaxRestartRetries = globalParameterSet()->getInt("maxRestartRetries", 	0);
}


//
// ~SoftwareMonitor()
//
SoftwareMonitor::~SoftwareMonitor()
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
GCFEvent::TResult SoftwareMonitor::initial_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("initial:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
   		break;

	case F_ENTRY: {
		// Get access to my own propertyset.
		LOG_DEBUG_STR ("Activating PropertySet " << PSN_SOFTWARE_MONITOR);
		itsTimerPort->setTimer(2.0);
		itsOwnPropertySet = new RTDBPropertySet(PSN_SOFTWARE_MONITOR,
												PST_SOFTWARE_MONITOR,
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
		// update PVSS.
		LOG_TRACE_FLOW ("Updateing state to PVSS");
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("Initial"));
		itsOwnPropertySet->setValue(PN_FSM_ERROR,  GCFPVString(""));
		
		LOG_DEBUG_STR("Going to read the software levels");
		TRAN (SoftwareMonitor::readSWlevels);
	}
	
	case DP_SET:
		break;

	case F_QUIT:
		TRAN (SoftwareMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("initial, DEFAULT");
		break;
	}    

	return (status);
}


//
// readSWlevels(event, port)
//
// Setup connection with Softwaredriver
//
GCFEvent::TResult SoftwareMonitor::readSWlevels(GCFEvent& 		  event, 
												GCFPortInterface& port)
{
	LOG_DEBUG_STR ("readSWlevels:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY: {
		// update PVSS
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("Reading swlevel file"));

		// Try to find and open the swlevel.conf file.
		ConfigLocator	cl;
		string			swFile (cl.locate("swlevel.conf"));
		ifstream		swfStream;
		swfStream.open(swFile.c_str(), ifstream::in);
		ASSERTSTR(swfStream, "Unable to open the swlevelfile '" << swFile << "'");

		// parse the file
		string	line;
		getline(swfStream, line);
		while(swfStream) {
			if (line[0] != '#' && line[0] != ' ') {
				// line syntax: level : up : down : root : mpi : program
				vector<string>	field = StringUtil::split(line, ':');
				ASSERTSTR(field.size() >= SW_FLD_NR_OF_FIELDS, "Strange formatted line in swlevel: " << line);

				// check if executable exists (this is what swlevel does also)
				struct	stat	statBuf;
				// note: stat return 0 on success.
				if (!stat(formatString("%s/%s", LOFAR_BIN_LOCATION, field[SW_FLD_NAME].c_str()).c_str(), &statBuf)) {
					// add line to our admin
					Process		proc;
					proc.name  		  = field[SW_FLD_NAME];
					proc.level 		  = atoi(field[SW_FLD_LEVEL].c_str());
					proc.mustBroot 	  = atoi(field[SW_FLD_ROOT].c_str());
					proc.runsUnderMPI = atoi(field[SW_FLD_MPI].c_str());
					proc.permSW		  = !field[SW_FLD_UP].empty();
					proc.pid		  = 0;
					proc.errorCnt	  = 0;
					itsLevelList.push_back(proc);
				}
			}
			getline(swfStream, line);
		}
		swfStream.close();
		ASSERTSTR(itsLevelList.size(), "File swlevel does not contain legal lines.");
		LOG_INFO_STR("Found " << itsLevelList.size() << " programs I should watch.");
		TRAN(SoftwareMonitor::checkPrograms);
	}
	break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (SoftwareMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("readSWlevels: DEFAULT");
		break;
	}    

	return (status);
}

//
// checkPrograms(event, port)
//
// check the level of all programs.
//
GCFEvent::TResult SoftwareMonitor::checkPrograms(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("checkPrograms:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY: {
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("Checking programs"));

		_buildProcessMap();		// reconstruct map for current processlist.(name,pid)

		// Note: swlevel v1.6 20081229 (svn 12378) returns current level as return value.
		int curLevel = system("swlevel >>/dev/null") >> 8;
		ASSERTSTR(curLevel >= 0, "Program 'swlevel' not in my execution path");
		LOG_DEBUG_STR("Current level is " << curLevel);

		// loop over the level list and check that with the new process map.
		vector<Process>::iterator	iter = itsLevelList.begin();
		vector<Process>::iterator	end  = itsLevelList.end();
		while (iter != end) {
			// TODO: BE ABLE TO HANDLE MULTIPLE INSTANCES OF A PROCESS (e.g. ObservationControl)
			processMap_t::iterator	procPtr = itsProcessMap.find(iter->name);
			iter->pid = (procPtr != itsProcessMap.end()) ? procPtr->second : 0;
			// TODO: BE ABLE TO HANDLE LOFAR_ObsSW DATAPOINTS
			string	DPname(formatString(iter->permSW ? 
							((iter->level == 1) ? "LOFAR_PermSW_Daemons_%s" : "LOFAR_PermSW_%s") : "LOFAR_ObsSW_%s", 
							iter->name.c_str()));
			if (iter->pid) {					// process is running?
				setObjectState(getName(), DPname, RTDB_OBJ_STATE_OPERATIONAL);		// mark it operational whether or not it should be running
				iter->errorCnt = 0;
			}
			else {								// process is not running
				if (iter->level > curLevel) {	// should it be down?
					setObjectState(getName(), DPname, RTDB_OBJ_STATE_OFF);			// yes
					iter->errorCnt = 0;
				}
				else {
					if (iter->errorCnt >= itsBrokenThreshold) {		// serious problem
						setObjectState(getName(), DPname, RTDB_OBJ_STATE_BROKEN);
					}
					else if (iter->errorCnt >= itsSuspThreshold) {		// allow start/stop times
						setObjectState(getName(), DPname, RTDB_OBJ_STATE_SUSPICIOUS);
					}
					else {
						setObjectState(getName(), DPname, RTDB_OBJ_STATE_OFF);
					}
				}
				iter->errorCnt++;
			}
			iter++;
		}
		TRAN(SoftwareMonitor::waitForNextCycle);
	}
	break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (SoftwareMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("checkPrograms: DEFAULT");
		break;
	}    

	return (status);
}


//
// waitForNextCycle(event, port)
//
// Take subscription on clock modifications
//
GCFEvent::TResult SoftwareMonitor::waitForNextCycle(GCFEvent& event, 
													GCFPortInterface& port)
{
	if (eventName(event) != "DP_SET") {
		LOG_DEBUG_STR ("waitForNextCycle:" << eventName(event) << "@" << port.getName());	}

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY: {
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("Wait for next cycle"));
		int		waitTime = itsPollInterval - (time(0) % itsPollInterval);
		if (waitTime == 0) {
			waitTime = itsPollInterval;
		}
		itsTimerPort->cancelAllTimers();
		itsTimerPort->setTimer(double(waitTime));
		LOG_INFO_STR("Waiting " << waitTime << " seconds for next cycle");
	}
	break;

	case F_TIMER: {
		TRAN(SoftwareMonitor::checkPrograms);
	}
	break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (SoftwareMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("waitForNextCycle, DEFAULT");
		break;
	}    

	return (status);
}


//
// finish_state(event, port)
//
// Write controller state to PVSS
//
GCFEvent::TResult SoftwareMonitor::finish_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("finish_state:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// update PVSS
		itsOwnPropertySet->setValue(string(PN_FSM_CURRENT_ACTION),GCFPVString("Software:finished"));
//		itsOwnPropertySet->setValue(string(PN_HWM_Software_ERROR),GCFPVString(""));
		break;
	}
  
	case DP_SET:
		break;

	default:
		LOG_DEBUG("finishing_state, DEFAULT");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}

//
// _buildProcessMap()
//
// Reconstruct a multimap with all the processes currently running.
//
void SoftwareMonitor::_buildProcessMap()
{
	const int STAT_BUFFER_SIZE = 1024;

	itsProcessMap.clear();

	DIR*	procDir = opendir("/proc");
	ASSERTSTR(procDir, "Cannot open directory /proc to check programlist");
	chdir("/proc");

	struct dirent*	dirPtr;
	while ((dirPtr = readdir(procDir))) {
		if (!isdigit(dirPtr->d_name[0])) {
			continue;
		}
		int			fd;
		char		statFile  [256];
		char		statBuffer[STAT_BUFFER_SIZE];
		sprintf(statFile, "/proc/%s/cmdline", dirPtr->d_name);
		if ((fd = open(statFile, O_RDONLY)) != -1) {
			if (read(fd, statBuffer, STAT_BUFFER_SIZE-1)) {
				itsProcessMap.insert(pair<string,int>(basename(statBuffer), atoi(dirPtr->d_name)));
			}
			close(fd);
		}
	}
	closedir(procDir);
	ASSERTSTR(itsProcessMap.size(), "No processes found!? Programming error!?");
}


}; // StationCU
}; // LOFAR
