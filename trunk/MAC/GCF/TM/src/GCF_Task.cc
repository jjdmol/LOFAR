//#  GCF_Task.cc: task class which encapsulates a task and its behaviour as a 
//#  finite state machine (FSM).
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <GCF/TM/GCF_Protocols.h>
#include <GCF/TM/GCF_Task.h>
#include <GTM_Defines.h>
#include <GCF/TM/GCF_Handler.h>

#include <GCF/TM/GCF_PortInterface.h>

#include <Common/lofar_fstream.h>
#include <Common/LofarLocators.h>
#include <Common/ParameterSet.h>
using std::ifstream;

#include <signal.h>
#include <sys/time.h>

namespace LOFAR {
  namespace GCF {
    namespace TM {

// static member initialisation
bool				GCFTask::_doExit = false;
GCFTask::THandlers	GCFTask::_handlers;
GCFTask::THandlers	GCFTask::_tempHandlers;
GCFTask::TProtocols	GCFTask::_protocols;
int 				GCFTask::_argc = 0;
char** 				GCFTask::_argv = 0;
bool				GCFTask::_delayedQuit = false;

//
// GCFTask(state, name)
//
GCFTask::GCFTask(State initial, const string& name) :
  GCFFsm(initial), _name(name)
{
	// new style registration
	registerProtocol(F_FSM_PROTOCOL, F_FSM_PROTOCOL_STRINGS);
	registerProtocol(F_PORT_PROTOCOL,F_PORT_PROTOCOL_STRINGS);
}

//
// ~GCFTask()
//
GCFTask::~GCFTask()
{
}

//
// init(argc, argv, logfile)
//
void GCFTask::init(int argc, char** argv, const string&	logfile)
{
	_argc = argc;
	_argv = argv;

	// Try to open the log_prop file, if process has its own log_prop file then use it
	// the INIT_LOGGER otherwise use the default mac.log_prop
	ConfigLocator	aCL;
	string 			procName(basename(argv[0]));
	string			logPropFile(procName + ".log_prop");
	// First try logpropfile <task>.log_prop
	if (aCL.locate(logPropFile) == "") {
		// locator could not find it try defaultname
		logPropFile = "mac.log_prop";
	}

	if (logfile.empty()) {
		INIT_LOGGER(aCL.locate(logPropFile).c_str());
		LOG_INFO_STR ("Initialized logsystem with: " << aCL.locate(logPropFile));
	}
	else {
		INIT_VAR_LOGGER(aCL.locate(logPropFile).c_str(), logfile);
		LOG_INFO_STR ("Initialized logsystem with: " << aCL.locate(logPropFile) <<
						"," << logfile);
	}

	// Read in the ParameterSet of the task (<task>.conf)
	ParameterSet*	pParamSet = globalParameterSet();
	string			configFile(aCL.locate(procName + ".conf"));
	if (!configFile.empty()) {
		LOG_INFO_STR ("Using parameterfile: " << configFile);
		pParamSet->adoptFile(configFile);
	}
	else {
		LOG_INFO_STR ("NO DEFAULT PARAMETERSET FOUND");
	}

	if (_doExit) {
		exit(-1);
	}
}

//
// run(secondsToRun)
//
void GCFTask::run(double maxSecondsToRun)
{
	// catch terminate signals
	signal(SIGINT,  GCFTask::signalHandler);
	signal(SIGTERM, GCFTask::signalHandler);
	signal(SIGPIPE, SIG_IGN);

	double	terminateTime(0.0);
	struct timeval	TV;
	if (maxSecondsToRun) {
		LOG_INFO_STR("Program execution stops over " << maxSecondsToRun << " seconds");
		gettimeofday(&TV, 0);
		terminateTime = TV.tv_sec + (TV.tv_usec / 10000000) + maxSecondsToRun;
	}

	// THE MAIN LOOP OF THE MAC PROCESS
	// can only be interrupted/stopped by calling stop or terminating the application
	while (!_doExit) {
		// new handlers can be add during processing the workProc
		// thus a temp handler map is used
		_tempHandlers.clear();
		_tempHandlers.insert(_handlers.begin(), _handlers.end()); 
		for (THandlers::iterator iter = _tempHandlers.begin() ;
				iter != _tempHandlers.end() && !_doExit; ++iter) {
			if (iter->second) {
				iter->first->workProc();
			}
		} // for

		// time to quit?
		if (maxSecondsToRun) {
			gettimeofday(&TV, 0);
			if ((TV.tv_sec + (TV.tv_usec / 10000000)) >= terminateTime) {
				_doExit = true;
			}
		}
	} // while

	if (!_delayedQuit) {
		stopHandlers();
	}
	else {
		_delayedQuit = false;	// never delay twice
		_doExit = false;		// we will run again
	}
}

//
// stopHandlers()
//
// An application can be stopped in two ways, a task itself may call stop() or
// the user may kill the program.
//
void GCFTask::stopHandlers()
{
	for (THandlers::iterator iter = _handlers.begin() ; iter != _handlers.end() ; ++iter) {
		if (iter->second)  {	// if pointer is still valid
			iter->first->stop();	// give handler a way of stopping in a neat way
		}
	} 

	// STEP 2
	LOG_INFO("Process is stopped! Possible reasons: 'stop' called or terminated");
	GCFHandler* pHandler(0);
	for (THandlers::iterator iter = _handlers.begin() ; iter != _handlers.end(); ++iter) {
		if (!iter->second)  {
			continue; // handler pointer is not valid anymore, 
		}
		// because this handler was deleted by the user
		pHandler = iter->first;
		pHandler->leave(); // "process" is also a handler user, see registerHandler
		if (pHandler->mayDeleted())  { // no other object uses this handler anymore?
			delete pHandler;
		}
	} 

	_handlers.clear();
}

//
// registerHandler(handler)
//
// NOTE: There are only a few handlers defined, timerHandler, fileHandler, PVSS Handler.
//	Those handlers handle the work of all 'ports' of that type. E.g. the timerHandler handles
// 	the decrement of all timers defined in all tasks!!!!
//
void GCFTask::registerHandler(GCFHandler& handler)
{
	_handlers[&handler] = true; // valid pointer
	handler.use(); // released after stop
}

//
// deregisterHandler(handler)
//
void GCFTask::deregisterHandler(GCFHandler& handler)
{
	THandlers::iterator iter = _tempHandlers.find(&handler);
	if (iter != _tempHandlers.end()) {
		iter->second = false; 	// pointer will be made invalid because the user 
								// deletes the handler by itself                          
	}
	_handlers.erase(&handler);
}

//
// signalHandler(sig)
//
void GCFTask::signalHandler(int sig)
{
	if ((sig == SIGINT) || (sig == SIGTERM)) {
		_doExit = true;
	}
}                                            

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
