//#  TDSStatusRead.cc: implementation of the TDSStatusRead class
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
#include <Common/LofarConstants.h>
#include <APL/RSP_Protocol/EPA_Protocol.ph>

#include <APL/RTCCommon/PSAccess.h>
#include <string.h>

#include "TDSStatusRead.h"
#include "TDSi2cdefs.h"
#include "Cache.h"
#include "StationSettings.h"

#include <netinet/in.h>
#include <blitz/array.h>

// define offsets within the tdsreadstatus structure to access the fields.
#define TDS_STATUS_INDEX 0
#define TDS_READOK_INDEX 1
#define	TDS_V2_5_INDEX	 3
#define	TDS_V3_3_INDEX	 5
#define	TDS_TEMP_INDEX	 7
#define SPU_READOK_INDEX 9
#define	SPU_V2_5_INDEX	 10
#define	SPU_V3_3_INDEX	 12
#define	SPU_V12_INDEX	 14
#define	SPU_VCC_INDEX	 16
#define	SPU_TEMP_INDEX	 18

namespace LOFAR {
  namespace RSP {
	using namespace EPA_Protocol;
	using namespace blitz;
	using namespace RTC;

#if 0
void printbin(void* buf, int n)
{
  unsigned char* c = (unsigned char*)buf;
  fprintf(stderr, "TDSStatusRead: ");
  for (int i = 0; i< n; i++) {
    fprintf(stderr, "%02x ", c[i]);
  }
  fprintf(stderr, "\n");
}
#else
#define printbin(buf, n) do { } while (0)
#endif

//
// TDSStatusRead(port, boardID)
//
TDSStatusRead::TDSStatusRead(GCFPortInterface& board_port, int board_id): 
	SyncAction(board_port, board_id, 1)
{
	memset(&m_hdr, 0, sizeof(MEPHeader));
}

//
// ~TDSStatusRead()
//
TDSStatusRead::~TDSStatusRead()
{
	/* TODO: delete event? */
}

//
// sendrequest()
//
void TDSStatusRead::sendrequest()
{
	TDStatus& status = Cache::getInstance().getBack().getTDStatus();

	// always perform this action
	uint32 tds_control = 0;
	sscanf(GET_CONFIG_STRING("RSPDriver.TDS_CONTROL"), "%x", &tds_control);

	// only one RSPBoard controls the TD, if this is not the one just continue.
	if (!(tds_control & (1 << getBoardId()))) {
		status.board()(getBoardId()).invalid = 1;
		Cache::getInstance().getState().tdstatusread().read_ack(getBoardId());
		setContinue(true);
		return;
	}

	// start with unknown status
	status.board()(getBoardId()).unknown = 1;

	// send read event
	EPATdsResultEvent tdsresult;
	tdsresult.hdr.set(MEPHeader::TDS_RESULT_HDR, MEPHeader::DST_RSP, MEPHeader::READ, sizeof(tds_readstatus_result));
	m_hdr = tdsresult.hdr; // remember header to match with ack
	getBoardPort().send(tdsresult);
}

//
// sendrequest_status
//
void TDSStatusRead::sendrequest_status()
{
	// intentionally left empty
}

//
// handleack(event, port)
//
GCFEvent::TResult TDSStatusRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
	// is this the message type we are waiting for?
	if (EPA_TDS_RESULT != event.signal) {
		LOG_WARN("TDSStatusRead::handleack:: unexpected ack");
		return GCFEvent::NOT_HANDLED;
	}

	// convert to TDSResultEvent
	EPATdsResultEvent ack(event);
	if (!ack.hdr.isValidAck(m_hdr)) {
		LOG_ERROR("TDSStatusRead::handleack: invalid ack");
		return GCFEvent::NOT_HANDLED;
	}

	// debug the stuff
	printbin(ack.result, sizeof(tds_readstatus_result));

	// Note: the TD and the SPU info is handle to the user as seperate info so split the
	//		 the results we received into a TD part and a SPU part.

	// get pointers to the cache
	TDStatus&	TDstat  		= Cache::getInstance().getBack().getTDStatus();
	SPUStatus&	SPUstat 		= Cache::getInstance().getBack().getSPUStatus();
	struct TDBoardStatus*	TD 	= &(TDstat.board()(getBoardId()));
	struct SPUBoardStatus*	SPU = &(SPUstat.subrack()(getBoardId()/NR_RSPBOARDS_PER_SUBRACK));
	if (ack.result[TDS_READOK_INDEX] != 0x00) {
		// indicate that status is unknown
		TDstat.board()(getBoardId()).unknown = 1;
		Cache::getInstance().getState().tdstatusread().read_error(getBoardId());
	} else {
		// copy TDS info to cache
		LOG_DEBUG(formatString("LOCK: 0x%02x", ack.result[TDS_STATUS_INDEX]));
		memcpy(&TDstat.board()(getBoardId()), &ack.result + TDS_STATUS_INDEX, sizeof(uint8));
		TD->v2_5		 = ack.result[TDS_V2_5_INDEX];
		TD->v3_3		 = ack.result[TDS_V3_3_INDEX];
		TD->temperature	 = ack.result[TDS_TEMP_INDEX];
		// copy SPU info to cache
		SPU->v2_5 		 = ack.result[SPU_V2_5_INDEX];
		SPU->v3_3 		 = ack.result[SPU_V3_3_INDEX];
		SPU->v12  		 = ack.result[SPU_V12_INDEX];
		SPU->vcc  		 = ack.result[SPU_VCC_INDEX];
		SPU->temperature = ack.result[SPU_TEMP_INDEX];
		Cache::getInstance().getState().tdstatusread().read_ack(getBoardId());
	}

	return GCFEvent::HANDLED;
}

  } // namespace RSP
} // namespace LOFAR
