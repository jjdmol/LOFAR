//#  SetBitModeCmd.cc: implementation of the SetBitModeCmd class
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
//#  $Id: SetBitModeCmd.cc 18124 2011-05-29 19:54:09Z donker $

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "SetBitModeCmd.h"
#include "Sequencer.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

SetBitModeCmd::SetBitModeCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("SetBitMode", port, oper)
{
  itsEvent = new RSPSetbitmodeEvent(event);
}

SetBitModeCmd::~SetBitModeCmd()
{
  delete itsEvent;
}

void SetBitModeCmd::ack(CacheBuffer& /*cache*/)
{
  RSPSetbitmodeackEvent ack;

  ack.timestamp = getTimestamp();
  ack.status    = RSP_SUCCESS;
  
  getPort()->send(ack);
}

void SetBitModeCmd::apply(CacheBuffer& cache, bool setModFlag)
{
  cache.setBitMode(itsEvent->bit_mode);
  LOG_INFO_STR(formatString("Setting bitmode to %d bits @ ", itsEvent->bit_mode) << getTimestamp());

  if (setModFlag) {
    for (int b = 0; b < StationSettings::instance()->nrRspBoards(); b++) {
		cache.getCache().getState().sys().write(2*b); //TODO
	}
  }
}

void SetBitModeCmd::complete(CacheBuffer& /*cache*/)
{
}

const Timestamp& SetBitModeCmd::getTimestamp() const
{
  return itsEvent->timestamp;
}

void SetBitModeCmd::setTimestamp(const Timestamp& timestamp)
{
  itsEvent->timestamp = timestamp;
}

bool SetBitModeCmd::validate() const
{
  return (16 == itsEvent->bit_mode ||
          8  == itsEvent->bit_mode ||
          4  == itsEvent->bit_mode);
}
