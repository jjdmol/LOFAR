//#  SSSync.cc: implementation of the SSSync class
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

#include "RSP_Protocol.ph"
#include "EPA_Protocol.ph"

#include "SSSync.h"
#include "Cache.h"

#include <blitz/array.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;
using namespace blitz;

#define N_RETRIES 3

SSSync::SSSync(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, N_BLP)
{
}

SSSync::~SSSync()
{
  /* TODO: delete event? */
}

void SSSync::sendrequest(int local_blp)
{
  uint8 global_blp = (getBoardId() * N_BLP) + local_blp;
  LOG_DEBUG(formatString(">>>> SSSync(%s) global_blp=%d",
			 getBoardPort().getName().c_str(),
			 global_blp));
  
  // send subband select message
  EPASubbandselectEvent ss;
  MEP_SUBBANDSELECT(ss.hdr, MEPHeader::WRITE, local_blp);
  
  // create array to contain the subband selection
  uint16 nr_subbands = Cache::getInstance().getBack().getSubbandSelection().nrsubbands()(global_blp);
  LOG_DEBUG(formatString("nr_subbands=%d", nr_subbands));
  nr_subbands=256;
  Array<uint16, 1> subbands(nr_subbands);
  ss.ch    = subbands.data();
  ss.chDim = nr_subbands;

  subbands = 0; // init
  
  subbands = Cache::getInstance().getBack().getSubbandSelection()()(global_blp, Range(0, nr_subbands - 1));

  getBoardPort().send(ss);

  //subbands.free();
}

void SSSync::sendrequest_status()
{
  LOG_DEBUG("sendrequest_status");

  // send read status request to check status of the write
  EPARspstatusReadEvent rspstatus;
  MEP_RSPSTATUS(rspstatus.hdr, MEPHeader::READ);

  getBoardPort().send(rspstatus);
}

GCFEvent::TResult SSSync::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  EPARspstatusEvent ack(event);

  LOG_DEBUG("handleack");

  return GCFEvent::HANDLED;
}
