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
{
}

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

GCFEvent::TResult Scheduler::run(GCFEvent& event, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;
  const GCFTimerEvent* timeout = static_cast<const GCFTimerEvent*>(&event);
  
  if (F_TIMER == event.signal)
    {
      LOG_INFO("Scheduler::run");

      setCurrentTime(timeout->sec, 0);

      scheduleCommands();
      processCommands();
      status = syncCache(event, port);
      m_cache.swapBuffers();
      completeCommands();
    }
  else
    {
      LOG_ERROR("received invalid event != F_TIMER");
    }

  return status;
}

GCFEvent::TResult Scheduler::dispatch(GCFEvent& event, GCFPortInterface& port)
{
  return syncCache(event, port);
}

void Scheduler::addSyncAction(SyncAction* action)
{
  m_syncactions.push(action);
}

Timestamp Scheduler::enter(Command* command)
{
  m_later_queue.push(command);
  
  Timestamp t = command->getTimestamp();

  /* determine at which time the command can actually be carried out */
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

void Scheduler::setCurrentTime(long sec, long usec)
{
  /* adjust the current time */
  struct timeval tv;
  tv.tv_sec  = sec;
  tv.tv_usec = usec;
  m_current_time.set(tv);
}

void Scheduler::scheduleCommands()
{
  /* move appropriate client commands to the now queue */
  while (!m_later_queue.empty())
    {
      Command* command = m_later_queue.top();

      if (m_current_time > command->getTimestamp() + 1)
	{
	  /* discard old commands, the're too late! */
	  LOG_WARN("discarding late command");

	  m_later_queue.pop();
	  delete command;
	}
      else if (m_current_time == command->getTimestamp() + 1)
	{
	  m_now_queue.push(command);
	  m_later_queue.pop();
	}
      else break;
    }

#if 0
  /* copy period commands to the now queue */
  while (!m_period_queue.empty())
    {
      Command * command = m_period_queue.top();

      struct timeval now;
      m_current_time.get(&now);
      if (0 == (now.tv_sec + 1 % command->getPeriod()))
	{
	  /* copy the command and push on the now queue */
	}
    }
#endif
}

void Scheduler::processCommands()
{
  while (!m_now_queue.empty())
    {
      Command* command = m_now_queue.top();

      /* let the command apply its changes to the cache */
      command->apply(m_cache.getBack());

      /* move from the now queue to the done queue */
      m_now_queue.pop();
      m_done_queue.push(command);
    }
}

GCFEvent::TResult Scheduler::syncCache(GCFEvent& event, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  /* copy the m_syncactions queue */
  std::priority_queue<SyncAction*> runqueue = m_syncactions;

  if (!runqueue.empty())
    {
      SyncAction* action = runqueue.top();

      status = action->dispatch(event, port);

      if (action->isFinal()) runqueue.pop();
    }

  return status;
}

void Scheduler::completeCommands()
{
  while (!m_done_queue.empty())
    {
      Command* command = m_done_queue.top();

      command->complete(m_cache.getFront());

      m_done_queue.pop();
      delete command;
    }
}

