//#  WriterCmd.cc: implementation of the WriterCmd class
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

#include "WriterCmd.h"


using namespace LOFAR;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a WriterCmd object.----------------------------------------
WriterCmd::WriterCmd():
		itsBoardStatus(0)
{
	TS					= TbbSettings::instance();
	itsTPE 			= new TPWriterEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBWriterAckEvent();
	
	itsTBBackE->status_mask = 0;
	setWaitAck(true);
}
	  
//--Destructor for WriterCmd.---------------------------------------------------
WriterCmd::~WriterCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool WriterCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_WRITER)||(event.signal == TP_WRITER_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void WriterCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE	= new TBBWriterEvent(event);
		
	itsTBBackE->status_mask = 0;
	if (TS->isBoardActive(itsTBBE->board)) {	
		setBoardNr(itsTBBE->board);
	} else {
		itsTBBackE->status_mask |= TBB_NO_BOARD ;
		setDone(true);
	}
	
	// initialize TP send frame
	itsTPE->opcode			= TPWRITER;
	itsTPE->status			=	0;
	itsTPE->mp					= static_cast<uint32>(itsTBBE->mp);
	itsTPE->pid	 				= static_cast<uint32>(itsTBBE->pid);
	itsTPE->regid				= static_cast<uint32>(itsTBBE->regid);
	
	for(int an = 0; an < 3;an++) {
  		itsTPE->data[an] = itsTBBE->data[an];
  }
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void WriterCmd::sendTpEvent()
{
	//LOG_DEBUG_STR(formatString("send Writer tpevent %d %d %d %u %u %u",
	//							itsTBBE->mp,itsTBBE->pid,itsTBBE->regid,itsTBBE->data[0],itsTBBE->data[1],itsTBBE->data[2]));	
	TS->boardPort(getBoardNr()).send(*itsTPE);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void WriterCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsTBBackE->status_mask |= TBB_COMM_ERROR;
	}	else {
		itsTPackE = new TPWriterAckEvent(event);
	
		itsBoardStatus = itsTPackE->status;
		
		//LOG_DEBUG_STR(formatString("Received WriterAck from board %d [0x%08X]", getBoardNr(), itsTPackE->status));
		delete itsTPackE;
	}
	setDone(true);
}

// ----------------------------------------------------------------------------
void WriterCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	if (itsTBBackE->status_mask == 0)
			itsTBBackE->status_mask = TBB_SUCCESS;
	//LOG_DEBUG_STR(formatString("send Writer tbbackevent board %d [0x%08X]", getBoardNr(), itsTBBackE->status_mask)); 
	if (clientport->isConnected()) { clientport->send(*itsTBBackE); }
}
