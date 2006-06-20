//#  RADWrite.cc: implementation of the RADWrite class
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

#include <StationSettings.h>
#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>

#include "RADWrite.h"
#include "Cache.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RTC;

RADWrite::RADWrite(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, 1)
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

RADWrite::~RADWrite()
{
  /* TODO: delete event? */
}

void RADWrite::sendrequest()
{
  // skip update if the RAD does not need to be written
  if (RTC::RegisterState::WRITE != Cache::getInstance().getState().rad().get(getBoardId()))
  {
    Cache::getInstance().getState().rad().unmodified(getBoardId());
    setContinue(true);
    return;
  }

  EPARadBpEvent rad;

  rad.hdr.set(MEPHeader::RAD_BP_HDR);
  rad.lanemode = 0;

  /*
   * lane mode: one byte for each lane
   * format: XXXXAABB
   *
   * where XX = don't care
   *       AA = xlet mode
   *       BB = blet mode
   *
   * mode 0b00 = ignore remote data (only local)  DEFAULT
   * mode 0b01 = disable
   * mode 0b10 = combine local and remote data
   * mode 0b11 = ignore local data (only remote)
   */

  for (int lane = 0; lane < MEPHeader::N_SERDES_LANES; lane++) {
    uint8 mode = 0x0A; // default is to combine local and remote data

    int blet_out = GET_CONFIG(formatString("RSPDriver.LANE_%d_BLET_OUT", lane).c_str(), i);
    int xlet_out = GET_CONFIG(formatString("RSPDriver.LANE_%d_XLET_OUT", lane).c_str(), i);

    // if this board is the first in the ring (it is the board before the OUT board)
    // then set mode to ignore remote data (0b00)
    if (getBoardId() != (blet_out - 1) % StationSettings::instance()->nrRspBoards()) {
      mode &= ~0x02;
    }
    if (getBoardId() != (xlet_out - 1) % StationSettings::instance()->nrRspBoards()) {
      mode &= ~0x08;
    }
    
    rad.lanemode |= ((uint32)mode) << (8 * lane);

    LOG_INFO_STR(formatString("rad.lanemode(lane=%d, rspboard=%d)=0x%08x",
			      lane, getBoardId(), rad.lanemode));
  }

  m_hdr = rad.hdr;
  getBoardPort().send(rad);

}

void RADWrite::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult RADWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_WRITEACK != event.signal)
  {
    LOG_WARN("RADWrite::handleack: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }

  EPAWriteackEvent ack(event);

  if (!ack.hdr.isValidAck(m_hdr))
  {
    LOG_ERROR("RADWrite::handleack: invalid ack");
    Cache::getInstance().getState().rad().write_error(getBoardId());
    return GCFEvent::NOT_HANDLED;
  }

  Cache::getInstance().getState().rad().write_ack(getBoardId());
  
  return GCFEvent::HANDLED;
}
