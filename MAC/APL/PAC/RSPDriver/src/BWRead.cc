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
  : SyncAction(board_port, board_id, GET_CONFIG("RS.N_BLPS", i)),
    m_regid(regid)
{
}

BWRead::~BWRead()
{
}

void BWRead::sendrequest()
{
  uint8 global_blp = (getBoardId() * GET_CONFIG("RS.N_BLPS", i)) + getCurrentBLP();

  if (m_regid < MEPHeader::BFXRE || m_regid > MEPHeader::BFYIM)
  {
    LOG_FATAL("invalid regid");
    exit(EXIT_FAILURE);
  }

  LOG_DEBUG(formatString(">>>> BWRead(%s) global_blp=%d, regid=%d",
			 getBoardPort().getName().c_str(),
			 global_blp,
			 m_regid));
  
  // send next BF configure message
  EPABfcoefsReadEvent bfcoefsread;
      
  MEP_BF(bfcoefsread.hdr, MEPHeader::READ, getCurrentBLP(), m_regid);

  getBoardPort().send(bfcoefsread);

}

void BWRead::sendrequest_status()
{
  /* intentionally left empty */
}

GCFEvent::TResult BWRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (event.signal != EPA_BFCOEFS) return GCFEvent::HANDLED;

  uint8 global_blp = (getBoardId() * GET_CONFIG("RS.N_BLPS", i)) + getCurrentBLP();

  EPABfcoefsEvent bfcoefs(event);
  
  // copy weights from the message to the cache
  Array<int16, 1> weights((int16*)&bfcoefs.coef,
			  shape(N_BEAMLETS),
			  neverDeleteData);

  if (0 == (m_regid % 2))
  {
    real(Cache::getInstance().getBack().getBeamletWeights()()(0, global_blp, Range::all())) =
      weights;
  }
  else
  {
    imag(Cache::getInstance().getBack().getBeamletWeights()()(0, global_blp, Range::all())) =
      weights;
  }
  
  return GCFEvent::HANDLED;
}


