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

#include <APLConfig.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;
using namespace EPA_Protocol;
using namespace RSP_Protocol;
using namespace blitz;

StatsRead::StatsRead(GCFPortInterface& board_port, int board_id, uint8 type)
  : SyncAction(board_port, board_id, GET_CONFIG("N_BLPS", i)), m_type(type)
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

BZ_DECLARE_FUNCTION_RET(convert_uint16_to_double, complex<double>)

inline complex<double> convert_uint16_to_double(complex<uint16> val)
{
  return complex<double>((double)val.real(),
			 (double)val.imag());
}

GCFEvent::TResult StatsRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  uint8 global_blp = (getBoardId() * GET_CONFIG("N_BLPS", i)) + getCurrentBLP();

  if (m_type <= Statistics::SUBBAND_POWER)
  {
    EPAStsubstatsEvent ack(event);

    Array<complex<uint16>, 2> stats((complex<uint16>*)&ack.stat,
 				    shape(N_SUBBANDS, N_POL),
 				    neverDeleteData);
    //Array<complex<double>, 2> dstats(N_SUBBANDS, N_POL);
    //dstats = convert_uint16_to_double(stats);

    //cout << stats(Range::all(), 0) << endl;
    //cout << stats(Range::all(), 1) << endl;

#if 1

    Array<complex<double>, 3>& cache(Cache::getInstance().getBack().getSubbandStats()());
    
    cache(m_type, global_blp * 2,     Range::all()) = convert_uint16_to_double(stats(Range::all(), 0)); //dstats(Range::all(), 0);
    cache(m_type, global_blp * 2 + 1, Range::all()) = convert_uint16_to_double(stats(Range::all(), 1)); //dstats(Range::all(), 1);
    
    //cout << cache(m_type, global_blp * 2,     Range::all()) << endl;
    //cout << cache(m_type, global_blp * 2 + 1, Range::all()) << endl;
    cout << "writing to (" << (int)m_type << ", " << global_blp*2   << ", all)"
	 << " at address " << cache(m_type, global_blp*2+1, Range::all()).data() << endl;
    cout << "writing to (" << (int)m_type << ", " << global_blp*2+1 << ", all)"
	 << " at address " << cache(m_type, global_blp*2+1, Range::all()).data() << endl;

#else
    complex<double>* cacheptr =
      Cache::getInstance().getBack().getSubbandStats()()((int)m_type, global_blp * 2, Range::all()).data();
    complex<uint16>* msgptr = stats.data();
    
    // copy stats for x-polarization, convert from uint16 to double
    for (int bl = 0; bl < N_SUBBANDS; bl++)
    {
      *cacheptr++ = *msgptr++;
      msgptr++; // skip over y-polarization
    }

    cacheptr =
      Cache::getInstance().getBack().getSubbandStats()()((int)m_type, global_blp * 2 + 1, Range::all()).data();
    msgptr = stats.data();
    
    // copy stats for y-polarization, convert from uint16 to double
    msgptr++; // skip x-part
    for (int bl = 0; bl < N_SUBBANDS; bl++)
    {
      *cacheptr++ = *msgptr++;
      msgptr++;
    }
#endif
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

    cacheptr = Cache::getInstance().getBack().getBeamletStats()()((int)m_type - Statistics::BEAMLET_MEAN,
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
		 (getBoardId() * GET_CONFIG("N_BLPS", i)) + ack.hdr.m_fields.addr.dstid,
		 Range::all()).data(),
	 &ack.stat, N_BEAMLETS * sizeof(uint16));
#endif

  return GCFEvent::HANDLED;
}
