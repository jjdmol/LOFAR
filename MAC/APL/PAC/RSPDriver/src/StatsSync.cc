//#  StatsSync.cc: implementation of the StatsSync class
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

#include "StatsSync.h"
#include "Statistics.h"
#include "EPA_Protocol.ph"
#include "Cache.h"

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

using namespace RSP;
using namespace LOFAR;
using namespace EPA_Protocol;
using namespace RSP_Protocol;
using namespace blitz;

StatsSync::StatsSync(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, N_BLP)
{
}

StatsSync::~StatsSync()
{
  /* TODO: delete event? */
}

void StatsSync::sendrequest()
{
  EPAStstatsReadEvent statsread;
  MEP_ST(statsread.hdr, MEPHeader::READ, getCurrentBLP(), MEPHeader::MEAN);

  getBoardPort().send(statsread);
}

void StatsSync::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult StatsSync::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  EPAStstatsEvent ack(event);

  Statistics& stats = Cache::getInstance().getBack().getStatistics();

  memcpy(stats()(MEPHeader::MEAN,
		 (getBoardId() * N_BLP) + ack.hdr.m_fields.addr.dstid,
		 Range::all()).data(),
	 &ack.stat, MAX_N_BEAMLETS);

  return GCFEvent::HANDLED;
}
