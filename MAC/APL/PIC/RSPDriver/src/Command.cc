//#  Command.cc: implementation of the Command class
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

#include "Command.h"

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;

Command::Command() : m_period(0), m_port(0), m_operation(READ), m_owner(true)
{
}

Command::~Command()
{
}

void Command::setPeriod(int16 period)
{
  m_period = period;
}

int16 Command::getPeriod()
{
  return m_period;
}

void Command::setOperation(Operation oper)
{
  m_operation = oper;
}

Command::Operation Command::getOperation() const
{
  return m_operation;
}

void Command::setPort(GCFPortInterface& port)
{
  m_port = &port;
}

GCFPortInterface* Command::getPort()
{
  return m_port;
}

#if 0
void Command::setEvent(const GCFEvent& event, GCFPortInterface& port, Operation oper, int16 period)
{
  /* copy the event to the Command */
  m_event = (GCFEvent*)new char[sizeof(event) + event.length];
  memcpy(m_event, (const char*)&event, sizeof(event) + event.length);
  m_answerport = &port;
  m_operation = oper;
  m_period = period;
}
#endif

bool Command::operator<(const Command& right)
{
  return (this->getTimestamp() < right.getTimestamp());
}

void Command::setOwner(bool owner)
{
  m_owner = owner;
}

bool Command::isOwner() const
{
  return m_owner;
}

