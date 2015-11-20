//#  BypassWriteBP.cc: implementation of the BypassWriteBP class
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
//#  $Id: BypassWriteBP.cc 32819 2015-11-06 11:31:35Z donker $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RSP_Protocol/RSP_Protocol.ph>

#include <APL/RTCCommon/PSAccess.h>

#include <unistd.h>
#include <string.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "BypassWriteBP.h"
#include "Cache.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;
using namespace RTC;


BypassWriteBP::BypassWriteBP(GCFPortInterface& board_port, int board_id)
    : SyncAction(board_port, board_id, 1)
{
    memset(&m_hdr, 0, sizeof(MEPHeader));
}

BypassWriteBP::~BypassWriteBP()
{
}

void BypassWriteBP::sendrequest()
{
    EPADiagBypassEvent request;
    // cache modified, or initialising
    if ((Cache::getInstance().getState().bypasssettings_bp().get(getBoardId()) != RTC::RegisterState::WRITE) ||
        (Cache::getInstance().getBack().getBypassSettingsBP()()(getBoardId()).isSDOset() == false)) {
        Cache::getInstance().getState().bypasssettings_bp().unmodified(getBoardId());
        setContinue(true);
        return;
    }
    
    if (Cache::getInstance().getBack().getBypassSettingsBP()()(getBoardId()).isSDOset()) {
        Cache::getInstance().getBack().getBypassSettingsBP()()(getBoardId()).resetSDOset();
    }
    request.hdr.set(MEPHeader::DIAG_BYPASS_HDR, MEPHeader::DST_RSP, MEPHeader::WRITE);
    
    // read values from cache
    BypassSettings& s = Cache::getInstance().getBack().getBypassSettingsBP(); // whole array
    request.bypass = s()(getBoardId()).getRaw();        // one element

    m_hdr = request.hdr;
    LOG_DEBUG(formatString("BypassWriteBP: sendrequest: sending it(%04X)", request.bypass));
    getBoardPort().send(request);
}

void BypassWriteBP::sendrequest_status()
{
// intentionally left empty
}

GCFEvent::TResult BypassWriteBP::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
    if (event.signal != EPA_WRITEACK) {
        LOG_WARN("BypassWriteBP::handleack: unexpected ack");
        return GCFEvent::NOT_HANDLED;
    }

    EPAWriteackEvent ack(event);

    if (!ack.hdr.isValidAck(m_hdr)) {
        LOG_ERROR("BypassWriteBP::handleack: invalid ack");
        Cache::getInstance().getState().bypasssettings_bp().write_error(getBoardId());
        return GCFEvent::NOT_HANDLED;
    }
    LOG_DEBUG_STR("BypassWriteBP: handleack: board= " << getBoardId());
    Cache::getInstance().getState().bypasssettings_bp().write_ack(getBoardId()); // mark finished
    return GCFEvent::HANDLED;
}


