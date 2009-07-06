//#  SetRSUCmd.cc: implementation of the SetRSUCmd class
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
#include "SetRSUCmd.h"
#include "Sequencer.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

SetRSUCmd::SetRSUCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("SetRSU", port, oper)
{
  m_event = new RSPSetrsuEvent(event);

  LOG_INFO(formatString("RSUcontrol=0x%02x", m_event->settings()(0).getRaw()));
}

SetRSUCmd::~SetRSUCmd()
{
  delete m_event;
}

void SetRSUCmd::ack(CacheBuffer& /*cache*/)
{
  RSPSetrsuackEvent ack;

  ack.timestamp = getTimestamp();
  ack.status = RSP_SUCCESS;

  getPort()->send(ack);
}

void SetRSUCmd::apply(CacheBuffer& cache, bool setModFlag)
{
  for (int cache_rsp = 0; cache_rsp < StationSettings::instance()->nrRspBoards(); cache_rsp++) { 
    if (m_event->rspmask[cache_rsp]) {
      LOG_INFO (formatString("RSUcontrol for board %d", cache_rsp));
      cache.getRSUSettings()()(cache_rsp) = m_event->settings()(0);
    }
  }

  if (setModFlag) {
    Sequencer::getInstance().startSequence(Sequencer::RSPCLEAR);
  }
}

void SetRSUCmd::complete(CacheBuffer& /*cache*/)
{
//  LOG_INFO_STR("SetRSUCmd completed at time=" << getTimestamp());
}

const Timestamp& SetRSUCmd::getTimestamp() const
{
  return (m_event->timestamp);
}

void SetRSUCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool SetRSUCmd::validate() const
{
  return ((m_event->rspmask.count() <= (unsigned int)StationSettings::instance()->nrRspBoards())
	  && (1 == m_event->settings().dimensions())
	  && (1 == m_event->settings().extent(firstDim)));
}
