//#  UpdStatusCmd.cc: implementation of the UpdStatusCmd class
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
#include "UpdStatusCmd.h"

#include <blitz/array.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;
using namespace RSP_Protocol;
using namespace blitz;

UpdStatusCmd::UpdStatusCmd(GCFEvent& event, GCFPortInterface& port, Operation oper)
{
  m_event = new RSPSubstatusEvent(event);

  setOperation(oper);
  setPeriod(0);
  setPort(port);
}

UpdStatusCmd::~UpdStatusCmd()
{
  if (isOwner()) delete m_event;
}

void UpdStatusCmd::ack(CacheBuffer& /*cache*/)
{
  // intentionally left empty
}

void UpdStatusCmd::apply(CacheBuffer& /*cache*/)
{
  // no-op
}

void UpdStatusCmd::complete(CacheBuffer& cache)
{
  RSPUpdstatusEvent ack;

  ack.timestamp = getTimestamp();
  ack.status = SUCCESS;
  ack.handle = 1; // should be greater than 0!

  ack.sysstatus.board().resize(GET_CONFIG("N_RSPBOARDS", i));
  ack.sysstatus.board() = cache.getSystemStatus().board();

  ack.sysstatus.rcu().resize(m_event->rcumask.count());

  int result_rcu = 0;
  for (int cache_rcu = 0; cache_rcu < GET_CONFIG("N_RCU", i); cache_rcu++)
  {
    if (m_event->rcumask[result_rcu])
    {
      if (result_rcu < GET_CONFIG("N_RCU", i))
      {
	ack.sysstatus.rcu()(result_rcu)
	  = cache.getSystemStatus().rcu()(cache_rcu);
      }
      else
      {
	LOG_WARN(formatString("invalid RCU index %d, there are only %d RCU's",
			      result_rcu, GET_CONFIG("N_RCU", i)));
      }
      
      result_rcu++;
    }
  }

  getPort()->send(ack);
}

const Timestamp& UpdStatusCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void UpdStatusCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool UpdStatusCmd::validate() const
{
  return (m_event->rcumask.count() <= (unsigned int)GET_CONFIG("N_RCU", i));
}

void UpdStatusCmd::ack_fail()
{
  RSPUpdstatusEvent ack;

  ack.timestamp = Timestamp(0,0);
  ack.status = FAILURE;

  ack.sysstatus.board().resize(GET_CONFIG("N_RSPBOARDS", i));
  ack.sysstatus.rcu().resize(GET_CONFIG("N_RCU", i));

  BoardStatus boardinit;
  RCUStatus rcuinit;

  memset(&boardinit, 0, sizeof(BoardStatus));
  memset(&rcuinit, 0, sizeof(RCUStatus));
  
  ack.sysstatus.board() = boardinit;
  ack.sysstatus.rcu()   = rcuinit;

  getPort()->send(ack);
}

