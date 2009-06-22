//#  SetClocksCmd.cc: implementation of the SetClocksCmd class
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

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "SetClocksCmd.h"
#include "Sequencer.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

SetClocksCmd::SetClocksCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("SetClock", port, oper)
{
  m_event = new RSPSetclockEvent(event);
}

SetClocksCmd::~SetClocksCmd()
{
  delete m_event;
}

void SetClocksCmd::ack(CacheBuffer& /*cache*/)
{
  RSPSetclockackEvent ack;

  ack.timestamp = getTimestamp();
  ack.status    = RSP_SUCCESS;
  
  getPort()->send(ack);
}

void SetClocksCmd::apply(CacheBuffer& cache, bool setModFlag)
{
  cache.getClock() = m_event->clock;
  LOG_INFO_STR(formatString("Setting clock to %d MHz @ ", m_event->clock) << getTimestamp());

  if (setModFlag) {
    Sequencer::getInstance().startSequence(Sequencer::SETCLOCK);
  }
}

void SetClocksCmd::complete(CacheBuffer& /*cache*/)
{
}

const Timestamp& SetClocksCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void SetClocksCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool SetClocksCmd::validate() const
{
  return (0   == m_event->clock ||
	  160 == m_event->clock ||
	  200 == m_event->clock);
}
