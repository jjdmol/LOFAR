//#  SyncAction.cc: implementation of the SyncAction class
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

#include "SyncAction.h"
#include "EPA_Protocol.ph"

using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;

#define N_RETRIES 10

SyncAction::SyncAction(GCFPortInterface& board_port, int board_id, int n_blps)
  : GCFFsm((State)&SyncAction::idle_state),
    m_board_port(board_port),
    m_board_id(board_id),
    m_completed(false),
    m_n_blps(n_blps),
    m_current_blp(0),
    m_retries(0)
{
}

SyncAction::~SyncAction()
{
}

GCFEvent::TResult SyncAction::idle_state(GCFEvent& event, GCFPortInterface& /*port*/)
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
      m_current_blp = 0;
      m_retries     = 0;
    }
    break;
    
    case F_TIMER:
    {
      // Scheduler::run send a timer signal to start of the next update
      TRAN(SyncAction::sendrequest_state);
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return GCFEvent::HANDLED;
}

GCFEvent::TResult SyncAction::sendrequest_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      // send next set of coefficients
      sendrequest();

      // is sendrequest indicates completion, simply transition to idle_state
      if (hasCompleted())
      {
	TRAN(SyncAction::idle_state);
      }
      else
      {
	TRAN(SyncAction::waitack_state);
      }
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

GCFEvent::TResult SyncAction::waitack_state(GCFEvent& event, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch(event.signal)
  {
    case F_ENTRY:
    {
      sendrequest_status();
    }
    break;

    case F_TIMER:
    {
      LOG_FATAL("missed real-time deadline, should have been caught in Scheduler::run");
      exit(EXIT_FAILURE);
    }
    break;

    case F_EXIT:
      break;

    case EPA_READACK_ERROR:
    case EPA_WRITEACK_ERROR:
      LOG_ERROR("Read/write error during SyncAction. Aborting sync action.");

      setCompleted(true); // done with this statemachine
      TRAN(SyncAction::idle_state);
      break;
      
    default:
    {
      status = handleack(event, port);
      
      // check status of previous write
      if (GCFEvent::HANDLED == status)
      {
	// OK, move on to the next BLP
	m_current_blp++;
	m_retries = 0;

	if (m_current_blp < m_n_blps)
	{
	  // send next bit of data
	  TRAN(SyncAction::sendrequest_state);
	}
	else
	{
	  // we've completed the update
	  setCompleted(true); // done with this statemachine
	  TRAN(SyncAction::idle_state);
	}
      }
      else
      {
	//
	// didn't receive what we expected, simply wait
	// for another (hopefully correct) event,
	// but only allow m_retries of these situations.
	//
	if (m_retries++ > N_RETRIES)
	{
	  // abort
	  LOG_FATAL("maximum retries reached!");
	  exit(EXIT_FAILURE);
	}
      }
    }
    break;
  }

  return GCFEvent::HANDLED;
}

int SyncAction::getBoardId()
{
  return m_board_id;
}

GCFPortInterface& SyncAction::getBoardPort()
{
  return m_board_port;
}

void SyncAction::setCompleted(bool completed)
{
  m_completed = completed;
}

bool SyncAction::hasCompleted() const
{
  return m_completed;
}

int SyncAction::getCurrentBLP() const
{
  return m_current_blp;
}

/**
 * Force completion by resetting to idle state
 * and resetting various state variables.
 */
void SyncAction::reset()
{
  setCompleted(true);
  TRAN(SyncAction::idle_state);
}

