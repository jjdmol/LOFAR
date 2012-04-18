//#  SetDatastreamCmd.cc: implementation of the SetDatastreamCmd class
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
//#  $Id: SetDatastreamCmd.cc 14660 2009-12-10 13:33:18Z overeem $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/hexdump.h>

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "SetDatastreamCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;


//
// SetSPlitterCmd(event, port, oper)
//
SetDatastreamCmd::SetDatastreamCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("SetDatastream", port, oper)
{
	itsEvent = new RSPSetdatastreamEvent(event);
}

//
// ~SetSPlitterCmd()
//
SetDatastreamCmd::~SetDatastreamCmd()
{
	delete itsEvent;
}

//
// ack(cache)
//
void SetDatastreamCmd::ack(CacheBuffer& cache)
{
	complete(cache);
	// moved code to the complete function so that the response is
	// sent back after it was applied.
}

//
// apply(cache, setMod);
//
void SetDatastreamCmd::apply(CacheBuffer& cache, 	bool setModFlag)
{
	if (itsEvent->switch_on) {
		cache.setCepEnabled(true); // enable datastream to cep
	}
	else {
		cache.setCepEnabled(false); // disable datastream to cep
	}

	// mark registers that the serdes registers should be written.
	if (setModFlag) {
		for (int b = 0; b < StationSettings::instance()->nrRspBoards(); b++) {
			cache.getCache().getState().cdo().write(2*b);
			cache.getCache().getState().cdo().write(2*b+1);
		}
	}
}

//
// complete(cache)
//
void SetDatastreamCmd::complete(CacheBuffer& /*cache*/)
{
	RSPSetdatastreamackEvent ack;
	ack.timestamp = getTimestamp();
	ack.status = RSP_SUCCESS;
	getPort()->send(ack);
}

//
// getTimestamp()
//
const Timestamp& SetDatastreamCmd::getTimestamp() const
{
	return itsEvent->timestamp;
}

//
// setTimestamp(timestamp)
//
void SetDatastreamCmd::setTimestamp(const Timestamp& timestamp)
{
	itsEvent->timestamp = timestamp;
}

//
// validate()
//
bool SetDatastreamCmd::validate() const
{
	return (true);
}
