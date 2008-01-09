//#  GetTDStatusCmd.cc: implementation of the GetTDStatusCmd class
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
#include "GetTDStatusCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

GetTDStatusCmd::GetTDStatusCmd(GCFEvent& event, GCFPortInterface& port, Operation oper)
{
  m_event = new RSPGettdstatusEvent(event);

  setOperation(oper);
  setPeriod(0);
  setPort(port);
}

GetTDStatusCmd::~GetTDStatusCmd()
{
  delete m_event;
}

void GetTDStatusCmd::ack(CacheBuffer& cache)
{
  RSPGettdstatusackEvent ack;

  ack.timestamp = getTimestamp();
  ack.status = SUCCESS;

  ack.tdstatus.board().resize(m_event->rspmask.count());

  int result_rsp = 0;
  for (int cache_rsp = 0; cache_rsp < StationSettings::instance()->nrRspBoards(); cache_rsp++)
  {
    if (m_event->rspmask[cache_rsp])
    {
      ack.tdstatus.board()(result_rsp) = cache.getTDStatus().board()(cache_rsp);
      result_rsp++;
    }
  }
  ASSERT(result_rsp == (int)m_event->rspmask.count());

  getPort()->send(ack);
}

void GetTDStatusCmd::apply(CacheBuffer& /*cache*/, bool /*setModFlag*/)
{
  // no-op
}

void GetTDStatusCmd::complete(CacheBuffer& cache)
{
  ack(cache);
}

const Timestamp& GetTDStatusCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void GetTDStatusCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool GetTDStatusCmd::validate() const
{
  return (m_event->rspmask.count() <= (unsigned int)StationSettings::instance()->nrRspBoards());
}

bool GetTDStatusCmd::readFromCache() const
{
  return m_event->cache;
}

void GetTDStatusCmd::ack_fail()
{
  RSPGettdstatusackEvent ack;

  ack.timestamp = Timestamp(0,0);
  ack.status = FAILURE;

  // send back dummy status array
  ack.tdstatus.board().resize(1);
  TDBoardStatus tdstatusinit;
  memset(&tdstatusinit, 0, sizeof(BoardStatus));
  tdstatusinit.invalid = 1;

  ack.tdstatus.board()(0) = tdstatusinit;

  getPort()->send(ack);
}

