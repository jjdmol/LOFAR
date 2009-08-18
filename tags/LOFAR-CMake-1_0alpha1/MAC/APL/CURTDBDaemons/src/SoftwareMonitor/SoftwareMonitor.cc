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
#include <ApplCommon/StationInfo.h>

#include <MACIO/MACServiceInfo.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/PVSS/PVSSinfo.h>
#include <GCF/RTDB/DP_Protocol.ph>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/RTDBCommon/RTDButilities.h>
#include <APL/RTDBCommon/CM_Protocol.ph>

#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "SoftwareMonitor.h"
#include "PVSSDatapointDefs.h"
#include <boost/date_time/posix_time/posix_time.hpp>

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

using namespace boost::posix_time;
namespace LOFAR {
	using namespace APLCommon;
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
	itsDPservice		(0),
	itsClaimMgrTask		(0)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");

	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "TimerPort");

	itsDPservice = new DPservice(this);	// don't report back
	ASSERTSTR(itsDPservice, "Can't allocate DPservice");

	itsClaimMgrTask = ClaimMgrTask::instance();
	ASSERTSTR(itsClaimMgrTask, "Can't construct a claimMgrTask");

	itsPollInterval      = globalParameterSet()->getInt("pollInterval", 		15);
	itsSuspThreshold     = globalParameterSet()->getInt("suspisciousThreshold", 2);
	itsBrokenThreshold   = globalParameterSet()->getInt("brokenThreshold", 		4);
	itsMaxRestartRetries = globalParameterSet()->getInt("maxRestartRetries", 	0);

	registerProtocol(CM_PROTOCOL, CM_PROTOCOL_STRINGS);
}


//
// ~SoftwareMonitor()
//
SoftwareMonitor::~SoftwareMonitor()
{
	LOG_TRACE_OBJ_STR (getName() << " destruction");

	if (itsDPservice)		delete itsDPservice;

	if (itsTimerPort)		delete itsTimerPort;
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
					ProcessDef		proc;
					proc.name  		  = field[SW_FLD_NAME];
					proc.level 		  = atoi(field[SW_FLD_LEVEL].c_str());
					proc.mustBroot 	  = atoi(field[SW_FLD_ROOT].c_str());
					proc.runsUnderMPI = atoi(field[SW_FLD_MPI].c_str());
					proc.permSW		  = !field[SW_FLD_UP].empty();
					itsLevelList.push_back(proc);
				}
			}
			getline(swfStream, line);
		}
		swfStream.close();
		ASSERTSTR(itsLevelList.size(), "File swlevel does not contain legal lines.");
		LOG_INFO_STR("Found " << itsLevelList.size() << " programs I should watch.");
		// copy permSW entries to itsPermProcs vector
		_constructPermProcsList();
		
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

		// loop over the permanent processes and update their status in PVSS
		vector<Process>::iterator	iter = itsPermProcs.begin();
		vector<Process>::iterator	end  = itsPermProcs.end();
		while (iter != end) {
			processMap_t::iterator	procPtr = itsProcessMap.find(iter->name);	// search in ps-ef list
			iter->pid = (procPtr != itsProcessMap.end()) ? procPtr->second : 0;
			_updateProcess(iter, iter->pid, curLevel);	// pid=0: not running, pid!=0: running
			if (iter->pid) {					// did we found the process?
				itsProcessMap.erase(procPtr);	// remove it from the list
			}
			iter++;
		} // while iter

		// --- pseudo code for next loop ---
		// for all current processes (processMap)
		//   process == obs process(levelList) ?
		//     No: try next
		//	   is process already in ObsProcslist ?
		//	     Yes: update PVSS, try next
		//       is obsID already in the ObsMap ?
		//         No:  send a req. to the claimMgr for DPname, try next (*1)
		//         Yes: add process to ObsProcsList, update PVSS, try next
		//
		// (*1) When answer arrives some time later, the obsID is added to the ObsMap and
		//      all Obs-bound processes are added to the ObsProcsList.

		// loop over the rest of the current processes and see if they match one of ours.
		processMap_t::iterator			cpIter = itsProcessMap.begin();	// Current Process iter
		processMap_t::iterator			cpEnd  = itsProcessMap.end();
		while (cpIter != cpEnd) {
			// Is this process an observation bound process?
			vector<ProcessDef_t>::iterator	llIter = itsLevelList.begin();
			vector<ProcessDef_t>::iterator	llEnd  = itsLevelList.end();
			while (llIter != llEnd) {
				if (!llIter->permSW && llIter->name == cpIter->first) {	// proc is an obs-bound proc?
					// is proc already in our obsProc list?
					vector<Process>::iterator	opIter = _searchObsProcess(cpIter->second);
					if (opIter != itsObsProcs.end()) {
						_updateProcess(opIter, opIter->pid, curLevel);
					} 
					else {
						// proc is not in our obsProcList, do we know this observation??
						int	obsID = _solveObservationID(cpIter->second);
						obsMap_t::iterator		obsIter = itsObsMap.find(obsID);
						if (obsIter == itsObsMap.end()) {	// new observationID?
							itsClaimMgrTask->claimObject("Observation", observationName(obsID), port);	// ask claim manager
							break;							// process this later.
						}
						else {	// obsID is known but proces not (strange), just add it.
							Process		newProc(llIter->name, obsIter->second.DPname+"_"+llIter->name, obsID, llIter->level);
							newProc.pid = cpIter->second;
							itsObsProcs.push_back(newProc);
							LOG_DEBUG_STR("new unknown process for obs: " << obsID << ":" << llIter->name);
							_updateProcess(_searchObsProcess(newProc.pid), newProc.pid, curLevel);
						}
					} // process (not) in ObsProcList
				} // process is obs-bound
				llIter++;
			} // loop over levelList
			cpIter++;
		} // loop over current process list.

		TRAN(SoftwareMonitor::waitForNextCycle);
	}
	break;

	case DP_SET:
		break;

	case CM_CLAIM_RESULT: {
			CMClaimResultEvent	cmEvent(event);
			LOG_INFO_STR(cmEvent.nameInAppl << " is mapped to " << cmEvent.DPname);
			_updateObservationMap(cmEvent.nameInAppl, cmEvent.DPname);
		}
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
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("Wait for next cycle"));
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

	case CM_CLAIM_RESULT: {
			CMClaimResultEvent	cmEvent(event);
			LOG_INFO_STR(cmEvent.nameInAppl << " is mapped to " << cmEvent.DPname);
			_updateObservationMap(cmEvent.nameInAppl, cmEvent.DPname);
		}
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
// _updateProcess(iter, pid, curlevel)
//
// Update the information of the given process in PVSS
//
void SoftwareMonitor::_updateProcess(vector<Process>::iterator	iter, int	pid, int	curLevel)
{
	if (pid) {					// process is running?
		// mark it operational whether or not it should be running
		setObjectState(getName(), iter->DPname, RTDB_OBJ_STATE_OPERATIONAL, true);	// force
		iter->errorCnt = 0;

		// update startTime when not done before
		if (iter->startTime) {
			return;
		}

		int				fd;
		char			statFile  [256];
		struct stat		statStruct;
		sprintf(statFile, "/proc/%d/cmdline", iter->pid);
		if ((fd = stat(statFile, &statStruct)) != -1) {
			iter->startTime = statStruct.st_ctime;
		}
		else {	// retrieval of time failed assume 'now'
			iter->startTime = time(0);
		}
		LOG_DEBUG_STR("starttime of " << iter->name << " = " << to_simple_string(from_time_t(iter->startTime)));
		itsDPservice->setValue(iter->DPname+".process.startTime", 
								GCFPVString(to_simple_string(from_time_t(iter->startTime))));
		itsDPservice->setValue(iter->DPname+".process.processID", GCFPVInteger(iter->pid));
		return;
	}

	// pid = 0 ==> process is not running
	if (iter->level > curLevel) {										// should it be down?
		setObjectState(getName(), iter->DPname, RTDB_OBJ_STATE_OFF, true);	// yes
		iter->errorCnt = 0;
	}
	else {
		// When switching from swlevel 1 to eg. swlevel 5 may take some time when the RSPboards
		// are running in low-power mode. During this time you don't want the processes that
		// are not yet running being reported are broken. With the conf file of the SoftwareMonitor
		// you can set the number of cycles a process is not reported as suspicious or broken.
		if (iter->errorCnt >= itsBrokenThreshold) {				// serious problem
			setObjectState(getName(), iter->DPname, RTDB_OBJ_STATE_BROKEN);
		}
		else if (iter->errorCnt >= itsSuspThreshold) {			// allow start/stop times
			setObjectState(getName(), iter->DPname, RTDB_OBJ_STATE_SUSPICIOUS);
		}
		else {
			setObjectState(getName(), iter->DPname, RTDB_OBJ_STATE_OFF, true);	// force
		}
		iter->errorCnt++;
	} // proces not running but it should have been running
		
	// update stopTime is not done already.
	if (iter->startTime > iter->stopTime) {
		iter->stopTime = time(0);
		LOG_DEBUG_STR("stoptime of " << iter->name << " = " << to_simple_string(from_time_t(iter->stopTime)));
		itsDPservice->setValue(iter->DPname+".process.stopTime", 
									GCFPVString(to_simple_string(from_time_t(iter->stopTime))));
		itsDPservice->setValue(iter->DPname+".process.processID", GCFPVInteger(0));
	}
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

//
// _updateObservationMap(obsName, DPname)
//
// Add the given observation to the activeObsMap and add process-entries to the ObsProcList.
//
void SoftwareMonitor::_updateObservationMap(const string&	orgName, const string&	DPname)
{
	// note: orgName: LOFAR_ObsSW_Observation9999
	//		 DPname : LOFAR_ObsSW_TempObs9999
	string	obsName(orgName);	// modifyable copy
	ltrim(obsName, string("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_:"));	// strip off text
	int		obsID = atoi(obsName.c_str());
	obsMap_t::iterator		obsIter = itsObsMap.find(obsID);
	if (obsIter != itsObsMap.end()) {
		LOG_DEBUG_STR("Strange, obs " << obsID << " already exists in the ObservationMap");
		return;
	}

	// Add a processObject for each observation-bound process to the ObservationProcs.
	// We expect for each of these entries a running process.
	LOG_INFO_STR("Adding observation " << obsID << " to my administration");
//	itsObsMap[obsID] = ObsInfo(DPname, true);
	string	fullDPname(PVSSDatabaseName()+":"+DPname);
	itsObsMap.insert(make_pair(obsID, ObsInfo(fullDPname, true)));

	vector<ProcessDef_t>::iterator	procDefIter = itsLevelList.begin();
	vector<ProcessDef_t>::iterator	procDefEnd  = itsLevelList.end();
	while (procDefIter != procDefEnd) {
		if (!procDefIter->permSW) {
			itsObsProcs.push_back(Process(procDefIter->name, fullDPname+"_"+procDefIter->name, obsID, procDefIter->level));
			LOG_DEBUG_STR("new obs entry:" << procDefIter->name << ", " << fullDPname+"_"+procDefIter->name << ", " << obsID);
		}
		procDefIter++;
	}
}

//
// _constructPermProcsList()
//
// Construct the PermProcs list from the swLevel list.
//
void SoftwareMonitor::_constructPermProcsList()
{
	vector<ProcessDef_t>::iterator	procDefIter = itsLevelList.begin();
	vector<ProcessDef_t>::iterator	procDefEnd  = itsLevelList.end();
	while (procDefIter != procDefEnd) {
		if (procDefIter->permSW) {
			string	DPname(formatString((procDefIter->level == 1) ? "%s:LOFAR_PermSW_Daemons_%s" : "%s:LOFAR_PermSW_%s", 
							PVSSDatabaseName().c_str(), procDefIter->name.c_str()));
			itsPermProcs.push_back(Process(procDefIter->name, DPname, -1, procDefIter->level));
			LOG_DEBUG_STR("new perm entry:" << procDefIter->name << ", " << DPname);
		}
		procDefIter++;
	}
}

//
// _searchObsProcess(pid)
//
// Returns iterator to Obs process with given pid or iter to end.
//
vector<SoftwareMonitor::Process>::iterator SoftwareMonitor::_searchObsProcess(int	pid)
{
	vector<Process>::iterator	opIter = itsObsProcs.begin();
	vector<Process>::iterator	opEnd  = itsObsProcs.end();
	while (opIter != opEnd) {
		if (opIter->pid == pid) {	// known process?
			return (opIter);
		}
		++opIter;
	} 
	return (opEnd);
}



//
// _solveObservationID(pid)
//
// Try to find out the observationnumber from the given pid by analysing the cmdline.
//
int	SoftwareMonitor::_solveObservationID(int		pid)
{
	int			fd;
	char		fileName[256];
	char		buffer  [1024];
	int			nrBytes;
	sprintf(fileName, "/proc/%d/cmdline", pid);
	if ((fd = open(fileName, O_RDONLY)) != -1) {
		if ((nrBytes = read(fd, buffer, 1024-1)) > 0) {
			buffer[nrBytes] ='\0';
			char*	obsPos = strstr(buffer, "Observation");
			if (obsPos) {
				int		obsID = 0;
				sscanf (obsPos, "Observation%d%*s", &obsID);
				return (obsID);
			}
			LOG_DEBUG_STR("No observationID found in:" << buffer);
		}
		close(fd);
	}

	LOG_DEBUG_STR("No observationId found for process " << pid);
	return (0);
}

  }; // StationCU
}; // LOFAR
