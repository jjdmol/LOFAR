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

#include <APLConfig.h>

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
  : SyncAction(board_port, board_id, GET_CONFIG("RS.N_BLPS", i)),
    m_regid(regid)
{
}

BWWrite::~BWWrite()
{
}

void BWWrite::sendrequest()
{
  uint8 global_blp = (getBoardId() * GET_CONFIG("RS.N_BLPS", i)) + getCurrentBLP();

  if (m_regid < MEPHeader::BFXRE || m_regid > MEPHeader::BFYIM)
  {
    LOG_FATAL("invalid regid");
    exit(EXIT_FAILURE);
  }

  LOG_DEBUG(formatString(">>>> BWWrite(%s) global_blp=%d, regid=%d",
			 getBoardPort().getName().c_str(),
			 global_blp,
			 m_regid));
  
  // send next BF configure message
  EPABfcoefsEvent bfcoefs;
      
  MEP_BF(bfcoefs.hdr, MEPHeader::WRITE, getCurrentBLP(), m_regid);

  // copy weights from the cache to the message
  Array<complex<int16>, 2> weights((complex<int16>*)&bfcoefs.coef,
				   shape(N_BEAMLETS, N_POL),
				   neverDeleteData);

  weights = Cache::getInstance().getBack().getBeamletWeights()()(0, global_blp, Range::all(), Range::all());

  switch (m_regid)
  {
    case MEPHeader::BFXRE:
    {
      // weights for x-real part
      // no added conversions needed
    }
    break;

    case MEPHeader::BFXIM:
    {
      // weights for x-imaginary part
      weights *= complex<int16>(0,1);
    }
    break;
    
    case MEPHeader::BFYRE:
    {
      // weights for y-real part
      // no added conversions needed
    }
    break;
    
    case MEPHeader::BFYIM:
    {
      // weights for y-imaginary part
      weights *= complex<int16>(0,1);
    }
    break;

    default:
      LOG_ERROR("Invalid m_refid.");
      break;
  }
  
  getBoardPort().send(bfcoefs);
}

void BWWrite::sendrequest_status()
{
  LOG_DEBUG("sendrequest_status");

#if WRITE_ACK_VERREAD
  // send version read request
  EPAFwversionReadEvent versionread;
  MEP_FWVERSION(versionread.hdr, MEPHeader::READ);

  getBoardPort().send(versionread);
#else
  // send read status request to check status of the write
  EPARspstatusReadEvent rspstatus;
  MEP_RSPSTATUS(rspstatus.hdr, MEPHeader::READ);

  getBoardPort().send(rspstatus);
#endif
}

GCFEvent::TResult BWWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
#if WRITE_ACK_VERREAD
  EPAFwversionEvent ack(event);
#else
  EPARspstatusEvent rspstatus(event);
#endif

  LOG_DEBUG("handleack");

  return GCFEvent::HANDLED;
}


