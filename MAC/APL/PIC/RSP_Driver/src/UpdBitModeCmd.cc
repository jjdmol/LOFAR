//#  UpdBitModeCmd.cc: implementation of the UpdBitModeCmd class
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
//#  $Id: UpdBitModeCmd.cc 13440 2009-06-22 13:26:54Z overeem $

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "UpdBitModeCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

UpdBitModeCmd::UpdBitModeCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("SubBitMode", port, oper),
	itsEvent(0), 
	itsCurrentBitsPerSample(0)
{
  itsEvent = new RSPSubbitmodeEvent(event);

  setPeriod(itsEvent->period);
}

UpdBitModeCmd::~UpdBitModeCmd()
{
  delete itsEvent;
}

void UpdBitModeCmd::ack(CacheBuffer& /*cache*/)
{
  // intentionally left empty
}

void UpdBitModeCmd::apply(CacheBuffer& /*cache*/, bool /*setModFlag*/)
{
  // no-op
}

void UpdBitModeCmd::complete(CacheBuffer& cache)
{
  if (cache.getBitsPerSample() != itsCurrentBitsPerSample) {

    RSPUpdbitmodeEvent ack;
    
    ack.timestamp = getTimestamp();
    ack.status = RSP_SUCCESS;
    ack.handle = (memptr_t)this; // opaque pointer used to refer to the subscription
    
    for (int i = 0; i < StationSettings::instance()->nrRspBoards(); ++i) {
        ack.bitmode_version[i] = cache.getBitModeInfo()()(i).bm_max;
        
        uint8 select = cache.getBitModeInfo()()(i).bm_select;
        if (select == 0) { 
            ack.bits_per_sample[i] = 16;
        }
        else if (select == 1) {
            ack.bits_per_sample[i] = 8;
        }
        else if (select == 2) {
            ack.bits_per_sample[i] = 4;
        }
    }
    getPort()->send(ack);
  }

  itsCurrentBitsPerSample = cache.getBitsPerSample();
}

const Timestamp& UpdBitModeCmd::getTimestamp() const
{
  return itsEvent->timestamp;
}

void UpdBitModeCmd::setTimestamp(const Timestamp& timestamp)
{
  itsEvent->timestamp = timestamp;
}

bool UpdBitModeCmd::validate() const
{
  return (true);
}
