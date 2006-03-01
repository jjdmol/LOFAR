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

GetStatusCmd::GetStatusCmd(GCFEvent& event, GCFPortInterface& port, Operation oper)
{
  m_event = new RSPGetstatusEvent(event);

  setOperation(oper);
  setPeriod(0);
  setPort(port);
}

GetStatusCmd::~GetStatusCmd()
{
  delete m_event;
}

void GetStatusCmd::ack(CacheBuffer& cache)
{
  RSPGetstatusackEvent ack;

  ack.timestamp = getTimestamp();
  ack.status = SUCCESS;

  ack.sysstatus.board().resize(StationSettings::instance()->nrRspBoards());
  ack.sysstatus.board() = cache.getSystemStatus().board();

  for (int boardNr = 0; boardNr < StationSettings::instance()->nrRspBoards(); boardNr++) {
	ack.sysstatus.board()(boardNr) = cache.getSystemStatus().board()(boardNr);
  }

  getPort()->send(ack);
}

void GetStatusCmd::apply(CacheBuffer& /*cache*/, bool /*setModFlag*/)
{
  // no-op
}

void GetStatusCmd::complete(CacheBuffer& cache)
{
  ack(cache);
}

const Timestamp& GetStatusCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void GetStatusCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool GetStatusCmd::validate() const
{

  return (true);
//  return (m_event->rspmask.count() <= (unsigned int)StationSettings::instance()->nrRspBoards());
}

bool GetStatusCmd::readFromCache() const
{
  return m_event->cache;
}

void GetStatusCmd::ack_fail()
{
  RSPGetstatusackEvent ack;

  ack.timestamp = Timestamp(0,0);
  ack.status = FAILURE;

#if 1
  ack.sysstatus.board().resize(0);
#else
  ack.sysstatus.board().resize(StationSettings::instance()->nrRspBoards());

  BoardStatus boardinit;

  memset(&boardinit, 0, sizeof(BoardStatus));
  
  ack.sysstatus.board() = boardinit;
#endif

  getPort()->send(ack);
}

