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

/*
	Ieder commando heeft zijn eigen BoardCmdHandler, alle Tbb borden worden vanuit een module aangesproken. 
	Ieder commando spreekt dus 12 borden aan, de Tbb borden waar iets naar toe moet worden gestuurt, 
	worden bepaald door het mask.
*/

#ifndef BOARDCMDHANDLER_H_
#define BOARDCMDHANDLER_H_

#include <GCF/TM/GCF_Control.h>

#include <Command.h>

namespace LOFAR {
	namespace TBB {
	
		class BoardCmdHandler : public GCFFsm
		{
	
			public:

				// Constructors for a SyncAction object.
				// Default direction is read.
				TbbBoardCmdHandler(GCFPortInterface& board_port, int board_id, int n_indices);

				// Destructor for SyncAction. */
				~TbbBoardCmdHandler();

				// The states of the statemachine.
				GCFEvent::TResult send_state(GCFEvent& event, GCFPortInterface& port);
				GCFEvent::TResult waitack_state(GCFEvent& event, GCFPortInterface& port);
								
				void setBoardPorts(GCFPortInterface& m_board);
				bool SetCmd(Command cmd);

			protected:

			private:
				
				
			private:
				int								m_NrOfBoards;
				GCFPortInterface&	m_BoardPort[m_nr_of_boards];
				int								m_BoardId[m_nr_of_boards];
				int								m_BoardRetries;
				
				Command						m_Cmd;
		};
	};
};

#endif /* BOARDACTION_H_ */
