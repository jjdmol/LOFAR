//#  StatsRead.cc: implementation of the StatsRead class
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

#include "StatsRead.h"
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

StatsRead::StatsRead(GCFPortInterface& board_port, int board_id, uint8 type)
  : SyncAction(board_port, board_id, N_BLP), m_type(type)
{
}

StatsRead::~StatsRead()
{
  /* TODO: delete event? */
}

void StatsRead::sendrequest()
{
  if (m_type <= Statistics::SUBBAND_POWER)
  {
    EPAStsubstatsReadEvent statsread;

    MEP_STSUB(statsread.hdr, MEPHeader::READ, getCurrentBLP(), m_type);

    getBoardPort().send(statsread);
  }
  else
  {
    EPAStstatsReadEvent statsread;

    MEP_ST(statsread.hdr, MEPHeader::READ, getCurrentBLP(), m_type - Statistics::BEAMLET_MEAN);

    getBoardPort().send(statsread);
  }
}

void StatsRead::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult StatsRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  uint8 global_blp = (getBoardId() * N_BLP) + getCurrentBLP();

  if (m_type <= Statistics::SUBBAND_POWER)
  {
    EPAStsubstatsEvent ack(event);

    Array<complex<uint16>, 2> stats((complex<uint16>*)&ack.stat,
				    shape(N_BEAMLETS, N_POL),
				    neverDeleteData);

    complex<double>* cacheptr = Cache::getInstance().getBack().getSubbandStats()()((int)m_type,
										   global_blp * 2, Range::all()).data();
    complex<uint16>* msgptr = stats.data();
    
    // copy stats for x-polarization, convert from uint16 to double
    for (int bl = 0; bl < N_BEAMLETS; bl++)
    {
      *cacheptr++ = *msgptr++;
      *msgptr++; // skip to next
    }

    cacheptr = Cache::getInstance().getBack().getSubbandStats()()((int)m_type,
								  global_blp * 2 + 1, Range::all()).data();
    msgptr = stats.data();
    
    // copy stats for y-polarization, convert from uint16 to double
    msgptr++; // skip x-part
    for (int bl = 0; bl < N_BEAMLETS; bl++)
    {
      *cacheptr++ = *msgptr++;
      msgptr++;
    }
  }
  else
  {
    EPAStstatsEvent ack(event);

    Array<complex<uint16>, 2> stats((complex<uint16>*)&ack.stat,
				    shape(N_BEAMLETS, N_POL),
				    neverDeleteData);

    complex<double>* cacheptr = Cache::getInstance().getBack().getBeamletStats()()((int)m_type - Statistics::BEAMLET_MEAN,
										   global_blp * 2, Range::all()).data();
    complex<uint16>* msgptr = stats.data();
    
    // copy stats for x-polarization, convert from uint16 to double
    for (int bl = 0; bl < N_BEAMLETS; bl++)
    {
      *cacheptr++ = *msgptr++;
      *msgptr++; // skip to next
    }

    cacheptr = Cache::getInstance().getBack().getSubbandStats()()((int)m_type - Statistics::BEAMLET_MEAN,
								  global_blp * 2 + 1, Range::all()).data();
    msgptr = stats.data();
    
    // copy stats for y-polarization, convert from uint16 to double
    msgptr++; // skip x-part
    for (int bl = 0; bl < N_BEAMLETS; bl++)
    {
      *cacheptr++ = *msgptr++;
      msgptr++;
    }
  }

#if 0
  memcpy(stats()(MEPHeader::MEAN,
		 (getBoardId() * N_BLP) + ack.hdr.m_fields.addr.dstid,
		 Range::all()).data(),
	 &ack.stat, N_BEAMLETS * sizeof(uint16));
#endif

  return GCFEvent::HANDLED;
}
