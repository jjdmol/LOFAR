//#  WriteReg.cc: implementation of the WriteReg class
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
#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RSP_Protocol/RSP_Protocol.ph>

#include <APL/RTCCommon/PSAccess.h>

#include <unistd.h>
#include <string.h>
#include <blitz/array.h>

#include "WriteReg.h"
#include "Cache.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;
using namespace RTC;

WriteReg::WriteReg(GCFPortInterface& board_port, int board_id,
		   uint16 dstid, uint8 pid, uint8 regid, uint16 size,
		   uint16 offset)
  : SyncAction(board_port, board_id, 1),
    m_dstid(dstid), m_pid(pid), m_regid(regid), m_size(size), m_offset(offset),
    m_source_address(0)
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

WriteReg::~WriteReg()
{
}

void WriteReg::setSrcAddress(void* address)
{
  m_source_address = address;
}

void WriteReg::sendrequest()
{
  EPAWriteEvent write;
  
  write.hdr.set(MEPHeader::WRITE,
		m_dstid,
		m_pid,
		m_regid,
		m_size,
		m_offset);

  write.payload.setBuffer(m_source_address, m_size);
  m_hdr = write.hdr;

  getBoardPort().send(write);
}

void WriteReg::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult WriteReg::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_WRITEACK != event.signal)
  {
    LOG_WARN("WriteReg::handleack: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }

  EPAWriteackEvent ack(event);

  if (!ack.hdr.isValidAck(m_hdr))
  {
    LOG_ERROR("WriteReg::handleack: invalid ack");
    return GCFEvent::NOT_HANDLED;
  }

  //
  // Sleep 73msec after receiving SYNC WRITE_ACK
  //
  usleep(73000);
 
  return GCFEvent::HANDLED;
}


