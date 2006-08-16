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

#include "BoardCmdHandler.h"

namespace LOFAR {
	namespace TBB {

#define N_RETRIES 10

BoardCmdHandler::BoardCmdHandler(GCFPortInterface* board_ports)
  : GCFFsm((State)&BoardCmdHandler::send_state),
    itsRetries(0),
		itsNrOfBoards(12)
{
	itsBoardPorts = board_ports; // save address of boards array		
}

BoardCmdHandler::~BoardCmdHandler()
{
		
}

GCFEvent::TResult BoardCmdHandler::send_state(GCFEvent& event, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
			
  switch (event.signal)	{
  	case F_ENTRY: {
		}	break;			
		
		default: {
			if(itsCmd->isValid(event)) { // isValid returns true if event is a valid cmd
				itsClientPort = &port;
				itsCmd->saveTbbEvent(event);
				itsCmd->makeTpEvent();
				
				for(int boardid = 0;boardid < itsNrOfBoards;boardid++) {
					if((1 << boardid) & itsCmd->getSendMask()) {
						itsCmd->sendTpEvent(itsBoardPorts[boardid]);
					}
				}
				TRAN(BoardCmdHandler::waitack_state);
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
  	case F_ENTRY: {
		}	break;
		    
		case F_TIMER: {
			// time-out, retry 
			if(itsBoardRetries[PortToBoardNr(port)] < N_RETRIES) {
				itsCmd->sendTpEvent(port);	
			}
			else {
				// max retries, save zero's
				itsCmd->saveTpAckEvent(event,PortToBoardNr(port));
			}
			
			if(itsCmd->done()) {
				itsCmd->sendTbbAckEvent(itsClientPort);
				TRAN(BoardCmdHandler::send_state);
			}
		}	break;
		
    case F_EXIT: {
		} break;
      
    default: {
			if(itsCmd->isValid(event)) {
				itsBoardPorts[PortToBoardNr(port)].cancelAllTimers();
				itsCmd->saveTpAckEvent(event,PortToBoardNr(port));
				
				if(itsCmd->done()) {
					itsCmd->sendTbbAckEvent(itsClientPort);
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

void BoardCmdHandler::SetCmd(Command *cmd)
{
	itsCmd = cmd;
}

bool BoardCmdHandler::done()
{
	return itsCmd->done();
}

int BoardCmdHandler::PortToBoardNr(GCFPortInterface& port)
{
	int boardnr;
	
	for(boardnr = 0;boardnr < itsNrOfBoards;boardnr++) {
		if(&port == &itsBoardPorts[boardnr]) { 				
			return boardnr;
		}
	}	
	return -1;	
}
	} // end TBB namespace
} // end LOFAR namespace

