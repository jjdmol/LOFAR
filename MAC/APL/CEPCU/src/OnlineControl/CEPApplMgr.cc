//#  CEPApplMgr.cc: Implementation of the Virtual CEPApplMgr task
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
#include "CEPApplMgr.h"

namespace LOFAR {
  using namespace ACC::ALC;
  namespace CEPCU {

     
//
// CEPApplMgr(interface, appl)
//
CEPApplMgr::CEPApplMgr(CEPApplMgrInterface& interface, 
											 const string& appName) :
	itsCAMInterface(interface),
	itsACclient	   (this, appName, 10, 100, 1, 0),
	itsContinuePoll(false),
	itsLastOkCmd   (ACC::ALC::ACCmdNone),
	itsProcName	   (appName)
{ 
	use(); // to avoid that this object will be deleted in GCFTask::stop;
}

//
// ~CEPApplMgr()
//
CEPApplMgr::~CEPApplMgr()
{
	GCFTask::deregisterHandler(*this);
}

//
// workProc()
//
void CEPApplMgr::workProc()
{
	if (itsContinuePoll) {
		itsACclient.processACmsgFromServer();
	}
}

//
// handleAckMsg(cmd, result, info)
//
void  CEPApplMgr::handleAckMsg(ACCmd         cmd, 
                                          uint16        result,
                                          const string& info)
{
	LOG_INFO(formatString("command: %d, result: %d, info: %s", cmd, result, info.c_str()));

	switch (cmd) {
	case ACCmdBoot:
		if (result == AcCmdMaskOk) {
			itsLastOkCmd = cmd;
		}
		itsCAMInterface.appBooted(itsProcName, result);
	break;

	case ACCmdQuit:
		if (result == AcCmdMaskOk && result == 0) {
			itsContinuePoll = false;
		}
		itsCAMInterface.appQuitDone(itsProcName, result);
	break;

	case ACCmdDefine:
		if (result == AcCmdMaskOk) {
			itsLastOkCmd = cmd;
		}
		itsCAMInterface.appDefined(itsProcName, result);
	break;

	case ACCmdInit:
		if (result == AcCmdMaskOk) {
			itsLastOkCmd = cmd;
		}
		itsCAMInterface.appInitialized(itsProcName, result);
	break;

	case ACCmdPause:
		itsCAMInterface.appPaused(itsProcName, result);
	break;

	case ACCmdRun:
		if (result == AcCmdMaskOk) {
			itsLastOkCmd = cmd;
		}
		itsCAMInterface.appRunDone(itsProcName, result);
	break;

	case ACCmdSnapshot:
		itsCAMInterface.appSnapshotDone(itsProcName, result);
	break;

	case ACCmdRecover:
		itsCAMInterface.appRecovered(itsProcName, result);
	break;

	case ACCmdReinit:
		itsCAMInterface.appReinitialized(itsProcName, result);
	break;

	case ACCmdReplace:
		itsCAMInterface.appReplaced(itsProcName, result);
	break;

	default:
		LOG_WARN_STR("Received command = " << cmd << ", result = " << result
						<< ", info = " << info << " not handled!");
		break;
	}
}                                                

//
// handleAnswerMsg(answer)
//
void  CEPApplMgr::handleAnswerMsg   (const string& answer)
{
	itsCAMInterface.appSupplyInfoAnswer(itsProcName, answer);
}

//
// supplyInfoFunc(keyList)
//
string  CEPApplMgr::supplyInfoFunc  (const string& keyList)
{
	return (itsCAMInterface.appSupplyInfo(itsProcName, keyList));
}

  } // namespace CEPCU
} // namespace LOFAR
