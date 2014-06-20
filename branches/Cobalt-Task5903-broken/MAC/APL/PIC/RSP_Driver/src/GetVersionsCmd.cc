//#  GetVersionsCmd.cc: implementation of the GetVersionsCmd class
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

#include "GetVersionsCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

GetVersionsCmd::GetVersionsCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("GetVersion", port, oper)
{
  m_event = new RSPGetversionEvent(event);
}

GetVersionsCmd::~GetVersionsCmd()
{
  delete m_event;
}

void GetVersionsCmd::ack(CacheBuffer& cache)
{
  RSPGetversionackEvent ack;

  ack.timestamp = getTimestamp();
  ack.status = RSP_SUCCESS;

  ack.versions.bp().resize(cache.getVersions().bp().extent(firstDim));
  ack.versions.bp() = cache.getVersions().bp();

  ack.versions.ap().resize(cache.getVersions().ap().extent(firstDim));
  ack.versions.ap() = cache.getVersions().ap();
  
  getPort()->send(ack);
}

void GetVersionsCmd::apply(CacheBuffer& /*cache*/, bool /*setModFlag*/)
{
  /* intentionally left empty */
}

void GetVersionsCmd::complete(CacheBuffer& cache)
{
  ack(cache);
}

const Timestamp& GetVersionsCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void GetVersionsCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool GetVersionsCmd::validate() const
{
  return true;
}

bool GetVersionsCmd::readFromCache() const
{
  return m_event->cache;
}
