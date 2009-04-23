//#  BoardCmdHandler.cc: implementation of the BoardCmdHandler class
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
//#  $Id: BoardCmdHandler.cc,v 1.1 2006/07/13 14:22:09 donker Exp 

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <GCF/TM/GCF_ETHRawPort.h>
#include <GCF/TM/GCF_DevicePort.h>
#include <GCF/TM/GCF_TimerPort.h>

#include <APL/TBB_Protocol/TBB_Protocol.ph>
#include "BoardCmdHandler.h"

using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using	namespace TBB;

BoardCmdHandler::BoardCmdHandler(GCFTimerPort* port):
	GCFTask((State)&BoardCmdHandler::idle_state, "BoardHandler")
{
	itsSleepTimer = port;
	TS = TbbSettings::instance();
	itsDone = true;
	itsRetries = 0;
	itsClientPort	= 0;
	itsCmd = 0;
}

BoardCmdHandler::~BoardCmdHandler()
{
	delete itsSleepTimer;
}


GCFEvent::TResult BoardCmdHandler::idle_state(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	switch (event.signal) {
		case F_INIT: {
		} break;

		case F_ENTRY: {
			//itsCmd = 0;
			if (itsCmd) delete itsCmd;
		} break;			

		case F_TIMER: {
		} break;
	  	
		case F_EXIT: {
		} break;

		default: {
			if (itsCmd && itsCmd->isValid(event)) { // isValid returns true if event is a valid cmd
				LOG_DEBUG_STR("==== NEW CMD ==================================================");
				itsClientPort = &port;
				itsDone = false;
				itsCmd->reset();
				itsCmd->saveTbbEvent(event);
				TRAN(BoardCmdHandler::send_state);
			}	else {
				status = GCFEvent::NOT_HANDLED;
			}
		} break;
	}
	return(status);
}

GCFEvent::TResult BoardCmdHandler::send_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	switch (event.signal)	{
		case F_INIT: {
		} break;

		case F_ENTRY: {
			if (itsCmd != 0) {
				if (itsDone) {
					itsCmd->sendTbbAckEvent(itsClientPort);
					TRAN(BoardCmdHandler::idle_state);
				} else {
					itsRetries = 0;
					itsCmd->sendTpEvent();
					TRAN(BoardCmdHandler::waitack_state);	
				}
			}
		} break;
		
		case F_TIMER: {
		} break;			
		
		case F_EXIT: {
		} break;

		default: {
		} break;
	}
	return(status);
}


GCFEvent::TResult BoardCmdHandler::waitack_state(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	switch(event.signal) {
		case F_INIT: {
		} break;

		case F_ENTRY: {
			// if cmd returns no ack or board/channel is not selected or not responding return to send_state		
			if (!itsCmd->waitAck()) {
				TRAN(BoardCmdHandler::send_state);
			}
		} break;

		case F_TIMER: {
			if (&port == itsSleepTimer) {
				TRAN(BoardCmdHandler::send_state);	
				break;
			}
			
			// time-out, retry
			if ((itsRetries < TS->maxRetries()) && (itsCmd->retry())) { 
				LOG_DEBUG("=TIME-OUT=");
				itsCmd->sendTpEvent();
				itsRetries++;
				LOG_DEBUG_STR(formatString("itsRetries[%d] = %d", itsCmd->getBoardNr(), itsRetries));	
			} else {
				if (itsRetries == TS->maxRetries()) {
					TS->setBoardState(itsCmd->getBoardNr(),boardError);
				}
				itsCmd->saveTpAckEvent(event); // max retries or done, save zero's
				if (itsCmd->getSleepTime() > 0.0) {
					itsSleepTimer->setTimer(itsCmd->getSleepTime());
					itsCmd->setSleepTime(0.0);
				} else {
					if (itsCmd->isDone()) {
						itsDone = true;
					}
					TRAN(BoardCmdHandler::send_state);
				}
			}	
		} break;
		
		case F_EXIT: {
		} break;

		default: {
			LOG_DEBUG_STR("default cmd");
			if (itsCmd->isValid(event)) {
				port.cancelAllTimers();
				itsCmd->saveTpAckEvent(event);
				if (itsCmd->getSleepTime() > 0.0) {
					itsSleepTimer->setTimer(itsCmd->getSleepTime());
					itsCmd->setSleepTime(0);
				} else {
					if (itsCmd->isDone()) {
						itsDone = true;
					}
					TRAN(BoardCmdHandler::send_state);
				}
			} else {
				status = GCFEvent::NOT_HANDLED;
			}
		}	break;
	}
	return(status);
}
	
void BoardCmdHandler::setTpCmd(Command *cmd)
{
	itsCmd = cmd;
}

bool BoardCmdHandler::tpCmdDone()
{
	return (itsDone);
}
