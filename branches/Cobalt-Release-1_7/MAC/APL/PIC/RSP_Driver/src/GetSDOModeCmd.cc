//#  GetSDOModeCmd.cc: implementation of the GetSDOModeCmd class
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
//#  $Id: GetSDOModeCmd.cc 13440 2009-06-22 13:26:54Z donker $

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <APL/RSP_Protocol/RSP_Protocol.ph>

#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "GetSDOModeCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

GetSDOModeCmd::GetSDOModeCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("GetSDOMode", port, oper)
{
    itsEvent = new RSPGetsdomodeEvent(event);
}

GetSDOModeCmd::~GetSDOModeCmd()
{
    delete itsEvent;
}

void GetSDOModeCmd::ack(CacheBuffer& cache)
{
    RSPGetsdomodeackEvent ack;

    ack.timestamp = getTimestamp();
    ack.status = RSP_SUCCESS;
    for (int i = 0; i < StationSettings::instance()->nrRspBoards(); ++i) {
        ack.bitmode_version[i] = cache.getSDOModeInfo()()(i).bm_max;
        
        uint8 select = cache.getSDOModeInfo()()(i).bm_select;
        if (select == 0) { 
            ack.bits_per_sample[i] = 16;
        }
        else if (select == 1) {
            ack.bits_per_sample[i] = 8;
        }
        else if (select == 2) {
            ack.bits_per_sample[i] = 5;
        }
        else if (select == 3) {
            ack.bits_per_sample[i] = 4;
        }
        else {
            LOG_WARN_STR("unknown sdo mode (" << select << ") for rsp " << i);
        }
    }
    getPort()->send(ack);
}

void GetSDOModeCmd::apply(CacheBuffer& /*cache*/, bool /*setModFlag*/)
{
    /* intentionally left empty */
}

void GetSDOModeCmd::complete(CacheBuffer& cache)
{
    ack(cache);
}

const RTC::Timestamp& GetSDOModeCmd::getTimestamp() const
{
    return itsEvent->timestamp;
}

void GetSDOModeCmd::setTimestamp(const RTC::Timestamp& timestamp)
{
    itsEvent->timestamp = timestamp;
}

bool GetSDOModeCmd::validate() const
{
    return (true);
}

bool GetSDOModeCmd::readFromCache() const
{
    return itsEvent->cache;
}
