//#  GetBitModeCmd.cc: implementation of the GetBitModeCmd class
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
//#  $Id: GetBitModeCmd.cc 13440 2009-06-22 13:26:54Z donker $

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <APL/RSP_Protocol/RSP_Protocol.ph>

#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "GetBitModeCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

GetBitModeCmd::GetBitModeCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("GetBitMode", port, oper)
{
    itsEvent = new RSPGetbitmodeEvent(event);
}

GetBitModeCmd::~GetBitModeCmd()
{
    delete itsEvent;
}

void GetBitModeCmd::ack(CacheBuffer& cache)
{
    RSPGetbitmodeackEvent ack;

    ack.timestamp = getTimestamp();
    ack.status = RSP_SUCCESS;
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

void GetBitModeCmd::apply(CacheBuffer& /*cache*/, bool /*setModFlag*/)
{
    /* intentionally left empty */
}

void GetBitModeCmd::complete(CacheBuffer& cache)
{
    ack(cache);
}

const RTC::Timestamp& GetBitModeCmd::getTimestamp() const
{
    return itsEvent->timestamp;
}

void GetBitModeCmd::setTimestamp(const RTC::Timestamp& timestamp)
{
    itsEvent->timestamp = timestamp;
}

bool GetBitModeCmd::validate() const
{
    return (true);
}

bool GetBitModeCmd::readFromCache() const
{
    return itsEvent->cache;
}
