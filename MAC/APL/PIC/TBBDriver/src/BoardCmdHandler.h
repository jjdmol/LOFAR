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

#include "Command.h"

namespace LOFAR {
	namespace TBB {
	
		class BoardCmdHandler : public GCFFsm
		{
	
			public:
				// Constructors for a SyncAction object.
				// Default direction is read.
				BoardCmdHandler();

				// Destructor for SyncAction. */
				~BoardCmdHandler();

				// The states of the statemachine.
				GCFEvent::TResult send_state(GCFEvent& event, GCFPortInterface& port);
				GCFEvent::TResult waitack_state(GCFEvent& event, GCFPortInterface& port);
								
				void setBoardPorts(GCFPortInterface* board_ports);
				void setTpCmd(Command *cmd);
				void setTpRetries(int32 Retries);
				void setNrOfTbbBoards(int32 NrOfBoards);
				void setActiveBoards(int32 ActiveBoards);
				void setMaxTbbBoards(int32 MaxBoards);
				void setTpTimeOut(double TimeOut);
				
				bool tpCmdDone();
				
			protected:

			private:
				// convert port to portnr, used in the port-array
				int portToBoardNr(GCFPortInterface& port);
				
				
			private:
				
				int32								itsRetries;  // max number of retries
				int32								itsNrOfBoards;
				uint32							itsActiveBoards;
				int32								itsMaxBoards;
				double							itsTimeOut;
				bool								itsCmdDone;
				
				GCFPortInterface*		itsClientPort; // return port of the actual commmand
				GCFPortInterface*		itsBoardPorts; // array of tbb board ports
				
				int32								itsBoardRetries[MAX_N_TBBBOARDS];
				Command*						itsCmd; // command to use			
				
		};
	} // end TBB namespace
} // end LOFAR namespace

#endif /* BOARDACTION_H_ */
