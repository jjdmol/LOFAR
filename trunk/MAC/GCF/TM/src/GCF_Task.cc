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

#include <GCF/TM/GCF_Task.h>
#include <GTM_Defines.h>
#include <GCF/TM/GCF_Protocols.h>
#include <GCF/TM/GCF_Handler.h>

#include <GCF/TM/GCF_PortInterface.h>

#include <GCF/ParameterSet.h>
#include <Common/lofar_fstream.h>
using std::ifstream;

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

// static member initialisation
bool GCFTask::_doExit = false;
GCFTask::THandlers GCFTask::_handlers;
GCFTask::TProtocols GCFTask::_protocols;
int GCFTask::_argc = 0;
char** GCFTask::_argv = 0;

GCFTask::GCFTask(State initial, const string& name) :
  GCFFsm(initial), _name(name)
{
  // framework protocols
  registerProtocol(F_FSM_PROTOCOL, F_FSM_PROTOCOL_names);
  registerProtocol(F_PORT_PROTOCOL, F_PORT_PROTOCOL_names);
}


GCFTask::~GCFTask()
{
}

void GCFTask::init(int argc, char** argv)
{
  _argc = argc;
  _argv = argv;
  
  ParameterSet* pParamSet = ParameterSet::instance();
  string appName(argv[0]);
  pParamSet->adoptFile(appName + ".conf");

  ifstream    logPropFile;

  //# Try to open the file
  string logPropFileName = appName + ".log_prop";
  logPropFile.open(logPropFileName.c_str(), ifstream::in);
  if (!logPropFile) 
  {
    logPropFileName = "./mac.log_prop";
  }
  else
  {
    logPropFile.close();
  }
  
  INIT_LOGGER(logPropFileName.c_str());   

  if (_doExit)
    exit(-1);
}

void GCFTask::run()
{
  signal(SIGINT,  GCFTask::signalHandler);
  signal(SIGTERM, GCFTask::signalHandler);
  signal(SIGPIPE, SIG_IGN);

  // THE MAIN LOOP OF THE APPLICATION
  // can only be interrupted/stopped by calling stop or terminating the application
  while (!_doExit)
  {
    // new handlers can add during processing the workProc
    // thus a temp handler list is made
    THandlers tempHandlers(_handlers); 

    for (THandlers::iterator iter = tempHandlers.begin() ;
          iter != tempHandlers.end() && !_doExit; 
          ++iter)
    {
      (*iter)->workProc();
    }
  }
  stop();
}

void GCFTask::stop()
{
  // stops the application in 2 steps
  // first it stops the handlers and secondly it deletes the handler objects if
  // they are not used anymore by other object followed by deleting the handlers list
  if (_doExit)
  {
    LOG_INFO("Application is stopped! Possible reasons: 'stop' called or terminated");
    GCFHandler* pHandler;
    for (THandlers::iterator iter = _handlers.begin() ;
          iter != _handlers.end() ; 
          ++iter)
    {
      pHandler = (*iter);
      pHandler->leave(); // "application" is also a handler user, see registerHandler
      if (pHandler->mayDeleted()) // no other object uses this handler?
      {
        delete pHandler;
      }
    } 
    _handlers.clear();
  }
  else
  {
    GCFHandler* pHandler;
    for (THandlers::iterator iter = _handlers.begin() ;
          iter != _handlers.end() ; 
          ++iter)
    {
      pHandler = (*iter);
      pHandler->stop();
    } 
    _doExit = true;
  }     
}

void GCFTask::registerHandler(GCFHandler& handler)
{
  _handlers.push_back(&handler);
  handler.use(); // released after stop
}

void GCFTask::registerProtocol(unsigned short protocolID,
              const char* signal_names[])
{
  assert((protocolID << 8) <= F_EVT_PROTOCOL_MASK);
  _protocols[protocolID] = signal_names;
}

const char* GCFTask::evtstr(const GCFEvent& e)  const
{
  static const char* unknown = "unknown signal";
  const char* signame(0);
  TProtocols::const_iterator iter = _protocols.find(F_EVT_PROTOCOL(e));
  if (iter != _protocols.end())
  {
    signame = (iter->second)[F_EVT_SIGNAL(e)];
  }

  return (signame?signame:unknown);
}

void GCFTask::signalHandler(int sig)
{
  if ( (sig == SIGINT) || (sig == SIGTERM) )
    _doExit = true;
}                                            
