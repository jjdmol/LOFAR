//#  SerdesRead.cc: implementation of the SerdesRead class
//#
//#  Copyright (C) 2009
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
#include <Common/hexdump.h>

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "SerdesRead.h"
#include "Cache.h"

#define N_RETRIES 3

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RTC;

SerdesRead::SerdesRead(GCFPortInterface& board_port, int board_id, int nrCycles)
  : SyncAction(board_port, board_id, nrCycles)
{
	memset(&itsHeader, 0, sizeof(MEPHeader));
}

SerdesRead::~SerdesRead()
{
  /* TODO: delete event? */
}

void SerdesRead::sendrequest()
{
	//LOG_INFO(formatString(">>>> SerdesRead:board=%d,index=%d", getBoardId(), getCurrentIndex()));

	// If Cache was not written, we are ready
	if (Cache::getInstance().getState().sbrState().get(getBoardId()) != RTC::RegisterState::WRITE) {
		Cache::getInstance().getState().sbrState().unmodified(getBoardId());
		setContinue(true);
		setFinished();
		return;
	}
	//LOG_INFO(">>>> SerdesRead:Read actions active");

	SerdesBuffer&	sdBuf = Cache::getInstance().getBack().getSdsWriteBuffer();
	int				dataOffset  = (getCurrentIndex()-1)/2;	// place in the SerdesBuffer where the data is
	int				sendDataMsg = getCurrentIndex()%2;		// send MDIO data or MDIO header message
	int				sendReadHdr = getCurrentIndex()%4;		// send 'address' header or 'read' header

	// is this board in the RSPmask?
	if (!sdBuf.hasRSP(getBoardId())) {
		//LOG_INFO(">>>> SerdesRead:Wrong board");
		setContinue(true);
		return;
	}
	
	// are we already beyond the data?
	if (getCurrentIndex()/2 >= sdBuf.getDataLen()) {		// reached EOdata?
		Cache::getInstance().getState().sbrState().write_ack(getBoardId());	// mark cache ready
		//LOG_INFO(">>>> SerdesRead:Beyond data");
		setContinue(true);
		setFinished();
		SerdesBuffer&	rdBuf = Cache::getInstance().getBack().getSdsReadBuffer(getBoardId());
		string hd;
		hexdump (hd, rdBuf.getBufferPtr(), dataOffset+1);
		LOG_INFO_STR(hd);
		return;
	}

	// still data left, send it at HEADER or DATA message
	if (!sendDataMsg) {
		EPAMdioHeaderEvent	event;
		event.hdr.set(MEPHeader::SERDES_HEADER_HDR);
		event.header[0] = 0x16;
		event.header[1] = sendReadHdr ? 0x30 : 0x00;
		itsHeader = event.hdr;
		//LOG_INFO(formatString(">>>> SerdesRead:Write header:%02x %02x", event.header[0], event.header[1]));
		getBoardPort().send(event);
	}
	else {
		EPAMdioDataEvent	event;
		if (sendReadHdr == 3) {		// second data message is an EPA READ command
			event.hdr.set(MEPHeader::SERDES_DATA_HDR, MEPHeader::DST_RSP, MEPHeader::READ);
			event.data[0] = 0xDE;
			event.data[1] = 0xAD;
		}
		else {
			event.hdr.set(MEPHeader::SERDES_DATA_HDR);
			memcpy(&event.data[0], sdBuf.getBufferPtr() + dataOffset, MEPHeader::MDIO_DATA_SIZE);
		}
		itsHeader = event.hdr;
		//LOG_INFO(formatString(">>>> SerdesRead:Write data:%02x %02x", event.data[0], event.data[1]));
		getBoardPort().send(event);
	}
}

void SerdesRead::sendrequest_status()
{
	// intentionally left empty
}

GCFEvent::TResult SerdesRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
	//LOG_INFO(formatString(">>>> SerdesRead:handleack:board=%d,index=%d,signal=%d", getBoardId(), getCurrentIndex(), event.signal));
	bool	wantReadAck = (getCurrentIndex() % 4 == 3);
	if (wantReadAck) {
		if (event.signal != EPA_MDIO_DATA) {
			LOG_WARN(formatString("SerdesRead::handleack: unexpected read ack, signal = %04X", event.signal));
			return (GCFEvent::NOT_HANDLED);		// stop msg stream to this RSPboard
		}

		EPAMdioDataEvent	ack(event);
		if (!ack.hdr.isValidAck(itsHeader)) {
			Cache::getInstance().getState().sbrState().write_error(getBoardId());
			LOG_ERROR("SerdesRead::handleack: invalid read ack");
			return GCFEvent::NOT_HANDLED;
		}

		// TODO: Do something with the results.
		SerdesBuffer&	backBuf  = Cache::getInstance().getBack().getSdsReadBuffer(getBoardId());
		SerdesBuffer&	frontBuf = Cache::getInstance().getFront().getSdsReadBuffer(getBoardId());
		int				dataOffset  = (getCurrentIndex()-3)/2;	// place in the SerdesBuffer where the data is
		EPAMdioDataEvent	answer(event);
		//LOG_INFO(formatString("SerdesRead::handleack: %02x %02x", answer.data[0], answer.data[1]));
		if (dataOffset == 0) {
			backBuf.newCommand((char*)&answer.data[0], 2);
			frontBuf.newCommand((char*)&answer.data[0], 2);
		}
		else {
			backBuf.appendCommand((char*)&answer.data[0], 2);
			frontBuf.appendCommand((char*)&answer.data[0], 2);
		}

		return GCFEvent::HANDLED;
	}

	// expecting a writeack
	if (event.signal != EPA_WRITEACK) {
		LOG_WARN(formatString("SerdesRead::handleack: unexpected write ack, signal = %04X", event.signal));
		return (GCFEvent::NOT_HANDLED);		// stop msg stream to this RSPboard
	}

	EPAWriteackEvent	ack(event);
	if (!ack.hdr.isValidAck(itsHeader)) {
		Cache::getInstance().getState().sbrState().write_error(getBoardId());
		LOG_ERROR("SerdesRead::handleack: invalid write ack");
		return GCFEvent::NOT_HANDLED;
	}
	return GCFEvent::HANDLED;

}
