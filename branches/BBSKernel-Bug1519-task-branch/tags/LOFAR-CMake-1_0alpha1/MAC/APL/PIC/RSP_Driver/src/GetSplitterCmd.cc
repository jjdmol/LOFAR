//#  GetSplitterCmd.cc: implementation of the GetSplitterCmd class
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
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "GetSplitterCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

// NOTE: The foolowing three table should match with each other.
//	The first table contains the registers we want the read back,
//	the second and third contain the expected values of these registers.
static char ASK_SERDES_CMD[] = {
//	0x00, 0x00,
//	0x00, 0xC0,
	0x90, 0xD0,
	0x91, 0xD0,
	0x94, 0xD0,
	0x95, 0xD0,
	0x96, 0xD0,
	0x97, 0xD0,
//	0x80, 0xD0,
};
static int ASK_SERDES_CMD_LEN = sizeof(ASK_SERDES_CMD);

static char SERDES_ON_RESP[] = {
	0x04, 0x05,
	0x06, 0x07,
	0x08, 0x09,
	0x0A, 0x0B,
	0x0C, 0x0D,
	0x0E, 0x0F
};
static int SERDES_ON_RESP_LEN = sizeof(SERDES_ON_RESP);

static char SERDES_OFF_RESP[] = {
	0x00, 0x01,
	0x02, 0x03,
	0x08, 0x09,
	0x0A, 0x0B,
	0x0C, 0x0D,
	0x0E, 0x0F
};
static int SERDES_OFF_RESP_LEN = sizeof(SERDES_OFF_RESP);


//
// GetSplitterCmd(event, port, oper)
//
GetSplitterCmd::GetSplitterCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("GetSplitter", port, oper)
{
	itsEvent = new RSPGetsplitterEvent(event);
}

//
// ~GetSplitterCmd()
//
GetSplitterCmd::~GetSplitterCmd()
{
	delete itsEvent;
}

//
// ack(cache)
//
void GetSplitterCmd::ack(CacheBuffer& cache)
{
	RSPGetsplitterackEvent ack;
	ack.timestamp = getTimestamp();
	ack.status = RSP_SUCCESS;
	ack.rspmask = itsEvent->rspmask;
	ack.splitter.reset();

	// note always return the state of all splitters
	for (int rsp = 0; rsp < MAX_N_RSPBOARDS; rsp++) {
		if (itsEvent->rspmask.test(rsp)) {
			SerdesBuffer&	serdesBuf = cache.getSdsReadBuffer(rsp);
			if (!memcmp(serdesBuf.getBufferPtr(), SERDES_ON_RESP, SERDES_ON_RESP_LEN)) {	// ON?
				ack.splitter.set(rsp);
			}
			else if (memcmp(serdesBuf.getBufferPtr(), SERDES_OFF_RESP, SERDES_OFF_RESP_LEN)) {	// !OFF?
				LOG_ERROR_STR("Serdes splitter from RSPboard " << rsp  << "is in an undefined state!");
				string	hd;
				hexdump(hd, serdesBuf.getBufferPtr(), SERDES_OFF_RESP_LEN);
				LOG_INFO_STR("Serdes splitter responses from RSPboard " << rsp << ": " << hd);
			}
		}
	}

	getPort()->send(ack);
}

//
// apply(cache, setMod);
//
void GetSplitterCmd::apply(CacheBuffer& cache, 	bool setModFlag)
{
	// Note: apply the data to the WRITE buffer since we must write to the Serdes chip.
	//       the sdsReadBuffer if for storing the results.
	SerdesBuffer&	SerdesBuf = cache.getSdsWriteBuffer();
	SerdesBuf.setRSPmask(itsEvent->rspmask);
	SerdesBuf.newCommand(ASK_SERDES_CMD, ASK_SERDES_CMD_LEN);
	string	hd;
	hexdump(hd, SerdesBuf.getBufferPtr(), SerdesBuf.getDataLen());
	LOG_INFO_STR(hd);

	// mark registers that the serdes registers should be written.
	if (setModFlag) {
		for (int b= 0; b < MAX_N_RSPBOARDS; b++) {
			if (SerdesBuf.hasRSP(b)) {
				cache.getCache().getState().sbrState().write(b);
			}
		}
	}
}

//
// complete(cache)
//
void GetSplitterCmd::complete(CacheBuffer& cache)
{
	ack(cache);
}

//
// getTimestamp()
//
const Timestamp& GetSplitterCmd::getTimestamp() const
{
	return itsEvent->timestamp;
}

//
// setTimestamp(timestamp)
//
void GetSplitterCmd::setTimestamp(const Timestamp& timestamp)
{
	itsEvent->timestamp = timestamp;
}

//
// validate()
//
bool GetSplitterCmd::validate() const
{
	// return true when everything is right
	if (itsEvent->rspmask.count() <= (unsigned int)StationSettings::instance()->nrRspBoards()) {
		return (true);
	}

	// show our validation values.
    LOG_ERROR(formatString("cmd rspmask.count = %d",itsEvent->rspmask.count()));
    return (false);
}
