//#  GetVersions.cc: implementation of the GetVersions class
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

#include <APL/TBB_Protocol/TBB_Protocol.ph>
#include <APL/TBB_Protocol/TP_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "GetVersions.h"

using namespace blitz;
using namespace LOFAR;
using namespace TBB;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using namespace RTC;

GetVersions::GetVersions(GCFEvent& event, GCFPortInterface& port)
	: BoardAction(board_port, board_id, StationSettings::instance()->nrBlpsPerBoard() + 1 /* BP */)
{
	// TODO
}

GetVersions::~GetVersionsCmd()
{
	// TODO
}


void GetVersions::tp_buildrequest()
{
	tp_event.opcode = VERSION;
}

void GetVersions::tp_sendrequest(GCFPortInterface& port)
{
	port.send(tp_event);
}


GCFEvent::TResult GetVersions::tp_handleack(GCFEvent& event, GCFPortInterface& port)
{
	if (TP_VERSION != event.signal)
	{
		LOG_WARN("GetVersions::handleack: unexpected ack");
		return GCFEvent::NOT_HANDLED;
	}
  
	EPARsrVersionEvent ack(event);

	if (!ack.hdr.isValidAck(m_hdr))
	{
		LOG_ERROR("VersionsRead::handleack: invalid ack");
		return GCFEvent::NOT_HANDLED;
	}

	if (MEPHeader::DST_RSP == ack.hdr.m_fields.addr.dstid) {

		Cache::getInstance().getBack().getVersions().bp()(getBoardId()) = ack.version;

	} else {

		int ap_index = -1;
		switch (ack.hdr.m_fields.addr.dstid) {
			case MEPHeader::DST_BLP0:
				ap_index = 0;
				break;
			case MEPHeader::DST_BLP1:
				ap_index = 1;
				break;
			case MEPHeader::DST_BLP2:
				ap_index = 2;
				break;
			case MEPHeader::DST_BLP3:
				ap_index = 3;
				break;
			default:
				LOG_WARN("Invalid BLP in EPARsrVersionEvent");
				break;
		}

		if (ap_index >= 0) {
			Cache::getInstance().getBack().getVersions().ap()((getBoardId() * StationSettings::instance()->nrBlpsPerBoard()) + ap_index)
					= ack.version;  
		}
	}
	return GCFEvent::HANDLED;
}



void GetVersions::tbb_sendack()
{
	tbb_ackevent.status = SUCCESS;
	
	tbb_ackevent.tbbversion[tbbnr].boardid = ;
	tbb_ackevent.tbbversion[tbbnr].swversion = ;
	
	getPort()->send(tbb_ackevent);
}

void GetVersionsCmd::apply(CacheBuffer& /*cache*/, bool /*setModFlag*/)
{
  // intentionally left empty 
}

void GetVersionsCmd::complete(CacheBuffer& cache)
{
	ack(cache);
}

bool GetVersionsCmd::validate() const
{
	return true;
}

