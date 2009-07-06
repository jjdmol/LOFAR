//#  SetSplitterCmd.cc: implementation of the SetSplitterCmd class
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
#include "SetSplitterCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

// NOTE: The following the tables have the next format:
//	2 bytes that is the register addres + 2 bytes that are the values that are written to the register.
static char SERDES_OFF_CMD[] = {
	0x00, 0x00,  0x40, 0x20,	// Enable primary port
	0x00, 0xC0,  0x40, 0x20,	// Enable redundant port
	0x90, 0xD0,  0x00, 0x01,	// Recv from prim, connect RX_XAUI_P to RX_XGMII
	0x91, 0xD0,  0x02, 0x03,
	0x94, 0xD0,  0x08, 0x09,	// Transmit on prim, connect TX_XGMII to TX_XAUI_P
	0x95, 0xD0,  0x0A, 0x0B,
	0x96, 0xD0,  0x0C, 0x0D,	// MUX_PRESET
	0x97, 0xD0,  0x0E, 0x0F,	// MUX_PRESET
	0x81, 0xD0,  0x0F, 0xFF,	// ??
	0x80, 0xD0,  0x00, 0x00,	// Failover on rising edge
	0x80, 0xD0,  0x02, 0x00
};
static int SERDES_OFF_CMD_LEN = sizeof(SERDES_OFF_CMD);

static char SERDES_ON_CMD[] = {
	0x00, 0x00,  0x40, 0x20,	// Enable primary port
	0x00, 0xC0,  0x40, 0x20,	// Enable redundant port
	0x90, 0xD0,  0x04, 0x05,	// Recv from prim, connect RX_XAUI_P to RX_XGMII
	0x91, 0xD0,  0x06, 0x07,
	0x94, 0xD0,  0x08, 0x09,	// Recv from prim, connect RX_XAUI_P to RX_XGMII
	0x95, 0xD0,  0x0A, 0x0B,
	0x96, 0xD0,  0x0C, 0x0D,	// MUX_PRESET
	0x97, 0xD0,  0x0E, 0x0F,	// MUX_PRESET
	0x81, 0xD0,  0x0F, 0xFF,	// ??
	0x80, 0xD0,  0x00, 0x00,	// Failover on rising edge
	0x80, 0xD0,  0x02, 0x00
};
static int SERDES_ON_CMD_LEN = sizeof(SERDES_ON_CMD);


//
// SetSPlitterCmd(event, port, oper)
//
SetSplitterCmd::SetSplitterCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("SetSPlitter", port, oper)
{
	itsEvent = new RSPSetsplitterEvent(event);
}

//
// ~SetSPlitterCmd()
//
SetSplitterCmd::~SetSplitterCmd()
{
	delete itsEvent;
}

//
// ack(cache)
//
void SetSplitterCmd::ack(CacheBuffer& /*cache*/)
{
	// moved code to the complete function so that the response is
	// sent back after it was applied.
}

//
// apply(cache, setMod);
//
void SetSplitterCmd::apply(CacheBuffer& cache, 	bool setModFlag)
{
	SerdesBuffer&	SerdesBuf = cache.getSdsWriteBuffer();
	SerdesBuf.setRSPmask(itsEvent->rspmask);
	if (itsEvent->switch_on) {
		SerdesBuf.newCommand(SERDES_ON_CMD, SERDES_ON_CMD_LEN);
	}
	else {
		SerdesBuf.newCommand(SERDES_OFF_CMD, SERDES_OFF_CMD_LEN);
	}
	string	hd;
	hexdump(hd, SerdesBuf.getBufferPtr(), SerdesBuf.getDataLen());
	LOG_INFO_STR(hd);

	// mark registers that the serdes registers should be written.
	if (setModFlag) {
		for (int b = 0; b < MAX_N_RSPBOARDS; b++) {
			if (SerdesBuf.hasRSP(b)) {
				cache.getCache().getState().sbwState().write(b);
			}
		}
	}
}

//
// complete(cache)
//
void SetSplitterCmd::complete(CacheBuffer& /*cache*/)
{
	RSPSetsplitterackEvent ack;
	ack.timestamp = getTimestamp();
	ack.status = RSP_SUCCESS;
	getPort()->send(ack);
}

//
// getTimestamp()
//
const Timestamp& SetSplitterCmd::getTimestamp() const
{
	return itsEvent->timestamp;
}

//
// setTimestamp(timestamp)
//
void SetSplitterCmd::setTimestamp(const Timestamp& timestamp)
{
	itsEvent->timestamp = timestamp;
}

//
// validate()
//
bool SetSplitterCmd::validate() const
{
	// return true when everything is right
	if (itsEvent->rspmask.count() <= (unsigned int)StationSettings::instance()->nrRspBoards()) {
		return (true);
	}

	// show our validation values.
    LOG_ERROR(formatString("cmd rspmask.count = %d",itsEvent->rspmask.count()));
    return (false);
}
