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
//#  $Id$

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <APL/TBB_Protocol/TP_Protocol.ph>
#include <APL/TBB_Protocol/TBB_Protocol.ph>

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
			m_SendMask = 0;
		}	break;			
		
		case TBB_ALLOC:
		{
			TBBAlloc alloc(e);
			alloc.tbbmask ...
			}	
		case TBB_FREE:
		{
			TBBFree free(e);
			free.kljlkjlkj
					}
		case TBB_RECORD: 
		case TBB_STOP:
		case TBB_TRIGCLR:
		case TBB_READ:
		case TBB_UDP:
		case TBB_VERSION:
		case TBB_SIZE:
		case TBB_CLEAR:
		case TBB_RESET:
		case TBB_CONFIG:
		{
			if(Cmd.isValid(event)) // isValid returns true if event is a valid cmd
			{
				m_ClientEvent = event;
				m_ClientPort = port;
				m_SendMask = event.tbbmask;
				m_TpEvent = Cmd.getTpEvent(m_ClientEvent);
				sendToBoards();
			}
		} break;
		
		case TBB_ERASEF:
		case TBB_READF:
		case TBB_WRITEF:
		case TBB_READW:
		case TBB_WRITEW:
		{
			if(Cmd.isValid(event)) // isValid returns true if event is a valid cmd
			{
				m_ClientEvent = event;
				m_ClientPort = port;
				m_SendMask = getMask(event.boardid);
				m_TpEvent = Cmd.getTpEvent(event);
				sendToBoards();
			}
		} break;
						
		default: {
				status = GCFEvent::NOT_HANDLED;
    } break;
	}
	
	// board_send_mask must be cleared if command tbb_sendack_state is done
	if(m_SendMask)
	{
		// TODO Set time-out Timer
		TRAN(BoardCmdHandler::waitack_state);
	}
	return status;
}


GCFEvent::TResult BoardCmdHandler::waitack_state(GCFEvent& event, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
	  
	switch(event.signal)
	{
  	case F_ENTRY: {
			m_RecvMask = 0;
		}	break;
		    
		case F_TIMER: {
			m_ClienPort.send(getTbbAckEvent());
			TRAN(BoardCmdHandler::send_state);
		}	break;
		
    case F_EXIT: {
		} break;
      
    default: {
			if(Cmd.isValid(event))
			{
				saveTpAckEvent(event,port);
			}
			else
			{
				status = GCFEvent::NOT_HANDLED;
			}
		}	break;
	}
	 
	if(m_SendMask == m_RecvMask)
	{
		m_ClienPort.send(getTbbAckEvent());
		TRAN(BoardCmdHandler::send_state);
	}
	return status;
}

bool SetCmd(Command cmd)
{
	m_cmd = cmd;
}

void BoardCmdHandler::sendToBoards(void)
{
	// send command to all the boards in the send-mask
	for (int boardnr = 0;boardnr < nr_of_boards;boardnr++)
	{
		if(user_event.tbbmask & (1 << boardnr))
		{
			board_port[boardnr].send(m_TpEvent);
		}
	}
}

uint32 BoardCmdHandler::getMask(uint32 boardid)
{
	// send command to all the boards in the send-mask
	for (int boardnr = 0;boardnr < m_nr_of_boards;boardnr++)
	{
		if(boardid == board_id[boardnr]))
		{
			return (1 << boardnr);
		}
	}
}

