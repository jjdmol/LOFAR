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

#include "WriteReg.h"
#include "EPA_Protocol.ph"
#include "RSP_Protocol.ph"
#include "Cache.h"

#include <PSAccess.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <unistd.h>
#include <string.h>
#include <blitz/array.h>

#define N_RETRIES 3

using namespace RSP;
using namespace LOFAR;
using namespace EPA_Protocol;
using namespace blitz;

WriteReg::WriteReg(GCFPortInterface& board_port, int board_id,
		   uint8 dstid, uint8 pid, uint8 regid, uint16 size,
		   uint16 offset)
  : SyncAction(board_port, board_id, GET_CONFIG("RS.N_BLPS", i)),
    m_dstid(dstid), m_pid(pid), m_regid(regid), m_size(size), m_offset(offset)
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
  if (MEPHeader::DST_RSP == getCurrentBLP())
  {
    LOG_INFO(formatString(">>>> WriteReg(%s) RSP, pid=%d, regid=%d, size=%d, offset=%d",
			  getBoardPort().getName().c_str(),
			  m_pid,
			  m_regid,
			  m_size,
			  m_offset));
  }
  else
  {
    uint8 global_blp = (getBoardId() * GET_CONFIG("RS.N_BLPS", i)) + getCurrentBLP();

    LOG_INFO(formatString(">>>> WriteReg(%s) BLP=%d, pid=%d, regid=%d, size=%d, offset=%d",
			  getBoardPort().getName().c_str(),
			  global_blp,
			  m_pid,
			  m_regid,
			  m_size,
			  m_offset));
  }
  
  EPAWriteEvent write;
  
  write.hdr.set(MEPHeader::WRITE,
		(m_dstid & MEPHeader::DST_RSP
		 ? MEPHeader::DST_RSP
		 : getCurrentBLP()),
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
 
  return GCFEvent::HANDLED;
}


