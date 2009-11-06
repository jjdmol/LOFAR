//#  XWWrite.cc: implementation of the XWWrite class
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

#include "XWWrite.h"
#include "Cache.h"
#include "StationSettings.h"

#define N_RETRIES 3

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;

XWWrite::XWWrite(GCFPortInterface& board_port, int board_id, int blp, int regid)
  : SyncAction(board_port, board_id, 1),
    m_blp(blp), m_regid(regid), m_remaining(0), m_offset(0)
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

XWWrite::~XWWrite()
{
}

void XWWrite::sendrequest()
{
  uint8 global_blp = (getBoardId() * StationSettings::instance()->nrBlpsPerBoard()) + m_blp;

  if (m_regid < MEPHeader::BF_XROUT || m_regid > MEPHeader::BF_YIOUT)
  {
    LOG_FATAL("invalid regid");
    exit(EXIT_FAILURE);
  }

  LOG_DEBUG(formatString(">>>> XWWrite(%s) global_blp=%d, regid=%d",
			 getBoardPort().getName().c_str(),
			 global_blp,
			 m_regid));
  
  // send next BF configure message
  EPABfCoefsWriteEvent bfcoefs;

  size_t size = MEPHeader::N_LOCAL_XLETS * MEPHeader::WEIGHT_SIZE;
  switch (m_regid)
  {
    case MEPHeader::BF_XROUT:
      bfcoefs.hdr.set(MEPHeader::BF_XROUT_HDR, 1 << m_blp,
		      MEPHeader::WRITE, size, m_offset);
      break;
    case MEPHeader::BF_XIOUT:
      bfcoefs.hdr.set(MEPHeader::BF_XIOUT_HDR, 1 << m_blp,
		      MEPHeader::WRITE, size, m_offset);
      break;
    case MEPHeader::BF_YROUT:
      bfcoefs.hdr.set(MEPHeader::BF_YROUT_HDR, 1 << m_blp,
		      MEPHeader::WRITE, size, m_offset);
      break;
    case MEPHeader::BF_YIOUT:
      bfcoefs.hdr.set(MEPHeader::BF_YIOUT_HDR, 1 << m_blp,
		      MEPHeader::WRITE, size, m_offset);
      break;
  }

  // create blitz view om the weights in the bfcoefs message to be sent to the RSP hardware
  Array<complex<int16>, 2> weights(MEPHeader::N_LOCAL_XLETS, MEPHeader::N_POL);
  bfcoefs.coef.setBuffer(weights.data(), weights.size() * sizeof(complex<uint16>));

  weights = complex<int16>(0, 0);

  int xc_gain = GET_CONFIG("RSPDriver.XC_GAIN", i);

  switch (m_regid)
  {
    case MEPHeader::BF_XROUT:
    {
      // weights for x-real part
      // no added conversions needed

      // overwrite first weights for cross correlation
      weights(m_blp, 0) = complex<int16>(xc_gain, 0);
    }
    break;

    case MEPHeader::BF_XIOUT:
    {
      // overwrite first weights for cross correlation
      weights(m_blp, 0) = complex<int16>(0, xc_gain);
    }
    break;
    
    case MEPHeader::BF_YROUT:
    {
      // weights for y-real part
      // no added conversions needed

      // overwrite first weights for cross correlation
      weights(m_blp, 1) = complex<int16>(xc_gain, 0);
    }
    break;
    
    case MEPHeader::BF_YIOUT:
    {
      // overwrite first weights for cross correlation
      weights(m_blp, 1) = complex<int16>(0, xc_gain);
    }
    break;
  }

  m_hdr = bfcoefs.hdr;
  getBoardPort().send(bfcoefs);
}

void XWWrite::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult XWWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_WRITEACK != event.signal)
  {
    LOG_WARN("XWWrite::handleack: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }
  
  EPAWriteackEvent ack(event);

#if 0
  uint8 global_blp = (getBoardId() * StationSettings::instance()->nrBlpsPerBoard()) + m_blp;
#endif

  if (!ack.hdr.isValidAck(m_hdr))
  {
#if 0
    Cache::getInstance().getState().bf().write_error(global_blp * MEPHeader::N_PHASEPOL + m_regid);
#endif

    LOG_ERROR("XWWrite::handleack: invalid ack");
    return GCFEvent::NOT_HANDLED;

  } else {

    // need separate xbf().write_ack register?
#if 0
    Cache::getInstance().getState().bf().write_ack(global_blp * MEPHeader::N_PHASEPOL + m_regid);
#endif

  }

  return GCFEvent::HANDLED;
}


