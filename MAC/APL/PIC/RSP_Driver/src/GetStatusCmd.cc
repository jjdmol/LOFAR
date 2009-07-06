//#  GetStatusCmd.cc: implementation of the GetStatusCmd class
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
#include "GetStatusCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

GetStatusCmd::GetStatusCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("GetStatus", port, oper)
{
  m_event = new RSPGetstatusEvent(event);
}

GetStatusCmd::~GetStatusCmd()
{
  delete m_event;
}

//
// ack(cache)
//
void GetStatusCmd::ack(CacheBuffer& cache)
{
  RSPGetstatusackEvent ack;

  ack.timestamp = getTimestamp();
  ack.status = RSP_SUCCESS;

  ack.sysstatus.board().resize(m_event->rspmask.count());

  int result_rsp = 0;
  for (int cache_rsp = 0; cache_rsp < StationSettings::instance()->nrRspBoards(); cache_rsp++) {
    if (m_event->rspmask[cache_rsp]) {
      ack.sysstatus.board()(result_rsp) = cache.getSystemStatus().board()(cache_rsp);
      result_rsp++;
    }
  }
  ASSERT(result_rsp == (int)m_event->rspmask.count());

  getPort()->send(ack);
}

//
// apply(cache, modFlag)
//
void GetStatusCmd::apply(CacheBuffer& /*cache*/, bool /*setModFlag*/)
{
  // no-op
}

//
// complete(cache)
//
void GetStatusCmd::complete(CacheBuffer& cache)
{
  ack(cache);
}

//
// getTimeStamp()
//
const Timestamp& GetStatusCmd::getTimestamp() const
{
  return m_event->timestamp;
}

//
// setTimeStamp(timestamp)
//
void GetStatusCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

//
// validate()
//
bool GetStatusCmd::validate() const
{
  return (m_event->rspmask.count() <= (unsigned int)StationSettings::instance()->nrRspBoards());
}

//
// readFromCache()
//
bool GetStatusCmd::readFromCache() const
{
  return m_event->cache;
}

//
// ack_fail()
//
void GetStatusCmd::ack_fail()
{
  RSPGetstatusackEvent ack;

  ack.timestamp = Timestamp(0,0);
  ack.status = RSP_FAILURE;

  // send back dummy status array
  ack.sysstatus.board().resize(1);
  BoardStatus boardinit;
  memset(&boardinit, 0, sizeof(BoardStatus));
  ack.sysstatus.board()(0) = boardinit;

  getPort()->send(ack);
}

