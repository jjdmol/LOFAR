//#  GetSDOCmd.cc: implementation of the GetSDOCmd class
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
//#  $Id: GetSDOCmd.cc 22248 2012-10-08 12:34:59Z overeem $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarBitModeInfo.h>

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "GetSDOCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

GetSDOCmd::GetSDOCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("GetSDO", port, oper)
{
  m_event = new RSPGetsdoEvent(event);
}

GetSDOCmd::~GetSDOCmd()
{
  delete m_event;
}

void GetSDOCmd::ack(CacheBuffer& cache)
{
	RSPGetsdoackEvent ack;

	ack.timestamp = getTimestamp();
	ack.status    = RSP_SUCCESS;

	Range range;
	int input_rcu;
	int nBanks = (MAX_BITS_PER_SAMPLE / cache.getSDOBitsPerSample());
		
    ack.subbands.subbands().resize(m_event->rcumask.count(), nBanks, MEPHeader::N_SDO_SUBBANDS);
    
    input_rcu = 0;
    range = Range(0, ack.subbands.subbands().extent(thirdDim) - 1);
    for (int cache_rcu = 0; cache_rcu < StationSettings::instance()->nrRcus(); cache_rcu++) {
        if (m_event->rcumask[cache_rcu]) {
            
            for (int bank = 0; bank < nBanks; bank++) {
                ack.subbands.subbands()(input_rcu, bank, range) =
                    cache.getSDOSelection().subbands()(cache_rcu, bank, range);
            } // for each bank
            
            LOG_DEBUG_STR("GetSDO:subbands(cache[0]): " << cache.getSDOSelection().subbands()(0, Range::all(), Range::all()));
            input_rcu++;
        } // if rcu selected
    } // for each rcu
	getPort()->send(ack);
}

void GetSDOCmd::apply(CacheBuffer& /*cache*/, bool /*setModFlag*/)
{
  /* intentionally left empty */
}

void GetSDOCmd::complete(CacheBuffer& cache)
{
  ack(cache);
}

const Timestamp& GetSDOCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void GetSDOCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool GetSDOCmd::validate() const
{
  return (m_event->rcumask.count() <= (unsigned int)StationSettings::instance()->nrRcus());
}

bool GetSDOCmd::readFromCache() const
{
  return m_event->cache;
}
