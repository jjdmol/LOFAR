//#  SetSDOCmd.cc: implementation of the SetSDOCmd class
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
//#  $Id: SetSDOCmd.cc 22248 2012-10-08 12:34:59Z overeem $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarBitModeInfo.h>

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "SetSDOCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

SetSDOCmd::SetSDOCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("SetSDO", port, oper)
{
	m_event = new RSPSetsdoEvent(event);
}

SetSDOCmd::~SetSDOCmd()
{
	delete m_event;
}

void SetSDOCmd::ack(CacheBuffer& /*cache*/)
{
	RSPSetsdoackEvent ack;

	ack.timestamp = getTimestamp();
	ack.status = RSP_SUCCESS;

	getPort()->send(ack);
}

void SetSDOCmd::apply(CacheBuffer& cache, bool /*setModFlag*/)
{
	#undef MIN
	#define	MIN(A,B) ((A)<(B)?(A):(B))

	Range	range;
	int nBanks = (MAX_BITS_PER_SAMPLE / cache.getSDOBitsPerSample());

    //dst_range = Range(MEPHeader::N_LOCAL_XLETS, MEPHeader::N_LOCAL_XLETS + MEPHeader::N_BEAMLETS - 1);
    range = Range(0, m_event->subbands.subbands().extent(thirdDim) - 1);
    for (int rcu = 0; rcu < StationSettings::instance()->nrRcus(); rcu++) {
        if (m_event->rcumask[rcu]) {
            int pol = rcu%2;
            for (int bank = 0; bank < nBanks; bank++) {
                cache.getSDOSelection().subbands()(rcu, bank, range) = 0;
                for (int sb = 0; sb < m_event->subbands.subbands().extent(thirdDim); sb++) {
                    cache.getSDOSelection().subbands()(rcu, bank, sb) = (m_event->subbands.subbands()(0, bank, sb) * 2) + pol;
                    Cache::getInstance().getState().sdoSelectState().reset(rcu/2);
		            Cache::getInstance().getState().sdoSelectState().write(rcu/2);
                }
            } // for each bank

            if (rcu == 0) {
                LOG_DEBUG_STR("m_event->subbands.subbands() = " << m_event->subbands.subbands());
                LOG_DEBUG_STR("cache->subbands.subbands(0) = " << cache.getSDOSelection().subbands()(0, Range::all(), Range::all()));
            }
        } // if rcu selected
    } // for each rcu
}

void SetSDOCmd::complete(CacheBuffer& /*cache*/)
{
//	LOG_INFO_STR("SetSDOCmd completed at time=" << getTimestamp());
}

const Timestamp& SetSDOCmd::getTimestamp() const
{
	return m_event->timestamp;
}

void SetSDOCmd::setTimestamp(const Timestamp& timestamp)
{
	m_event->timestamp = timestamp;
}

bool SetSDOCmd::validate() const
{
	bool valid = false;
    if (StationSettings::instance()->hasAartfaac() == false) {
        LOG_DEBUG("No AARTFAAC available");
        return (false);
    }
        
    if (   (m_event->subbands.subbands().extent(thirdDim) <= MEPHeader::N_SDO_SUBBANDS)
        && (3 == m_event->subbands.subbands().dimensions())
        && (1 == m_event->subbands.subbands().extent(firstDim))) {
        valid = true;
    }

	// return true when everything is right
	if (   (m_event->rcumask.count() <= (unsigned int)StationSettings::instance()->nrRcus())
		&& valid) {
		return (true);
	}

	// show our validation values.
    LOG_DEBUG(formatString("cmd rcumask.count    = %d",m_event->rcumask.count()));
    LOG_DEBUG(formatString("nr Rcus              = %d",StationSettings::instance()->nrRcus()));
    LOG_DEBUG(formatString("first dim subbands   = %d",m_event->subbands.subbands().extent(firstDim)));
    LOG_DEBUG(formatString("second dim subbands  = %d",m_event->subbands.subbands().extent(secondDim)));
    LOG_DEBUG(formatString("thirth dim subbands  = %d",m_event->subbands.subbands().extent(thirdDim)));
    return (false);

}
