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

#include "SyncAction.h"

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;

SyncAction::SyncAction(State handler, GCFPortInterface& board_port, int board_id) 
  : GCFFsm(handler), m_board_port(board_port), m_board_id(board_id), m_completed(false)
{
}

SyncAction::~SyncAction()
{
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
