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

	  void run(GCFEvent& event, GCFPortInterface& port, time_t tick);
	  void enter(Command command);

      private:
	  std::priority_queue<Command*> m_later_queue;
	  std::priority_queue<Command*> m_now_queue;
	  std::priority_queue<Command*> m_periodic_queue;
	  std::priority_queue<Command*> m_done_queue;

	  std::priority_queue<SyncAction*> m_syncactions;
      };
};
     
#endif /* SCHEDULER_H_ */
