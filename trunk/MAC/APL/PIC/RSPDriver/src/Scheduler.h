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
#include <GCF/GCF_Control.h>
#include "Command.h"
#include "SyncAction.h"
#include "Timestamp.h"
#include "Cache.h"

namespace RSP
{
  class Scheduler
      {
      public:
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
	   * Add a synchronization action to be carried out
	   * periodically with the specified period.
	   */
	  void addSyncAction(SyncAction* action);

	  /**
	   * Enter a new command into the scheduler in response
	   * to receiving a command event from on of the RSPDriver
	   * client processes.
	   */
	  RSP_Protocol::Timestamp enter(Command* command);

	  /**
	   * Set the current time (from the update triggering timeout event).
	   */
	  void setCurrentTime(long sec, long usec);

      private:
	  /*@{*/
	  /**
	   * Helper methods for the Scheduler::run method.
	   */
	  void              scheduleCommands();
	  void              processCommands();
	  GCFEvent::TResult syncCache(GCFEvent& event, GCFPortInterface& port);
	  void              completeCommands();
	  /*@}*/

	  std::priority_queue<Command*> m_later_queue;
	  std::priority_queue<Command*> m_now_queue;
	  std::priority_queue<Command*> m_periodic_queue;
	  std::priority_queue<Command*> m_done_queue;

	  std::priority_queue<SyncAction*> m_syncactions;

	  RSP_Protocol::Timestamp m_current_time;

	  Cache m_cache;
      };
};
     
#endif /* SCHEDULER_H_ */
