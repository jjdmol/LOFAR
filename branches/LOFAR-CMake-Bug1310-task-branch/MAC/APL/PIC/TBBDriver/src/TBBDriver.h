//#  TBBDriver.h: Driver for the TBB board
//#
//#  Copyright (C) 2006
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

#ifndef LOFAR_TBBDRIVER_TBBDRIVER_H
#define LOFAR_TBBDRIVER_TBBDRIVER_H

#include <APL/TBB_Protocol/TBB_Protocol.ph>
#include "TP_Protocol.ph"

#include <GCF/TM/GCF_Control.h>
#include <GCF/TM/GCF_ETHRawPort.h>
#include <GCF/TM/GCF_DevicePort.h>
#include <GCF/TM/GCF_TimerPort.h>

#include <Common/lofar_deque.h>

#include "BoardCmdHandler.h"
#include "MsgHandler.h"
#include "DriverSettings.h"

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

namespace LOFAR{
  namespace TBB{

    // Description of class.
    class TBBDriver : public GCFTask
    {
    public:
		
			// The constructor of the TBBDriver task.
			// @param name The name of the task. The name is used for looking
			// up connection establishment information using the GTMNameService and
			// GTMTopologyService classes.
			TBBDriver(string name);
      ~TBBDriver();
			
			// 
			int32 portToBoardNr(GCFPortInterface& port);
			
						// open all board ports
			void openBoards();
			
			// check if all boardports are open
			bool isEnabled();
			
			
      // The init state. 
			// This state is used to wait for events from the client
      // and board ports. When a event is received the command is executed 
			// and transition to the busy state is made.
      GCFEvent::TResult init_state(GCFEvent& event, GCFPortInterface &port);
      
      // The setup state. 
			// This state is used to setup the boards after a new connection or 
      // board reset, if completed an transition to the idle state is made.
      GCFEvent::TResult setup_state(GCFEvent& event, GCFPortInterface &port);
      
      // The idle state. 
			// This state is used to wait for events from the client
      // and board ports. When a event is received the command is executed 
			// and transition to the busy state is made.
      GCFEvent::TResult idle_state(GCFEvent& event, GCFPortInterface &port);
		
      // The busy state.
			// In this state the task can receive TBBEvents,
			// but they will be put on the que.
			// All TPEvent will be transmittted imediallie
      GCFEvent::TResult busy_state(GCFEvent& event, GCFPortInterface &port);
			
			//void undertaker();
		
    private:
      // Copying is not allowed ??
      TBBDriver (const TBBDriver& that);
      TBBDriver& operator= (const TBBDriver& that);
			
			void sendMessage(GCFEvent& event);
			bool CheckAlive(GCFEvent& event, GCFPortInterface& port);
			bool CheckSize(GCFEvent& event, GCFPortInterface& port);
			bool sendInfo(GCFEvent& event, GCFPortInterface& port);
			bool SetTbbCommand(unsigned short signal);
			
			TbbSettings *TS;
			
			// define some variables
			TPAliveEvent			*itsAlive;
			BoardCmdHandler		*cmdhandler;
			MsgHandler				*msghandler;
			Command 					*cmd;
			bool							itsAliveCheck;
			bool							itsSizeCheck;
			//uint32						itsActiveBoards;
			uint32						itsNewBoards;
			bool							itsActiveBoardsChange;
			int32 						*itsResetCount;
			
			struct TbbEvent{
				GCFPortInterface	*port;
				uint32						length;
				uint8							*event;
			};
			
			std::deque<TbbEvent*> *itsTbbQueue;
			
			GCFTCPPort      itsAcceptor;     // listen for clients on this port
			GCFETHRawPort*	itsBoard;        // array of ports, one for each TBB board
			GCFTimerPort*		itsAliveTimer;  // used to check precence and reset of the boards
			GCFTimerPort*		itsSetupTimer;  // used in the setup state
			GCFTimerPort*		itsCmdTimer;  // used by CommandHandler
			GCFTimerPort*		itsSaveTimer;  // used to save triggers to a file
			std::list<GCFPortInterface*> itsClientList;  // list of clients
    };
	} // end TBB namespace
} // end LOFAR namespace

#endif
