//#  -*- mode: c++ -*-
//#
//#  BoardCmdHandler.h: TBB Driver boardaction class
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


//
// BoardCmdHandler handle's all commands from the client to the boards.
// The command will be send to all connected TBB boards, that are listed in the 
// given send-mask.
// If there is a response from all boards, this also could be a time-out, the
// data is send back to the client. The data for boards not responding or connected,
// is filled with zero's .
// In the return status a error mask is included 
//

#ifndef BOARDCMDHANDLER_H_
#define BOARDCMDHANDLER_H_

#include <GCF/TM/GCF_Control.h>
#include <GCF/TM/GCF_ETHRawPort.h>
#include <GCF/TM/GCF_DevicePort.h>
#include <GCF/TM/GCF_TimerPort.h>

#include "Command.h"
#include "DriverSettings.h"

namespace LOFAR {
	namespace TBB {
	
		class BoardCmdHandler : public GCFFsm
		{
	
			public:
				// Constructors for a SyncAction object.
				// Default direction is read.
				BoardCmdHandler(GCFTimerPort* port);

				// Destructor for SyncAction. */
				~BoardCmdHandler();

				// The states of the statemachine.
				GCFEvent::TResult idle_state(GCFEvent& event, GCFPortInterface& port);
				GCFEvent::TResult send_state(GCFEvent& event, GCFPortInterface& port);
				GCFEvent::TResult waitack_state(GCFEvent& event, GCFPortInterface& port);
								
				void setTpCmd(Command *cmd);
				bool tpCmdDone();
				
			protected:

			private:
				TbbSettings* 			TS;
				bool							itsDone;
				bool							itsFlashMode;
				// variables
				int32							 itsRetries;
				GCFPortInterface* itsClientPort; // return port of the actual commmand
				Command* itsCmd; // command to use
				GCFTimerPort* itsSleepTimer;			
				
		};
	} // end TBB namespace
} // end LOFAR namespace

#endif
