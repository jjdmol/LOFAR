//#  SSSync.cc: implementation of the SSSync class
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

#include "RSP_Protocol.ph"
#include "EPA_Protocol.ph"

#include "SSSync.h"
#include "Cache.h"

#include <blitz/array.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;
using namespace blitz;

#define N_RETRIES 3

SSSync::SSSync(GCFPortInterface& board_port, int board_id)
  : SyncAction((State)&SSSync::initial_state, board_port, board_id),
    m_current_blp(0),
    m_retries(0)
{
}

SSSync::~SSSync()
{
  /* TODO: delete event? */
}

GCFEvent::TResult SSSync::initial_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
    {
    }
    break;
      
    case F_ENTRY:
    {
      // reset extended state variables on initialization
      m_current_blp   = 0;
      m_retries       = 0;
    }
    break;
    
    case F_TIMER:
    {
      TRAN(SSSync::writedata_state);
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return GCFEvent::HANDLED;
}

GCFEvent::TResult SSSync::writedata_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      // send next set of coefficients
      writedata((getBoardId() * N_BLP) + m_current_blp);

      TRAN(SSSync::readstatus_state);
    }
    break;

    case F_TIMER:
    {
      LOG_FATAL("missed real-time deadline");
      exit(EXIT_FAILURE);
    }
    break;


    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return GCFEvent::HANDLED;
}

GCFEvent::TResult SSSync::readstatus_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch(event.signal)
  {
    case F_ENTRY:
    {
      readstatus();

      // TODO: start timer to check for broken comms link
    }
    break;

    case F_TIMER:
    {
      LOG_FATAL("missed real-time deadline");
      //exit(EXIT_FAILURE);
    }
    break;
      
    case EPA_RSPSTATUS:
    {
      EPARspstatusEvent rspstatus(event);
      
      // check status of previous write
      if (rspstatus.write_status.error == 0)
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

      if (m_current_blp < N_BLP)
      {
	// send next bit of data
	TRAN(SSSync::writedata_state);
      }
      else
      {
	// we've completed the update
	setCompleted(true); // done with this statemachine
	TRAN(SSSync::initial_state);
      }
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return GCFEvent::HANDLED;
}

void SSSync::writedata(uint8 blp)
{
  LOG_DEBUG(formatString(">>>> SSSync(%s) blp=%d",
			 getBoardPort().getName().c_str(),
			 blp));
  
  // send subband select message
  EPASubbandselectEvent ss;
  MEP_SUBBANDSELECT(ss.hdr, MEPHeader::WRITE, 0);
  ss.hdr.m_fields.addr.dstid = blp;
  
  // create array to contain the subband selection
  uint16 nr_subbands = Cache::getInstance().getBack().getSubbandSelection().nrsubbands()(blp);
  Array<uint16, 1> subbands(nr_subbands);
  ss.ch    = subbands.data();
  ss.chDim = nr_subbands;

  subbands = 0; // init
  
  subbands = Cache::getInstance().getBack().getSubbandSelection()()(blp, Range(0, nr_subbands - 1));

  getBoardPort().send(ss);

  subbands.free();
}

void SSSync::readstatus()
{
  // send read status request to check status of the write
  EPARspstatusEvent rspstatus;
  MEP_RSPSTATUS(rspstatus.hdr, MEPHeader::READ);
  
  // clear from first field onwards
  memset(&rspstatus.rsp_status, 0, MEPHeader::RSPSTATUS_SIZE);

#if 0
  // on the read request don't send the data
  rspstatus.length -= RSPSTATUS_SIZE;
#endif

  getBoardPort().send(rspstatus);
}
