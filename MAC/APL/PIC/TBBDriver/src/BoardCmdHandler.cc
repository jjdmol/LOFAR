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

#include <APL/TBB_Protocol/TBB_Protocol.ph>
#include "BoardCmdHandler.h"
#include "DriverSettings.h"

namespace LOFAR {
	using namespace TBB_Protocol;
	namespace TBB {

BoardCmdHandler::BoardCmdHandler()
  : GCFFsm((State)&BoardCmdHandler::send_state)
{
	itsMaxRetries = 10;
	itsRetries = 0;
	itsNrOfBoards	= 0;
	itsNrOfChannels = 0;
	itsCmdType = BoardCmd;
	itsBoardNr = 0;
	itsChannelNr = 0;
	itsNextCmd = false;
	itsClientPort	= 0;
	itsCmd = 0;		
}

BoardCmdHandler::~BoardCmdHandler() {  }

GCFEvent::TResult BoardCmdHandler::send_state(GCFEvent& event, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
				
  switch (event.signal)	{
  	
  	case F_INIT: {
  	} break;
  	
  	case F_ENTRY: {
			if (itsCmd) {
				sendCmd(); 
				TRAN(BoardCmdHandler::waitack_state);
			}
		} break;			
	  
    case F_EXIT: {
  	} break;

		default: {
			
			if (itsCmd && itsCmd->isValid(event)) { // isValid returns true if event is a valid cmd
				itsClientPort = &port;
				itsCmd->saveTbbEvent(event);
				sendCmd();
				TRAN(BoardCmdHandler::waitack_state);
			}
			else {
				status = GCFEvent::NOT_HANDLED;
			}
    } break;
	}
	return(status);
}
 

GCFEvent::TResult BoardCmdHandler::waitack_state(GCFEvent& event, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
	  
	switch(event.signal) {
  	case F_INIT: {
		}	break;
  	
  	case F_ENTRY: {
			// if cmd returns no ack or board/channel is not selected or not responding return to send_stae		
			if (!itsCmd->waitAck() || itsNextCmd) {
				nextCmd();
				TRAN(BoardCmdHandler::send_state);
			}
		}	break;
		    
		case F_TIMER: {
			// time-out, retry 
			if (itsRetries < itsMaxRetries) {
				sendCmd();
				LOG_DEBUG_STR(formatString("itsRetries[%d] = %d", itsBoardNr, itsRetries));	
			}
			else {
				itsCmd->saveTpAckEvent(event,itsBoardNr); // max retries, save zero's
				nextCmd();
			}
			TRAN(BoardCmdHandler::send_state);
		}	break;
		
    case F_EXIT: {
		} break;
      
    default: {
			if (itsCmd->isValid(event)) {
				port.cancelAllTimers();
				itsCmd->saveTpAckEvent(event,itsBoardNr);
				nextCmd();
				TRAN(BoardCmdHandler::send_state);
			}
			else {
				status = GCFEvent::NOT_HANDLED;
			}
		}	break;
	}
	return(status);
}

void BoardCmdHandler::sendCmd()
{
	itsNextCmd = true; // if true, go to next board or channel
	uint32 boardmask = (1 << itsBoardNr);
		
	if (boardmask & DriverSettings::instance()->activeBoardsMask()) {
		if (itsCmd->getCmdType() == BoardCmd) {
			if (boardmask & itsCmd->getBoardMask()) {
				if (itsCmd->sendTpEvent(itsBoardNr, itsChannelNr)) {
					itsRetries++;
					itsNextCmd = false;
				}
			}	
		}	
		if (itsCmd->getCmdType() == ChannelCmd) {
			if (DriverSettings::instance()->getChSelected(itsChannelNr)) {
				if (itsCmd->sendTpEvent(itsBoardNr, itsChannelNr)) {
					itsRetries++;
					itsNextCmd = false;
				}
			}
		}
	}
}

void BoardCmdHandler::nextCmd()
{
	if (itsCmd->getCmdType() == BoardCmd) {
		itsBoardNr++;
	}
	if (itsCmd->getCmdType() == ChannelCmd) {
		itsChannelNr++;
		itsBoardNr = DriverSettings::instance()->getChBoardNr(itsChannelNr);
	}
	// if all command done send ack meassage to client and clear all variables
	if ((itsBoardNr == DriverSettings::instance()->maxBoards()) ||
		  (itsChannelNr == DriverSettings::instance()->maxChannels())) {
		itsCmd->sendTbbAckEvent(itsClientPort);
		itsCmd = 0;
		itsBoardNr = 0;
		itsChannelNr = 0;
		itsRetries = 0;	
	}	
}
	
void BoardCmdHandler::setTpCmd(Command *cmd)
{
	itsCmd = cmd;
}

void BoardCmdHandler::setTpRetries(int32 Retries)
{
	itsMaxRetries = Retries;
}

bool BoardCmdHandler::tpCmdDone()
{
	if (itsCmd == 0) return(true);
	return(false);
}

	} // end namespace TBB
} // end namespace LOFAR
