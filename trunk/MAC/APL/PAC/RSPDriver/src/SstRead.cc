//#  SstRead.cc: implementation of the SstRead class
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

#include "SstRead.h"
#include "Statistics.h"
#include "EPA_Protocol.ph"
#include "Cache.h"

#include <PSAccess.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;
using namespace EPA_Protocol;
using namespace RSP_Protocol;
using namespace blitz;
using namespace RTC;

SstRead::SstRead(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, GET_CONFIG("RS.N_BLPS", i) * SST_N_FRAGMENTS)
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

SstRead::~SstRead()
{
  /* TODO: delete event? */
}

void SstRead::sendrequest()
{
  EPAReadEvent sstread;

  uint16 byteoffset = (getCurrentBLP() % SST_N_FRAGMENTS) * MEPHeader::FRAGMENT_SIZE;

  sstread.hdr.set(MEPHeader::SST_POWER_HDR, getCurrentBLP() / SST_N_FRAGMENTS,
		  MEPHeader::READ, N_STATS * sizeof(uint32), byteoffset);

  m_hdr = sstread.hdr;
  getBoardPort().send(sstread);
}

void SstRead::sendrequest_status()
{
  // intentionally left empty
}

/**
 * Function to convert the semi-floating point representation used by the
 * EPA firmware to a double.
 */
BZ_DECLARE_FUNCTION_RET(convert_uint32_to_double, double)
inline double convert_uint32_to_double(uint32 val)
{
  uint64 val64 = val;
  
  // check if bit 31 is set
  if ((1<<31) & val64) val64 = (val64 & ((1<<31)-1)) << 25;

  return (double)val64;
}

GCFEvent::TResult SstRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_STATS != event.signal)
  {
    LOG_WARN("SstRead::handleack: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }

  EPAStatsEvent ack(event);

  if (!ack.hdr.isValidAck(m_hdr))
  {
    LOG_ERROR("SstRead::handleack: invalid ack");
    return GCFEvent::NOT_HANDLED;
  }

  uint8 global_blp = (getBoardId() * GET_CONFIG("RS.N_BLPS", i)) + (getCurrentBLP() / SST_N_FRAGMENTS);

  uint16 offset = ack.hdr.m_fields.offset / sizeof(int32);
  
  LOG_DEBUG(formatString("SstRead::handleack: global_blp=%d, offset=%d",
			 global_blp, offset));

  Range fragment_range(offset / MEPHeader::N_POL,
		       (offset / MEPHeader::N_POL) + (N_STATS / MEPHeader::N_POL) - 1);

  LOG_DEBUG_STR("fragment_range=" << fragment_range);
  
  if (MEPHeader::SST_POWER != ack.hdr.m_fields.addr.regid)
  {
    LOG_ERROR("invalid sst ack");
    return GCFEvent::HANDLED;
  }

  Array<uint32, 2> stats((uint32*)&ack.stat,
			 shape(N_STATS / MEPHeader::N_POL,
			       MEPHeader::N_POL),
			 neverDeleteData);

  Array<double, 2>& cache(Cache::getInstance().getBack().getSubbandStats()());

  // x-pol subband statistics: copy and convert to double
  cache(global_blp * 2,     fragment_range) = convert_uint32_to_double(stats(Range::all(), 0));

  // y-pol subband statistics: copy and convert to double
  cache(global_blp * 2 + 1, fragment_range) = convert_uint32_to_double(stats(Range::all(), 1));
  
  return GCFEvent::HANDLED;
}
