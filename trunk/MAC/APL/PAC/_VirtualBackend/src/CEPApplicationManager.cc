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

#include "CEPApplicationManager.h"


namespace LOFAR
{

using namespace ACC;
  
  namespace AVB
  {
INIT_TRACER_CONTEXT(CEPApplicationManager, LOFARLOGGER_PACKAGE);

     
void CEPApplicationManager::workProc()
{
  if (_continuePoll)
  {
    _acClient.processACmsgFromServer();
  }
}

void  CEPApplicationManager::handleAckMsg      (ACCmd         cmd, 
                                                uint16        result,
                                                const string& info)
{
  LOG_INFO(formatString("command: %d, result: %d, info: %s", cmd, result, info.c_str()));
  switch (cmd)
  {
    case ACCmdBoot:
      _interface.appBooted(result);
      break;
    case ACCmdQuit:
      if (result == AcCmdMaskOk)
      {
        _continuePoll = false;
      }
      _interface.appQuitDone(result);
      break;
    case ACCmdDefine:
      _interface.appDefined(result);
      break;
    case ACCmdInit:
      _interface.appInitialized(result);
      break;
    case ACCmdPause:
      _interface.appPaused(result);
      break;
    case ACCmdRun:
      _interface.appRunDone(result);
      break;
    case ACCmdSnapshot:
      _interface.appSnapshotDone(result);
      break;
    case ACCmdRecover:
      _interface.appRecovered(result);
      break;
    case ACCmdReinit:
      _interface.appReinitialized(result);
      break;
    case ACCmdReplace:
      _interface.appReplaced(result);
      break;
    default:
      LOG_WARN_STR("Received command = " << cmd << ", result = " << result
          << ", info = " << info << "not handled!");
      break;
  }
}                                                

void  CEPApplicationManager::handleAnswerMsg   (const string& answer)
{
  _interface.appSupplyInfoAnswer(answer);
}

string  CEPApplicationManager::supplyInfoFunc  (const string& keyList)
{
  return _interface.appSupplyInfo(keyList);
}

  } // namespace AVB
} // namespace LOFAR
