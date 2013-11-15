//#  GCF_SysConnGuard.cc:
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
//#  MERCHANTABILITY or FITNESS FOR A PMRTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <lofar_config.h>

#define LOFARLOGGER_SUBPACKAGE "SAL"

#include <GCF/PAL/GCF_SysConnGuard.h>
#include "GSA_SysConnGuard.h"
#include <GCF/TM/GCF_Task.h>
#include <strings.h>
#include <stdio.h>
#include <unistd.h>

namespace LOFAR {
 namespace GCF {
using namespace TM;
  namespace PAL {
/**
 * F_SCG_PROTOCOL signal names
 */
const char* F_SCG_PROTOCOL_signalnames[] =
{
  "F_SCG_PROTOCOL: invalid signal",
  "F_SYSGONE (IN)",
  "F_SYSCONN (IN)",
};

GCFSysConnGuard* GCFSysConnGuard::_instance = 0;

GCFSysConnGuard::GCFSysConnGuard() :
  _pSysConnGuardService(0),
  _dummyPort(0, "sysconnguard", 0)
{
  _pSysConnGuardService = new GSASysConnGuard(*this);
}

GCFSysConnGuard::~GCFSysConnGuard()
{
  if (_pSysConnGuardService)
    delete _pSysConnGuardService;
  _instance = 0;
}

GCFSysConnGuard* GCFSysConnGuard::instance ()
{
  if (_instance == 0) {
    _instance = new GCFSysConnGuard();
  }
  ASSERT(_instance);
  return _instance;
}

bool GCFSysConnGuard::isRunning() const
{
  ASSERT(_pSysConnGuardService);
  return _pSysConnGuardService->isSubscribed();
}

bool GCFSysConnGuard::registerTask(GCFTask& task)
{
  if (_registeredTasks.find(&task) != _registeredTasks.end()) {
    LOG_WARN(formatString(
        "Task '%s' is already registered in the System Connection Guard",
        task.getName().c_str()));
    return false;
  }
  else {
    if (_registeredTasks.empty()) {
      ASSERT(_pSysConnGuardService);
      _pSysConnGuardService->start();
    }
    _registeredTasks.insert(&task);
    return true;
  }
}

bool GCFSysConnGuard::unregisterTask(GCFTask& task)
{
  if (_registeredTasks.find(&task) != _registeredTasks.end()) {
    LOG_WARN(formatString(
        "Task '%s' was not registered in the System Connection Guard",
        task.getName().c_str()));
    return false;
  }
  else {
    _registeredTasks.erase(&task);
    if (_registeredTasks.empty()) {
      ASSERT(_pSysConnGuardService);
      _pSysConnGuardService->stop();
    }
    return true;
  }
}

void GCFSysConnGuard::serviceEvent(const string& sysName, bool gone)
{
  GCFSysConnGuardEvent e((gone ? F_SYSGONE : F_SYSCONN));
  e.pSysName = sysName.c_str();
  LOG_DEBUG(formatString("Remote PVSS system '%s' is %s!",
			  e.pSysName, (gone ? "gone" : "connected")));
  for (TRegisteredTasks::iterator iter = _registeredTasks.begin();
       iter != _registeredTasks.end(); ++iter) {
    (*iter)->dispatch(e, _dummyPort); 
  }
}
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR

