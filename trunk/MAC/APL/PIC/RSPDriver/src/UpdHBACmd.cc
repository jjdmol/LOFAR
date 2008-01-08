//#  UpdHBACmd.cc: implementation of the UpdHBACmd class
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
#include "UpdHBACmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

//
// UpdHBACmd(event, port, oper)
//
UpdHBACmd::UpdHBACmd(GCFEvent& event, GCFPortInterface& port, Operation oper)
{
  // Constructor of UpdHBACmd is only called on a SubHBAEvent
  // convert event to Subhba event
  m_event = new RSPSubhbaEvent(event);

  setOperation(oper);
  setPeriod(m_event->period);
  setPort(port);
}

//
// ~UpdHBACmd()
//
UpdHBACmd::~UpdHBACmd()
{
  // delete our own event again
  delete m_event;
}

//
// ack(cache)
//
void UpdHBACmd::ack(CacheBuffer& /*cache*/)
{
  // intentionally left empty
}

//
// apply(cache)
//
void UpdHBACmd::apply(CacheBuffer& /*cache*/, bool /*setModFlag*/)
{
  // no-op
}

//
// complete(cache)
//
void UpdHBACmd::complete(CacheBuffer& cache)
{
  // construct ack message
  RSPUpdhbaEvent ack;

  ack.timestamp = getTimestamp();
  ack.status 	  = SUCCESS;
  ack.handle 	  = (memptr_t)this; // opaque ptr used to refer to the subscr.

  // Allocate room in subbands array
  ack.settings().resize(m_event->rcumask.count(), MEPHeader::N_HBA_DELAYS);

  // loop over RCU's to get the results.
  int result_rcu = 0;
  for (int cache_rcu = 0; cache_rcu < StationSettings::instance()->nrRcus(); cache_rcu++) {

    if (m_event->rcumask[cache_rcu]) {
      if (cache_rcu < StationSettings::instance()->nrRcus()) {
	ack.settings()(result_rcu, Range::all()) = cache.getHBASettings()()(cache_rcu, Range::all());
      }
      else {
	LOG_WARN(
		 formatString("invalid RCU index %d, there are only %d RCU's",
			      cache_rcu, StationSettings::instance()->nrRcus())); 
      }

      result_rcu++;
    }
  }

  // Finally send the answer
  getPort()->send(ack);
}

//
// getTimestamp()
//
const Timestamp& UpdHBACmd::getTimestamp() const
{
  return m_event->timestamp;
}

//
// setTimestamp(timestamp)
//
void UpdHBACmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

//
// validate()
//
bool UpdHBACmd::validate() const
{
  // check ranges
  return ((m_event->rcumask.count() <= 
	   (unsigned int)StationSettings::instance()->nrRcus()));
}
