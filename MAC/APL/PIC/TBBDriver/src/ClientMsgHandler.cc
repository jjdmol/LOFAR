//#  ClientMsgHandler.cc: implementation of the ClientMsgHandler class
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
//#  $Id: ClientMsgHandler.cc,v 1.1 2006/07/13 14:22:09 donker Exp 

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include "ClientMsgHandler.h"

namespace LOFAR {
	namespace TBB {

#define N_RETRIES 10

ClientMsgHandler::ClientMsgHandler(GCFPortInterface& port)
  : GCFFsm((State)&ClientMsgHandler::send_state),
    itsRetries(0), itsMsgPort(&port)
{
}

ClientMsgHandler::~ClientMsgHandler()
{	
}

GCFEvent::TResult ClientMsgHandler::send_state(GCFEvent& event, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
			
  switch (event.signal)	{
  	case F_ENTRY: {
		}	break;			
		
		default: {
			if(itsMsg->isValid(event)) { // isValid returns true if event is a valid cmd
				itsBoardPort = &port;
				itsMsg->saveTpEvent(event);
				itsMsg->makeTbbEvent();
				itsMsg->sendTbbEvent(*itsMsgPort);
				TRAN(ClientMsgHandler::waitack_state);
			}
			else {
				status = GCFEvent::NOT_HANDLED;
			}
    } break;
	}
	return status;
}


GCFEvent::TResult ClientMsgHandler::waitack_state(GCFEvent& event, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
	  
	switch(event.signal) {
  	case F_ENTRY: {
		}	break;
		    
		case F_TIMER: {
			// time-out, retry 
			if(itsRetries < N_RETRIES) {
				itsMsg->sendTbbEvent(port);	
			}
			else {
				TRAN(ClientMsgHandler::send_state);
			}
		}	break;
		
    case F_EXIT: {
		} break;
      
    default: {
			if(itsMsg->isValid(event)) {
					itsMsg->sendTpEvent(*itsBoardPort);
					TRAN(ClientMsgHandler::send_state);	
			}
			else {
				status = GCFEvent::NOT_HANDLED;
			}
		}	break;
	}
	return status;
}

void ClientMsgHandler::SetMessage(Message *msg)
{
	itsMsg = msg;
}
	} // end TBB namespace
} // end LOFAR namespace