//#  SSWrite.cc: implementation of the SSWrite class
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

#include "SSWrite.h"
#include "Cache.h"

#include <blitz/array.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

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
using namespace blitz;

SSWrite::SSWrite(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, N_BLP * 2 /* for NOF_SUBBANDS and SUBBANDS*/)
{
}

SSWrite::~SSWrite()
{
  /* TODO: delete event? */
}

void SSWrite::sendrequest()
{
  if (0 == getCurrentBLP() % 2)
  {
    // send 'number of selected subbands'
    EPANrsubbandsEvent nrsubbands;
    MEP_NRSUBBANDS(nrsubbands.hdr, MEPHeader::WRITE, getCurrentBLP() / 2);

    nrsubbands.nof_subbands = N_BEAMLETS * 2 - 1;
    
    getBoardPort().send(nrsubbands);
  }
  else
  {
    uint8 global_blp = (getBoardId() * N_BLP) + (getCurrentBLP() / 2);
    LOG_DEBUG(formatString(">>>> SSWrite(%s) global_blp=%d",
			   getBoardPort().getName().c_str(),
			   global_blp));
    
    // send subband select message
    EPASubbandselectEvent ss;
    MEP_SUBBANDSELECT(ss.hdr, MEPHeader::WRITE, getCurrentBLP() / 2);
    
    // create array to contain the subband selection
    Array<uint16, 1> subbands((uint16*)&ss.ch,
			      EPA_Protocol::N_BEAMLETS * N_POL,
			      neverDeleteData);
    
    // copy the actual values from the cache
    subbands = Cache::getInstance().getBack().getSubbandSelection()()(global_blp, Range(0, N_BEAMLETS - 1));
    
    getBoardPort().send(ss);
  }
}

void SSWrite::sendrequest_status()
{
  LOG_DEBUG("sendrequest_status");

  // send read status request to check status of the write
  EPARspstatusReadEvent rspstatus;
  MEP_RSPSTATUS(rspstatus.hdr, MEPHeader::READ);

  getBoardPort().send(rspstatus);
}

GCFEvent::TResult SSWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  EPARspstatusEvent ack(event);

  LOG_DEBUG("handleack");

  if (ack.board.write.error ||
      ack.board.read.error)
  {
    LOG_ERROR_STR("\nSSWrite " 
		  << (ack.board.write.error
		      ? "write error"
		      : "read error\n"));
  }

  return GCFEvent::HANDLED;
}
