//#  SetSwapxyCmd.cc: implementation of the SetSwapxyCmd class
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
//#  $Id: SetSwapxyCmd.cc 15023 2010-02-18 15:24:31Z donker $

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "SetSwapxyCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

SetSwapXYCmd::SetSwapXYCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("SetSwapxy", port, oper)
{
  m_event = new RSPSetswapxyEvent(event);
}

SetSwapXYCmd::~SetSwapXYCmd()
{
  delete m_event;
}

void SetSwapXYCmd::ack(CacheBuffer& cache)
{
  complete(cache);
  // moved code to the complete function so that the response is
	// sent back after it was applied.
}

void SetSwapXYCmd::apply(CacheBuffer& cache, bool /*setModFlag*/)
{
    bitset<MAX_ANTENNAS> swappedxy = cache.getSwappedXY();
    
    for (int ant = 0; ant < MAX_ANTENNAS; ant++) {
        if (m_event->antennamask.test(ant)) {
            if (m_event->swapxy) {
                swappedxy.set(ant);
            }
            else {
                swappedxy.reset(ant);
            }
        }
    }
    cache.setSwappedXY(swappedxy);
}

void SetSwapXYCmd::complete(CacheBuffer& /*cache*/)
{
  RSPSetswapxyackEvent ack;

  ack.timestamp = getTimestamp();
  ack.status = RSP_SUCCESS;
  getPort()->send(ack);
}

const Timestamp& SetSwapXYCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void SetSwapXYCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool SetSwapXYCmd::validate() const
{
  return true;
}
