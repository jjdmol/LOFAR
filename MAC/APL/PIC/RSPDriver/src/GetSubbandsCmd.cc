//#  GetSubbandsCmd.cc: implementation of the GetSubbandsCmd class
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

#include <APLConfig.h>
#include "RSP_Protocol.ph"
#include "GetSubbandsCmd.h"

#include <blitz/array.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;
using namespace RSP_Protocol;
using namespace blitz;

GetSubbandsCmd::GetSubbandsCmd(GCFEvent& event, GCFPortInterface& port, Operation oper)
{
  m_event = new RSPGetsubbandsEvent(event);

  setOperation(oper);
  setPeriod(0);
  setPort(port);
}

GetSubbandsCmd::~GetSubbandsCmd()
{
  delete m_event;
}

void GetSubbandsCmd::ack(CacheBuffer& cache)
{
  RSPGetsubbandsackEvent ack;

  ack.timestamp = getTimestamp();
  ack.status = SUCCESS;

  ack.subbands().resize(m_event->blpmask.count(), N_BEAMLETS * N_POL);
  
  int result_blp = 0;
  for (int cache_blp = 0; cache_blp < GET_CONFIG("N_BLPS", i); cache_blp++)
  {
    if (m_event->blpmask[result_blp])
    {
      if (result_blp < GET_CONFIG("N_BLPS", i))
      {
	ack.subbands()(result_blp, Range::all())
	  = cache.getSubbandSelection()()(cache_blp, Range::all());
      }
      else
      {
	LOG_WARN(formatString("invalid BLP index %d, there are only %d BLP's",
			      result_blp, GET_CONFIG("N_BLPS", i)));
      }
      
      result_blp++;
    }
  }
  
  getPort()->send(ack);
}

void GetSubbandsCmd::apply(CacheBuffer& /*cache*/)
{
  /* intentionally left empty */
}

void GetSubbandsCmd::complete(CacheBuffer& cache)
{
  ack(cache);
}

const Timestamp& GetSubbandsCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void GetSubbandsCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool GetSubbandsCmd::validate() const
{
  return ((m_event->blpmask.count() <= (unsigned int)GET_CONFIG("N_BLPS", i)));
}

bool GetSubbandsCmd::readFromCache() const
{
  return m_event->cache;
}
