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
#include "EPA_Protocol.ph"

#include <GCF/GCF_Control.h>
#include <GCF/GCF_ETHRawPort.h>

#include "Scheduler.h"

#include <list>

namespace RSP
{
  class RSPDriverTask : public GCFTask
  {
    public:
      /**
       * Constants. Should probably be moved somewhere else at some point.
       */
      static const int N_RSPBOARDS = 2; // eventually this should be 24
      static const int N_RCU = N_RSPBOARDS * N_BLP; // eventually should be 24 * 8 = 192

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
       * Add all required synchronization actions.
       */
      void addAllSyncActions();

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
       * Delete the client ports on the m_garbage_list.
       */
      void RSPDriverTask::collect_garbage();

      /**
       * The enabled state. In this state the task can receive
       * commands.
       */
      GCFEvent::TResult enabled(GCFEvent& event, GCFPortInterface &port);

      /*@{*/
      /**
       * Handlers for the different events.
       */
      void rsp_setweights(GCFEvent& event, GCFPortInterface &port);
      void rsp_getweights(GCFEvent& event, GCFPortInterface &port);

      void rsp_setsubbands(GCFEvent& event, GCFPortInterface &port);
      void rsp_getsubbands(GCFEvent& event, GCFPortInterface &port);

      void rsp_setrcu(GCFEvent& event, GCFPortInterface &port);
      void rsp_getrcu(GCFEvent& event, GCFPortInterface &port);

      void rsp_setwg(GCFEvent& event, GCFPortInterface &port);
      void rsp_getwg(GCFEvent& event, GCFPortInterface &port);

      void rsp_substatus(GCFEvent& event, GCFPortInterface &port);
      void rsp_unsubstatus(GCFEvent& event, GCFPortInterface &port);
      void rsp_getstatus(GCFEvent& event, GCFPortInterface &port);

      void rsp_substats(GCFEvent& event, GCFPortInterface &port);
      void rsp_unsubstats(GCFEvent& event, GCFPortInterface &port);
      void rsp_getstats(GCFEvent& event, GCFPortInterface &port);

      void rsp_getversions(GCFEvent& event, GCFPortInterface &port);
      /*@}*/

    private:
      // ports
      GCFTCPPort m_acceptor; // listen for clients on this port
      GCFPort m_board[N_RSPBOARDS];
      std::list<GCFPortInterface*> m_client_list;  // list of clients
      std::list<GCFPortInterface*> m_garbage_list; // list of clients to cleanup

      Scheduler m_scheduler;
  };

};
     
#endif /* RSPDRIVERTASK_H_ */
