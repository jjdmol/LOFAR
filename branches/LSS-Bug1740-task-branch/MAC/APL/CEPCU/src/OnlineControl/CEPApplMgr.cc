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
#include <Common/StringUtil.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <GCF/TM/GCF_Scheduler.h>
#include "CEPApplMgr.h"

namespace LOFAR {
  using namespace GCF::TM;
  using namespace ACC::ALC;
  using namespace APLCommon;
  namespace CEPCU {

     
//
// CEPApplMgr(interface, appl)
//
CEPApplMgr::CEPApplMgr(CEPApplMgrInterface& interface, 
					   const string& 		appName,
					   uint32				expRuntime,
					   const string&		acdHost,
					   const string&		paramFile) :
	itsProcName	   (appName),
	itsParamFile   (paramFile),
	itsCAMInterface(interface),
	//			   (nrProcs, expectedlifetime,activityLevel,architecture);
	itsACclient	   (this, appName, 10, expRuntime, 1, 0, acdHost),
	itsReqState	   (CTState::NOSTATE),
	itsCurState	   (CTState::NOSTATE),
	itsContinuePoll(true)
{ 
	use(); // to avoid that this object will be deleted in GCFTask::stop;
}

//
// ~CEPApplMgr()
//
CEPApplMgr::~CEPApplMgr()
{
	GCFScheduler::instance()->deregisterHandler(*this);
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
// Translate ACC ack into MAC state.
//
void  CEPApplMgr::handleAckMsg(ACCmd         cmd, 
                               uint16        ACCresult,
                               const string& info)
{
	LOG_INFO(formatString("command: %d, result: %d, info: %s", cmd, ACCresult, info.c_str()));

	uint16	MACresult = (ACCresult & AcCmdMaskOk) ? 
											CT_RESULT_NO_ERROR : CT_RESULT_UNSPECIFIED;

	switch (cmd) {
	case ACCmdBoot:
		if (ACCresult == AcCmdMaskOk) {
			itsCurState = CTState::CONNECTED;
		}
		itsCAMInterface.appSetStateResult(itsProcName, CTState::CONNECT, MACresult);
	break;

	case ACCmdDefine:
		if (ACCresult == AcCmdMaskOk) {
			itsCurState = CTState::CLAIMED;
		}
		itsCAMInterface.appSetStateResult(itsProcName, CTState::CLAIM, MACresult);
	break;

	case ACCmdInit:
		if (ACCresult == AcCmdMaskOk) {
			itsCurState = CTState::PREPARED;
		}
		itsCAMInterface.appSetStateResult(itsProcName, CTState::PREPARE, MACresult);
	break;

	case ACCmdRun:
		if (ACCresult == AcCmdMaskOk) {
			itsCurState = CTState::RESUMED;
		}
		itsCAMInterface.appSetStateResult(itsProcName, CTState::RESUME, MACresult);
	break;

	case ACCmdPause:
		if (ACCresult == AcCmdMaskOk) {
			itsCurState = CTState::SUSPENDED;
		}
		itsCAMInterface.appSetStateResult(itsProcName, CTState::SUSPEND, MACresult);
	break;

	case ACCmdRelease:
		if (ACCresult == AcCmdMaskOk) {
			itsCurState = CTState::RELEASED;
		}
		itsCAMInterface.appSetStateResult(itsProcName, CTState::RELEASE, MACresult);
	break;

	case ACCmdQuit:
		if (ACCresult == AcCmdMaskOk) {
//			itsContinuePoll = false;
			itsCurState = CTState::QUITED;
		}
		itsCAMInterface.appSetStateResult(itsProcName, CTState::QUIT, MACresult);
	break;

	default:
		LOG_WARN_STR("Received command = " << cmd << ", result = " << ACCresult
						<< ", info = " << info << " not handled!");
		break;
	}
}                                                

//
// sendCommand (newState, options)
//
// Translate MAC commands into ACC commands.
//
void CEPApplMgr::sendCommand (CTState::CTstateNr	newState, const string&		options)
{
	switch (newState) {
	case CTState::CONNECT:
		itsACclient.boot(0, itsParamFile);
		break;
	case CTState::CLAIM:
		itsACclient.define(0);
		break;
	case CTState::PREPARE:
		itsACclient.init(0);
		break;
	case CTState::RESUME:
		itsACclient.run(0);
		break;
	case CTState::SUSPEND:
		itsACclient.pause(0, 0, options);
		break;
	case CTState::RELEASE:
		itsACclient.release(0);
		break;
	case CTState::QUIT:
		itsACclient.quit(0);
		break;
	default:
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
