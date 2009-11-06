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
#include <Common/StringUtil.h>

#include "AllocCmd.h"

using namespace LOFAR;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using namespace TBB;

	
//--Constructors for a AllocCmd object.----------------------------------------
AllocCmd::AllocCmd(): itsStage(0),itsRcuStatus(0)
{
	TS					= TbbSettings::instance();
	itsTPE 			= new TPAllocEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE	= new TBBAllocAckEvent();
	
	for(int boardnr = 0;boardnr < TS->maxBoards();boardnr++) { 
		itsTBBackE->status_mask[boardnr]	= 0;
		itsChannelMask[boardnr]	= 0;
	}
	setWaitAck(true);
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
	if ((event.signal == TBB_ALLOC)
		|| (event.signal == TP_ALLOC_ACK)
		|| (event.signal == TP_SIZE_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void AllocCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE	= new TBBAllocEvent(event);
	
	// convert rcu-bitmask to tbb-channelmask
	int32 board;				// board 0 .. 11
	int32 board_channel;// board_channel 0 .. 15	
	int32 channel;			// channel 0 .. 191 (= maxboard * max_channels_on_board)	
	for (int rcunr = 0; rcunr < TS->maxChannels(); rcunr++) {
		if(itsTBBE->rcu_mask.test(rcunr)) {
			TS->convertRcu2Ch(rcunr,&board,&board_channel);
			channel = (board * TS->nrChannelsOnBoard()) + board_channel;	
			itsChannelMask[board] |= (1 << board_channel);
			TS->setChSelected(channel,true);
		}
	} 
	
	uint32 boardmask = 0;	
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		itsTBBackE->status_mask[boardnr] = 0;
		if (itsChannelMask[boardnr] != 0) boardmask |= (1 << boardnr); 
		
		if (TS->isBoardActive(boardnr)) {
			
			if ((itsChannelMask[boardnr] & ~0xFFFF) != 0) 
				itsTBBackE->status_mask[boardnr] |= (TBB_SELECT_ERROR | TBB_CHANNEL_SEL_ERROR);	
			
			for (int ch = 0; ch < 16; ch++) {
				if (itsChannelMask[boardnr] & (1 << ch)) {
					if (TS->getChState((ch + (boardnr * 16))) != 'F') {
						itsTBBackE->status_mask[boardnr] |= TBB_ALLOC_ERROR;
					}	else {	
						LOG_DEBUG_STR(formatString("Ch[%d] is selected",ch + (boardnr * 16)));
					}
				}
			} 		
		} else {
			itsTBBackE->status_mask[boardnr] |= TBB_NO_BOARD;
		}
		
		if (itsTBBackE->status_mask[boardnr] != 0) {
			LOG_DEBUG_STR(formatString("AllocCmd savetbb bnr[%d], status[0x%08X], channelmask[%08X]",
														boardnr, itsTBBackE->status_mask[boardnr], itsChannelMask[boardnr]));
		}
	}
	
	setBoardMask(boardmask);
	LOG_DEBUG_STR(formatString("boardmask = 0x%08X",boardmask));
		
	// select first board to handle
	nextBoardNr();
	
	// initialize TP send frame
	itsTPE->opcode			= TPALLOC;
	itsTPE->status			=	0;
		
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void AllocCmd::sendTpEvent()
{
	//if (itsTBBackE->status_mask[getBoardNr()] == 0) {
	
		switch (itsStage) {
			// stage 1, get board memory size
			case 0: {
				TPSizeEvent *sizeEvent = new TPSizeEvent();
				sizeEvent->opcode	= TPSIZE;
				sizeEvent->status	=	0;
				TS->boardPort(getBoardNr()).send(*sizeEvent);
				delete sizeEvent;
			} break;
			
			// stage 2, allocate memory
			case 1: {
				itsRcuStatus = 0;
				
				// fill in calculated allocations
				itsTPE->channel = TS->getChInputNr(getChannelNr());
				itsTPE->pageaddr = TS->getChStartAddr(getChannelNr());
				itsTPE->pagelength = TS->getChPageSize(getChannelNr());
		
				if (TS->getChState(getChannelNr()) != 'F')
					itsRcuStatus |= TBB_RCU_NOT_FREE;
		
				// send cmd if no errors
				//if (itsRcuStatus == 0) {
					TS->boardPort(getBoardNr()).send(*itsTPE);
					LOG_DEBUG_STR(formatString("Sending Alloc to boardnr[%d], channel[%d]",getBoardNr(),getChannelNr()));
			//}
			} break;
			
			default: {
			} break;
		}
	//}	
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void AllocCmd::saveTpAckEvent(GCFEvent& event)
{
	switch (itsStage) {
		// stage 1, get board memory size
		case 0: {
			// in case of a time-out, set error mask
			if (event.signal == F_TIMER) {
				itsTBBackE->status_mask[getBoardNr()] |= TBB_COMM_ERROR;
			}	else {
				TPSizeAckEvent *sizeAckEvent = new TPSizeAckEvent(event);
		
				if ((sizeAckEvent->status >= 0xF0) && (sizeAckEvent->status <= 0xF6)) {
					itsTBBackE->status_mask[getBoardNr()] |= (1 << (16 + (sizeAckEvent->status & 0x0F)));	
				}
		
				TS->setMemorySize(getBoardNr(),sizeAckEvent->npages);
				//TS->setMemorySize(getBoardNr(),4194304);
				
				if (!devideChannels(getBoardNr())) // calculate allocations
					itsTBBackE->status_mask[getBoardNr()] |= TBB_ALLOC_ERROR;
				
				LOG_DEBUG_STR(formatString("Alloc-sizecmd: board[%d] status[0x%08X] pages[%u]", 
											getBoardNr(), sizeAckEvent->status, sizeAckEvent->npages));
				delete sizeAckEvent;
			}
			nextBoardNr();
			
			if (isDone()) {
				setDone(false);
				resetChannelNr(); 
				nextSelectedChannelNr();
				itsStage = 1;
			}
		} break;
		
		// stage 2, allocate memory
		case 1: {
			// in case of a time-out, set status mask
			if (event.signal == F_TIMER) {
				itsTBBackE->status_mask[getBoardNr()] |= TBB_RCU_COMM_ERROR;
				itsRcuStatus |= TBB_TIMEOUT_ETH;
				LOG_INFO_STR(formatString("F_TIMER AllocCmd DriverStatus[0x%08X], RcuStatus[0x%08X]",
										 itsTBBackE->status_mask[getBoardNr()],itsRcuStatus));
			}	else {
				itsTPackE = new TPAllocAckEvent(event);
				
				if (itsTPackE->status == 0) {
					TS->setChState(getChannelNr(),'A');
					//LOG_DEBUG_STR(formatString("channel %d is set to %c(A)",getChannelNr(),TS->getChState(getChannelNr())));
				} else {
					if ((itsTPackE->status > 0x0) && (itsTPackE->status < 0x6)) 
						itsRcuStatus |= (1 << (23 + itsTPackE->status));
		
					if ((itsTPackE->status >= 0xF0) && (itsTPackE->status <= 0xF6)) 
						itsRcuStatus |= (1 << (16 + (itsTPackE->status & 0x0F)));	
				}	
		
				LOG_DEBUG_STR(formatString("Received AllocAck from boardnr[%d], status[0x%08X]", 
											getBoardNr(),itsTPackE->status));
				
				delete itsTPackE;
			}
	
			if (itsRcuStatus) TS->setChState(getChannelNr(), 'E');
			TS->setChStatus(getChannelNr(),(uint16)(itsRcuStatus >> 16));
	
			if (itsRcuStatus || itsTBBackE->status_mask[getBoardNr()]) {
				int32 rcu;
				TS->convertCh2Rcu(getChannelNr(),&rcu);
				LOG_INFO_STR(formatString("ERROR AllocCmd Rcu[%d], DriverStatus[0x%08x], RcuStatus[0x%08x]",
										 rcu, itsTBBackE->status_mask[getBoardNr()],itsRcuStatus));
			}
	
			itsTBBackE->status_mask[getBoardNr()] |= itsRcuStatus;
			/*
			if (itsTBBackE->status_mask[getBoardNr()] != 0) {
				setChannelNr((getBoardNr() * 16) + 15);
			}
			*/
			nextSelectedChannelNr();
		} break;
		
		default: {
		} break;
	}
}
	

// ----------------------------------------------------------------------------
void AllocCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	int32 rcunr;
	
	itsTBBackE->rcu_mask.reset();
	for (int32 channelnr = 0; channelnr < TS->maxChannels(); channelnr++) {
		if (TS->getChStatus(channelnr)) {
			TS->convertCh2Rcu(channelnr,&rcunr);
			itsTBBackE->rcu_mask.set(rcunr);
		}
		//LOG_DEBUG_STR(formatString("channel %d is set to %c",channelnr,TS->getChState(channelnr)));
	}
	
	for (int32 boardnr = 0; boardnr < TS->maxBoards(); boardnr++) { 
		if (itsTBBackE->status_mask[boardnr] == 0)
			itsTBBackE->status_mask[boardnr] = TBB_SUCCESS;
	}
	
	if (clientport->isConnected()) { clientport->send(*itsTBBackE); }
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
	struct sMP {
		uint32 start_addr;
		uint32 used_channels;
	};
	
	// for 400 MHz mode there is only one ring, for 800 MHz there are 2 rings
	uint32 rings = 1; // usefull rings for dataflow between MP's
	bool success = false;
	uint32 totalChannels;
	uint32 totalMemorySize;
	uint32 mpMemorySize;
	uint32 channelMask;
	sMP		mp[TS->nrMpsOnBoard()];
		
	channelMask = itsChannelMask[boardnr];
	totalMemorySize = TS->getMemorySize(boardnr);
	mpMemorySize = totalMemorySize / TS->nrMpsOnBoard();
	totalChannels = getNrOfBits(channelMask);

	if (totalMemorySize == 0) return (false);
		
	// check if all board inputs are free, if not exit
	for (int32 ch = 0; ch < TS->nrChannelsOnBoard(); ch++) {
		if (TS->getChState(ch + (boardnr * 16)) != 'F')
			return (false);
	}
	
	// setup mp struct
	for (int i = 0; i < TS->nrMpsOnBoard(); i++) {
		uint32 ui = static_cast<uint32>(i);
		mp[i].start_addr = ui * mpMemorySize;
		uint32 mpChannelMask = itsChannelMask[boardnr] & (0x000F << (i * 4));
		mp[i].used_channels = getNrOfBits(mpChannelMask);
		LOG_DEBUG_STR("start MP" << i << " =" << formatString("%08x",mp[i].start_addr)); 
	}

	if (totalChannels == 1) {
		for (int ch = 0; ch < TS->nrChannelsOnBoard(); ch++) {
			if (itsChannelMask[boardnr] & (1 << ch)) {
				TS->setChStartAddr((ch + (boardnr * TS->nrChannelsOnBoard())),
														mp[0].start_addr);
				TS->setChPageSize( (ch + (boardnr * TS->nrChannelsOnBoard())),
														totalMemorySize);
				break;
			}	
		}		
	} 
	
	if (totalChannels == 2) {
		int last_mp = -1;
		for (int ch = 0; ch < TS->nrChannelsOnBoard(); ch++) {
			if (itsChannelMask[boardnr] & (1 << ch)) {
				int mp_nr = ch / TS->nrChannelsOnMp();
				if (mp_nr == last_mp) {
					mp_nr++;
					if (mp_nr == TS->nrMpsOnBoard()) {
						mp_nr = 0;
					}
				}
				TS->setChStartAddr((ch + (boardnr * TS->nrChannelsOnBoard())),
														mp[mp_nr].start_addr);
				TS->setChPageSize( (ch + (boardnr * TS->nrChannelsOnBoard())),
														mpMemorySize);	
				if (last_mp != -1) break; // 2 channels done
				last_mp = mp_nr;
			}
		}	
	}
	
	if (totalChannels > 2) {
		uint32 use_nr;
		for (int mpnr = 0; mpnr < TS->nrMpsOnBoard(); mpnr++) {
			if (mp[mpnr].used_channels == 0) continue;
			uint32 channelMemorySize = mpMemorySize / mp[mpnr].used_channels;
			LOG_DEBUG_STR("channelMemorySize: " << channelMemorySize << " pages");
			use_nr = 0;	
			for (int chnr = 0; chnr < TS->nrChannelsOnMp(); chnr++) {
				int ch = (mpnr * TS->nrChannelsOnMp()) + chnr;
				if (itsChannelMask[boardnr] & (1 << ch)) {
					LOG_DEBUG_STR("alloc settings: addr=" 
											<< formatString("%08x",(mp[mpnr].start_addr + (use_nr * channelMemorySize)))
											<< " pages=" << channelMemorySize);
					TS->setChStartAddr((ch + (boardnr * TS->nrChannelsOnBoard())),
															(mp[mpnr].start_addr + (use_nr * channelMemorySize)));
					TS->setChPageSize( (ch + (boardnr * TS->nrChannelsOnBoard())),
															channelMemorySize);
					use_nr++;
				}
			}
		}
	}
	LOG_DEBUG_STR("deviding done");
	return(true);
}
