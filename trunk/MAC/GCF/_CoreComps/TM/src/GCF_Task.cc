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

#include <GCF_Task.h>
#include <PortInterface/GCF_PortInterface.h>
#include <GCF_TMProtocols.h>
#include <GCF_Handler.h>

#include <stdio.h>

bool GCFTask::_doExit = false;

GCFTask::GCFTask(State initial, string& name) :
  GCFFsm(initial), _name(name)
{
    registerProtocol(F_FSM_PROTOCOL, F_FSM_PROTOCOL_names);
    registerProtocol(F_PORT_PROTOCOL, F_PORT_PROTOCOL_names);
}


GCFTask::~GCFTask()
{
}

void GCFTask::run()
{
    while (!_doExit)
    {
        for (vector<THandler>::iterator iter = _handlers.begin() ;
                iter != _handlers.end() ; 
                ++iter)
        {
            iter->pHandler->workProc();
        }
    }
}

void GCFTask::start()
{
    initFsm();
}

void GCFTask::stop()
{
    for (vector<THandler>::iterator iter = _handlers.begin() ;
           iter != _handlers.end() ; 
           ++iter)
    {
        iter->pHandler->stop();
    }  
    _doExit = true;
}

void GCFTask::registerHandler(GCFHandler& handler)
{
    THandler handlerEntry;
    handlerEntry.pHandler = &handler;
    _handlers.insert(&handlerEntry);
}

void GCFTask::registerProtocol(unsigned short protocol_id,
              const char* signal_names[])
{
}

void GCFTask::debug_signal(const GCFEvent& e, GCFPortInterface& p, const char* info)
{
}

const char* GCFTask::evtstr(const GCFEvent& e)
{
    return 0;
}
