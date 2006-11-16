//#  AllocCmd.cc: implementation of the AllocCmd class
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

#include "AllocCmd.h"
#include "DriverSettings.h"


namespace LOFAR {
	using namespace TBB_Protocol;
	using namespace TP_Protocol;
	namespace TBB {

//--Constructors for a AllocCmd object.----------------------------------------
AllocCmd::AllocCmd():
		itsBoardMask(0),itsErrorMask(0),itsBoardsMask(0),itsChannel(0)
{
	itsTPE 			= new TPAllocEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE	= new TBBAllocackEvent();
	
	for(int boardnr = 0;boardnr < DriverSettings::instance()->maxBoards();boardnr++) { 
		itsTBBackE->status[boardnr]	= 0;
		itsChannelMask[boardnr]	= 0;
	}
	LOG_DEBUG_STR(formatString("AllocCmd construct"));
}
	  
//--Destructor for AllocCmd.---------------------------------------------------
AllocCmd::~AllocCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool AllocCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_ALLOC)||(event.signal == TP_ALLOCACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void AllocCmd::saveTbbEvent(GCFEvent& event)
{
	LOG_DEBUG_STR(formatString("AllocCmd savetbb"));
	itsTBBE	= new TBBAllocEvent(event);
	
	// mask for the installed boards
	itsBoardsMask = DriverSettings::instance()->activeBoardsMask();
		
	for (int boardnr = 0; boardnr < DriverSettings::instance()->maxBoards(); boardnr++) {
		//
		if (!(itsBoardsMask & (1 << boardnr))) 
			itsTBBackE->status[boardnr] |= NO_BOARD;
		
		itsChannelMask[boardnr] = itsTBBE->channelmask[boardnr];
		
		for (int ch = 0; ch < 16; ch++) {
			if (itsChannelMask[boardnr] & (1 << ch)) {
				if (DriverSettings::instance()->getChSelected((ch + (boardnr * 16))))
					itsTBBackE->status[boardnr] |= ALLOC_ERROR;		
				else	
					DriverSettings::instance()->setChSelected((ch + (boardnr * 16)), true);	
			}
		} 		
		
		if (itsChannelMask[boardnr] != 0)
			itsBoardMask |= (1 << boardnr);
			
		if ((itsChannelMask[boardnr] & ~0xFFFF) != 0) 
			itsTBBackE->status[boardnr] |= (SELECT_ERROR & CHANNEL_SEL_ERROR);
				
		if (!(itsBoardsMask & (1 << boardnr)) &&  (itsChannelMask[boardnr] != 0))
			itsTBBackE->status[boardnr] |= (SELECT_ERROR & BOARD_SEL_ERROR);
		
		if ((itsBoardsMask & (1 << boardnr)) &&  (itsChannelMask[boardnr] != 0)) {
			if (!devideChannels(boardnr)) // calculate allocations
				itsTBBackE->status[boardnr] |= ALLOC_ERROR;
		}	
		LOG_DEBUG_STR(formatString("AllocCmd savetbb bnr[%d], status[0x%08X]",boardnr,itsTBBackE->status[boardnr]));
	}
		
	// Send only commands to boards installed
	itsErrorMask = itsBoardMask & ~itsBoardsMask;
	itsBoardMask = itsBoardMask & itsBoardsMask;
	
	// initialize TP send frame
	itsTPE->opcode			= TPALLOC;
	itsTPE->status			=	0;
		
	delete itsTBBE;	
	LOG_DEBUG_STR(formatString("AllocCmd savetbb done"));
}

// ----------------------------------------------------------------------------
bool AllocCmd::sendTpEvent(int32 boardnr, int32 channelnr)
{
	bool sending = false;
	DriverSettings*		ds = DriverSettings::instance();
	
	// fill in calculated allocations
	itsTPE->channel = ds->getChInputNr(channelnr);
	itsTPE->pageaddr = ds->getChStartAddr(channelnr);
	itsTPE->pagelength =	ds->getChPageSize(channelnr);
	itsChannel = channelnr; // set active channel
	
	// send cmd if no errors
	if 	(ds->boardPort(boardnr).isConnected() &&
			(itsTBBackE->status[boardnr] == 0) && 
			(itsTPE->channel < 16) &&
			!ds->getChAllocated(channelnr)) {
		ds->boardPort(boardnr).send(*itsTPE);
		ds->boardPort(boardnr).setTimer(ds->timeout());
		sending = true;
		LOG_DEBUG_STR(formatString("Sending Alloc to boardnr[%d], channel[%d]", 
																boardnr,channelnr));
	}
	else 
		itsErrorMask |= (1 << boardnr);
	
	return(sending);
}

// ----------------------------------------------------------------------------
void AllocCmd::saveTpAckEvent(GCFEvent& event, int32 boardnr)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsTBBackE->status[boardnr] |= COMM_ERROR;
	}
	else {
		itsTPackE = new TPAllocackEvent(event);
		
		if ((itsTPackE->status > 0x0) && (itsTPackE->status < 0x6)) 
			itsTBBackE->status[boardnr] |= (1 << (23 + itsTPackE->status));
		
		if ((itsTPackE->status >= 0xF0) && (itsTPackE->status <= 0xF6)) 
			itsTBBackE->status[boardnr] |= (1 << (16 + (itsTPackE->status & 0x0F)));	
			
		if(itsTPackE->status == 0) 
			DriverSettings::instance()->setChAllocated(itsChannel, true); 
		
		LOG_DEBUG_STR(formatString("Received AllocAck from boardnr[%d], status[0x%08X]", 
																boardnr,itsTBBackE->status[boardnr]));
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void AllocCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	for (int32 boardnr = 0; boardnr < DriverSettings::instance()->maxBoards(); boardnr++) { 
		if (itsTBBackE->status[boardnr] == 0)
			itsTBBackE->status[boardnr] = SUCCESS;
		//LOG_DEBUG_STR(formatString("AllocCmd sendtbb bnr[%d]st[0x%08X]",boardnr,itsTBBackE->status[boardnr]));
	}
	
	clientport->send(*itsTBBackE);
}

// ----------------------------------------------------------------------------
uint32 AllocCmd::getBoardMask()
{
	return(itsBoardMask);
}

// ----------------------------------------------------------------------------
uint32 AllocCmd::getChannelMask(int32 boardnr)
{
	return(itsChannelMask[boardnr]);
}

// ----------------------------------------------------------------------------
bool AllocCmd::waitAck()
{
	return(true);
}

// ----------------------------------------------------------------------------
CmdTypes AllocCmd::getCmdType()
{
	return(ChannelCmd);
}

// ----------------------------------------------------------------------------
uint32 getNrOfBits(uint32 mask)
{
	uint32 bits = 0;
	for (uint nr = 0; nr < (sizeof(mask) * 8); nr++) {
		if (mask & (1 << nr)) bits++;
	}
	return(bits);
}

// ----------------------------------------------------------------------------
bool AllocCmd::devideChannels(int32 boardnr)
{
	bool selected = false;
	bool success = false;
	uint32 channels;
	uint32 memorysize;
	uint32 channelsize;
	uint32 channeladdr[16];
	uint32 channelmask;
	uint32 addr;
	uint32 mp0ch;
	uint32 mp1ch;
	uint32 mp2ch;
	uint32 mp3ch;
		
	channelmask = itsChannelMask[boardnr];
	memorysize = DriverSettings::instance()->getMemorySize(boardnr);
	channels = getNrOfBits(channelmask);
	channelsize = memorysize / channels;
	mp0ch = getNrOfBits((itsChannelMask[boardnr] & 0x000F));
	mp1ch = getNrOfBits((itsChannelMask[boardnr] & 0x00F0));
	mp2ch = getNrOfBits((itsChannelMask[boardnr] & 0x0F00));
	mp3ch = getNrOfBits((itsChannelMask[boardnr] & 0xF000));
	addr = 0;
	
	
	for (int32 ch = 0; ch < DriverSettings::instance()->nrChannelsPerBoard(); ch++) {
		if (DriverSettings::instance()->getChAllocated(ch + (boardnr * 16)))
			selected = true;
	}
	if (!selected) {
	success = true;
	switch (channels) {
		case 1: {
			
			for (int32 ch = 0; ch < 16; ch++) {
				if (itsChannelMask[boardnr] & (1 << ch)) {
					channeladdr[ch] = addr;
					addr += channelsize;
				}
			}			
		} break;
		
		case 2: {
			for (int32 ch = 0; ch < 16; ch++) {
				if (itsChannelMask[boardnr] & (1 << ch)) {
					channeladdr[ch] = addr;
					addr += channelsize;
				}
			}
		} break;
		
		case 4: {
			//only 2 memory lane's, max 2 channels per Mp
			if ((mp0ch > 2) || (mp1ch > 2) || (mp2ch > 2) || (mp3ch > 2)) {
				success = false;
				break;
			}
							
			if (((mp0ch == 2) && (mp1ch == 2)) || ((mp2ch == 2) && (mp3ch == 2)) ||
				   (mp0ch == 1) || (mp1ch == 1) || (mp2ch = 1) || (mp3ch == 1)){
				// divide order mp0;mp1;mp0;mp1 || mp2;mp3;mp2;mp3 enz.
				while (channelmask) {
					for (int32 ch = 0; ch < 4; ch++) {
						if (channelmask & (1 << ch)) {
							channelmask &= ~ch;
							channeladdr[ch] = addr;
							addr += channelsize;
							ch = 4; // break;								
						}
					}
					for (int32 ch = 5; ch < 8; ch++) {
						if (channelmask & (1 << ch)) {
							channelmask &= ~ch;
							channeladdr[ch] = addr;
							addr += channelsize;
							ch = 8;	// break;							
						}
					}
					for (int32 ch = 9; ch < 12; ch++) {
						if (channelmask & (1 << ch)) {
							channelmask &= ~ch;
							channeladdr[ch] = addr;
							addr += channelsize;
							ch = 12;	// break;							
						}
					}
					for (int32 ch = 12; ch < 16; ch++) {
						if (channelmask & (1 << ch)) {
							channelmask &= ~ch;
							channeladdr[ch] = addr;
							addr += channelsize;
							ch = 16;	// break;							
						}
					}
				}
			}
			else {
				// divide order firt mp; second mp	
				for (int32 ch = 0; ch < 16; ch++) {
					if (itsChannelMask[boardnr] & (1 << ch)) {
						channeladdr[ch] = addr;
						addr += channelsize;
					}
				}
			}
		} break;
		
		case 8: {
			if ((mp0ch > 2) || (mp1ch > 2) || (mp2ch > 2) || (mp3ch > 2)) {
				success = false;
				break;
			}
			for (int32 ch = 0; ch < 16; ch++) {
				if (itsChannelMask[boardnr] & (1 << ch)) {
					channeladdr[ch] = addr;
					addr += channelsize;
				}
			}
		} break;
		
		case 16: {
			for (int32 ch = 0; ch < 16; ch++) {
				if (itsChannelMask[boardnr] & (1 << ch)) {
					channeladdr[ch] = addr;
					addr += channelsize;
				}
			}
		} break;
		
		default: {
			success = false;
		}	break;
	}
	if (success) {	// save to DriverSettings
		for (int32 ch = 0; ch < 16; ch++) {
			if (DriverSettings::instance()->getChSelected(ch + (boardnr * 16))) {
				DriverSettings::instance()->setChStartAddr((ch + (boardnr * 16)), channeladdr[ch]);
				DriverSettings::instance()->setChPageSize((ch + (boardnr * 16)), channelsize);		
			}
		}
	}
	}
	
	return(success);
}


	} // end TBB namespace
} // end LOFAR namespace
