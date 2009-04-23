//#  Command.h: TBB Driver command class
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


#ifndef COMMAND_H_
#define COMMAND_H_

#include <Common/LofarTypes.h>
#include <GCF/TM/GCF_Control.h>

#include "DriverSettings.h"

namespace LOFAR {
	namespace TBB {

class Command
{
public:
	// Constructor for Command
	Command();

	// Destructor for Command.
	virtual ~Command();
	
	virtual bool isValid(GCFEvent& event) = 0;
		
	virtual void saveTbbEvent(GCFEvent& event) = 0;

	virtual void sendTpEvent() = 0;
	
	virtual void saveTpAckEvent(GCFEvent& event) = 0;

	virtual void sendTbbAckEvent(GCFPortInterface* clientport) = 0;
	
	bool retry();
	
	void setRetry(bool retry);

	bool waitAck();
	
	void setWaitAck(bool waitack);
	
	void setSleepTime(double sleeptime);
	
	double	getSleepTime();
	
	void reset();
	
	bool isDone();
	
	void setDone(bool done);
	
	void resetBoardNr();
	
	void resetChannelNr();
	
	int32 getBoardNr();
	
	int32 getChannelNr();
	
	void setBoardNr(int32 boardnr);
	
	void setChannelNr(int32 channelnr);
		
	void nextBoardNr();
	
	void nextChannelNr();
	
	void nextSelectedChannelNr();
	
	void setBoardMask(uint32 mask);
	
private:
	TbbSettings *TS;
	
	bool		itsRetry;		// true if resending of a command is needed
	bool		itsWaitAck;		// true if an ack is expected
	bool		itsDone;			// true if the command is completed
	bool		itsAllPorts;	// true if command must be send to all available ports
	int32		itsBoard;
	int32		itsChannel;
	uint32	itsBoardMask;
	double	itsSleepTime;
};
	} // end TBB namespace
} // end LOFAR namespace

#endif /* COMMAND_H_ */
