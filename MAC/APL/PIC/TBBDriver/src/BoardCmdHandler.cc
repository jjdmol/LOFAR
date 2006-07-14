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

using namespace LOFAR;
using namespace TBB;

#define N_RETRIES 10

BoardCmdHandler::BoardCmdHandler()
  : GCFFsm((State)&BoardCmdHandler::tp_send_state),
    m_retries(0),
		m_nr_of_boards(12)
{
}

BoardCmdHandler::~BoardCmdHandler()
{	
}

GCFEvent::TResult BoardCmdHandler::send_state(GCFEvent& event, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
			
  switch (event.signal)
	{
  	case F_ENTRY: {
		}	break;			
		
		default: {
			if(m_Cmd.isValid(event)) // isValid returns true if event is a valid cmd
			{
				m_Cmd.sendTpEvent(board_port);
				// TODO set timeout timer
				TRAN(BoardCmdHandler::waitack_state);
			}
			else
			{
				status = GCFEvent::NOT_HANDLED;
			}
    } break;
	}
	return status;
}


GCFEvent::TResult BoardCmdHandler::waitack_state(GCFEvent& event, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
	  
	switch(event.signal)
	{
  	case F_ENTRY: {
		}	break;
		    
		case F_TIMER: {
			sendTbbAckEvent();
			TRAN(BoardCmdHandler::send_state);
		}	break;
		
    case F_EXIT: {
		} break;
      
    default: {
			if(Cmd.isValid(event))
			{
				if(saveTpAckEvent(event,port)) // if saveTpAckEvent() returns true, all data is received
				{
					sendTbbAckEvent();
					TRAN(BoardCmdHandler::send_state);	
				}
			}
			else
			{
				status = GCFEvent::NOT_HANDLED;
			}
		}	break;
	}
	return status;
}

void BoardCmdHandler::setBoardPorts(GCFPortInterface& boards)
{
	m_BoardPort = boards;		
}

bool BoardCmdHandler::SetCmd(Command cmd)
{
	m_Cmd = cmd;
}


