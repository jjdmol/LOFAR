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
#include "RSP_Protocol.ph"
#include "Cache.h"

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <unistd.h>
#include <string.h>
#include <blitz/array.h>

using namespace RSP;
using namespace LOFAR;
using namespace EPA_Protocol;
using namespace blitz;

#define N_RETRIES 3

BWSync::BWSync(GCFPortInterface& board_port, int board_id, int regid)
  : SyncAction((State)&BWSync::initial_state, board_port, board_id),
    m_current_blp(0),
    m_retries(0),
    m_regid(regid)
{
}

BWSync::~BWSync()
{
}

GCFEvent::TResult BWSync::initial_state(GCFEvent& event, GCFPortInterface& /*port*/)
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
      TRAN(BWSync::writedata_state);
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return GCFEvent::HANDLED;
}

GCFEvent::TResult BWSync::writedata_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      // send next set of coefficients
      writedata((getBoardId() * N_BLP) + m_current_blp);

      TRAN(BWSync::readstatus_state);
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

GCFEvent::TResult BWSync::readstatus_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch(event.signal)
  {
    case F_ENTRY:
    {
      readstatus();

      // TODO start timer to check for broken comms link
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

      if (m_current_blp < N_BLP)
      {
	// send next bit of data
	TRAN(BWSync::writedata_state);
      }
      else
      {
	// we've completed the update
	setCompleted(true); // done with this statemachine
	TRAN(BWSync::initial_state);
      }
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return GCFEvent::HANDLED;
}

void BWSync::writedata(uint8 blp)
{
  if (m_regid <= MEPHeader::BFXRE || m_regid > MEPHeader::BFYIM)
  {
    m_regid = MEPHeader::BFXRE; // HACK
  }

  LOG_DEBUG(formatString(">>>> BWSync(%s) blp=%d, regid=%d",
			 getBoardPort().getName().c_str(),
			 blp,
			 m_regid));
  
  // send next BF configure message
  EPABfcoefsEvent bfcoefs;
      
  MEP_BF(bfcoefs.hdr, MEPHeader::WRITE, 0, m_regid);
  bfcoefs.hdr.m_fields.addr.dstid = blp;

  // copy weights from the cache to the message
  Array<int16, 1> weights((int16*)&bfcoefs.coef,
			  shape(RSP_Protocol::N_BEAMLETS),
			  neverDeleteData);
  
  //
  // TODO
  // Make sure we're actually sending the correct weights.
  //
  if (0 == (m_regid % 2))
  {
    weights = real(Cache::getInstance().getBack().getBeamletWeights().\
		   weights()(0, blp, Range::all()));
  }
  else
  {
    weights = imag(Cache::getInstance().getBack().getBeamletWeights().\
		   weights()(0, blp, Range::all()));
  }

  getBoardPort().send(bfcoefs);
}

void BWSync::readstatus()
{
  // send read status request to check status of the write
  EPARspstatusEvent rspstatus;
  MEP_RSPSTATUS(rspstatus.hdr, MEPHeader::READ);

  memset(&rspstatus.rsp, 0, MEPHeader::RSPSTATUS_SIZE);

#if 0
  // on the read request don't send the data
  rspstatus.length -= RSPSTATUS_SIZE;
#endif

  getBoardPort().send(rspstatus);
}




