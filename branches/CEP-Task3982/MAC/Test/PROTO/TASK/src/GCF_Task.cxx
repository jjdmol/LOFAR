//  GCFTask.cc: task class which encapsulates a task and its behaviour as a 
//  finite state machine (FSM).
//
//  Copyright (C) 2003
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
//  $Id$

#include "GCF_Task.hxx"
#include "GCF_PortInterface.hxx"
#include "GCF_Protocols.hxx"
#include "GCF_SCADAAPI.hxx"

#include <stdio.h>

template class DynPtrArray<GCFTask::signal_table_entry>;

//
// GCFTask implementation
//

//typedef pair<const unsigned short, const char**> value_type;

//template class map<unsigned short, const char**>;

void GCFTask::run()
{
  if (_pScadaApi) _pScadaApi->init();
  // initialize the state machine: perform the initial transition
  initFsm();
  init();
  while(!_stopped)
  {
    if (_pScadaApi) _pScadaApi->workProc();
    workProc();
  }
}

void GCFTask::registerProtocol(unsigned short protocol_id,
			     const char* signal_names[],
			     int nr_signals)
{
  /**
   * nr_signals parameters should be used to do
   * range check on signal_names indexing.
   */
  nr_signals = nr_signals;

  signal_table_entry* newEntry = new signal_table_entry;
  newEntry->protID =  protocol_id;
  newEntry->sigTable = signal_names;

  _signal_name_map.updateOrInsertAndMaintainSort(newEntry);

  //_signal_name_map.push_back(e);
  //_signal_name_map[protocol_id] = signal_names;
}

void GCFTask::debug_signal(const GCFEvent& e, GCFPortInterface& p, const char* info)
{
  if (info != 0)
  {
    fprintf(stderr, "signal (%s) received on port: %s (%s)\n",
	     evtstr(e), p.getName(), info);
  }
  else
  {
    fprintf(stderr, "signal (%s) received on port: %s\n",
	     evtstr(e), p.getName());
  }
}

const char* GCFTask::evtstr(const GCFEvent& e)
{
  static const char* unknown = "unknown signal";
         const char* signame = 0;
  signal_table_entry ste;
  signal_table_entry* item;
  ste.protID = F_EVT_PROTOCOL(e);
  if ((item = _signal_name_map.findAndGetItem(&ste)) != 0)
  {
    signame = item->sigTable[F_EVT_SIGNAL(e)];
  }
  //signame = _signal_name_map[F_EVT_PROTOCOL(e)][F_EVT_SIGNAL(e)];
  
  return (signame ? signame : unknown);
}

const char* GCFTask::getName() const
{
  return _name;
}

GCFTask::GCFTask(State initial, const char* name) :
  GCFFsm(initial), _pScadaApi(0), _stopped(false)
{
  if (name != 0)
  {
    // copy in name
    _name = name;
  }

  _signal_name_map.setCompareFunction(GCFTask::sortFunc);
  
  // last argument in call to registerProtocol needs to change
  // to something sensible
  registerProtocol(F_FSM_PROTOCOL, F_FSM_PROTOCOL_names, 0);
  registerProtocol(F_PORT_PROTOCOL, F_PORT_PROTOCOL_names, 0);
  registerProtocol(F_PVSS_PROTOCOL, F_PVSS_PROTOCOL_names, 0);
}


GCFTask::~GCFTask()
{
}

void GCFTask::stop()
{
  if (_pScadaApi) _pScadaApi->stop();
  _stopped = true;
}

int GCFTask::sortFunc(const signal_table_entry *e1, const signal_table_entry *e2)
{
  if (e1->protID < e2->protID) return -1;
  if (e1->protID > e2->protID) return  1;

  return 0;
}

void GCFTask::attachScadaApi(GCFScadaApi *pScadaApi)
{
  _pScadaApi = pScadaApi;
}
