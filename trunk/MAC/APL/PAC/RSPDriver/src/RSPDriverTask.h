//#  -*- mode: c++ -*-
//#
//#  RSPDriverTask.h: class definition for the Beam Server task.
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

#ifndef RSPDRIVERTASK_H_
#define RSPDRIVERTASK_H_

#include "RSP_Protocol.ph"

#include <GCF/GCF_Control.h>
#include <GCF/GCF_ETHRawPort.h>

#include "Scheduler.h"

namespace RSP
{
  class RSPDriverTask : public GCFTask
  {
    public:
      /**
       * Constants. Should probably be moved somewhere else at some point.
       */
      static const int N_RSPBOARDS = 2; // eventually this should be 24

    public:
      /**
       * The constructor of the RSPDriver task.
       * @param name The name of the task. The name is used for looking
       * up connection establishment information using the GTMNameService and
       * GTMTopologyService classes.
       */
      RSPDriverTask(string name);
      virtual ~RSPDriverTask();

      /**
       * Open all ports to boards.
       */
      void openBoards();

      /**
       * @return true if ready to transition to the enabled
       * state.
       */
      bool isEnabled();

      /**
       * The initial state. This state is used to connect the client
       * and board ports. When they are both connected a transition
       * to the enabled state is made.
       */
      GCFEvent::TResult initial(GCFEvent& event, GCFPortInterface &port);

      /**
       * The enabled state. In this state the task can receive
       * commands.
       */
      GCFEvent::TResult enabled(GCFEvent& event, GCFPortInterface &port);

    private:
      // ports
      GCFPort m_client;
      GCFPort m_board[N_RSPBOARDS];

      Scheduler m_scheduler;
  };

};
     
#endif /* RSPDRIVERTASK_H_ */
