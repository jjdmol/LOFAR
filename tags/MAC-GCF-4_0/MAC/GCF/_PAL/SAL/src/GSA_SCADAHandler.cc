//#  GSA_SCADAHandler.cc: describes the SCADA handler for connection with the 
//#                      PVSS system
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

#include "GSA_SCADAHandler.h"
#include <GSA_Resources.h>
#include <GCF/TM/GCF_Task.h>

GSASCADAHandler* GSASCADAHandler::_pInstance = 0;

GSASCADAHandler* GSASCADAHandler::instance()
{
  if (0 == _pInstance)
  {
    char buf[100];
    snprintf(buf, 100, "%s/LCU/config/config", getenv("LOFARHOME"));
    setenv("PVSS_II", buf, true);
    
    GSAResources::init(GCFTask::_argc, GCFTask::_argv);
        
    _pInstance = new GSASCADAHandler();
    assert(!_pInstance->mayDeleted());
  }

  _pInstance->use();
  return _pInstance;
}

void GSASCADAHandler::release()
{
  assert(_pInstance);
  assert(!_pInstance->mayDeleted());
  _pInstance->leave(); 
  if (_pInstance->mayDeleted())
  {
    delete _pInstance;
    assert(!_pInstance);
  }
}

GSASCADAHandler::GSASCADAHandler() :
  _running(true)
  
{  
  GCFTask::registerHandler(*this);
}

void GSASCADAHandler::stop()
{
 
  _pvssApi.stop();

  _running = false;
}
 
void GSASCADAHandler::workProc()
{ 
  if (_running) _pvssApi.workProc();
}

TSAResult GSASCADAHandler::isOperational()
{ 
  TSAResult result(SA_SCADA_NOT_AVAILABLE);
  if (_pvssApi.getManagerState() == Manager::STATE_RUNNING && _running)
    result = SA_NO_ERROR;
  return result;
}
