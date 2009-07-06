//#  BWRead.cc: implementation of the BWRead class
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

#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RSP_Protocol/RSP_Protocol.ph>

#include <APL/RTCCommon/PSAccess.h>

#include <unistd.h>
#include <string.h>
#include <blitz/array.h>

#include "BWRead.h"
#include "Cache.h"
#include "StationSettings.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;

BWRead::BWRead(GCFPortInterface& board_port, int board_id, int blp, int regid)
  : SyncAction(board_port, board_id, MEPHeader::BF_N_FRAGMENTS),
    m_blp(blp), m_regid(regid), m_remaining(0), m_offset(0)
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

BWRead::~BWRead()
{
}

void BWRead::sendrequest()
{
  uint8 global_blp = (getBoardId() * StationSettings::instance()->nrBlpsPerBoard()) + m_blp;

  if (m_regid < MEPHeader::BF_XROUT || m_regid > MEPHeader::BF_YIOUT)
  {
    LOG_FATAL("invalid regid");
    exit(EXIT_FAILURE);
  }

  // reset m_offset and m_remaining for each register
  if (0 == getCurrentIndex()) {
    m_remaining = MEPHeader::BF_XROUT_SIZE; // representative for XR, XI, YR, YI size
    m_offset = 0;
  }

  LOG_DEBUG(formatString(">>>> BWRead(%s) global_blp=%d, regid=%d",
			 getBoardPort().getName().c_str(),
			 global_blp,
			 m_regid));
  
  // send next BF configure message
  EPAReadEvent bfcoefs;
      
  size_t size = MIN(MEPHeader::FRAGMENT_SIZE, m_remaining);
  switch (m_regid)
  {
    case MEPHeader::BF_XROUT:
      bfcoefs.hdr.set(MEPHeader::BF_XROUT_HDR, 1 << m_blp,
		      MEPHeader::READ, size, m_offset);
      break;
    case MEPHeader::BF_XIOUT:
      bfcoefs.hdr.set(MEPHeader::BF_XIOUT_HDR, 1 << m_blp,
		      MEPHeader::READ, size, m_offset);
      break;
    case MEPHeader::BF_YROUT:
      bfcoefs.hdr.set(MEPHeader::BF_YROUT_HDR, 1 << m_blp,
		      MEPHeader::READ, size, m_offset);
      break;
    case MEPHeader::BF_YIOUT:
      bfcoefs.hdr.set(MEPHeader::BF_YIOUT_HDR, 1 << m_blp,
		      MEPHeader::READ, size, m_offset);
      break;
  }

  m_hdr = bfcoefs.hdr;
  getBoardPort().send(bfcoefs);
}

void BWRead::sendrequest_status()
{
  /* intentionally left empty */
}

GCFEvent::TResult BWRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_BF_COEFS_READ != event.signal)
  {
    LOG_WARN("BWRead::handleack: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }

  EPABfCoefsReadEvent bfcoefs(event);

  if (!bfcoefs.hdr.isValidAck(m_hdr))
  {
    LOG_ERROR("BWRead::handlack: invalid ack");
    return GCFEvent::NOT_HANDLED;
  }
  
  uint8 global_blp = (getBoardId() * StationSettings::instance()->nrBlpsPerBoard()) + m_blp;
  size_t size = MIN(MEPHeader::FRAGMENT_SIZE, m_remaining);
  size_t elem_size   = size / (sizeof(complex<uint16>) * MEPHeader::N_POL);
  size_t elem_offset = m_offset / (sizeof(complex<uint16>) * MEPHeader::N_POL);

  Range target_range(elem_offset, elem_offset + elem_size - 1);

  // copy weights from the message to the cache
  Array<complex<int16>, 2> weights((complex<int16>*)&bfcoefs.coef,
				   shape(elem_size, MEPHeader::N_POL),
				   neverDeleteData);

  //
  // In mode 0, check that the received message matches with what's in the cache.
  // In other modes, store the data in the cache.
  //
  if (0 == GET_CONFIG("RSPDriver.LOOPBACK_MODE", i))
  {
    //
    // substract cache contents from weights
    // if there is a difference, log a warning
    //
    weights -= Cache::getInstance().getBack().getBeamletWeights()()(0, Range(global_blp * 2, global_blp * 2 + 1), target_range);

    complex<int16> errorsum(sum(weights));
    if (complex<int16>(0) != errorsum)
    {
      LOG_WARN(formatString("LOOPBACK CHECK FAILED: BWRead mismatch [blp=%d, errorsum=(%d,i%d)]",
			    global_blp, real(errorsum), imag(errorsum)));
    }
  }
  else
  {
    // X
    Cache::getInstance().getBack().getBeamletWeights()()(0, global_blp * 2, target_range)
      = weights(Range::all(), 0);

    // Y
    Cache::getInstance().getBack().getBeamletWeights()()(0, global_blp * 2 + 1, target_range)
      = weights(Range::all(), 1);
  }
  
  // update m_remaining and m_offset for next read
  m_remaining -= size;
  m_offset    += size;

  return GCFEvent::HANDLED;
}


