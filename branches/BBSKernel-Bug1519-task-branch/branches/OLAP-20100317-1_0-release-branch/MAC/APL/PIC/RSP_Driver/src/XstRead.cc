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

#include <stdlib.h>

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
  //
  // The RSP boards are assigned an ID based on the hardware ID of the backplane and their position
  // within the backplane. For the final station all backplanes will be installed and in use, but
  // during development we may for example be using only two RSP boards with ID's 18 and 19.
  // In this case we need to offset cross correlation indexing to access to correct items of the global
  // cross-correlation array as computed by the firmware.
  //
  uint16 first_rsp_board = strtol(GET_CONFIG_STRING("RSPDriver.XST_FIRST_RSP_BOARD"), 0, 16);
  LOG_DEBUG_STR("first_rsp_board = " << first_rsp_board);
  uint16 offset = first_rsp_board * MEPHeader::XLET_SIZE;

  if (m_regid < MEPHeader::XST_STATS || m_regid >= MEPHeader::XST_NR_STATS)
  {
    LOG_FATAL("invalid regid");
    exit(EXIT_FAILURE);
  }

  Cache::getInstance().getState().xst().read(getBoardId() * MEPHeader::XST_NR_STATS + (m_regid - MEPHeader::XST_STATS));

  EPAReadEvent xstread;

  xstread.hdr.set(MEPHeader::XST_STATS_HDR,
		  MEPHeader::DST_RSP,
		  MEPHeader::READ,
		  (StationSettings::instance()->nrBlps() / MEPHeader::N_SERDES_LANES) * MEPHeader::XLET_SIZE,
		  offset);
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

//
// The layout of cross-correlations in the register on RSP boards
//
//                            LANE0           LANE1           LANE2           LANE3           LANE0           LANE1           LANE2           LANE3
//                 RCU:    ANT0X   ANT0Y   ANT1X   ANT1Y   ANT2X   ANT2Y   ANT3X   ANT3Y   ANT4X   ANT4Y   ANT5X   ANT5Y   ANT6X   ANT6Y   ANT7X   ANT7Y   .....
//         RSP0    ANT0X       REG0            REG8            REG16           REG24           REG0            REG8            REG16           REG24
//                 ANT0Y       REG1            REG9            REG17           REG25           REG1            REG9            REG17           REG25
//                 ANT1X       REG2            REG10           REG18           REG26           REG2            REG10           REG18           REG26
//                 ANT1Y       REG3            REG11           REG19           REG27           REG3            REG11           REG19           REG27
//                 ANT2X       REG4            REG12           REG20           REG28           REG4            REG12           REG20           REG28
//                 ANT2Y       REG5            REG13           REG21           REG29           REG5            REG13           REG21           REG29
//                 ANT3X       REG6            REG14           REG22           REG30           REG6            REG14           REG22           REG30
//                 ANT3Y       REG7            REG15           REG23           REG31           REG7            REG15           REG23           REG31
//         RSP1    ANT4X       REG0            REG8            REG16           REG24           REG0            REG8            REG16           REG24
//                 ANT4Y       REG1            REG9            REG17           REG25           REG1            REG9            REG17           REG25
//                 ANT5X       REG2            REG10           REG18           REG26           REG2            REG10           REG18           REG26
//                 ANT5Y       REG3            REG11           REG19           REG27           REG3            REG11           REG19           REG27
//                 ANT6X       REG4            REG12           REG20           REG28           REG4            REG12           REG20           REG28
//                 ANT6Y       REG5            REG13           REG21           REG29           REG5            REG13           REG21           REG29
//                 ANT7X       REG6            REG14           REG22           REG30           REG6            REG14           REG22           REG30
//                 ANT7Y       REG7            REG15           REG23           REG31           REG7            REG15           REG23           REG31
//         RSP 2
//         ....
//
// For example REG0 contains the correlations of local RCU ANT0X with ANT0X ANT0Y ANT4X ANT4Y ANT8X ANT8Y ANT12X ANT12Y etc.
//

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
    Cache::getInstance().getState().xst().read_error(getBoardId() * MEPHeader::XST_NR_STATS + (m_regid - MEPHeader::XST_STATS));
    LOG_ERROR("XstRead::handleack: invalid ack");
    return GCFEvent::NOT_HANDLED;
  }

  if (ack.hdr.m_fields.addr.regid >= MEPHeader::XST_NR_STATS)
  {
    Cache::getInstance().getState().xst().read_error(getBoardId() * MEPHeader::XST_NR_STATS + (m_regid - MEPHeader::XST_STATS));
    LOG_ERROR("invalid xst ack");
    return GCFEvent::HANDLED;
  }

  int nrBlps         = StationSettings::instance()->nrBlps();
  int nrBlpsPerBoard = StationSettings::instance()->nrBlpsPerBoard();
  int nrRcusPerBoard = StationSettings::instance()->nrRcusPerBoard();

  //
  // This register m_regid corresponds to a specific RCU along the vertical axis of
  // the cross-correlation matrix as it is shown in the the comment above.
  // The rcu index is a global index into the cross correlation matrix
  //
  int rcu = (getBoardId() * nrRcusPerBoard) + (m_regid % nrRcusPerBoard);

  Array<complex<double>, 4>& cache(Cache::getInstance().getBack().getXCStats()());

  Array<complex<uint32>, 2> xststats((complex<uint32>*)&ack.xst_stat,
				     shape(nrBlps, MEPHeader::N_POL),
				     neverDeleteData);

  // strided range, stride = nrBlpsPerBoard
  Range dst_range(m_regid / nrRcusPerBoard, nrBlps - 1, nrBlpsPerBoard);

  LOG_DEBUG_STR(formatString("m_regid=%02d rcu=%03d cache(%02d,Range(0,1),%02d,",
			     m_regid, rcu, rcu %  MEPHeader::N_POL, rcu / MEPHeader::N_POL) << dst_range << ")");

  // rcu with X cross-correlations
  cache(rcu % MEPHeader::N_POL, 0, rcu / MEPHeader::N_POL, dst_range) = convert_cuint32_to_cdouble(xststats(Range::all(), 0));
  cache(rcu % MEPHeader::N_POL, 1, rcu / MEPHeader::N_POL, dst_range) = convert_cuint32_to_cdouble(xststats(Range::all(), 1));

  Cache::getInstance().getState().xst().read_ack(getBoardId() * MEPHeader::XST_NR_STATS + (m_regid - MEPHeader::XST_STATS));

  return GCFEvent::HANDLED;
}
