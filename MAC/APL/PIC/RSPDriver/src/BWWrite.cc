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

#include "BWWrite.h"
#include "EPA_Protocol.ph"
#include "RSP_Protocol.ph"
#include "Cache.h"

#include <PSAccess.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <unistd.h>
#include <string.h>
#include <blitz/array.h>

#define N_RETRIES 3

using namespace RSP;
using namespace LOFAR;
using namespace EPA_Protocol;
using namespace blitz;

BWWrite::BWWrite(GCFPortInterface& board_port, int board_id, int regid)
  : SyncAction(board_port, board_id, GET_CONFIG("RS.N_BLPS", i) * BF_N_FRAGMENTS),
    m_regid(regid)
{
}

BWWrite::~BWWrite()
{
}

void BWWrite::sendrequest()
{
  uint8 global_blp = (getBoardId() * GET_CONFIG("RS.N_BLPS", i)) + (getCurrentBLP() / BF_N_FRAGMENTS);

  // coef int16 offset - divide by N_PHASE to get offset in complex<int16>
  uint16 offset = ((getCurrentBLP() % BF_N_FRAGMENTS) * MEPHeader::FRAGMENT_SIZE) / sizeof(int16);

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

  switch (m_regid)
  {
    case MEPHeader::BF_XROUT:
      bfcoefs.hdr.set(MEPHeader::BF_XROUT_HDR, getCurrentBLP() / BF_N_FRAGMENTS);
      break;
    case MEPHeader::BF_XIOUT:
      bfcoefs.hdr.set(MEPHeader::BF_XIOUT_HDR, getCurrentBLP() / BF_N_FRAGMENTS);
      break;
    case MEPHeader::BF_YROUT:
      bfcoefs.hdr.set(MEPHeader::BF_YROUT_HDR, getCurrentBLP() / BF_N_FRAGMENTS);
      break;
    case MEPHeader::BF_YIOUT:
      bfcoefs.hdr.set(MEPHeader::BF_YIOUT_HDR, getCurrentBLP() / BF_N_FRAGMENTS);
      break;
  }
  
  // copy weights from the cache to the message
  Array<complex<int16>, 2> weights((complex<int16>*)&bfcoefs.coef + offset,
				   shape(MEPHeader::FRAGMENT_SIZE / sizeof(int16) / MEPHeader::N_POL, MEPHeader::N_POL),
				   neverDeleteData);

  LOG_FATAL_STR("offset=" << offset << "; global_blp=" << (int)global_blp << "; blp=" << getCurrentBLP() / BF_N_FRAGMENTS);
  LOG_FATAL_STR("weights shape=" << weights.shape());
  LOG_FATAL_STR("weights range=" << Range(offset / MEPHeader::N_PHASE,
					  (MEPHeader::FRAGMENT_SIZE / sizeof(int16)) / MEPHeader::N_PHASE));

  weights = Cache::getInstance().getBack().getBeamletWeights()()(0, global_blp,
								 Range(offset / MEPHeader::N_PHASE,
								       (MEPHeader::FRAGMENT_SIZE / sizeof(int16)) / MEPHeader::N_PHASE),
								 Range::all());

  switch (m_regid)
  {
    case MEPHeader::BF_XROUT:
    {
      // weights for x-real part
      // no added conversions needed
    }
    break;

    case MEPHeader::BF_XIOUT:
    {
      // weights for x-imaginary part
      weights *= complex<int16>(0,1);
    }
    break;
    
    case MEPHeader::BF_YROUT:
    {
      // weights for y-real part
      // no added conversions needed
    }
    break;
    
    case MEPHeader::BF_YIOUT:
    {
      // weights for y-imaginary part
      weights *= complex<int16>(0,1);
    }
    break;
  }
  
  getBoardPort().send(bfcoefs);
}

void BWWrite::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult BWWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  LOG_DEBUG("handleack");
  
  EPAWriteackEvent ack(event);
  
  if (ack.hdr.m_fields.error)
  {
    LOG_ERROR_STR("BWWrite::handleack: error " << ack.hdr.m_fields.error);
  }

  return GCFEvent::HANDLED;
}


