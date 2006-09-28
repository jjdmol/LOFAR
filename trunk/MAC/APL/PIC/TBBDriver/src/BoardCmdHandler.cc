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

namespace LOFAR {
	using namespace TBB_Protocol;
	namespace TBB {

BoardCmdHandler::BoardCmdHandler()
  : GCFFsm((State)&BoardCmdHandler::send_state)
{
	itsRetries 		= 10;
	itsNrOfBoards	= 12;
	itsActiveBoards = 0;
	itsMaxBoards	= 12;
	itsTimeOut		= 0.2;
	itsCmdDone		= true;
	itsClientPort	= 0;
	itsBoardPorts	= 0;
	
	for (int boardnr = 0; boardnr < itsMaxBoards; boardnr++) {
		itsBoardRetries[boardnr] = 0;
	}
	
	itsCmd = 0;		
}

BoardCmdHandler::~BoardCmdHandler()
{
		
}

GCFEvent::TResult BoardCmdHandler::send_state(GCFEvent& event, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
	uint32		boardmask;
			
  switch (event.signal)	{
  	
  	case F_INIT: {
  	} break;
  	
  	case F_ENTRY: {
  		for (int boardnr = 0; boardnr < itsMaxBoards; boardnr++) {
				itsBoardRetries[boardnr] = 0;
			}
		} break;			
	  
    case F_EXIT: {
  	} break;

		default: {
			if(itsCmd && itsCmd->isValid(event)) { // isValid returns true if event is a valid cmd
				itsClientPort = &port;
				itsCmd->saveTbbEvent(event,itsActiveBoards);
				
				if(itsCmd->getSendMask()) {				
					itsCmdDone = false;
					for(int boardnr = 0; boardnr < itsMaxBoards; boardnr++) {
						boardmask = (1 << boardnr);

						LOG_DEBUG_STR(formatString("TP sending boardmask    [0x%08X]", boardmask));
						LOG_DEBUG_STR(formatString("TP sending activeboards [0x%08X]", itsActiveBoards));
						LOG_DEBUG_STR(formatString("TP sending sendmask     [0x%08X]", itsCmd->getSendMask()));

						if((boardmask & itsActiveBoards) && (boardmask & itsCmd->getSendMask())) {
								
							if(itsBoardPorts[boardnr].isConnected()) {
								itsCmd->sendTpEvent(itsBoardPorts[boardnr]);
								if(itsCmd->waitAck()) 
									itsBoardPorts[boardnr].setTimer(itsTimeOut);
							}
							else {
								itsCmd->portError(boardnr);
							}
							itsBoardRetries[boardnr]++;
						}
					}
				}
				if(itsCmd->waitAck()) {					
					TRAN(BoardCmdHandler::waitack_state);
				}
				else {
					itsCmd->sendTbbAckEvent(itsClientPort);
					itsCmdDone = true;
					itsCmd = 0;
				}
			}
			else {
				status = GCFEvent::NOT_HANDLED;
			}
    } break;
	}
	return status;
}
 

GCFEvent::TResult BoardCmdHandler::waitack_state(GCFEvent& event, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
	  
	switch(event.signal) {
  	case F_INIT: {
		}	break;
  	
  	case F_ENTRY: {
			if(itsCmd->done()) {
				itsCmd->sendTbbAckEvent(itsClientPort);
				itsCmdDone = true;
				itsCmd = 0;
				TRAN(BoardCmdHandler::send_state);	
			}
		}	break;
		    
		case F_TIMER: {
			// time-out, retry 
			if(itsBoardRetries[portToBoardNr(port)] < itsRetries) {
				if(port.isConnected()) {
					itsCmd->sendTpEvent(port);
					port.setTimer(itsTimeOut);
				}
				itsBoardRetries[portToBoardNr(port)]++;
				LOG_DEBUG_STR(formatString("itsRetries[%d] = %d", portToBoardNr(port), itsBoardRetries[portToBoardNr(port)]));	
			}
			else {
				// max retries, save zero's
				itsCmd->saveTpAckEvent(event,portToBoardNr(port));
			}
			
			if(itsCmd->done()) {
				itsCmd->sendTbbAckEvent(itsClientPort);
				itsCmdDone = true;
				itsCmd = 0;
				TRAN(BoardCmdHandler::send_state);
			}
		}	break;
		
    case F_EXIT: {
		} break;
      
    default: {
			if(itsCmd->isValid(event)) {
				port.cancelAllTimers();
				itsCmd->saveTpAckEvent(event,portToBoardNr(port));
				
				if(itsCmd->done()) {
					itsCmd->sendTbbAckEvent(itsClientPort);
					itsCmdDone = true;
					itsCmd = 0;
					TRAN(BoardCmdHandler::send_state);	
				}
			}
			else {
				status = GCFEvent::NOT_HANDLED;
			}
		}	break;
	}
	return status;
}

void BoardCmdHandler::setBoardPorts(GCFPortInterface* board_ports)
{
	itsBoardPorts	= board_ports; // save address of boards array		
}

void BoardCmdHandler::setTpCmd(Command *cmd)
{
	itsCmd = cmd;
}

void BoardCmdHandler::setTpRetries(int32 Retries)
{
	itsRetries = Retries;
}

void BoardCmdHandler::setNrOfTbbBoards(int32 NrOfBoards)
{
	itsNrOfBoards = NrOfBoards;
}

void BoardCmdHandler::setActiveBoards(int32 ActiveBoards)
{
	itsActiveBoards = ActiveBoards;
}
	
void BoardCmdHandler::setMaxTbbBoards(int32 MaxBoards)
{
	itsMaxBoards = MaxBoards;
}
				
void BoardCmdHandler::setTpTimeOut(double TimeOut)
{
	itsTimeOut = TimeOut;
}

bool BoardCmdHandler::tpCmdDone()
{
	return itsCmdDone;
}

int32 BoardCmdHandler::portToBoardNr(GCFPortInterface& port)
{
	int32 boardnr;
	
	for(boardnr = 0; boardnr < itsMaxBoards; boardnr++) {
		if(&port == &itsBoardPorts[boardnr]) { 				
			return boardnr;
		}
	}	
	return -1;	
}
	
	} // end namespace TBB
} // end namespace LOFAR
