//#  -*- mode: c++ -*-
//#
//#  SetSplitterCmd.h: Set SerdesRing splitter on or off.
//#
//#  Copyright (C) 2009
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

#ifndef SETSPLITTERCMD_H_
#define SETSPLITTERCMD_H_

#include "Command.h"
#include <APL/RSP_Protocol/RSP_Protocol.ph>

#include <Common/LofarTypes.h>
#include <GCF/TM/GCF_Control.h>

namespace LOFAR {
  namespace RSP {

class SetSplitterCmd : public Command
{
public:
	// Constructors for a SetSplitterCmd object.
	SetSplitterCmd(GCFEvent& event, GCFPortInterface& port, Operation oper);

	// Destructor for SetSplitterCmd. */
	virtual ~SetSplitterCmd();

	// Acknowledge the command by sending the appropriate
	// response on m_port.
	virtual void ack(CacheBuffer& cache);

	// Make necessary changes to the cache for the next synchronization.
	// Any changes will be sent to the RSP boards.
	virtual void apply(CacheBuffer& cache, bool setModFlag = true);

	// Complete the command by sending the appropriate response on
	// the m_answerport;
	virtual void complete(CacheBuffer& cache);

	/*@{*/
	// get timestamp of the event
	virtual const RTC::Timestamp& getTimestamp() const;
	virtual void setTimestamp(const RTC::Timestamp& timestamp);
	/*@}*/

	// Validate the event that underlies the command.
	virtual bool validate() const;

private:
	SetSplitterCmd();

	RSPSetsplitterEvent* itsEvent;
};

  }; // namespace RSP
}; // namespace LOFAR
     
#endif /* SETSUBBANDSCMD_H_ */
