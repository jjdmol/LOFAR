//#  -*- mode: c++ -*-
//#
//#  GetVersions.h: Get Tbb board versions
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

#ifndef GETVERSIONS_H_
#define GETVERSIONS_H_

#include "BoardAction.h"
#include <APL/TBB_Protocol/TBB_Protocol.ph>
#include <APL/TBB_Protocol/TP_Protocol.ph>

#include <Common/LofarTypes.h>
#include <GCF/TM/GCF_Control.h>

namespace LOFAR {
  namespace TBB {

		class GetVersions : public BoardAction 
		{
			public:
				/// Constructors for a GetVersions object.
				GetVersionsCmd(GCFEvent& event);
	  
				/// Destructor for GetVersions.
				virtual ~GetVersionsCmd();
				
				/// Build the message that will be send to the board
				///
				virtual void tp_buildrequest();
				
				/// send the message to the given port
				///
				virtual void tp_sendrequest(GCFPortInterface& port);
				
				/// wait for response from the boards
				///
				virtual GCFEvent::TResult tp_handleack(GCFEvent& event, GCFPortInterface& /*port*/);
						
				/// Acknowledge the command by sending the appropriate
				/// response on m_port.
				virtual void tbb_sendack();
				
				/// Make necessary changes to the cache for the next synchronization.
				/// Any changes will be sent to the RSP boards.
				virtual void apply();

				/// Complete the command by sending the appropriate response on
				/// the m_answerport;
				virtual void complete();


				/// Validate the event that underlies the command.
				virtual bool validate() const;

      
			private:
				GetVersions();

				TBBVersionEvent 		tbb_event;
				TBBVersionackEvent	tbb_ackevent;
				TPVersionEvent			tp_event;
		};
	};
};

#endif /* GETVERSIONS_H_ */
