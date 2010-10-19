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

#include "BWRead.h"
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

BWRead::BWRead(GCFPortInterface& board_port, int board_id, int regid)
  : SyncAction(board_port, board_id, GET_CONFIG("RS.N_BLPS", i) * BF_N_FRAGMENTS),
    m_regid(regid)
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

BWRead::~BWRead()
{
}

void BWRead::sendrequest()
{
  uint8 global_blp = (getBoardId() * GET_CONFIG("RS.N_BLPS", i)) + (getCurrentBLP() / BF_N_FRAGMENTS);

  uint16 offset = ((getCurrentBLP() % BF_N_FRAGMENTS) * MEPHeader::FRAGMENT_SIZE) / sizeof(int16);

  if (m_regid < MEPHeader::BF_XROUT || m_regid > MEPHeader::BF_YIOUT)
  {
    LOG_FATAL("invalid regid");
    exit(EXIT_FAILURE);
  }

  LOG_DEBUG(formatString(">>>> BWRead(%s) global_blp=%d, regid=%d",
			 getBoardPort().getName().c_str(),
			 global_blp,
			 m_regid));
  
  // send next BF configure message
  EPAReadEvent bfcoefs;
      
  switch (m_regid)
  {
    case MEPHeader::BF_XROUT:
      bfcoefs.hdr.set(MEPHeader::BF_XROUT_HDR, getCurrentBLP() / BF_N_FRAGMENTS,
		      MEPHeader::READ, N_COEF * sizeof(int16), offset * sizeof(int16));
      break;
    case MEPHeader::BF_XIOUT:
      bfcoefs.hdr.set(MEPHeader::BF_XIOUT_HDR, getCurrentBLP() / BF_N_FRAGMENTS,
		      MEPHeader::READ, N_COEF * sizeof(int16), offset * sizeof(int16));
      break;
    case MEPHeader::BF_YROUT:
      bfcoefs.hdr.set(MEPHeader::BF_YROUT_HDR, getCurrentBLP() / BF_N_FRAGMENTS,
		      MEPHeader::READ, N_COEF * sizeof(int16), offset * sizeof(int16));
      break;
    case MEPHeader::BF_YIOUT:
      bfcoefs.hdr.set(MEPHeader::BF_YIOUT_HDR, getCurrentBLP() / BF_N_FRAGMENTS,
		      MEPHeader::READ, N_COEF * sizeof(int16), offset * sizeof(int16));
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
  if (EPA_BF_COEFS != event.signal)
  {
    LOG_WARN("BWRead::handleack: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }

  EPABfCoefsEvent bfcoefs(event);

  if (!bfcoefs.hdr.isValidAck(m_hdr))
  {
    LOG_ERROR("BWRead::handlack: invalid ack");
    return GCFEvent::NOT_HANDLED;
  }
  
  uint16 offset = ((getCurrentBLP() % BF_N_FRAGMENTS) * MEPHeader::FRAGMENT_SIZE) / sizeof(int16);

  uint8 global_blp = (getBoardId() * GET_CONFIG("RS.N_BLPS", i)) + (getCurrentBLP() / BF_N_FRAGMENTS);

  // copy weights from the message to the cache
  Array<complex<int16>, 2> weights((complex<int16>*)&bfcoefs.coef,
				   shape(N_COEF / MEPHeader::N_PHASEPOL, MEPHeader::N_POL),
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
    weights -= Cache::getInstance().getBack().getBeamletWeights()()(0, global_blp,
								    Range(offset / MEPHeader::N_PHASEPOL,
									  (offset / MEPHeader::N_PHASEPOL) + (N_COEF / MEPHeader::N_PHASEPOL) - 1),
	
								    Range::all());
    complex<int16> errorsum(sum(weights));
    if (complex<int16>(0) != errorsum)
    {
      LOG_WARN(formatString("LOOPBACK CHECK FAILED: BWRead mismatch [blp=%d, errorsum=(%d,i%d)]",
			    global_blp, real(errorsum), imag(errorsum)));
    }
  }
  else
  {
    Cache::getInstance().getBack().getBeamletWeights()()(0, global_blp,
							 Range(offset / MEPHeader::N_PHASEPOL,
							       (offset / MEPHeader::N_PHASEPOL) + (N_COEF / MEPHeader::N_PHASEPOL) - 1),
							 Range::all()) = weights;
  }
  
  return GCFEvent::HANDLED;
}


