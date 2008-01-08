//#  UpdRegisterStateCmd.cc: implementation of the UpdRegisterStateCmd class
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
#include "UpdRegisterStateCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

UpdRegisterStateCmd::UpdRegisterStateCmd(GCFEvent& event, GCFPortInterface& port, Operation oper)
{
  m_event = new RSPSubclockEvent(event);

  setOperation(oper);
  setPeriod(m_event->period);
  setPort(port);
}

UpdRegisterStateCmd::~UpdRegisterStateCmd()
{
  delete m_event;
}

void UpdRegisterStateCmd::ack(CacheBuffer& /*cache*/)
{
  // intentionally left empty
}

void UpdRegisterStateCmd::apply(CacheBuffer& /*cache*/, bool /*setModFlag*/)
{
  // no-op
}

void UpdRegisterStateCmd::complete(CacheBuffer& cache)
{
  RSPUpdregisterstateEvent ack;

  ack.timestamp = getTimestamp();
  ack.status = SUCCESS;
  ack.handle = (memptr_t)this; // opaque pointer used to refer to the subscription

  ack.state = cache.getCache().getState();

  getPort()->send(ack);
}

const Timestamp& UpdRegisterStateCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void UpdRegisterStateCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool UpdRegisterStateCmd::validate() const
{
  return (true);
}
