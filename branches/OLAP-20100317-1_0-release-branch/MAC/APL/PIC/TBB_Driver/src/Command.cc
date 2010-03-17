//#  Command.cc: implementation of the Command class
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

#include "Command.h"

using namespace LOFAR;
using namespace TBB;

// ----------------------------------------------------------------------------
Command::Command() : 
	itsRetry(true), itsWaitAck(false), itsDone(false), itsAllPorts(false), itsBoard(-1), 
	itsChannel(-1), itsSleepTime(0)
{
	TS = TbbSettings::instance();
	itsBoards.reset();
	itsChannels.reset();
	for (int i = 0 ; i < MAX_N_TBBOARDS; i++) {
		itsStatus[i] = TBB_SUCCESS;
	}
}

// ----------------------------------------------------------------------------
Command::~Command() { }

// ----------------------------------------------------------------------------
void Command::setBoard(int32 board)
{
	if (board >= TS->maxBoards()) {
		itsStatus[0] = TBB_NO_BOARD;
	} else {

		if (TS->isBoardReady(board) == false) {
			itsStatus[0] = TBB_NOT_READY;
		}

		if (TS->isBoardActive(board) == false) {
			itsStatus[0] = TBB_NOT_ACTIVE;
		}
				
		if (itsStatus[0] == TBB_SUCCESS) {
			itsBoards.set(board);
		}
	}
}

// ----------------------------------------------------------------------------
void Command::setBoards(uint32 board_mask)
{
	for (int i = 0; i < MAX_N_TBBOARDS; i++) {
		
		if (i >= TS->maxBoards()) {
			itsStatus[i] = TBB_NO_BOARD;
			continue;
		}
		
		if (board_mask & (1 << i)) {

			if (TS->isBoardReady(i) == false) {
				itsStatus[i] = TBB_NOT_READY;
			}

			if (TS->isBoardActive(i) == false) {
				itsStatus[i] = TBB_NOT_ACTIVE;
			}
			
			if (itsStatus[i] == TBB_SUCCESS) {
				itsBoards.set(i);
			}
		}
	}
}

// ----------------------------------------------------------------------------
void Command::setChannel(int32 rcu)
{
	// convert rcu-bitmask to tbb-channelmask
	int32 board;         // board 0 .. 11
	int32 board_channel; // board_channel 0 .. 15	
	int32 channel;       // channel 0 .. 191 (= maxboard * max_channels_on_board)	
	
	TS->convertRcu2Ch(rcu,&board,&board_channel);
	
	if (board >= TS->maxBoards()) {
		itsStatus[0] = TBB_NO_BOARD;
	} else {
		
		if (TS->isBoardReady(board) == false) {
			itsStatus[0] = TBB_NOT_READY;
		}
		
		if (TS->isBoardActive(board) == false) {
			itsStatus[0] = TBB_NOT_ACTIVE;
		}
			
		if (itsStatus[0] == TBB_SUCCESS) {
			itsBoards.set(board);
			channel = (board * TS->nrChannelsOnBoard()) + board_channel;
			itsChannels.set(channel);
		}
	}
}

// ----------------------------------------------------------------------------
void Command::setChannels(bitset<MAX_N_RCUS> rcus)
{
	// convert rcu-bitmask to tbb-channelmask
	int32 board;         // board 0 .. 11
	int32 board_channel; // board_channel 0 .. 15	
	int32 channel;       // channel 0 .. 191 (= maxboard * max_channels_on_board)	

	for (int i = 0; i < MAX_N_RCUS; i++) {
		TS->convertRcu2Ch(i,&board,&board_channel);
		
		if (board >= TS->maxBoards()) {
			itsStatus[board] = TBB_NO_BOARD;
			continue;
		} 

		if (rcus.test(i)) {
	
			if (TS->isBoardReady(board) == false) {
				itsStatus[board] = TBB_NOT_READY;
			}
			
			if (TS->isBoardActive(board) == false) {
				itsStatus[board] = TBB_NOT_ACTIVE;
			}
	
			if (itsStatus[board] == TBB_SUCCESS) {
				itsBoards.set(board);
				channel = (board * TS->nrChannelsOnBoard()) + board_channel;
				itsChannels.set(channel);
			}
		}
	}
}

// ----------------------------------------------------------------------------
uint32 Command::getBoardChannels(int32 board)
{
	uint board_channels = 0;
	int start_channel = board * TS->nrChannelsOnBoard(); 
	for (int i = 0; i < TS->nrChannelsOnBoard(); i ++) {
		if (itsChannels.test(start_channel + i)) {
			board_channels |= (1 << i);
		}
	}
	return(board_channels);
}

// ----------------------------------------------------------------------------	
uint32 Command::getMpChannels(int32 board, int32 mp)
{
	uint mp_channels = 0;
	int start_channel = (board * TS->nrChannelsOnBoard()) + (mp * TS->nrChannelsOnMp()); 
	for (int i = 0; i < TS->nrChannelsOnMp(); i ++) {
		if (itsChannels.test(start_channel + i)) {
			mp_channels |= (1 << i);
		}
	}
	return(mp_channels);
}

// ----------------------------------------------------------------------------
void Command::setChannelNr(int32 channelnr)
{
	itsChannel = channelnr;
	itsBoard = TS->getChBoardNr(itsChannel); 
}

// ----------------------------------------------------------------------------
void Command::reset()
{
	itsDone = false;
	itsWaitAck = true;
	itsBoard = -1;
	itsChannel = -1;
	itsBoards.reset();
	itsChannels.reset();
}

// ----------------------------------------------------------------------------
void Command::setDone(bool done)
{
	itsDone = done;
	if (itsDone) {
		itsWaitAck = true;
		itsBoard = -1;
		itsChannel = -1;	
	}
}

// ----------------------------------------------------------------------------
void Command::nextBoardNr()
{
	bool validNr = false;
	
	do {
		itsBoard++;
		if (itsBoard == TS->maxBoards()) { break; }
		itsChannel = itsBoard * TS->nrChannelsOnBoard();
		if (itsBoards.test(itsBoard) && (itsStatus[itsBoard] == TBB_SUCCESS)) {
			validNr = true;
		}
	} while (validNr == false);
		
	// if all nr done send clear all variables
	if (validNr == false) {
		itsDone = true;
		itsBoard = -1;
		itsChannel = -1;
	}
	LOG_DEBUG_STR(formatString("nextBoardNr() = %d",itsBoard));		
}

// ----------------------------------------------------------------------------
void Command::nextChannelNr()
{
	bool validNr = false;
	
	do {
		itsChannel++;
		if (itsChannel == TS->maxChannels()) { break; }
		itsBoard = TS->getChBoardNr(itsChannel);
		if (itsChannels.test(itsChannel)  && (itsStatus[itsBoard] == TBB_SUCCESS)) {
			validNr = true;
		}
	} while (validNr == false);
		
	// if all nr done send clear all variables
	if (validNr == false) {
		itsDone = true;
		itsBoard = -1;
		itsChannel = -1;
	}
	LOG_DEBUG_STR(formatString("nextChannelNr() = %d",itsChannel));	
}

