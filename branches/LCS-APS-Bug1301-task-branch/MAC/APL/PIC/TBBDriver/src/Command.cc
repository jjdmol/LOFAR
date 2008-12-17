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
			
Command::Command() : 
	itsRetry(true), itsWaitAck(false), itsDone(false), itsAllPorts(false), itsBoard(-1), 
	itsChannel(-1), itsBoardMask(0), itsSleepTime(0)
{
	TS = TbbSettings::instance();
}

Command::~Command()
{
}

// ----------------------------------------------------------------------------
int32 Command::getBoardNr()
{
	return(itsBoard);
}

// ----------------------------------------------------------------------------
int32 Command::getChannelNr()
{
	return(itsChannel);
}
// ----------------------------------------------------------------------------
void Command::setBoardNr(int32 boardnr)
{
	itsBoard = boardnr;
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
void Command::resetBoardNr()
{
	itsBoard = -1;
}

// ----------------------------------------------------------------------------
void Command::resetChannelNr()
{
	itsBoard = -1;
}

// ----------------------------------------------------------------------------
bool Command::isDone()
{
	return(itsDone);
}

// ----------------------------------------------------------------------------
bool Command::retry()
{
	return(itsRetry);
}

// ----------------------------------------------------------------------------			
void Command::setRetry(bool retry)
{
	itsRetry = retry;
}

// ----------------------------------------------------------------------------
bool Command::waitAck()
{
	return(itsWaitAck);
}

// ----------------------------------------------------------------------------			
void Command::setWaitAck(bool waitack)
{
	itsWaitAck = waitack;
}

// ----------------------------------------------------------------------------
void Command::setSleepTime(double sleeptime)
{
	itsSleepTime = sleeptime;
}

// ----------------------------------------------------------------------------
double Command::getSleepTime()
{
	return (itsSleepTime);
}

// ----------------------------------------------------------------------------
void Command::setBoardMask(uint32 mask)
{
	itsBoardMask = mask;
}

// ----------------------------------------------------------------------------
void Command::nextBoardNr()
{
	bool validNr = false;
	
	do {
		itsBoard++;
		if (itsBoard < TS->maxBoards()) {
			// see if board is active
			if (TS->boardPort(itsBoard).isConnected() 
						&& (TS->getBoardState(itsBoard) == boardReady)
						&& (itsBoardMask & (1 << itsBoard))) 
			{
				validNr = true;
			}
		} else {
			break;	
		}
	} while (!validNr);
		
	// if all nr done send clear all variables
	if (!validNr) {
		itsDone = true;
		itsBoard = -1;
		itsBoardMask = 0;
	}
	LOG_DEBUG_STR(formatString("nextBoardNr() = %d",itsBoard));		
}

// ----------------------------------------------------------------------------
void Command::nextChannelNr()
{
	bool validNr = false;
	
	do {
		itsChannel++;
		if (itsChannel == TS->maxChannels()) break;
		itsBoard = TS->getChBoardNr(itsChannel);
		
		if (itsBoard < TS->maxBoards()) {
			// see if board is active 
			if (	TS->boardPort(itsBoard).isConnected()
						&& (TS->getBoardState(itsBoard) == boardReady)
						&& (itsChannel < TS->maxChannels())) 
			{
				validNr = true;
			}
		} else {
			break;	
		}
	} while (!validNr && (itsChannel < TS->maxChannels()));
		
	// if all nr done send clear all variables
	if (!validNr) {
		itsDone = true;
		itsChannel = -1;
		for (int ch = 0; ch < TS->maxChannels(); ch++) {
			TS->setChSelected(ch, false);
		}
	}
	LOG_DEBUG_STR(formatString("nextChannelNr() = %d",itsChannel));	
}

// ----------------------------------------------------------------------------
// only channels with a state different than 'F' are selected
void Command::nextSelectedChannelNr()
{
	bool validNr = false;
	
	do {
		itsChannel++;
		if (itsChannel == TS->maxChannels()) break;
		itsBoard = TS->getChBoardNr(itsChannel);

		if (itsBoard < TS->maxBoards()) {
			// see if board is active and channel is selected
			if (TS->boardPort(itsBoard).isConnected()
						&& (TS->getBoardState(itsBoard) == boardReady)
						&& (itsChannel < TS->maxChannels()) 
						&& TS->isChSelected(itsChannel)) 
			{
				validNr = true;
			}
		} else {
			break;	
		}
	} while (!validNr && (itsChannel < TS->maxChannels()));
		
	// if all nr done send clear all variables
	if (!validNr) {
		itsDone = true;
		itsChannel = -1;
		for (int ch = 0; ch < TS->maxChannels(); ch++) {
			TS->setChSelected(ch, false);
		}
	}
	LOG_DEBUG_STR(formatString("nextSelectedChannelNr() = %d",itsChannel));	
}
