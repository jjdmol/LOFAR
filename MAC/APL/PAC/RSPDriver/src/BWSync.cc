//#  BWSync.cc: implementation of the BWSync class
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

#include "BWSync.h"
#include "EPA_Protocol.ph"
#include "RSP_Protocol.ph"
#include "Cache.h"

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <unistd.h>
#include <string.h>
#include <blitz/array.h>

//
// Final RSP board will have 4 BLPs (N_BLP == 4)
// Proto2 board has one BLP (N_PROTO2_BLP == 1)
//
#ifdef N_PROTO2_BLP
#undef N_BLP
#define N_BLP N_PROTO2_BLP
#endif

#define N_RETRIES 3

using namespace RSP;
using namespace LOFAR;
using namespace EPA_Protocol;
using namespace blitz;

BWSync::BWSync(GCFPortInterface& board_port, int board_id, int regid)
  : SyncAction(board_port, board_id, N_BLP),
    m_regid(regid)
{
}

BWSync::~BWSync()
{
}

void BWSync::sendrequest(int local_blp)
{
  uint8 global_blp = (getBoardId() * N_BLP) + local_blp;

  if (m_regid <= MEPHeader::BFXRE || m_regid > MEPHeader::BFYIM)
  {
    m_regid = MEPHeader::BFXRE; // HACK
  }

  LOG_DEBUG(formatString(">>>> BWSync(%s) global_blp=%d, regid=%d",
			 getBoardPort().getName().c_str(),
			 global_blp,
			 m_regid));
  
  // send next BF configure message
  EPABfcoefsEvent bfcoefs;
      
  MEP_BF(bfcoefs.hdr, MEPHeader::WRITE, local_blp, m_regid);

  // copy weights from the cache to the message
  Array<int16, 1> weights((int16*)&bfcoefs.coef,
			  shape(RSP_Protocol::MAX_N_BEAMLETS),
			  neverDeleteData);
  
  //
  // TODO
  // Make sure we're actually sending the correct weights.
  //
  if (0 == (m_regid % 2))
  {
    weights = real(Cache::getInstance().getBack().getBeamletWeights()()(0, global_blp, Range::all()));
  }
  else
  {
    weights = imag(Cache::getInstance().getBack().getBeamletWeights()()(0, global_blp, Range::all()));
  }

  getBoardPort().send(bfcoefs);
}

void BWSync::sendrequest_status()
{
  LOG_DEBUG("sendrequest_status");

  // send read status request to check status of the write
  EPARspstatusReadEvent rspstatus;
  MEP_RSPSTATUS(rspstatus.hdr, MEPHeader::READ);

  getBoardPort().send(rspstatus);
}

GCFEvent::TResult BWSync::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  EPARspstatusEvent rspstatus(event);

  LOG_DEBUG("handleack");

  return GCFEvent::HANDLED;
}


