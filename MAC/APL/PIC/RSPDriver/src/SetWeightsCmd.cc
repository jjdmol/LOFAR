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
#include "RSPConfig.h"
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

SetWeightsCmd::SetWeightsCmd(RSPSetweightsEvent& sw_event, GCFPortInterface& port,
			     Operation oper, int timestep)
{
  RSPSetweightsEvent* event = new RSPSetweightsEvent();
  m_event = event;
  
  event->timestamp = sw_event.timestamp + timestep;
  event->rcumask   = sw_event.rcumask;

  setOperation(oper);
  setPeriod(0);
  setPort(port);
}

SetWeightsCmd::~SetWeightsCmd()
{
  delete m_event;
}

void SetWeightsCmd::setWeights(Array<complex<int16>, BeamletWeights::NDIM> weights)
{
  RSPSetweightsEvent* event = static_cast<RSPSetweightsEvent*>(m_event);
  
  event->weights().resize(BeamletWeights::SINGLE_TIMESTEP,
			  event->rcumask.count(), weights.extent(thirdDim));
  event->weights() = weights;
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
  for (int cache_rcu = 0; cache_rcu < GET_CONFIG(N_RCU); cache_rcu++)
  {
    if (m_event->rcumask[cache_rcu])
    {
      if (cache_rcu < GET_CONFIG(N_RCU))
      {
	cache.getBeamletWeights()()(0, cache_rcu, Range::all())
	  = m_event->weights()(0, input_rcu, Range::all());
      }
      else
      {
	LOG_WARN(formatString("invalid RCU index %d, there are only %d RCU's",
			      cache_rcu, GET_CONFIG(N_RCU)));
      }

      input_rcu++;
    }
  }
}

void SetWeightsCmd::complete(CacheBuffer& /*cache*/)
{
  LOG_INFO_STR("SetWeightsCmd completed at time=" << getTimestamp());
}

const Timestamp& SetWeightsCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void SetWeightsCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool SetWeightsCmd::validate() const
{
  // validation is done in the caller (RSPDriverTask)
  return true;
}

