//#  SetSDOModeCmd.cc: implementation of the SetSDOModeCmd class
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
//#  $Id: SetSDOModeCmd.cc 18124 2011-05-29 19:54:09Z donker $

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "SetSDOModeCmd.h"
#include "Sequencer.h"
#include "Cache.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

SetSDOModeCmd::SetSDOModeCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("SetSDOMode", port, oper)
{
  itsEvent = new RSPSetsdomodeEvent(event);
}

SetSDOModeCmd::~SetSDOModeCmd()
{
  delete itsEvent;
}

void SetSDOModeCmd::ack(CacheBuffer& /*cache*/)
{
  RSPSetsdomodeackEvent ack;

  ack.timestamp = getTimestamp();
  ack.status    = RSP_SUCCESS;
  
  getPort()->send(ack);
}

void SetSDOModeCmd::apply(CacheBuffer& cache, bool setModFlag)
{
    int select;
    
    LOG_INFO_STR(formatString("Setting sdomode to %d bits @ ", itsEvent->bits_per_sample) << getTimestamp());
    
    switch (itsEvent->bits_per_sample) {
        case 16:
            select = 0;
            break;
        case 8:
            select = 1;
            break;
        case 5:
            select = 2;
            break;
        case 4:
            select = 3;
            break;    
        default: 
            return;
            break;
    }
    
    cache.setSDOBitsPerSample(itsEvent->bits_per_sample);
        
    for (int i = 0; i < StationSettings::instance()->nrRspBoards(); ++i) {
        if (itsEvent->rspmask.test(i)) {
            cache.getSDOModeInfo()()(i).bm_select = (uint8)select;
            if (setModFlag) {
            	cache.getCache().getState().sdoState().write(i);
            	cache.getCache().getState().cdo().write(2*i);
    			cache.getCache().getState().cdo().write(2*i+1);
            }
        }
    }
}

void SetSDOModeCmd::complete(CacheBuffer& /*cache*/)
{
}

const Timestamp& SetSDOModeCmd::getTimestamp() const
{
  return itsEvent->timestamp;
}

void SetSDOModeCmd::setTimestamp(const Timestamp& timestamp)
{
  itsEvent->timestamp = timestamp;
}

bool SetSDOModeCmd::validate() const
{
    if (StationSettings::instance()->hasAartfaac() == false) {
        LOG_DEBUG("No AARTFAAC available");
        return (false);
    }
    
    // check if BP version of all boards >= 9.3
    for (int i = 0; i < StationSettings::instance()->nrRspBoards(); ++i) {
        if (((Cache::getInstance().getBack().getVersions().bp()(i).fpga_maj * 10) + 
              Cache::getInstance().getBack().getVersions().bp()(i).fpga_min) < 93) {
            LOG_WARN_STR(formatString("Wrong firmware version on board[%d], NO sdomode support", i));
            return(false);
        }
    }
    return (16 == itsEvent->bits_per_sample ||
            8  == itsEvent->bits_per_sample ||
            5  == itsEvent->bits_per_sample ||
            4  == itsEvent->bits_per_sample);
}
