//#  -*- mode: c++ -*-
//#
//#  RSPDriver.h: class definition for the Beam Server task.
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

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RSP_Protocol/EPA_Protocol.ph>

#include <GCF/TM/GCF_Control.h>
#include <GCF/TM/GCF_ETHRawPort.h>
#include <GCF/TM/GCF_DevicePort.h>

#include "Scheduler.h"

#include <list>

#ifdef HAVE_SYS_TIMEPPS_H
#include <sys/timepps.h>
#endif

namespace LOFAR {
  namespace RSP {

    class RSPDriver : public GCFTask
    {
    public:
      /**
       * The constructor of the RSPDriver task.
       * @param name The name of the task. The name is used for looking
       * up connection establishment information using the GTMNameService and
       * GTMTopologyService classes.
       */
      RSPDriver(string name);
      virtual ~RSPDriver();
      
      /**
       * Add all required synchronization actions.
       */
      void addAllSyncActions();

      /**
       * Open or close all ports to boards.
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
       * Delete the client ports on the m_dead_clients.
       */
      void RSPDriver::undertaker();

      /**
       * The enabled state. In this state the task can receive
       * commands.
       */
      GCFEvent::TResult enabled(GCFEvent& event, GCFPortInterface &port);

      /**
       * Is this port connected to a board?
       */
      bool isBoardPort(GCFPortInterface& port);

      /**
       * Handle a F_DATAIN on the clock port.
       */
      GCFEvent::TResult clock_tick(GCFPortInterface& port);

      /*@{*/
      /**
       * Handlers for the different events.
       */
      void rsp_setweights(GCFEvent& event, GCFPortInterface &port);
      void rsp_getweights(GCFEvent& event, GCFPortInterface &port);

      void rsp_setsubbands  (GCFEvent& event, GCFPortInterface &port);
      void rsp_getsubbands  (GCFEvent& event, GCFPortInterface &port);
      void rsp_subsubbands  (GCFEvent& event, GCFPortInterface &port);
      void rsp_unsubsubbands(GCFEvent& event, GCFPortInterface &port);

      void rsp_setrcu  (GCFEvent& event, GCFPortInterface &port);
      void rsp_getrcu  (GCFEvent& event, GCFPortInterface &port);
      void rsp_subrcu  (GCFEvent& event, GCFPortInterface &port);
      void rsp_unsubrcu(GCFEvent& event, GCFPortInterface &port);

      void rsp_setwg(GCFEvent& event, GCFPortInterface &port);
      void rsp_getwg(GCFEvent& event, GCFPortInterface &port);

      void rsp_substatus  (GCFEvent& event, GCFPortInterface &port);
      void rsp_unsubstatus(GCFEvent& event, GCFPortInterface &port);
      void rsp_getstatus  (GCFEvent& event, GCFPortInterface &port);

      void rsp_substats  (GCFEvent& event, GCFPortInterface &port);
      void rsp_unsubstats(GCFEvent& event, GCFPortInterface &port);
      void rsp_getstats  (GCFEvent& event, GCFPortInterface &port);

      void rsp_subxcstats  (GCFEvent& event, GCFPortInterface &port);
      void rsp_unsubxcstats(GCFEvent& event, GCFPortInterface &port);
      void rsp_getxcstats  (GCFEvent& event, GCFPortInterface &port);

      void rsp_getversions(GCFEvent& event, GCFPortInterface &port);

      void rsp_getconfig(GCFEvent& event, GCFPortInterface &port);
      void rsp_setclocks(GCFEvent& event, GCFPortInterface &port);
      void rsp_getclocks(GCFEvent& event, GCFPortInterface &port);
      void rsp_subclocks(GCFEvent& event, GCFPortInterface &port);
      void rsp_unsubclocks(GCFEvent& event, GCFPortInterface &port);
      /*@}*/

    private:
      // ports
      GCFDevicePort  m_clock;    // clock pulse will arrive on this port
      GCFTCPPort     m_acceptor; // listen for clients on this port
      GCFETHRawPort* m_board;    // array of ports, one for each RSP board
      std::list<GCFPortInterface*> m_client_list;  // list of clients
      std::list<GCFPortInterface*> m_dead_clients; // list of clients to cleanup

      Scheduler m_scheduler;
      int m_update_counter; // nr of updates completed in one second
      int m_n_updates;      // number of completed updates
      int m_elapsed;        // elapsed number of seconds

#ifdef HAVE_SYS_TIMEPPS_H
      int          m_ppsfd;     // file descriptor for PPS device
      pps_handle_t m_ppshandle; // handle to PPS API interface
      pps_info_t   m_ppsinfo;   // most recent ppsinfo
#endif
    };

  };
};
     
#endif /* RSPDRIVERTASK_H_ */
