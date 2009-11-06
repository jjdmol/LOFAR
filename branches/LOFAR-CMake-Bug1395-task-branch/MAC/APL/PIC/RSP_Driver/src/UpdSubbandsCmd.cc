//#  UpdSubbandsCmd.cc: implementation of the UpdSubbandsCmd class
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

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "UpdSubbandsCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

//
// UpdSubbandsCmd(event, port, oper)
//
UpdSubbandsCmd::UpdSubbandsCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("SubSubbands", port, oper)
{
	// convert event to SubSubbands event
	m_event = new RSPSubsubbandsEvent(event);

	setPeriod(m_event->period);
}

//
// ~UpdSubbandsCmd()
//
UpdSubbandsCmd::~UpdSubbandsCmd()
{
	// delete our own event again
	delete m_event;
}

//
// ack(cache)
//
void UpdSubbandsCmd::ack(CacheBuffer& /*cache*/)
{
	// intentionally left empty
}

//
// apply(cache)
//
void UpdSubbandsCmd::apply(CacheBuffer& /*cache*/, bool /*setModFlag*/)
{
	// no-op
}

//
// complete(cache)
//
void UpdSubbandsCmd::complete(CacheBuffer& cache)
{
	// construct ack message
	RSPUpdsubbandsEvent ack;

	ack.timestamp = getTimestamp();
	ack.status 	  = RSP_SUCCESS;
	ack.handle 	  = (memptr_t)this; // opaque ptr used to refer to the subscr.

	// Allocate room in subbands array
	// Note: XLETS are allocated at the first 8 registers in the subbands
	// area. The beamlets are located behind it.
	Range src_range;
	switch (m_event->type) {
	case SubbandSelection::BEAMLET:
		ack.subbands().resize(m_event->rcumask.count(), MEPHeader::N_BEAMLETS);
		src_range = Range(MEPHeader::N_LOCAL_XLETS, MEPHeader::N_LOCAL_XLETS + MEPHeader::N_BEAMLETS - 1);
		break;

	case SubbandSelection::XLET:
		ack.subbands().resize(m_event->rcumask.count(), MEPHeader::N_LOCAL_XLETS);
		src_range = Range(0, MEPHeader::N_LOCAL_XLETS - 1);
		break;

	default:
		LOG_FATAL_STR("invalid subbandselection type " << m_event->type);
		exit(EXIT_FAILURE);
		break;
	}

	// loop over RCU's to get the results.
	int result_rcu = 0;
	for (int cache_rcu = 0; cache_rcu < StationSettings::instance()->nrRcus(); cache_rcu++) {

		if (m_event->rcumask[cache_rcu]) {
			ack.subbands()(result_rcu, Range::all()) = 
					cache.getSubbandSelection()()(cache_rcu, src_range);
			result_rcu++;
   		}
	}

	// Finally send the answer
	getPort()->send(ack);
}

//
// getTimestamp()
//
const Timestamp& UpdSubbandsCmd::getTimestamp() const
{
	return m_event->timestamp;
}

//
// setTimestamp(timestamp)
//
void UpdSubbandsCmd::setTimestamp(const Timestamp& timestamp)
{
	m_event->timestamp = timestamp;
}

//
// validate()
//
bool UpdSubbandsCmd::validate() const
{
	// check ranges and type
	return ((m_event->rcumask.count() <= (unsigned int)StationSettings::instance()->nrRcus()) 
		 && (m_event->type == SubbandSelection::BEAMLET || 
			 m_event->type == SubbandSelection::XLET));
}
