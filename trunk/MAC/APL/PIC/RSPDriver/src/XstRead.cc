//#  XstRead.cc: implementation of the XstRead class
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <APL/RSP_Protocol/EPA_Protocol.ph>

#include <APL/RTCCommon/PSAccess.h>
#include <APL/RSP_Protocol/Statistics.h>

#include "StationSettings.h"
#include "XstRead.h"
#include "Cache.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;
using namespace RSP_Protocol;
using namespace RTC;

XstRead::XstRead(GCFPortInterface& board_port, int board_id, int regid)
  : SyncAction(board_port, board_id, 1),
    m_regid(regid)
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

XstRead::~XstRead()
{
}

void XstRead::sendrequest()
{
  // offset in bytes
  //uint16 offset = (GET_CONFIG("RSPDriver.XST_FIRST_RSP_BOARD", i) + 1) * MEPHeader::N_LOCAL_XLETS * MEPHeader::XLET_SIZE;
  uint16 offset = GET_CONFIG("RSPDriver.XST_FIRST_RSP_BOARD", i) * MEPHeader::XLET_SIZE;

  LOG_DEBUG_STR("XstRead::offset=" << offset);

  if (m_regid < MEPHeader::XST_0X0 || m_regid > MEPHeader::XST_3Y3)
  {
    LOG_FATAL("invalid regid");
    exit(EXIT_FAILURE);
  }

  EPAReadEvent xstread;

  xstread.hdr.set(MEPHeader::XST_STATS_HDR, MEPHeader::DST_RSP,
		  MEPHeader::READ, MEPHeader::XST_STATS_SIZE, offset);
  xstread.hdr.m_fields.addr.regid = m_regid;

  m_hdr = xstread.hdr;
  getBoardPort().send(xstread);
}

void XstRead::sendrequest_status()
{
  // intentionally left empty
}

#define CONVERT_UINT32_TO_INT64(val,val64)	\
do {						\
  uint32 e = val & (1<<31);			\
  uint32 s = val & (1<<30);			\
  int32  m = val & ((1<<30)-1);			\
						\
  if (s) m = m - (1<<30);			\
  if (e) {					\
    val64 = (int64)m << 23;			\
  } else {					\
    val64 = m;					\
  }						\
} while (0)

/**
 * Function to convert the complex semi-floating point representation used by the
 * EPA firmware to a complex<double>.
 */
BZ_DECLARE_FUNCTION_RET(convert_cuint32_to_cdouble, complex<double>)
inline complex<double> convert_cuint32_to_cdouble(complex<uint32> val)
{
  int64 val64_re, val64_im;

  CONVERT_UINT32_TO_INT64(real(val), val64_re);
  CONVERT_UINT32_TO_INT64(imag(val), val64_im);

  // convert two int64's to complex double
  return complex<double>(val64_re, val64_im);
}

GCFEvent::TResult XstRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_XST_STATS != event.signal)
  {
    LOG_WARN("XstRead::handleack: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }

  EPAXstStatsEvent ack(event);

  if (!ack.hdr.isValidAck(m_hdr))
  {
    LOG_ERROR("XstRead::handleack: invalid ack");
    return GCFEvent::NOT_HANDLED;
  }

  if (ack.hdr.m_fields.addr.regid >= MEPHeader::XST_NR_STATS)
  {
    LOG_ERROR("invalid xst ack");
    return GCFEvent::HANDLED;
  }

  int global_blp = (getBoardId() * StationSettings::instance()->nrBlpsPerBoard()) + (m_regid / MEPHeader::N_POL);
  
  LOG_DEBUG(formatString("XstRead::handleack: global_blp=%d", global_blp));

  Array<complex<double>, 4>& cache(Cache::getInstance().getBack().getXCStats()());

  int serdes_lane = m_regid / (MEPHeader::N_POL * StationSettings::instance()->nrBlpsPerBoard());
  Range blp_target_range(serdes_lane,
			 MEPHeader::N_REMOTE_XLETS * StationSettings::instance()->nrBlpsPerBoard() - 1,
			 MEPHeader::N_PHASEPOL);
		  
  //Range rcu_range(remote_blp_offset, remote_blp_offset + MEPHeader::N_REMOTE_XLETS - 1);
  
  LOG_DEBUG_STR(endl << 
		"global_blp=" << global_blp << endl <<
		"blp_target_range=" << blp_target_range << endl <<
		"xststats.range=" << Range(0, MEPHeader::N_REMOTE_XLETS));

  LOG_DEBUG_STR("xststats shape=" << shape(MEPHeader::N_REMOTE_XLETS, MEPHeader::N_POL));
  
  Array<complex<uint32>, 2> xststats((complex<uint32>*)&ack.xst_stat,
				     shape(MEPHeader::N_REMOTE_XLETS, MEPHeader::N_POL),
				     neverDeleteData);
  
  cache(m_regid % MEPHeader::N_POL, Range::all(), global_blp, blp_target_range) =
    convert_cuint32_to_cdouble(xststats);

#if 0
  // convert and reorder dimensions
  for (int i = 0; i < MEPHeader::N_REMOTE_XLETS; i++) {
    for (int j = 0; j < MEPHeader::N_POL; j++) {
      cache(m_regid % MEPHeader::N_POL, j, global_blp, i + remote_blp_offset)
	= convert_cuint32_to_cdouble(xststats(i, j));
    }
  }
#endif

  return GCFEvent::HANDLED;
}
