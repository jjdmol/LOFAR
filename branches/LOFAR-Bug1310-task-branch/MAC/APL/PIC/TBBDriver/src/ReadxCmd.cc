//#  ReadxCmd.cc: implementation of the ReadxCmd class
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
#include <Common/StringUtil.h>

#include "ReadxCmd.h"

using namespace LOFAR;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a ReadxCmd object.----------------------------------------
ReadxCmd::ReadxCmd()
{
	TS					= TbbSettings::instance();
	itsTPE 			= new TPReadxEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBReadxAckEvent();
	
	itsTBBackE->status_mask = 0;
	setWaitAck(true);
}
	  
//--Destructor for ReadxCmd.---------------------------------------------------
ReadxCmd::~ReadxCmd()
{
	delete itsTPE;
	delete itsTBBackE;	
}

// ----------------------------------------------------------------------------
bool ReadxCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_READX)||(event.signal == TP_READX_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void ReadxCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE	= new TBBReadxEvent(event);
	
	itsTBBackE->status_mask = 0;	
	if (TS->isBoardActive(itsTBBE->board)) {	
		setBoardNr(itsTBBE->board);
	} else {
		itsTBBackE->status_mask |= TBB_NO_BOARD ;
		setDone(true);
	}
		
	// initialize TP send frame
	itsTPE->opcode	= TPREADX;
	itsTPE->status	=	0;
	itsTPE->mp	 		= static_cast<uint32>(itsTBBE->mp);
	itsTPE->pid	 		= static_cast<uint32>(itsTBBE->pid);
	itsTPE->regid		= static_cast<uint32>(itsTBBE->regid);
	itsTPE->pagelength	= itsTBBE->pagelength;
	itsTPE->pageaddr	= itsTBBE->pageaddr;
	LOG_DEBUG_STR(formatString("Readx[%x][%x][%x][%x][%x][%x]", 
															itsTPE->opcode,itsTBBE->mp,itsTBBE->pid,
															itsTBBE->regid,itsTBBE->pagelength,
															itsTBBE->pageaddr));
	delete itsTBBE;	
}


// ----------------------------------------------------------------------------
void ReadxCmd::sendTpEvent()
{
	TS->boardPort(getBoardNr()).send(*itsTPE);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void ReadxCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsTBBackE->status_mask |= TBB_COMM_ERROR;
	}
	else {
		itsTPackE = new TPReadxAckEvent(event);
		
		if ((itsTPackE->status >= 0xF0) && (itsTPackE->status <= 0xF6)) 
			itsTBBackE->status_mask |= (1 << (16 + (itsTPackE->status & 0x0F)));	
		
		for (int32 an = 0; an < 256;an++) {
			itsTBBackE->pagedata[an]	= itsTPackE->pagedata[an];
		}
		LOG_DEBUG_STR(formatString("Received ReadxAck from boardnr[%d]", getBoardNr()));
		delete itsTPackE;
	}
	setDone(true);
}

// ----------------------------------------------------------------------------
void ReadxCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	if (itsTBBackE->status_mask == 0)
			itsTBBackE->status_mask = TBB_SUCCESS;
	 
	if (clientport->isConnected()) { clientport->send(*itsTBBackE); }
}
