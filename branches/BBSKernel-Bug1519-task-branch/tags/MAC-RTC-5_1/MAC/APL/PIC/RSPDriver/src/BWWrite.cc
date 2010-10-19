//#  BWWrite.cc: implementation of the BWWrite class
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

#include "BWWrite.h"
#include "Cache.h"
#include "StationSettings.h"

#define N_RETRIES 3

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;

BWWrite::BWWrite(GCFPortInterface& board_port, int board_id, int regid)
  : SyncAction(board_port, board_id, StationSettings::instance()->nrBlpsPerBoard() * BF_N_FRAGMENTS),
    m_regid(regid)
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

BWWrite::~BWWrite()
{
}

void BWWrite::sendrequest()
{
  uint8 global_blp = (getBoardId() * StationSettings::instance()->nrBlpsPerBoard()) + (getCurrentIndex() / BF_N_FRAGMENTS);

  // coef int16 offset - divide by N_PHASE to get offset in complex<int16>
  uint16 offset = ((getCurrentIndex() % BF_N_FRAGMENTS) * MEPHeader::FRAGMENT_SIZE) / sizeof(int16);

  if (m_regid < MEPHeader::BF_XROUT || m_regid > MEPHeader::BF_YIOUT)
  {
    LOG_FATAL("invalid regid");
    exit(EXIT_FAILURE);
  }

  LOG_DEBUG(formatString(">>>> BWWrite(%s) global_blp=%d, regid=%d",
			 getBoardPort().getName().c_str(),
			 global_blp,
			 m_regid));
  
  // send next BF configure message
  EPABfCoefsEvent bfcoefs;

  uint16 blp = 1 << (getCurrentIndex() / BF_N_FRAGMENTS);
  switch (m_regid)
  {
    case MEPHeader::BF_XROUT:
      bfcoefs.hdr.set(MEPHeader::BF_XROUT_HDR, blp,
		      MEPHeader::WRITE, N_COEF * sizeof(int16), offset * sizeof(int16));
      break;
    case MEPHeader::BF_XIOUT:
      bfcoefs.hdr.set(MEPHeader::BF_XIOUT_HDR, blp,
		      MEPHeader::WRITE, N_COEF * sizeof(int16), offset * sizeof(int16));
      break;
    case MEPHeader::BF_YROUT:
      bfcoefs.hdr.set(MEPHeader::BF_YROUT_HDR, blp,
		      MEPHeader::WRITE, N_COEF * sizeof(int16), offset * sizeof(int16));
      break;
    case MEPHeader::BF_YIOUT:
      bfcoefs.hdr.set(MEPHeader::BF_YIOUT_HDR, blp,
		      MEPHeader::WRITE, N_COEF * sizeof(int16), offset * sizeof(int16));
      break;
  }
  
  // create blitz view om the weights in the bfcoefs message to be sent to the RSP hardware
  Array<complex<int16>, 2> weights((complex<int16>*)&bfcoefs.coef,
				   shape(N_COEF / MEPHeader::N_PHASEPOL, MEPHeader::N_POL),
				   neverDeleteData);

#if 0
  LOG_DEBUG_STR("offset=" << offset << "; global_blp=" << (int)global_blp << "; blp=" << blp);
  LOG_DEBUG_STR("weights shape=" << weights.shape());
  LOG_DEBUG_STR("weights range=" << Range(offset / MEPHeader::N_PHASEPOL,
					  (offset /MEPHeader::N_PHASEPOL) + (N_COEF / MEPHeader::N_PHASEPOL) - 1));
#endif
  
  // X
  weights(Range::all(), 0) = Cache::getInstance().getBack().getBeamletWeights()()(0, global_blp * 2,
										  Range(offset / MEPHeader::N_PHASEPOL,
											(offset / MEPHeader::N_PHASEPOL) + (N_COEF / MEPHeader::N_PHASEPOL) - 1));

  // Y
  weights(Range::all(), 1) = Cache::getInstance().getBack().getBeamletWeights()()(0, global_blp * 2 + 1,
										  Range(offset / MEPHeader::N_PHASEPOL,
											(offset / MEPHeader::N_PHASEPOL) + (N_COEF / MEPHeader::N_PHASEPOL) - 1));


  switch (m_regid)
  {
    case MEPHeader::BF_XROUT:
    {
      // weights for x-real part
      // no added conversions needed

      // y weights should be 0
      weights(Range::all(), 1) = 0;

      // overwrite first weights for cross correlation
      weights(getCurrentIndex(), 0) = complex<int16>(0x4000, 0);
    }
    break;

    case MEPHeader::BF_XIOUT:
    {
      // weights for x-imaginary part
      weights *= complex<int16>(0,1);

      // y weights should be 0
      weights(Range::all(), 1) = 0;

      // overwrite first weights for cross correlation
      weights(getCurrentIndex(), 0) = complex<int16>(0, 0x4000);
    }
    break;
    
    case MEPHeader::BF_YROUT:
    {
      // weights for y-real part
      // no added conversions needed

      // x weights should be 0
      weights(Range::all(), 0) = 0;

      // overwrite first weights for cross correlation
      weights(getCurrentIndex(), 1) = complex<int16>(0x4000, 0);
    }
    break;
    
    case MEPHeader::BF_YIOUT:
    {
      // weights for y-imaginary part
      weights *= complex<int16>(0,1);

      // x weights should be 0
      weights(Range::all(), 0) = 0;

      // overwrite first weights for cross correlation
      weights(getCurrentIndex(), 1) = complex<int16>(0, 0x4000);
    }
    break;
  }

  m_hdr = bfcoefs.hdr;
  getBoardPort().send(bfcoefs);
}

void BWWrite::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult BWWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_WRITEACK != event.signal)
  {
    LOG_WARN("BWWrite::handleack: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }
  
  EPAWriteackEvent ack(event);

  if (!ack.hdr.isValidAck(m_hdr))
  {
    LOG_ERROR("BWWrite::handleack: invalid ack");
    return GCFEvent::NOT_HANDLED;
  }

  return GCFEvent::HANDLED;
}


