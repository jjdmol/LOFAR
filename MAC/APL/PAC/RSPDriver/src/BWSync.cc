//#  BWSync.cc: implementation of the BWSync class
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

#include "BWSync.h"
#include "EPA_Protocol.ph"

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <unistd.h>
//#include <stdlib.h>

using namespace RSP;
using namespace LOFAR;
using namespace EPA_Protocol;

#define N_RETRIES 3

BWSync::BWSync(GCFPortInterface& board_port, int board_id)
  : SyncAction((State)&BWSync::handler, board_port, board_id),
    m_current_blp(0),
    m_retries(0)
{
}

BWSync::~BWSync()
{
}

GCFEvent::TResult BWSync::handler(GCFEvent& event, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  LOG_INFO("BWSync::initial_state");

  switch (event.signal)
  {
    case F_TIMER:
    {
      // reset extended state variables on initialization
      m_current_blp   = 0;
      m_retries       = 0;

      writecoef(port, (getBoardId() * N_BLP) + m_current_blp);
      readstatus(port);
    }
    break;

    case EPA_RSPSTATUS:
    {
      EPARspstatusEvent rspstatus(event);
      
      // check status of previous write
      if (rspstatus.rsp == 0)
      {
	// OK, move on to the next BLP
	m_current_blp++;
	m_retries = 0;
      }
      else
      {
	if (m_retries++ > N_RETRIES)
	{
	  // abort
	  LOG_FATAL("maximum retries reached!");
	  exit(EXIT_FAILURE);
	}
      }

      // write next and read status
      writecoef(port, (getBoardId() * N_BLP) + m_current_blp);
      readstatus(port);
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return GCFEvent::HANDLED;
}

void BWSync::writecoef(GCFPortInterface& port, uint8 blp)
{
  // send next BF configure message
  EPABfxreEvent bfxre;
  
  MEP_BFXRE(bfxre.hdr, MEPHeader::WRITE, 0);
  bfxre.hdr.m_fields.addr.dstid = blp;
  port.send(bfxre);
}

void BWSync::readstatus(GCFPortInterface& port)
{
  // send read status request to check status of the write
  EPARspstatusEvent rspstatus;
  MEP_RSPSTATUS(rspstatus.hdr, MEPHeader::READ);
  rspstatus.rsp = 0;

#if 0
  // on the read request don't send the data
  rspstatus.length -= RSPSTATUS_SIZE;
#endif

  port.send(rspstatus);
}




