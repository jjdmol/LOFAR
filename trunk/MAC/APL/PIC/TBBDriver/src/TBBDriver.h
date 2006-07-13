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
#include <APL/TBB_Protocol/TP_Protocol.ph>

#include <GCF/TM/GCF_Control.h>
#include <GCF/TM/GCF_ETHRawPort.h>
#include <GCF/TM/GCF_DevicePort.h>

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

namespace LOFAR{
  namespace TBB{
	 
    // Description of class.
    class TBBDriver
    {
    public:
		
			// The constructor of the TBBDriver task.
			// @param name The name of the task. The name is used for looking
			// up connection establishment information using the GTMNameService and
			// GTMTopologyService classes.
			TBBDriver(string name);
      ~TBBDriver();
		
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
		
		
    private:
      // Copying is not allowed ??
      TBBDriver (const TBBDriver& that);
      TBBDriver& operator= (const TBBDriver& that);
		
			bool SetTbbCommand(GCFEvent& event);
			bool SetTpCommand(GCFEvent& event);
			
			// define some constants
	  	// mode of operation

			static const int32	MODE_NORMAL	    = 0; // control all RSPboards
			static const int32	MODE_SUBSTATION = 1; // control only one RSPboard
			
			GCFTCPPort                   m_acceptor;     // listen for clients on this port
			GCFETHRawPort*               m_board;        // array of ports, one for each RSP board
			std::list<GCFPortInterface*> m_client_list;  // list of clients
			std::list<GCFPortInterface*> m_dead_clients; // list of clients to cleanup
    };
  } // namespace TBB
} // namespace LOFAR

#endif
