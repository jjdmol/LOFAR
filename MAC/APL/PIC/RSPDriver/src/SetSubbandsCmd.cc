//#  SetSubbandsCmd.cc: implementation of the SetSubbandsCmd class
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
#include "SetSubbandsCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

SetSubbandsCmd::SetSubbandsCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("SetSubbands", port, oper)
{
	m_event = new RSPSetsubbandsEvent(event);
}

SetSubbandsCmd::~SetSubbandsCmd()
{
	delete m_event;
}

void SetSubbandsCmd::ack(CacheBuffer& /*cache*/)
{
	RSPSetsubbandsackEvent ack;

	ack.timestamp = getTimestamp();
	ack.status = RSP_SUCCESS;

	getPort()->send(ack);
}

void SetSubbandsCmd::apply(CacheBuffer& cache, bool /*setModFlag*/)
{
	Range dst_range;

	switch (m_event->subbands.getType()) {

	case SubbandSelection::BEAMLET: {
		//dst_range = Range(MEPHeader::N_LOCAL_XLETS, MEPHeader::N_LOCAL_XLETS + MEPHeader::N_BEAMLETS - 1);
		dst_range = Range(MEPHeader::N_LOCAL_XLETS, MEPHeader::N_LOCAL_XLETS + m_event->subbands().extent(secondDim) - 1);
		for (int cache_rcu = 0; cache_rcu < StationSettings::instance()->nrRcus(); cache_rcu++) {
			if (m_event->rcumask[cache_rcu]) {
				cache.getSubbandSelection()()(cache_rcu, dst_range) = 0;
				cache.getSubbandSelection()()(cache_rcu, dst_range)
					= m_event->subbands()(0, Range::all()) * (int)MEPHeader::N_POL + (cache_rcu % MEPHeader::N_POL);

				LOG_DEBUG_STR("m_event->subbands() = " << m_event->subbands());
			}
		}
	}
	break;

	case SubbandSelection::XLET: {
		dst_range = Range(0, MEPHeader::N_LOCAL_XLETS - 1);
		for (int cache_rcu = 0; cache_rcu < StationSettings::instance()->nrRcus(); cache_rcu++) {
			if (m_event->rcumask[cache_rcu]) {
				cache.getSubbandSelection()()(cache_rcu, dst_range) = 0;
				cache.getSubbandSelection()()(cache_rcu, dst_range)
					= m_event->subbands()(0, 0) * MEPHeader::N_POL + (cache_rcu % MEPHeader::N_POL);

				LOG_DEBUG_STR("m_event->subbands() = " << m_event->subbands());
			}
		}
	}
	break;

	default:
		LOG_FATAL("invalid subbandselection type");
		exit(EXIT_FAILURE);
		break;
	}
}

void SetSubbandsCmd::complete(CacheBuffer& /*cache*/)
{
	LOG_INFO_STR("SetSubbandsCmd completed at time=" << getTimestamp());
}

const Timestamp& SetSubbandsCmd::getTimestamp() const
{
	return m_event->timestamp;
}

void SetSubbandsCmd::setTimestamp(const Timestamp& timestamp)
{
	m_event->timestamp = timestamp;
}

bool SetSubbandsCmd::validate() const
{
	bool valid = false;

	switch (m_event->subbands.getType()) {

	case SubbandSelection::BEAMLET:
		if (m_event->subbands().extent(secondDim) <= MEPHeader::N_BEAMLETS) valid = true;
		break;

	case SubbandSelection::XLET:
		if (1 == m_event->subbands().extent(secondDim)) valid = true;
		break;

	default:
		LOG_WARN("invalid SubbandSelection type");
		break;
	}

	// return true when everything is right
	if ((m_event->rcumask.count() <= (unsigned int)StationSettings::instance()->nrRcus())
		&& (2 == m_event->subbands().dimensions())
		&& (1 == m_event->subbands().extent(firstDim)) && valid) {
		return (true);
	}

	// show our validation values.
    LOG_DEBUG(formatString("cmd rcumask.count = %d",m_event->rcumask.count()));
    LOG_DEBUG(formatString("nr Rcus           = %d",StationSettings::instance()->nrRcus()));
    LOG_DEBUG(formatString("first dim         = %d",m_event->subbands().extent(firstDim)));
    LOG_DEBUG(formatString("second dim        = %d",m_event->subbands().extent(secondDim)));
    return (false);

}
