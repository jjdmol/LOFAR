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

#include <APLConfig.h>
#include "Scheduler.h"
#include "SyncAction.h"

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;
using namespace RSP_Protocol;

#define SCHEDULING_DELAY 2

int Scheduler::SYNC_INTERVAL_INT = 1; // default

Scheduler::Scheduler()
{
  SYNC_INTERVAL_INT = (int)trunc(GET_CONFIG("SYNC_INTERVAL", f)+0.5);
}

Scheduler::~Scheduler()
{
  /* clear the various queues */
  while (!m_later_queue.empty())    m_later_queue.pop();
  while (!m_now_queue.empty())      m_now_queue.pop();
  while (!m_periodic_queue.empty()) m_periodic_queue.pop();
  while (!m_done_queue.empty())     m_done_queue.pop();

  for (map< GCFPortInterface*, vector<SyncAction*> >::iterator port = m_syncactions.begin();
       port != m_syncactions.end();
       port++)
  {
    for (vector<SyncAction*>::iterator sa = (*port).second.begin();
	 sa != (*port).second.end();
	 sa++)
    {
      delete (*sa);
    }
  }
}

GCFEvent::TResult Scheduler::run(GCFEvent& event, GCFPortInterface& /*port*/)
{
  const GCFTimerEvent* timeout = static_cast<const GCFTimerEvent*>(&event);
  
  if (F_TIMER == event.signal)
  {
    LOG_TRACE_FLOW("Scheduler::run");

    if (!syncHasCompleted())
    {
      LOG_WARN("previous sync has not yet completed!, skipping sync");
      for (map< GCFPortInterface*, bool >::iterator it = m_sync_completed.begin();
	   it != m_sync_completed.end();
	   it++)
      {
	if (!(*it).second)
	{
	  LOG_INFO(formatString("port %s has not yet completed sync", (*it).first->getName().c_str()));
	}
      }

      return GCFEvent::NOT_HANDLED;
    }

    setCurrentTime(timeout->sec, 0);

    scheduleCommands();
    processCommands();

    initiateSync(event); // matched by completeSync
  }
  else
  {
    LOG_ERROR("\nreceived invalid event != F_TIMER\n");
  }

  return GCFEvent::HANDLED;
}

GCFEvent::TResult Scheduler::dispatch(GCFEvent& event, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;
  GCFTimerEvent timer;
  GCFEvent* current_event = &event;
  bool sync_completed = true;
  
  /**
   * Dispatch the event to the first SyncAction that
   * has not yet reached its 'final' state.
   */
  vector<SyncAction*>::iterator sa;
  int i = 0;
  for (sa = m_syncactions[&port].begin();
       sa != m_syncactions[&port].end();
       sa++, i++)
  {
    if (!(*sa)->hasCompleted())
    {
      // stil busy
      sync_completed = false;

      status = (*sa)->dispatch(*current_event, (*sa)->getBoardPort());

      //
      // if the syncaction has not yet been completed, break the loop
      // it will receive another event to continue
      //
      if (!(*sa)->hasCompleted()) break;
      else
      {
	sync_completed = true;
	current_event = &timer; 
      }
    }
  }

  if (sync_completed)
  {
    m_sync_completed[&port] = true;
  }

  if (syncHasCompleted())
  {
    completeSync();
  }

  return status;
}

bool Scheduler::syncHasCompleted()
{
  bool result = true;
  
  for (map< GCFPortInterface*, bool >::iterator it = m_sync_completed.begin();
       it != m_sync_completed.end();
       it++)
  {
    if (!(*it).second) { result = false; }
  }

  return result;
}

int Scheduler::pqueue_remove_commands(pqueue& pq,
				      GCFPortInterface& port,
				      uint32 handle)
{
  int count = 0;
  
  // copy pq
  pqueue tmp(pq);

  // clear pq, it will be filled again in the next loop
  while (!pq.empty()) pq.pop();

  while (!tmp.empty())
  {
    // pop item from the queue
    Ptr<Command> c = tmp.top();
    tmp.pop();

    // if port matches, delete c, else push back onto pq
    if ((c->getPort() == &port) && (0 == handle || &(*c) == (Command*)handle))
    {
      count++;
      // don't push back on pq, c will be deleted when it goes out of scope
    }
    else pq.push(c);
  }
  
  return count;
}

int Scheduler::cancel(GCFPortInterface& port)
{
  int count = 0;
  
  // cancel all commands related to this port
  // irrespective of the queue they are in

  count += pqueue_remove_commands(m_later_queue,    port);
  count += pqueue_remove_commands(m_now_queue,      port);
  count += pqueue_remove_commands(m_periodic_queue, port);
  count += pqueue_remove_commands(m_done_queue,     port);

  return count;
}

int Scheduler::remove_subscription(GCFPortInterface& port, uint32 handle)
{
  int count = 0;

  count += pqueue_remove_commands(m_later_queue,    port, handle);
  count += pqueue_remove_commands(m_now_queue,      port, handle);
  count += pqueue_remove_commands(m_periodic_queue, port, handle);
  count += pqueue_remove_commands(m_done_queue,     port, handle);

  return count;
}

void Scheduler::addSyncAction(SyncAction* action)
{
  m_syncactions[&(action->getBoardPort())].push_back(action);
}

Timestamp Scheduler::enter(Ptr<Command> command, QueueID queue)
{
  Timestamp scheduled_time = command->getTimestamp();

  /* determine at which time the command can actually be carried out */
  if (scheduled_time.sec() < m_current_time.sec() + SCHEDULING_DELAY)
  {
    if (scheduled_time.sec() > 0) // filter Timestamp(0,0) case
    {
      LOG_WARN(formatString("command missed deadline by %d seconds",
			    m_current_time.sec() - scheduled_time.sec()));
    }
    
    scheduled_time = m_current_time + SCHEDULING_DELAY;
  }
  else if (Command::READ == command->getOperation())
  {
    /**
     * A read command for time t should be
     * scheduled for time t + SYNC_INTERVAL_INT, to get the values
     * on time t.
     */
    scheduled_time = scheduled_time + SYNC_INTERVAL_INT;
  }
  
#if 0
  //
  // add increasing (in time) usec to commands that
  // are entered within the same second, to order
  // them properly
  //
  {
    struct timeval tv, now;
    scheduled_time.get(&tv);
    (void)gettimeofday(&now, 0);
    tv.tv_usec = now.tv_usec;
    scheduled_time.set(tv);
  }
#endif

  // set the actual time at which the command is sent to the boards
  command->setTimestamp(scheduled_time);
    
  // print time, ugly
  char timestr[32];
  time_t sec = scheduled_time.sec();
  strftime(timestr, 32, "%T", localtime(&sec));
  LOG_INFO(formatString("Scheduler::enter scheduled_time=%s.%d", timestr, scheduled_time.usec()));

  // push the command on the appropriate queue
  switch (queue)
  {
    case LATER:
      m_later_queue.push(command);
      break;

    case PERIODIC:
      m_periodic_queue.push(command);
      break;

    default:
      LOG_FATAL("invalid QueueID");
      exit(EXIT_FAILURE);
      break;
  }
  
  return Timestamp(0,0); // this return value should not be used anymore
}

void Scheduler::setCurrentTime(long sec, long usec)
{
  /* adjust the current time */
  struct timeval tv;
  tv.tv_sec  = sec;
  tv.tv_usec = usec;
  m_current_time.set(tv);
}

Timestamp Scheduler::getCurrentTime() const
{
  return m_current_time;
}

void Scheduler::scheduleCommands()
{
  /**
   * All commands with a timestamp equal to the
   * wall-clock time (m_current_time) plus one second
   * should be scheduled now.
   */
  while (!m_later_queue.empty())
  {
    Ptr<Command> command = m_later_queue.top();

    if (command->getTimestamp() < m_current_time)
    {
      /* discard old commands, the're too late! */
      LOG_WARN("discarding late command");

      m_later_queue.pop();
      //delete command;
    }
    else if (command->getTimestamp() <= m_current_time + SYNC_INTERVAL_INT)
    {
      LOG_INFO_STR("scheduling command with time=" << command->getTimestamp());

      m_now_queue.push(command);
      m_later_queue.pop();
    }
    else break;
  }

  /* copy periodic commands to the now queue */
  pqueue pq = m_periodic_queue;
  
  while (!pq.empty())
  {
    Ptr<Command> command = pq.top();

    if (command->getTimestamp() <= m_current_time + SYNC_INTERVAL_INT)
    {
      //
      // only schedule the period command when the period == 0
      // or when update period has arrived (0 == m_current_time.sec() % command->getPeriod())
      //
      if ((0 == command->getPeriod())
	  || (0 == (m_current_time.sec() % command->getPeriod())))
      {
	m_now_queue.push(command);
      }
    }

    pq.pop(); // next
  }
}

void Scheduler::processCommands()
{
  while (!m_now_queue.empty())
  {
    Ptr<Command> command = m_now_queue.top();

    /* *
     * Let the commands apply their changes to 
     * the front and back caches.
     */
    command->apply(Cache::getInstance().getFront());
    command->apply(Cache::getInstance().getBack());

    /* move from the now queue to the done queue */
    m_now_queue.pop();
    m_done_queue.push(command);
  }
}

void Scheduler::initiateSync(GCFEvent& event)
{
  m_sync_completed.clear();

  /**
   * Send the first syncaction for each board the timer
   * event to set of the data communication to each board.
   */
  for (map< GCFPortInterface*, vector<SyncAction*> >::iterator port = m_syncactions.begin();
       port != m_syncactions.end();
       port++)
  {
    // reset sync flag
    m_sync_completed[(*port).first] = false;

    for (vector<SyncAction*>::iterator sa = (*port).second.begin();
	 sa != (*port).second.end();
	 sa++)
    {
      (*sa)->setCompleted(false);
    }
    
    // dispatch F_TIMER event to first syncactions for each board
    if (!(*port).second.empty())
    {
      (*port).second[0]->dispatch(event, (*port).second[0]->getBoardPort());
    }
  }
}

void Scheduler::completeSync()
{
  // swap the buffers
  // new data from the boards which was in the back buffers
  // will end up in the front buffers.
  Cache::getInstance().swapBuffers();

  // complete any outstanding commands
  completeCommands();
}

void Scheduler::completeCommands()
{
  while (!m_done_queue.empty())
  {
    Ptr<Command> command = m_done_queue.top();
    
    /* set the timestamp */
    command->setTimestamp(getCurrentTime());
    command->complete(Cache::getInstance().getFront());

    m_done_queue.pop();
  }
}

