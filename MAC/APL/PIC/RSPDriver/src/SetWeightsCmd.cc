//#  SetWeightsCmd.cc: implementation of the SetWeightsCmd class
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
#include "SetWeightsCmd.h"

#include <blitz/array.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;
using namespace RSP_Protocol;
using namespace blitz;

SetWeightsCmd::SetWeightsCmd(GCFEvent& event, GCFPortInterface& port, Operation oper)
{
  m_event = new RSPSetweightsEvent(event);

  setOperation(oper);
  setPeriod(0);
  setPort(port);
}

SetWeightsCmd::~SetWeightsCmd()
{
  delete m_event;
}

void SetWeightsCmd::ack(CacheBuffer& /*cache*/)
{
  RSPSetweightsackEvent ack;

  ack.timestamp = getTimestamp();
  ack.status = SUCCESS;
  
  getPort()->send(ack);
}

void SetWeightsCmd::apply(CacheBuffer& cache)
{
  int input_rcu = 0;
  for (int cache_rcu = 0; cache_rcu < RSPDriverTask::N_RCU; cache_rcu++)
  {
    if (m_event->rcumask[cache_rcu])
    {
      cache.getBeamletWeights().weights()(cache_rcu, Range::all())
	= m_event->weights.weights()(input_rcu, Range::all());

      input_rcu++;
    }
  }
}

void SetWeightsCmd::complete(CacheBuffer& /*cache*/)
{
}

const Timestamp& SetWeightsCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void SetWeightsCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}
