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

using namespace RSP;
using namespace LOFAR;
using namespace EPA_Protocol;
using namespace blitz;

#define N_RETRIES 3

BWSync::BWSync(GCFPortInterface& board_port, int board_id, int regid)
  : SyncAction(board_port, board_id),
    m_regid(regid)
{
}

BWSync::~BWSync()
{
}

void BWSync::sendrequest(uint8 blp)
{
  if (m_regid <= MEPHeader::BFXRE || m_regid > MEPHeader::BFYIM)
  {
    m_regid = MEPHeader::BFXRE; // HACK
  }

  LOG_DEBUG(formatString(">>>> BWSync(%s) blp=%d, regid=%d",
			 getBoardPort().getName().c_str(),
			 blp,
			 m_regid));
  
  // send next BF configure message
  EPABfcoefsEvent bfcoefs;
      
  MEP_BF(bfcoefs.hdr, MEPHeader::WRITE, 0, m_regid);
  bfcoefs.hdr.m_fields.addr.dstid = blp;

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
    weights = real(Cache::getInstance().getBack().getBeamletWeights()()(0, blp, Range::all()));
  }
  else
  {
    weights = imag(Cache::getInstance().getBack().getBeamletWeights()()(0, blp, Range::all()));
  }

  getBoardPort().send(bfcoefs);
}

void BWSync::sendrequest_status()
{
  // send read status request to check status of the write
  EPARspstatusEvent rspstatus;
  MEP_RSPSTATUS(rspstatus.hdr, MEPHeader::READ);

  // clear from first field onwards
  memset(&rspstatus.board, 0, MEPHeader::RSPSTATUS_SIZE);

#if 0
  // on the read request don't send the data
  rspstatus.length -= RSPSTATUS_SIZE;
#endif

  getBoardPort().send(rspstatus);
}

GCFEvent::TResult BWSync::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  EPARspstatusEvent rspstatus(event);

  return GCFEvent::HANDLED;
}


