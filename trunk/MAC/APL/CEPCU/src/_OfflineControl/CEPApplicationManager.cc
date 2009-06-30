//#  CEPApplicationManager.cc: Implementation of the Virtual CEPApplicationManager task
//#
//#  Copyright (C) 2002-2004
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
#include "CEPApplicationManager.h"

namespace LOFAR
{

using namespace ACC::ALC;
  
  namespace CEPCU
  {
INIT_TRACER_CONTEXT(CEPApplicationManager, LOFARLOGGER_PACKAGE);

     
void CEPApplicationManager::workProc()
{
  if (_continuePoll)
  {
    _acClient.processACmsgFromServer();
  }
}

void  CEPApplicationManager::handleAckMsg(ACCmd         cmd, 
                                          uint16        result,
                                          const string& info)
{
  LOG_INFO(formatString("command: %d, result: %d, info: %s", cmd, result, info.c_str()));
  switch (cmd)
  {
    case ACCmdBoot:
      if (result == AcCmdMaskOk)
      {
        _lastOkCmd = cmd;
      }
      _interface.appBooted(_procName, result);
      break;

    case ACCmdQuit:
      if (result == AcCmdMaskOk && result == 0) {
        _continuePoll = false;
      }
      _interface.appQuitDone(_procName, result);
      break;

    case ACCmdDefine:
      if (result == AcCmdMaskOk) {
        _lastOkCmd = cmd;
      }
      _interface.appDefined(_procName, result);
      break;

    case ACCmdInit:
      if (result == AcCmdMaskOk) {
        _lastOkCmd = cmd;
      }
      _interface.appInitialized(_procName, result);
      break;

    case ACCmdPause:
      _interface.appPaused(_procName, result);
      break;

    case ACCmdRun:
      if (result == AcCmdMaskOk) {
        _lastOkCmd = cmd;
      }
      _interface.appRunDone(_procName, result);
      break;

    case ACCmdSnapshot:
      _interface.appSnapshotDone(_procName, result);
      break;

    case ACCmdRecover:
      _interface.appRecovered(_procName, result);
      break;

    case ACCmdReinit:
      _interface.appReinitialized(_procName, result);
      break;

    case ACCmdReplace:
      _interface.appReplaced(_procName, result);
      break;

    default:
      LOG_WARN_STR("Received command = " << cmd << ", result = " << result
          << ", info = " << info << " not handled!");
      break;
  }
}                                                

void  CEPApplicationManager::handleAnswerMsg   (const string& answer)
{
  _interface.appSupplyInfoAnswer(_procName, answer);
}

string  CEPApplicationManager::supplyInfoFunc  (const string& keyList)
{
  return _interface.appSupplyInfo(_procName, keyList);
}

  } // namespace CEPCU
} // namespace LOFAR
