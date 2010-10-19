//#  ReadfCmd.cc: implementation of the ReadfCmd class
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

#include "ReadfCmd.h"

using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;


//--Constructors for a ReadfCmd object.----------------------------------------
ReadfCmd::ReadfCmd():
		itsFile(0),itsImage(0),itsBlock(0)
{
	TS = TbbSettings::instance();
	setWaitAck(true);
}
	  
//--Destructor for ReadfCmd.---------------------------------------------------
ReadfCmd::~ReadfCmd() { }

// ----------------------------------------------------------------------------
bool ReadfCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_READ_IMAGE)||(event.signal == TP_READF_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void ReadfCmd::saveTbbEvent(GCFEvent& event)
{
	TBBReadImageEvent tbb_event(event);
	
	setBoard(tbb_event.board);
	
	itsImage = tbb_event.image;
	itsBlock = (itsImage * TS->flashBlocksInImage());
	itsFile = fopen("image.hex","wb");
	
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void ReadfCmd::sendTpEvent()
{
	TPReadfEvent tp_event;
	tp_event.opcode = oc_READF;
	tp_event.status = 0;
	
	tp_event.addr = static_cast<uint32>(itsBlock * TS->flashBlockSize());
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void ReadfCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		setStatus(0, TBB_TIME_OUT);
		setDone(true);	
	} else {
		TPReadfAckEvent tp_ack(event);
		LOG_DEBUG_STR(formatString("Received ReadfAck from boardnr[%d]", getBoardNr()));
		
		if (tp_ack.status != 0) {
			setStatus(0, (tp_ack.status << 24));
			setDone(true);
		} else {
			int byte_val;
			int nible_val;

			for (int dp = 0; dp < 256; dp++) {	// there are 256 words in 1 message
				for (int bn = 0; bn < 4; bn++) {
					byte_val = (tp_ack.data[dp] >> (bn * 8)) & 0xFF; // take 1 byte
				
					nible_val = ((byte_val & 0xF0) >> 4);
					if (nible_val < 10) {
						nible_val += 48;
					}	else {
						nible_val += 87;	
					}
					putc(nible_val,itsFile);
					
					nible_val = (byte_val & 0x0F);
					if (nible_val < 10) {
						nible_val += 48;
					}	else {
						nible_val += 87;	
					}
					putc(nible_val,itsFile);
				}
			}
			
			itsBlock++;
			if (itsBlock == ((itsImage + 1) * TS->flashBlocksInImage())) {
				setDone(true);
			}
		} 
	}
}

// ----------------------------------------------------------------------------
void ReadfCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	fclose(itsFile);
	
	TBBReadImageAckEvent tbb_ack;
		
	tbb_ack.status_mask = getStatus(0);
	
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
