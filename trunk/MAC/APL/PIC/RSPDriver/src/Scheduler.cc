//#  Scheduler.cc: implementation of the Scheduler class
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

#include "Scheduler.h"
#include "SyncAction.h"

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;
using namespace RSP_Protocol;

Scheduler::Scheduler()
{}

Scheduler::~Scheduler()
{
  /* clear the various queues */
  while (!m_later_queue.empty())
  {
      Command* c = m_later_queue.top();
      delete c;
      m_later_queue.pop();
  }
  while (!m_now_queue.empty())
  {
      Command* c = m_now_queue.top();
      delete c;
      m_now_queue.pop();
  }
  while (!m_periodic_queue.empty())
  {
      Command* c = m_periodic_queue.top();
      delete c;
      m_periodic_queue.pop();
  }
  while (!m_done_queue.empty())
  {
      Command* c = m_done_queue.top();
      delete c;
      m_done_queue.pop();
  }
  while (!m_syncactions.empty())
  {
      SyncAction* sa = m_syncactions.top();
      delete sa;
      m_syncactions.pop();
  }
}

void Scheduler::run(const GCFEvent& event, GCFPortInterface& /*port*/)
{
  const GCFTimerEvent* timeout = static_cast<const GCFTimerEvent*>(&event);
  
  if (F_TIMER == event.signal)
  {
      /* adjust the current time */
      struct timeval tv;
      tv.tv_sec  = timeout->sec;
      tv.tv_usec = 0;
      m_current_time.set(tv);

      while (!m_later_queue.empty())
      {
	  Command* command = m_later_queue.top();

	  /**
	   * Check later queue for events that need to be scheduled
	   * to the now queue.
	   */
//	  if (timeout->sec + 1 == command->m_event
      }

  }
  else
  {
      LOG_ERROR("received invalid event != F_TIMER");
  }
}

void Scheduler::dispatch(const GCFEvent& event, GCFPortInterface& port)
{
  //event = event;
  port = port;
}

void Scheduler::addSyncAction(SyncAction* action)
{
  m_syncactions.push(action);
}

Timestamp Scheduler::enter(Command* command)
{
  m_later_queue.push(command);
  
  Timestamp t = command->getTimestamp();

  if (t.sec() < m_current_time.sec() + 10)
  {
#if 0
      struct timeval tv;
      m_current_time.get(&tv);
      tv.sec += 10;
      t.set(tv);
#endif
      t = m_current_time + 10;
  }

  return t;
}
