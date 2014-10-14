//#  TDSProtocolWrite.cc: implementation of the TDSProtocolWrite class
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
#include <string.h>

#include "TDSProtocolWrite.h"
#include "TDSi2cdefs.h"
#include "Cache.h"

#include <netinet/in.h>

using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;

//
// maximum size of chunks written to the TDS register
//
#define TDS_CHUNK_SIZE 1400
#define TDS_N_CHUNKS   2

namespace LOFAR {
  namespace RSP {

    static uint8 tds_160MHz[
#ifndef DISABLE_PROGRAMPLL
			      TDS_INIT_SIZE 
			    + TDS_PROGRAMPLLS_SIZE
#endif
			    + TDS_160MHZ_SIZE
			    + TDS_C_END_SIZE] = {

			      // switch to 160MHz to backplane (using 10MHz reference at the front)
#ifndef DISABLE_PROGRAMPLL
			      TDS_INIT,
			      TDS_PROGRAMPLLS,
#endif
			      TDS_160MHZ,
			      TDS_C_END,

			    };
      

    uint8 tds_160MHz_result[
#ifndef DISABLE_PROGRAMPLL
			      TDS_INIT_RESULT_SIZE
			    + TDS_PROGRAMPLLS_RESULT_SIZE
#endif
			    + TDS_160MHZ_RESULT_SIZE
			    + TDS_C_END_RESULT_SIZE] = {

#ifndef DISABLE_PROGRAMPLL
			      TDS_INIT_RESULT,
			      TDS_PROGRAMPLLS_RESULT,
#endif
			      TDS_160MHZ_RESULT,
			      TDS_C_END_RESULT,

			    };

    static uint8 tds_200MHz[ 
#ifndef DISABLE_PROGRAMPLL
			      TDS_INIT_SIZE
			    + TDS_PROGRAMPLLS_SIZE
#endif
			    + TDS_200MHZ_SIZE
			    + TDS_C_END_SIZE] = {

			      // switch to 200MHz to backplane (using 10MHz reference at the front)
#ifndef DISABLE_PROGRAMPLL
			      TDS_INIT,
			      TDS_PROGRAMPLLS,
#endif
			      TDS_200MHZ,
			      TDS_C_END,

			    };

    uint8 tds_200MHz_result[
#ifndef DISABLE_PROGRAMPLL
			      TDS_INIT_RESULT_SIZE
			    + TDS_PROGRAMPLLS_RESULT_SIZE
#endif
			    + TDS_200MHZ_RESULT_SIZE
			    + TDS_C_END_RESULT_SIZE] = {

#ifndef DISABLE_PROGRAMPLL
			      TDS_INIT_RESULT,
			      TDS_PROGRAMPLLS_RESULT,
#endif
			      TDS_200MHZ_RESULT,
			      TDS_C_END_RESULT,

			    };

    static uint8 tds_off[  TDS_VCXO_OFF_SIZE
			 + TDS_OFF_SIZE
			 + TDS_C_END_SIZE] = {

			   // switch off clock to backplane, RSP should switch to 125MHz
			   TDS_VCXO_OFF,
			   TDS_OFF,
			   TDS_C_END,

			 };

    uint8 tds_off_result[  TDS_VCXO_OFF_RESULT_SIZE
			 + TDS_OFF_RESULT_SIZE
			 + TDS_C_END_RESULT_SIZE] = {

			   TDS_VCXO_OFF_RESULT,
			   TDS_OFF_RESULT,
			   TDS_C_END_RESULT,

			 };
  };
};

TDSProtocolWrite::TDSProtocolWrite(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, TDS_N_CHUNKS), m_remaining(0), m_offset(0) // need 2 messages at most
{
  memset(&m_hdr, 0, sizeof(MEPHeader));

  // this action should be performed at initialisation
  doAtInit();
}

TDSProtocolWrite::~TDSProtocolWrite()
{
  /* TODO: delete event? */
}

void TDSProtocolWrite::sendrequest()
{
  char* buf = 0;

  // skip update if the Clocks settings have not been modified
  if (RTC::RegisterState::WRITE != Cache::getInstance().getState().tdwrite().get(getBoardId())) {

    Cache::getInstance().getState().tdwrite().unmodified(getBoardId());
    setContinue(true);
    return;
  }

  uint32 tds_control = 0;
  sscanf(GET_CONFIG_STRING("RSPDriver.TDS_CONTROL"), "%x", &tds_control);

  if (!(tds_control & (1 << getBoardId()))) {
    Cache::getInstance().getState().tdwrite().write_ack(getBoardId());
    setContinue(true);

    return;
  }

  EPATdsProtocolEvent tdsprotocol;

  size_t size = 0;

  switch (Cache::getInstance().getBack().getClock()) {
  case 160:
    buf = (char*)tds_160MHz;
    if (0 == getCurrentIndex()) {
      m_remaining = sizeof(tds_160MHz);
      m_offset    = 0;
    }
    size = MIN(TDS_CHUNK_SIZE, m_remaining);
    tdsprotocol.hdr.set(MEPHeader::TDS_PROTOCOL_HDR, MEPHeader::DST_RSP, MEPHeader::WRITE, size, m_offset);
    break;

  case 200:
    buf = (char*)tds_200MHz;
    if (0 == getCurrentIndex()) {
      m_remaining = sizeof(tds_200MHz);
      m_offset    = 0;
    }
    size = MIN(TDS_CHUNK_SIZE, m_remaining);
    tdsprotocol.hdr.set(MEPHeader::TDS_PROTOCOL_HDR, MEPHeader::DST_RSP, MEPHeader::WRITE, size, m_offset);
    break;

  default:
    buf = (char*)tds_off;
    if (0 == getCurrentIndex()) {
      m_remaining = sizeof(tds_off);
      m_offset    = 0;
    }
    size = MIN(TDS_CHUNK_SIZE, m_remaining);
    tdsprotocol.hdr.set(MEPHeader::TDS_PROTOCOL_HDR, MEPHeader::DST_RSP, MEPHeader::WRITE, size, m_offset);
    break;
  }

  tdsprotocol.protocol.setBuffer((char*)buf + m_offset, size);

  // indicate that we're initialising the hardware
  LOG_INFO_STR(formatString("Sending clock setting (offset=%d) via RSP board %d: %d MHz",
			    m_offset, getBoardId(), Cache::getInstance().getBack().getClock()));

  // advance
  m_remaining -= size;
  m_offset    += size;

  m_hdr = tdsprotocol.hdr; // remember header to match with ack

  getBoardPort().send(tdsprotocol);
}

void TDSProtocolWrite::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult TDSProtocolWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_WRITEACK != event.signal)
  {
    LOG_WARN("TDSProtocolWrite::handleack:: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }
  
  EPAWriteackEvent ack(event);

  if (!ack.hdr.isValidAck(m_hdr))
  {
    LOG_ERROR("TDSProtocolWrite::handleack: invalid ack");
    Cache::getInstance().getState().tdwrite().write_error(getBoardId());
    return GCFEvent::NOT_HANDLED;
  }

  // Mark register modification as applied
  // Still needs to be confirmed by TDSRegisterRead
  if (0 == m_remaining) {
    Cache::getInstance().getState().tdwrite().write_ack(getBoardId());
  }

  return GCFEvent::HANDLED;
}
