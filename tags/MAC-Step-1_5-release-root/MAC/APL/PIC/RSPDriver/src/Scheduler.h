//#  -*- mode: c++ -*-
//#
//#  Scheduler.h: RSP Driver scheduler
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

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <queue>
#include <GCF/TM/GCF_Control.h>
#include "Command.h"
#include "SyncAction.h"
#include <APL/RTCCommon/Timestamp.h>

namespace LOFAR {
  namespace RSP {

    class Scheduler
    {
    public:
      /**
       * Types of queue.
       */
      enum QueueID
	{
	  LATER = 1,
	  PERIODIC
	};
      
      /**
       * Constructors for a Scheduler object.
       * Currently the tv_usec part is always set to 0 irrespective
       * of the value passed in.
       */
      Scheduler();
	  
      /* Destructor for Scheduler. */
      virtual ~Scheduler();

      /**
       * Run the scheduler in response to a timer event.
       */
      GCFEvent::TResult run(GCFEvent& event, GCFPortInterface& port);
      /**
       * Dispatch an event from the RSP board to the appropriate
       * synchronization action.
       */
      GCFEvent::TResult dispatch(GCFEvent& event, GCFPortInterface& port);

      /**
       * Indicate whether synchronization of all data has completed.
       */
      bool syncHasCompleted();

      /**
       * Cancel all commands in any queue for this port.
       */
      int cancel(GCFPortInterface& port);

      /**
       * Remove commands matching the specified port and handle.
       */
      int remove_subscription(GCFPortInterface& port, memptr_t handle);
      
      /**
       * Add a synchronization action to be carried out
       * periodically on the specified board.
       */
      void addSyncAction(SyncAction* action);

      /**
       * Enter a new command into the scheduler in response
       * to receiving a command event from on of the RSPDriver
       * client processes.
       */
      RTC::Timestamp enter(Ptr<Command> command, QueueID queue = LATER);

      /*@{*/
      /**
       * Set/get the current time (from the update triggering timeout event).
       */
      void setCurrentTime(long sec, long usec);
      RTC::Timestamp getCurrentTime() const;
      /*@}*/

    private:
      /**
       * Private types.
       */
      typedef std::priority_queue<Ptr<Command>, std::vector<Ptr<Command> >, RSP::Command_greater> pqueue;
      //typedef std::priority_queue<Ptr<Command> > pqueue;

      /**
       * Private helper methods.
       */
      int pqueue_remove_commands(pqueue& p,
				 GCFPortInterface& port,
				 memptr_t handle = 0);

      /**
       * Constants from the config file converted to the correct type.
       */

      static int SYNC_INTERVAL_INT;
      /*@{*/
      /**
       * Helper methods for the Scheduler::run method.
       */
      void              scheduleCommands();
      void              processCommands();
      void              resetSyncActions();
      void              initiateSync(GCFEvent& event);
      //GCFEvent::TswitchResult syncCache(GCFEvent& event, GCFPortInterface& port);
      void              resetSync(GCFPortInterface& port);
      void              completeSync();
      void              completeCommands();
      /*@}*/

      pqueue m_later_queue;
      pqueue m_now_queue;
      pqueue m_periodic_queue;
      pqueue m_done_queue;

      std::map< GCFPortInterface*,
				std::vector<SyncAction*> > m_syncactions;
      std::map< GCFPortInterface*, bool >  m_sync_completed;

      RTC::Timestamp m_current_time;

      int m_current_priority;
    };
  };
};
     
#endif /* SCHEDULER_H_ */
