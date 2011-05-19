//#  -*- mode: c++ -*-
//#
//#  Command.h: RSP Driver command class
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

#ifndef COMMAND_H_
#define COMMAND_H_

#include <APL/RTCCommon/Timestamp.h>
#include "Cache.h"
#include "RefCount.h"

#include <Common/LofarTypes.h>
#include <GCF/TM/GCF_Control.h>

namespace LOFAR {
  using GCF::TM::GCFPortInterface;
  namespace RSP {

class Command : public RefCount
{
public:
	/** types */
	enum Operation {
		READ = 1,
		WRITE,
	};

	// Constructors for a Command object.
	// Currently the tv_usec part is always set to 0 irrespective
	// of the value passed in.
	Command() : 
		m_period(0), m_port(0), m_operation(READ), itsIsDelayed(false), itsIsPostponed(false), itsName("???")  { }

	Command(const string&	name, GCFPortInterface&	port, Operation	oper) : 
		m_period(0), m_port(&port), m_operation(oper), itsIsDelayed(false), itsIsPostponed(false), itsName(name) { }

	// Destructor for Command.
	virtual ~Command() { }

	// Acknowledge the command by sending the appropriate
	// response on m_port.
	virtual void ack(CacheBuffer& cache) = 0;

	// Make necessary changes to the cache for the next synchronization.
	// Any changes will be sent to the RSP boards.
	virtual void apply(CacheBuffer& cache, bool setModFlag = true) = 0;

	// Complete the command by sending the appropriate response on
	// the m_port;
	virtual void complete(CacheBuffer& cache) = 0 ;

	// Get or set the timestamp of the underlying event
	// for a command.
	virtual const RTC::Timestamp& getTimestamp() const = 0;
	virtual void setTimestamp(const RTC::Timestamp& timestamp) = 0;

	// Validate the parameters of the event.
	// @return true if they are ok.
	virtual bool validate() const = 0;

	// Only for READ commands. Decides if the answer may be read from the curretnt cache
	// or that we have to wait for another PPS pulse.
	virtual bool readFromCache() const { return (true); }

	// Compare operator to order commands in the queue
	bool operator<(const Command& right) { return (this->getTimestamp() < right.getTimestamp()); }

	/*@{*/
	// Accessor methods for the period field.
	// The period (in seconds) with which this command should
	// be executed. The command will be executed every 'period' seconds.
	void  setPeriod(uint16 period) { m_period = period; }
	int16 getPeriod() const 	   { return m_period; }
	/*@}*/

	/*@{*/
	// Set/get the type of operation READ/WRITE.
	void 	  setOperation(Operation oper)	{ m_operation = oper; }
	Operation getOperation() const 			{ return m_operation; }
	/*@}*/

	/*@{*/
	// Accessor methods for the port member.
	void setPort(GCFPortInterface& port) { m_port = &port; }
	GCFPortInterface* getPort() 					  { return m_port; }
	void resetPort() { m_port = 0; }
	/*@}*/

	/*@{*/
	// Accessor methods for the delayed flag
	void delayedResponse(bool	delayIt) { itsIsDelayed = delayIt; }
	bool delayedResponse() const		 { return (itsIsDelayed);   }
	/*@}*/

	/*@{*/
	// Accessor methods for the postpone flag
	void postponeExecution(bool	postponeIt) { itsIsPostponed = postponeIt; }
	bool postponeExecution() const			{ return (itsIsPostponed);   }
	/*@}*/

	/*@{*/
	// Accessor methods for the name
	void		  name(const  string&	aName)	{ itsName = aName; }
	const string& name() const	 			{ return (itsName);  }
	/*@}*/

private:
	uint16				m_period;
	GCFEvent*			m_event;
	GCFPortInterface*	m_port;
	Operation			m_operation;
	bool				itsIsDelayed;
	bool				itsIsPostponed;
	string				itsName;
};

// Comparison function to order a priority_queue of Ptr<Command>* pointers
// as it is used in the Scheduler class.
struct Command_greater { 
	bool operator() (Ptr<Command>& x, Ptr<Command>& y) const 
	{ return x->getTimestamp() > y->getTimestamp(); }
};

  }; // namespace RSP
}; // namespace LOFAR

#endif /* COMMAND_H_ */
