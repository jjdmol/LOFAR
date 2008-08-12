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

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <APL/RTCCommon/PSAccess.h>
#include "Scheduler.h"
#include "SyncAction.h"

using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

#define SCHEDULING_DELAY 2

int Scheduler::SYNC_INTERVAL_INT = 1; // default

Scheduler::Scheduler()
{
	SYNC_INTERVAL_INT = (int)trunc(GET_CONFIG("RSPDriver.SYNC_INTERVAL", f)+0.5);
}

Scheduler::~Scheduler()
{
	/* clear the various queues */
	while (!m_later_queue.empty())    m_later_queue.pop();
	while (!m_now_queue.empty())      m_now_queue.pop();
	while (!m_periodic_queue.empty()) m_periodic_queue.pop();
	while (!m_done_queue.empty())     m_done_queue.pop();

	for (map< GCFPortInterface*, vector<SyncAction*> >::iterator port = m_syncactions.begin();
			port != m_syncactions.end(); port++) {
		for (vector<SyncAction*>::iterator sa = (*port).second.begin();
				sa != (*port).second.end(); sa++) {
			delete (*sa);
		}
	}
}

//
// run(event, port)
//
GCFEvent::TResult Scheduler::run(GCFEvent& event, GCFPortInterface& /*port*/)
{
	const GCFTimerEvent* timeout = static_cast<const GCFTimerEvent*>(&event);

	if (F_TIMER == event.signal) {
		LOG_DEBUG("Scheduler::run");

		if (!syncHasCompleted()) {
			LOG_ERROR("previous sync has not yet completed!, skipping sync");
			for (map< GCFPortInterface*, bool >::iterator it = m_sync_completed.begin();
					it != m_sync_completed.end(); it++) {
				if (!(*it).second) {
					LOG_WARN(formatString("port %s has not yet completed sync, trying to continue...",
							(*it).first->getName().c_str()));
				}

				// reset the statemachines of all SyncActions for this port
				resetSync(*(*it).first);
			}

			// This is caused by a problem with the firmware
			// or a loose cable, simply try to continue.
			//
			// In the future some more elaborate fault handling
			// might be required.
			completeSync();
		}

		// round to nearest second t, warn if time is too far off from top of second
		//
		//        t-1        t         t+1
		//            XXXX|<--->|XXXX
		unsigned long topofsecond = timeout->sec;
		if (timeout->usec >= 1e6-1000) {
			topofsecond++; // round to next whole second
		}
		if (timeout->usec > 1000 && timeout->usec < 1e6 - 1000) {
			LOG_WARN_STR("Scheduler time too far off from top off second: usec=" << timeout->usec);
		}
		setCurrentTime(topofsecond, 0);

		scheduleCommands();
		processCommands();

		initiateSync(event); // matched by completeSync
	}
	else {
		LOG_ERROR("\nreceived invalid event != F_TIMER\n");
	}

	return GCFEvent::HANDLED;
}

//
// dispatch (event, port)
//
GCFEvent::TResult Scheduler::dispatch(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::NOT_HANDLED;
	GCFTimerEvent 	  timer;
	GCFEvent* 	 	  current_event = &event;
	bool 			  sync_completed = true;

	// Dispatch the event to the first SyncAction that
	// has not yet reached its 'final' state.
	vector<SyncAction*>::iterator sa;
	int i = 0;
	for (sa = m_syncactions[&port].begin(); sa != m_syncactions[&port].end(); sa++, i++) {
		if (!(*sa)->hasCompleted()) {
			// stil busy
			sync_completed = false;
			status = (*sa)->dispatch(*current_event, (*sa)->getBoardPort());

			// if the syncaction has not yet been completed, break the loop
			// it will receive another event to continue
			if (!(*sa)->hasCompleted()) {
				break;
			}
			else {
				sync_completed = true;
				current_event = &timer; 
			}
		}
	}

	if (sync_completed) {
		m_sync_completed[&port] = true;
	}

	if (syncHasCompleted()) {		// all boards ready???
		completeSync();
	}

	return status;
}

//
// syncHasCompleted
//
// Check if all boards have finished their sequence.
//
bool Scheduler::syncHasCompleted()
{
	for (map< GCFPortInterface*, bool >::iterator it = m_sync_completed.begin();
			it != m_sync_completed.end(); it++) {
		if (!(*it).second) { 
			return(false);
		}
	}

	return (true);
}

//
// pqueue_remove_commands(pqueue, port, memptr)
//
int Scheduler::pqueue_remove_commands(pqueue&			pq,
									  GCFPortInterface& port,
									  memptr_t 			handle)
{
	int count = 0;

	// copy pq
	pqueue tmp(pq);

	// clear pq, it will be filled again in the next loop
	while (!pq.empty())  {
		pq.pop();
	}

	while (!tmp.empty()) {
		// pop item from the queue
		Ptr<Command> c = tmp.top();
		tmp.pop();

		// if port matches, delete c, else push back onto pq
		if ((c->getPort() == &port) && (0 == handle || &(*c) == (Command*)handle)) {
			count++;
			// don't push back on pq, c will be deleted when it goes out of scope
		}
		else {
			pq.push(c);
		}
	}

	return count;
}

//
// cancel(port)
//
int Scheduler::cancel(GCFPortInterface& port)
{
	// cancel all commands related to this port
	// irrespective of the queue they are in
	int count = 0;
	count += pqueue_remove_commands(m_later_queue,    port);
	count += pqueue_remove_commands(m_now_queue,      port);
	count += pqueue_remove_commands(m_periodic_queue, port);
	count += pqueue_remove_commands(m_done_queue,     port);

	return count;
}

//
// remove_subscription(port, memptr)
//
int Scheduler::remove_subscription(GCFPortInterface& port, memptr_t handle)
{
	int count = 0;
	count += pqueue_remove_commands(m_later_queue,    port, handle);
	count += pqueue_remove_commands(m_now_queue,      port, handle);
	count += pqueue_remove_commands(m_periodic_queue, port, handle);
	count += pqueue_remove_commands(m_done_queue,     port, handle);

	return count;
}

//
// addSyncAction(action)
//
void Scheduler::addSyncAction(SyncAction* action)
{
	m_syncactions[&(action->getBoardPort())].push_back(action);
}

//
// enter(command, queue)
//
Timestamp Scheduler::enter(Ptr<Command> command, QueueID queue)
{
	Timestamp scheduled_time = command->getTimestamp();

	/* determine at which time the command can actually be carried out */
	if (scheduled_time.sec() < m_current_time.sec() + SCHEDULING_DELAY) {
		if (scheduled_time.sec() > 0)  { // filter Timestamp(0,0) case
			LOG_WARN(formatString("command missed deadline by %d seconds",
			m_current_time.sec() - scheduled_time.sec()));
		}

		scheduled_time = m_current_time + (long)SCHEDULING_DELAY;
	}

	// set the actual time at which the command is sent to the boards
	command->setTimestamp(scheduled_time);

	// print time, ugly
	char timestr[32];
	time_t sec = scheduled_time.sec();
	strftime(timestr, 32, "%T", gmtime(&sec));
	LOG_DEBUG(formatString("Scheduler::enter scheduled_time=%s.%d UTC", timestr, scheduled_time.usec()));

	// push the command on the appropriate queue
	switch (queue) {
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

//
// setCurrentTime(sec, usec)
//
void Scheduler::setCurrentTime(long sec, long usec)
{
	/* adjust the current time */
	m_current_time = Timestamp(sec, usec);
}

//
// getCurrentTime()
//
Timestamp Scheduler::getCurrentTime() const
{
	return m_current_time;
}

//
// scheduleCommands()
//
// Schedule commands from later-queue and periodic-queue by setting them in the now-queue
//
void Scheduler::scheduleCommands()
{
	int scheduling_offset = 0;

	// All commands with a timestamp equal to the
	// wall-clock time (m_current_time) plus one second
	// should be scheduled now.
	while (!m_later_queue.empty()) {
		Ptr<Command> command = m_later_queue.top();

		// write commands need to be scheduled on sync period ahead of time
		// to be effective in the hardware at the specified timestamp
		scheduling_offset = 0;
		if (Command::WRITE == command->getOperation()) {
			scheduling_offset = SYNC_INTERVAL_INT;
		}

		/* detect late commands, but just execute them */
		if (command->getTimestamp() <= m_current_time + (long)(scheduling_offset - SYNC_INTERVAL_INT)) {
			LOG_WARN_STR("command is late, timestamp=" << command->getTimestamp()
						<< ", current_time=" << m_current_time);
		}

		if (command->getTimestamp() <= m_current_time + (long)scheduling_offset) {
			LOG_DEBUG_STR("scheduling command with time=" << command->getTimestamp());

			m_now_queue.push(command);
			m_later_queue.pop();
		}
		else {
			break;
		}
	}

	/* copy periodic commands to the now queue */
	pqueue pq = m_periodic_queue;

	while (!pq.empty()) {
		Ptr<Command> command = pq.top();

		// read commands need to be scheduled on sync period ahead of time
		// to be effective in the hardware at the specified timestamp
		scheduling_offset = 0;
		if (Command::WRITE == command->getOperation()) {
			scheduling_offset = SYNC_INTERVAL_INT;
		}

		/* detect late commands, but just execute them */
		if (command->getTimestamp() <= m_current_time + (long)(scheduling_offset - SYNC_INTERVAL_INT)) {
			LOG_WARN_STR("periodic command is late, timestamp=" << command->getTimestamp()
			<< ", current_time=" << m_current_time);
		}

		if (command->getTimestamp() <= m_current_time + (long)scheduling_offset) {
			LOG_DEBUG_STR("scheduling periodic command with time=" << command->getTimestamp());
			m_now_queue.push(command);
		}

		pq.pop(); // next
	}
}

//
// processCommands()
//
// Call 'apply' on every command in the now_queue
//
void Scheduler::processCommands()
{
	Cache::getInstance().getFront().setTimestamp(getCurrentTime());
	Cache::getInstance().getFront().setTimestamp(getCurrentTime());

	while (!m_now_queue.empty()) {
		Ptr<Command> command = m_now_queue.top();

		// Let the commands apply their changes to 
		// the front and back caches.
		command->apply(Cache::getInstance().getFront(), true);
		command->apply(Cache::getInstance().getBack(), false);

		// move from the now queue to the done queue
		m_now_queue.pop();
		m_done_queue.push(command);
	}
}

//
// initiateSync(event)
//
void Scheduler::initiateSync(GCFEvent& event)
{
	m_sync_completed.clear();

	// Send the first syncaction for each board the timer
	// event to set of the data communication to each board.
	for (map< GCFPortInterface*, vector<SyncAction*> >::iterator port = m_syncactions.begin();
			port != m_syncactions.end(); port++) { 
		// first reset our sync flag
		m_sync_completed[(*port).first] = false;

		// also mark all commands as not completed yet.
		for (vector<SyncAction*>::iterator sa = (*port).second.begin();
				sa != (*port).second.end(); sa++) { 
			(*sa)->setCompleted(false);
		}

		// dispatch F_TIMER event to first syncactions for each board
		if (!(*port).second.empty()) {
			for (unsigned int i = 0; i < (*port).second.size(); i++) {
				(*port).second[i]->dispatch(event, (*port).second[i]->getBoardPort());
				if (!(*port).second[i]->doContinue()) {
					break;
				}
			}
		}
	}
}

//
// resetSync(port)
//
// Reset the state machines of all Sync Actions
// for the port, to attempt another sync in the next
// update period.
//
void Scheduler::resetSync(GCFPortInterface& port)
{
	vector<SyncAction*>::iterator sa;
	for (sa = m_syncactions[&port].begin(); sa != m_syncactions[&port].end(); sa++) {
		(*sa)->reset();
	}
}

//
//completeSync()
//
void Scheduler::completeSync()
{
	// print current state for all registers
	ostringstream logStream;
	Cache::getInstance().getState().print(logStream);
	LOG_DEBUG_STR(logStream);

	// swap the buffers
	// new data from the boards which was in the back buffers
	// will end up in the front buffers.
	Cache::getInstance().swapBuffers();

	// complete any outstanding commands
	completeCommands();

	// clear from DONE to IDLE state
	Cache::getInstance().getState().clear();

	// schedule next update
	Cache::getInstance().getState().schedule(); // if IDLE transition to READ or CHECK
}

//
// completeCommands()
//
void Scheduler::completeCommands()
{
	while (!m_done_queue.empty()) {
		Ptr<Command> command = m_done_queue.top();

		command->complete(Cache::getInstance().getFront());

		// re-timestamp periodic commands for the next period
		if (command->getPeriod()) {
			// Set the next time at which the periodic command should be executed.
			//
			// This is not as simple as doing command->setTimestamp(command->getTimestamp() + command->getPeriod());
			// because command->getTimestamp() returns the previous time at which the command
			// was executed. In some cases (due to missed PPSes) the sum of the previous timestamp
			// and the period will be in the past causing the periodic command to be logged as
			// 'late' from that point onwards.
			//
			// To correctly compute the next time at which a periodic command should execute, take
			// the absolute current time and add the period, but make sure the periodic command
			// continues to be executed on the grid defined by the period. E.g. if a command is 
			// executed every 4 seconds starting on second 1, so 1,5,9, etc and some PPSes are missed
			// lets say PPS 10,11,12,13 (and current time is 13) then it should continue at time 17.
			//
			Timestamp newtime = getCurrentTime()
								+ ((long)command->getPeriod()
								- ((long)command->getTimestamp() % (long)command->getPeriod()));
			command->setTimestamp(newtime);
		}

		m_done_queue.pop();
	}
}

