//#  BstRead.cc: implementation of the BstRead class
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

#include "BstRead.h"
#include "Statistics.h"
#include "EPA_Protocol.ph"
#include "Cache.h"

#include <PSAccess.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;
using namespace EPA_Protocol;
using namespace RSP_Protocol;
using namespace blitz;

BstRead::BstRead(GCFPortInterface& board_port, int board_id, uint8 type)
  : SyncAction(board_port, board_id, BST_N_FRAGMENTS), m_type(type)
{
}

BstRead::~BstRead()
{
  /* TODO: delete event? */
}

void BstRead::sendrequest()
{
  EPAReadEvent bstread;

  uint16 byteoffset = (getCurrentBLP() % BST_N_FRAGMENTS) * MEPHeader::FRAGMENT_SIZE;

  switch (m_type)
  {
    case Statistics::BEAMLET_MEAN:
      bstread.hdr.set(MEPHeader::BST_MEAN_HDR, MEPHeader::DST_RSP,
			MEPHeader::READ, N_STATS * sizeof(int32), byteoffset);
      break;
    
    case Statistics::BEAMLET_POWER:
      bstread.hdr.set(MEPHeader::BST_POWER_HDR, MEPHeader::DST_RSP,
			MEPHeader::READ, N_STATS * sizeof(uint32), byteoffset);
      break;

    default:
      LOG_ERROR("invalid statistics type");
      break;
  }

  getBoardPort().send(bstread);
}

void BstRead::sendrequest_status()
{
  // intentionally left empty
}

/**
 * Functions to cast a complex<int32> and complex<uint32> to a complex<double>
 */
BZ_DECLARE_FUNCTION_RET(convert_int32_to_double, complex<double>)
inline complex<double> convert_int32_to_double(complex<int32> val)
{
  return complex<double>((double)val.real(),
			 (double)val.imag());
}

BZ_DECLARE_FUNCTION_RET(convert_uint32_to_double, complex<double>)
inline complex<double> convert_uint32_to_double(complex<uint32> val)
{
  return complex<double>((double)val.real(),
			 (double)val.imag());
}

GCFEvent::TResult BstRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  EPAStatsEvent ack(event);

  uint16 offset = ack.hdr.m_fields.offset / MEPHeader::N_PHASE / sizeof(int32);
  
  LOG_DEBUG(formatString("BstRead::handleack: boardid=%d, offset=%d",
			 getBoardId(), offset));

  Range fragment_range(offset / MEPHeader::N_POL,
		       (offset / MEPHeader::N_POL) + (N_STATS / MEPHeader::N_PHASEPOL) - 1);

  LOG_DEBUG_STR("fragment_range=" << fragment_range);
  
  switch (m_type)
  {
    case Statistics::BEAMLET_MEAN:
    {
      Array<complex<int32>, 2> stats((complex<int32>*)&ack.stat,
				      shape(N_STATS / MEPHeader::N_PHASEPOL,
					    MEPHeader::N_POL),
				      neverDeleteData);

      if ((m_type - Statistics::BEAMLET_MEAN) != ack.hdr.m_fields.addr.regid)
      {
	LOG_ERROR("invalid bst ack");
	return GCFEvent::HANDLED;
      }

      Array<complex<double>, 3>& cache(Cache::getInstance().getBack().getBeamletStats()());

      // x-pol beamlet statistics: copy and convert to double
      cache(m_type - Statistics::BEAMLET_MEAN, getBoardId() * 2,     fragment_range) =
	convert_int32_to_double(stats(Range::all(), 0));

      // y-pol beamlet statistics: copy and convert to double
      cache(m_type - Statistics::BEAMLET_MEAN, getBoardId() * 2 + 1, fragment_range) =
	convert_int32_to_double(stats(Range::all(), 1));
    }
    break;

    case Statistics::BEAMLET_POWER:
    {
      Array<complex<uint32>, 2> stats((complex<uint32>*)&ack.stat,
				      shape(N_STATS / MEPHeader::N_PHASEPOL,
					    MEPHeader::N_POL),
				      neverDeleteData);

      if ((m_type - Statistics::BEAMLET_MEAN) != ack.hdr.m_fields.addr.regid)
      {
	LOG_ERROR("invalid bst ack");
	return GCFEvent::HANDLED;
      }

      Array<complex<double>, 3>& cache(Cache::getInstance().getBack().getBeamletStats()());

      // x-pol beamlet statistics: copy and convert to double
      cache(m_type - Statistics::BEAMLET_MEAN, getBoardId() * 2,     fragment_range) =
	convert_uint32_to_double(stats(Range::all(), 0));

      // y-pol beamlet statistics: copy and convert to double
      cache(m_type - Statistics::BEAMLET_MEAN, getBoardId() * 2 + 1, fragment_range) =
	convert_uint32_to_double(stats(Range::all(), 1));
    }
    break;
      
    default:
      LOG_ERROR("invalid statistics type");
      break;
  }
  
  return GCFEvent::HANDLED;
}
