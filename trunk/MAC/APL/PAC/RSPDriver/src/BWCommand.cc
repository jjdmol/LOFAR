//#  BWCommand.cc: implementation of the BWCommand class
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

#include "RSP_Protocol.ph"
#include "RSPDriverTask.h"
#include "BWCommand.h"

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;
using namespace RSP_Protocol;

BWCommand::BWCommand(GCFEvent& event, GCFPortInterface& port, Operation oper)
{
  m_event = new RSPSetweightsEvent(event);

  setOperation(oper);
  setPeriod(0);
  setPort(port);
}

BWCommand::~BWCommand()
{
  delete m_event;
}

void BWCommand::ack(CacheBuffer& /*cache*/)
{
  RSPSetweightsackEvent ack;

  ack.timestamp = getTimestamp();
  ack.status = SUCCESS;
  
  getPort()->send(ack);
}

void BWCommand::apply(CacheBuffer& cache)
{
  int inrcu = 0;
  for (int outrcu = 0; outrcu < RSPDriverTask::N_RCU; outrcu++)
  {
    if (m_event->rcumask[outrcu])
    {
      cache.getBeamletWeights(outrcu).weights()
	= m_event->weights.weights()(inrcu);

      inrcu++;
    }
  }
}

void BWCommand::complete(CacheBuffer& /*cache*/)
{
}

const Timestamp& BWCommand::getTimestamp()
{
  return m_event->timestamp;
}

void BWCommand::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}
