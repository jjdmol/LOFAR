//#  ErasefCmd.cc: implementation of the ErasefCmd class
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

#include "ErasefCmd.h"

using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a ErasefCmd object.----------------------------------------
ErasefCmd::ErasefCmd():
		itsImage(0),itsSector(0)
{
	TS = TbbSettings::instance();
	
	setWaitAck(true);
}
	  
//--Destructor for ErasefCmd.---------------------------------------------------
ErasefCmd::~ErasefCmd() { }

// ----------------------------------------------------------------------------
bool ErasefCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_ERASE_IMAGE)||(event.signal == TP_ERASEF_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void ErasefCmd::saveTbbEvent(GCFEvent& event)
{
	TBBEraseImageEvent tbb_event(event);
	
	setBoard(tbb_event.board);
	
	itsImage = tbb_event.image;
	itsSector = (itsImage * TS->flashSectorsInImage());
	
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void ErasefCmd::sendTpEvent()
{
	TPErasefEvent tp_event;
	tp_event.opcode = oc_ERASEF;
	tp_event.status = 0;
	
	tp_event.addr = static_cast<uint32>(itsSector * TS->flashSectorSize());
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout()); // erase time of sector = 500 mSec
}

// ----------------------------------------------------------------------------
void ErasefCmd::saveTpAckEvent(GCFEvent& event)
{
	LOG_DEBUG_STR(formatString("Received ErasefAck from boardnr[%d]", getBoardNr()));
	
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		setStatus(0, TBB_TIME_OUT);
		setDone(true);
	} else {
		TPErasefAckEvent tp_ack(event);
		LOG_DEBUG_STR(formatString("Received ErasefAck from boardnr[%d]", getBoardNr()));
				
		if (tp_ack.status != 0) {
			setStatus(0, (tp_ack.status << 24));
			setDone(true);
		} else {
			itsSector++;
			if (itsSector == ((itsImage + 1) * TS->flashSectorsInImage())) {
				setDone(true);
			} 
		}
	}
}

// ----------------------------------------------------------------------------
void ErasefCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBEraseImageAckEvent tbb_ack;
	
	tbb_ack.status_mask = getStatus(0);
	
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
