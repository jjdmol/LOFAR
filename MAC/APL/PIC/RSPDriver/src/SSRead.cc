//#  SSRead.cc: implementation of the SSRead class
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

#include "SSRead.h"
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

SSRead::SSRead(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, N_BLP * 2 /* for nr_subbands and subbands */)
{
}

SSRead::~SSRead()
{
}

void SSRead::sendrequest()
{
  if (0 == (getCurrentBLP() % 2))
  {
    EPANrsubbandsReadEvent nrsubbandsread;
    MEP_NRSUBBANDS(nrsubbandsread.hdr, MEPHeader::READ, getCurrentBLP() / 2);

    getBoardPort().send(nrsubbandsread);
  }
  else
  {
    EPASubbandselectReadEvent ssread;
    MEP_SUBBANDSELECT(ssread.hdr, MEPHeader::READ, getCurrentBLP() / 2);

    getBoardPort().send(ssread);
  }
}

void SSRead::sendrequest_status()
{
  /* intentionally left empty */
}

GCFEvent::TResult SSRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  LOG_DEBUG("handleack");

  if (0 == (getCurrentBLP() % 2))
  {
    uint8 global_blp = (getBoardId() * N_BLP) + (getCurrentBLP() / 2);
  
    // unpack nrsubbands message
    EPANrsubbandsEvent nrsubbands(event);

    // copy into the cache
    Cache::getInstance().getBack().getSubbandSelection().nrsubbands()(global_blp)
      = nrsubbands.nof_subbands;
  }
  else
  {
    uint8 global_blp = (getBoardId() * N_BLP) + (getCurrentBLP() / 2);

    LOG_DEBUG("handleack");

    LOG_DEBUG(formatString(">>>> SSRead(%s) global_blp=%d",
			   getBoardPort().getName().c_str(), global_blp));
  
    // unpack ss message
    EPASubbandselectEvent ss(event);
  
    // create array point to data in the response event
    Array<uint16, 1> subbands((uint16*)&ss.ch,
			      shape(RSP_Protocol::MAX_N_BEAMLETS),
			      neverDeleteData);
  
    // copy into the cache
    Cache::getInstance().getBack().getSubbandSelection()()(global_blp, Range(0, MAX_N_BEAMLETS - 1))
      = subbands;
  }

  return GCFEvent::HANDLED;
}
