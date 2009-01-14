//#  ReadCmd.cc: implementation of the ReadCmd class
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

#include "ReadCmd.h"


using namespace LOFAR;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;


//--Constructors for a ReadCmd object.----------------------------------------
ReadCmd::ReadCmd()
{
	TS					= TbbSettings::instance();
	itsTPE 			= new TPReadEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBReadAckEvent();
	
	itsTBBackE->status_mask = 0;
	setWaitAck(true);
}
	  
//--Destructor for ReadCmd.---------------------------------------------------
ReadCmd::~ReadCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool ReadCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_READ)||(event.signal == TP_READ_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void ReadCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE = new TBBReadEvent(event);
	
	int32 boardnr;
	int32 channelnr;
	TS->convertRcu2Ch(itsTBBE->rcu,&boardnr,&channelnr);
		
	setChannelNr(channelnr);
	setBoardNr(boardnr);
	
	itsTBBackE->status_mask = 0;
	if (!TS->isBoardActive(getBoardNr())) {	
		itsTBBackE->status_mask |= TBB_NO_BOARD ;
		setDone(true);
	}
				
	// initialize TP send frame
	itsTPE->opcode			= TPREAD;
	itsTPE->status			=	0;
	itsTPE->channel			= getChannelNr();
	itsTPE->secondstime	= itsTBBE->secondstime;
	itsTPE->sampletime	= itsTBBE->sampletime;
	itsTPE->prepages		= itsTBBE->prepages;
	itsTPE->postpages		= itsTBBE->postpages;
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void ReadCmd::sendTpEvent()
{
	TS->boardPort(getBoardNr()).send(*itsTPE);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void ReadCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsTBBackE->status_mask |= TBB_COMM_ERROR;
	}	else {
		itsTPackE = new TPReadAckEvent(event);
		// check if busy
		if (itsTPackE->status == 2) {
			LOG_DEBUG_STR(formatString("TBB busy, %d pages left, trying until free", itsTPackE->pages_left));
			setSleepTime(0.1);		
		} else {
			setDone(true);
		}
		LOG_DEBUG_STR(formatString("Received ReadAck from boardnr[%d]", getBoardNr()));
		delete itsTPackE;
	}
	
}

// ----------------------------------------------------------------------------
void ReadCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	if (itsTBBackE->status_mask == 0)
			itsTBBackE->status_mask = TBB_SUCCESS;
	 
	if (clientport->isConnected()) {clientport->send(*itsTBBackE); }
}
