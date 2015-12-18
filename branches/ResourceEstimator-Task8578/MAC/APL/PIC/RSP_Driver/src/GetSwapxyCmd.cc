//#  GetSwapxyCmd.cc: implementation of the GetSwapxyCmd class
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
//#  $Id: GetSwapxyCmd.cc 15023 2010-02-18 15:24:31Z donker $

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "GetSwapxyCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

GetSwapXYCmd::GetSwapXYCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("GetSwapxy", port, oper)
{
  m_event = new RSPGetswapxyEvent(event);
}

GetSwapXYCmd::~GetSwapXYCmd()
{
  delete m_event;
}

void GetSwapXYCmd::ack(CacheBuffer& cache)
{
  RSPGetswapxyackEvent ack;

  ack.timestamp = getTimestamp();
  ack.status = RSP_SUCCESS;

  ack.antennamask = cache.getSwappedXY();
  
  getPort()->send(ack);
}

void GetSwapXYCmd::apply(CacheBuffer& /*cache*/, bool /*setModFlag*/)
{
  /* intentionally left empty */
}

void GetSwapXYCmd::complete(CacheBuffer& cache)
{
  ack(cache);
}

const Timestamp& GetSwapXYCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void GetSwapXYCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool GetSwapXYCmd::validate() const
{
  return true;
}

bool GetSwapXYCmd::readFromCache() const
{
  return m_event->cache;
}
