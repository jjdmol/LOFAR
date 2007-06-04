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

#include <GCF/TM/GCF_Protocols.h>
#include <GCF/TM/GCF_Task.h>
#include <GTM_Defines.h>
#include <GCF/TM/GCF_Handler.h>

#include <GCF/TM/GCF_PortInterface.h>

#include <Common/lofar_fstream.h>
#include <Common/LofarLocators.h>
#include <APS/ParameterSet.h>
using LOFAR::ACC::APS::ParameterSet;
using std::ifstream;

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace TM 
  {

// static member initialisation
bool GCFTask::_doExit = false;
GCFTask::THandlers GCFTask::_handlers;
GCFTask::THandlers GCFTask::_tempHandlers;
GCFTask::TProtocols GCFTask::_protocols;
int GCFTask::_argc = 0;
char** GCFTask::_argv = 0;

GCFTask::GCFTask(State initial, const string& name) :
  GCFFsm(initial), _name(name)
{
  // framework protocols old-style registration
  registerProtocol(F_FSM_PROTOCOL, F_FSM_PROTOCOL_names);
  registerProtocol(F_PORT_PROTOCOL, F_PORT_PROTOCOL_names);

	// new style registration
	TM::registerProtocol(F_FSM_PROTOCOL, F_FSM_PROTOCOL_STRINGS);
	TM::registerProtocol(F_PORT_PROTOCOL,F_PORT_PROTOCOL_STRINGS);
}


GCFTask::~GCFTask()
{
}

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
		LOG_DEBUG_STR ("Initialized logsystem with: " << aCL.locate(logPropFile));
	}
	else {
		INIT_VAR_LOGGER(aCL.locate(logPropFile).c_str(), logfile);   
		LOG_DEBUG_STR ("Initialized logsystem with: " << aCL.locate(logPropFile) <<
						"," << logfile);
	}

	// Read in the ParameterSet of the task (<task>.conf)
	ParameterSet*	pParamSet = ACC::APS::globalParameterSet();
	string			configFile(aCL.locate(procName + ".conf"));
	if (!configFile.empty()) {
		LOG_DEBUG_STR ("Using parameterfile: " << configFile);
		pParamSet->adoptFile(configFile);
	}
	else {
		LOG_DEBUG_STR ("NO DEFAULT PARAMETERSET FOUND");
	}

	if (_doExit) {
		exit(-1);
	}
}

void GCFTask::run()
{
  signal(SIGINT,  GCFTask::signalHandler);
  signal(SIGTERM, GCFTask::signalHandler);
  signal(SIGPIPE, SIG_IGN);

  // THE MAIN LOOP OF THE MAC PROCESS
  // can only be interrupted/stopped by calling stop or terminating the application
  while (!_doExit)
  {
    // new handlers can be add during processing the workProc
    // thus a temp handler map is used
    _tempHandlers.clear();
    _tempHandlers.insert(_handlers.begin(), _handlers.end()); 

    for (THandlers::iterator iter = _tempHandlers.begin() ;
         iter != _tempHandlers.end() && !_doExit; ++iter)
    {
      if (!iter->second) continue;
      iter->first->workProc();
    }
  }
  stop();
}

void GCFTask::stop()
{
  // stops the application in 2 steps
  // first it stops the handlers and secondly it deletes the handler objects if
  // they are not used anymore by other object followed by deleting the handlers list
  GCFHandler* pHandler(0);
  if (_doExit)
  {
    LOG_INFO("Process is stopped! Possible reasons: 'stop' called or terminated");
    for (THandlers::iterator iter = _handlers.begin() ;
         iter != _handlers.end(); ++iter)
    {
      if (!iter->second) continue; // handler pointer is not valid anymore, 
                                   // because this handler was deleted by the user
      pHandler = iter->first;
      pHandler->leave(); // "process" is also a handler user, see registerHandler
      if (pHandler->mayDeleted()) // no other object uses this handler anymore?
      {
        delete pHandler;
      }
    } 
    _handlers.clear();
  }
  else
  {
    for (THandlers::iterator iter = _handlers.begin() ;
          iter != _handlers.end() ; 
          ++iter)
    {
      if (!iter->second) continue; // handler pointer is not valid anymore, 
                                   // because this handler was deleted by the user
      pHandler = iter->first;
      pHandler->stop();
    } 
    _doExit = true;
  }     
}

void GCFTask::registerHandler(GCFHandler& handler)
{
  _handlers[&handler] = true; // valid pointer
  handler.use(); // released after stop
}

void GCFTask::deregisterHandler(GCFHandler& handler)
{
  THandlers::iterator iter = _tempHandlers.find(&handler);
  
  if (iter != _tempHandlers.end())
  {
    iter->second = false; // pointer will be made invalid because the user 
                          // deletes the handler by itself                          
  }
  _handlers.erase(&handler);
}

void GCFTask::registerProtocol(unsigned short protocolID,
              const char* signal_names[])
{
  ASSERT((protocolID << 8) <= F_EVT_PROTOCOL_MASK);
  _protocols[protocolID] = signal_names;
}

//
// evtstr(event&)
//
string GCFTask::evtstr(const GCFEvent& e)  const
{
	TProtocols::const_iterator iter = _protocols.find(F_EVT_PROTOCOL(e));
	if (iter != _protocols.end()) {
		return ((iter->second)[F_EVT_SIGNAL(e)]);
	}

	return (formatString("unknown signal(p=%d, s=%d)", F_EVT_PROTOCOL(e), F_EVT_SIGNAL(e)));
}

void GCFTask::signalHandler(int sig)
{
  if ( (sig == SIGINT) || (sig == SIGTERM) )
    _doExit = true;
}                                            
  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
