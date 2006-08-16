//#  -*- mode: c++ -*-
//#
//#  ClientMsgHandler.h: TBB Driver boardaction class
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



#ifndef CLIENTMSGHANDLER_H_
#define CLIENTMSGHANDLER_H_

#include <GCF/TM/GCF_Control.h>

#include "Message.h"

namespace LOFAR {
	namespace TBB {
	
		class ClientMsgHandler : public GCFFsm
		{
	
			public:
				Message*						itsMsg; // Message in use
				// Constructors for a SyncAction object.
				// Default direction is read.
				ClientMsgHandler(GCFPortInterface& port);

				// Destructor for SyncAction. */
				~ClientMsgHandler();

				// The states of the statemachine.
				GCFEvent::TResult send_state(GCFEvent& event, GCFPortInterface& port);
				GCFEvent::TResult waitack_state(GCFEvent& event, GCFPortInterface& port);
								
				void SetMessage(Message *msg);

			protected:
				
			private:
				int									itsRetries; // number of atempts
				GCFPortInterface*		itsMsgPort; // the port the message come from
				GCFPortInterface*		itsBoardPort; // return port of the actual message
				
				
				
		};
	} // end TBB namespace
} // end LOFAR namespace

#endif /* CLIENTMSGACTION_H_ */
