//#  BstRead.cc: implementation of the BstRead class
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

#include <APL/RSP_Protocol/Statistics.h>
#include <APL/RSP_Protocol/EPA_Protocol.ph>

#include <APL/RTCCommon/PSAccess.h>

#include "BstRead.h"
#include "Cache.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;
using namespace RSP_Protocol;

BstRead::BstRead(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, MEPHeader::N_SERDES_LANES)
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

BstRead::~BstRead()
{
}

void BstRead::sendrequest()
{
  EPAReadEvent bstread;

  Cache::getInstance().getState().bst().read(getBoardId());

  bstread.hdr.set(MEPHeader::READ,
		  MEPHeader::DST_RSP,
		  MEPHeader::BST,
		  getCurrentIndex(),
		  MEPHeader::BST_POWER_SIZE);

  m_hdr = bstread.hdr;
  getBoardPort().send(bstread);
}

void BstRead::sendrequest_status()
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
  int64 val64;

  uint32 e = val & (1<<31);
  uint32 s = val & (1<<30);
  int32  m = val & ((1<<30)-1);

  if (s) m = m - (1<<30);
  if (e) {
    val64 = (int64)m << 23;
  } else {
    val64 = m;
  }

  return (double)(val64);
}

GCFEvent::TResult BstRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_BST_STATS != event.signal)
  {
    LOG_WARN("BstRead::handleack: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }

  EPABstStatsEvent ack(event);

  if (!ack.hdr.isValidAck(m_hdr))
  {
    Cache::getInstance().getState().bst().read_error(getBoardId());
    LOG_ERROR("BstRead::handleack: invalid ack");
    return GCFEvent::NOT_HANDLED;
  }

  LOG_DEBUG(formatString("BstRead::handleack: boardid=%d",
			 getBoardId()));

  Range fragment_range(0, MEPHeader::N_DATA_SLOTS - 1);
  fragment_range = fragment_range + (getCurrentIndex() * MEPHeader::N_DATA_SLOTS);

  LOG_DEBUG_STR("fragment_range=" << fragment_range);
  
  if (getCurrentIndex() != ack.hdr.m_fields.addr.regid)
  {
    LOG_ERROR("invalid bst ack");
    return GCFEvent::HANDLED;
  }

  Array<uint32, 2> stats((uint32*)&ack.stat,
			 shape((MEPHeader::BST_POWER_SIZE / sizeof(uint32)) / MEPHeader::N_POL,
			       MEPHeader::N_POL),
			 neverDeleteData);

  Array<double, 2>& cache(Cache::getInstance().getBack().getBeamletStats()());

  // x-pol beamlet statistics: copy and convert to double
  cache(getBoardId() * 2,     fragment_range) =
    convert_uint32_to_double(stats(Range::all(), 0));

  // y-pol beamlet statistics: copy and convert to double
  cache(getBoardId() * 2 + 1, fragment_range) =
    convert_uint32_to_double(stats(Range::all(), 1));

  if (getCurrentIndex() == MEPHeader::N_SERDES_LANES - 1) {
    Cache::getInstance().getState().bst().read_ack(getBoardId());
  }

  return GCFEvent::HANDLED;
}
