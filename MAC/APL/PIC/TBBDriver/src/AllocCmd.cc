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
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using namespace TBB;

	
//--Constructors for a AllocCmd object.----------------------------------------
AllocCmd::AllocCmd(): 
	itsStage(0)
{
	TS = TbbSettings::instance();
	setWaitAck(true);
}
	  
//--Destructor for AllocCmd.---------------------------------------------------
AllocCmd::~AllocCmd() { }

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
	TBBAllocEvent tbb_event(event);
	
	setChannels(tbb_event.rcu_mask);
	
	int32 board;
	for (int i = 0; i < TS->maxChannels(); i++) {
		if (tbb_event.rcu_mask.test(i) == true) {
			if (TS->getChState(i) != 'F') {
				board = TS->getChBoardNr(i);
				setStatus(board, TBB_CH_NOT_FREE);
			}
		}
	}

	// select first board to handle
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void AllocCmd::sendTpEvent()
{
	switch (itsStage) {
		// stage 1, get board memory size
		case 0: {
			TPSizeEvent tp_event;
			tp_event.opcode = oc_SIZE;
			tp_event.status = 0;
			if (getStatus(getBoardNr()) == TBB_SUCCESS) {
				TS->boardPort(getBoardNr()).send(tp_event);
			}
		} break;
		
		// stage 2, allocate memory
		case 1: {
			TPAllocEvent tp_event;
			tp_event.opcode = oc_ALLOC;
			tp_event.status = 0;
			
			// fill in calculated allocations
			tp_event.channel = TS->getChInputNr(getChannelNr());
			tp_event.pageaddr = TS->getChStartAddr(getChannelNr());
			tp_event.pagelength = TS->getChPageSize(getChannelNr());

			
			// send cmd if no errors
			if (getStatus(getBoardNr()) == TBB_SUCCESS) {
				TS->boardPort(getBoardNr()).send(tp_event);
				LOG_DEBUG_STR(formatString("Sending Alloc to boardnr[%d], channel[%d]",getBoardNr(),getChannelNr()));
			}
		} break;
		
		default: {
		} break;
	}
	if (getStatus(getBoardNr()) == TBB_SUCCESS) {
		TS->boardPort(getBoardNr()).setTimer(TS->timeout());
	} else {
		TS->boardPort(getBoardNr()).setTimer(0.0);
	}
}

// ----------------------------------------------------------------------------
void AllocCmd::saveTpAckEvent(GCFEvent& event)
{
	switch (itsStage) {
		// stage 1, get board memory size
		case 0: {
			// in case of a time-out, set error mask
			if (event.signal == F_TIMER) {
				setStatus(getBoardNr(), TBB_TIME_OUT);
			}	else {
				TPSizeAckEvent tp_ack(event);
		
				if (tp_ack.status != 0) {
					addStatus(getBoardNr(), (tp_ack.status << 24));
				} else {
					TS->setMemorySize(getBoardNr(),tp_ack.npages);
				
					if (!devideChannels(getBoardNr())) { // calculate allocations
						setStatus(getBoardNr(), TBB_ALLOC_ERROR);
					}
				
					LOG_DEBUG_STR(formatString("Alloc-sizecmd: board[%d] status[0x%08X] pages[%u]", 
											getBoardNr(), tp_ack.status, tp_ack.npages));
				}
			}
			nextBoardNr();
			
			if (isDone()) {
				setDone(false);
				itsStage = 1;
				nextChannelNr();
			}
		} break;
		
		// stage 2, allocate memory
		case 1: {
			// in case of a time-out, set status mask
			if (event.signal == F_TIMER) {
				setStatus(getBoardNr(), TBB_TIME_OUT);
			}	else {
				TPAllocAckEvent tp_ack(event);
				LOG_DEBUG_STR(formatString("Received AllocAck from boardnr[%d]", getBoardNr()));
				
				TS->setChStatus(getChannelNr(),(uint16)(tp_ack.status));
				if (tp_ack.status != 0) {
					setStatus(getBoardNr(), (tp_ack.status & 0xFF) << 24);
					TS->setChState(getChannelNr(), 'E');
				} else {
					TS->setChState(getChannelNr(),'A');
				} 
				if (tp_ack.status || getStatus(getBoardNr())) {
					int32 rcu;
					TS->convertCh2Rcu(getChannelNr(),&rcu);
					LOG_INFO_STR(formatString("ERROR AllocCmd Rcu[%d], DriverStatus[0x%08x], RcuStatus[0x%08x]",
									 rcu, getStatus(getBoardNr()),tp_ack.status));
				}
			}
			nextChannelNr();
		} break;
		
		default: {
		} break;
	}
}
	

// ----------------------------------------------------------------------------
void AllocCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBAllocAckEvent tbb_ack;
	
	int32 rcunr;
	
	tbb_ack.rcu_mask.reset();
	for (int32 channelnr = 0; channelnr < TS->maxChannels(); channelnr++) {
		if (TS->getChStatus(channelnr)) {
			TS->convertCh2Rcu(channelnr,&rcunr);
			tbb_ack.rcu_mask.set(rcunr);
		}
		//LOG_DEBUG_STR(formatString("channel %d is set to %c",channelnr,TS->getChState(channelnr)));
	}
	
	for (int32 i = 0; i < MAX_N_TBBOARDS; i++) { 
		tbb_ack.status_mask[i] = getStatus(i);
	}
	
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
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
		
	uint32 totalChannels;
	uint32 totalMemorySize;
	uint32 mpMemorySize;
	uint32 channelMask;
	sMP    mp[TS->nrMpsOnBoard()];
		
	channelMask = getBoardChannels(boardnr);
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
		uint32 mpChannelMask = getMpChannels(boardnr, i);
		mp[i].used_channels = getNrOfBits(mpChannelMask);
		LOG_DEBUG_STR("start MP" << i << " =" << formatString("%08x",mp[i].start_addr)); 
	}

	if (totalChannels == 1) {
		for (int ch = 0; ch < TS->nrChannelsOnBoard(); ch++) {
			if (channelMask & (1 << ch)) {
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
			if (channelMask & (1 << ch)) {
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
				if (channelMask & (1 << ch)) {
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
