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
#include <PortInterface/GTM_NameService.h>
#include <PortInterface/GTM_TopologyService.h>

#include <GCF/CmdLine.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

bool GCFTask::_doExit = false;
vector<GCFHandler*> GCFTask::_handlers;
map<unsigned short, const char**> GCFTask::_protocols;
int GCFTask::_argc = 0;
char** GCFTask::_argv = 0;


GCFTask::GCFTask(State initial, string& name) :
  GCFFsm(initial), _name(name)
{
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
  
  string serviceFilesPrefix("mac");
  string serviceFilesPath(".");
  if (_argv != 0)
  {
    CCmdLine cmdLine;

    // parse argc,argv 
    if (cmdLine.SplitLine(_argc, _argv) > 0)
    {
      serviceFilesPrefix = cmdLine.GetSafeArgument("-sfp", 0, "mac");
      serviceFilesPath = cmdLine.GetSafeArgument("-p", 0, "./");
    }            
  }
  
  if (serviceFilesPrefix == "mac")
  {
    char* macConfigPath = getenv("MAC_CONFIG");
    if (macConfigPath)
    {
      serviceFilesPath = macConfigPath;
    }
  }
  if (serviceFilesPath.rfind("/") != (serviceFilesPath.length() - 1))
    serviceFilesPath += '/';
  
  serviceFilesPrefix = serviceFilesPath + serviceFilesPrefix;
    
  if (GTMNameService::instance()->init(serviceFilesPrefix.c_str()) < 0)
  {
    LOG_ERROR(LOFAR::formatString ( 
        "Could not open NameService configuration file: %s.ns",
        serviceFilesPrefix.c_str()));
    _doExit = true;
  }
  if (GTMTopologyService::instance()->init(serviceFilesPrefix.c_str()) < 0)
  {
    LOG_ERROR(LOFAR::formatString ( 
        "Could not open TopologyService configuration file: %s.top",
        serviceFilesPrefix.c_str()));
    _doExit = true;
  }
  if (_doExit)
    exit(-1);
}

void GCFTask::run()
{
  signal(SIGINT,  GCFTask::signalHandler);
  signal(SIGTERM, GCFTask::signalHandler);
  signal(SIGPIPE, SIG_IGN);

  while (!_doExit)
  {
    vector<GCFHandler*> tempHandlers(_handlers);

    for (THandlerIter iter = tempHandlers.begin() ;
          iter != tempHandlers.end() && !_doExit; 
          ++iter)
    {
      (*iter)->workProc();
    }
  }
  stop();
}

void GCFTask::start()
{
    initFsm();
}

void GCFTask::stop()
{
  if (_doExit)
  {
    vector<GCFHandler*> tempHandlers(_handlers);
  
    for (THandlerIter iter = tempHandlers.begin() ;
          iter != tempHandlers.end() ; 
          ++iter)
    {
      (*iter)->stop();
    } 
  }
  else
  {
    _doExit = true;
  }     
}

void GCFTask::registerHandler(GCFHandler& handler)
{
  _handlers.push_back(&handler);
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
  const char* signame;
  
  signame = _protocols[F_EVT_PROTOCOL(e)][F_EVT_SIGNAL(e)];

  return (signame?signame:unknown);
}

void GCFTask::signalHandler(int sig)
{
  if ( (sig == SIGINT) || (sig == SIGTERM) )
    _doExit = true;
}                                            
