//#  InitState.cc: class to keep track of initialiation state
//#  of the RSPDriver for a LOFAR station
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

#include "InitState.h"
#include "StationSettings.h"
#include <blitz/array.h>

using namespace LOFAR;
using namespace RSP;
using namespace blitz;

/*
 * Instance pointer for the InitState singleton class.
 */
InitState* InitState::m_instance = 0;

InitState::~InitState()
{
}

InitState& InitState::instance()
{
  if (0 == m_instance) {
    m_instance = new InitState();
  }
   
  return *m_instance;
}

InitState::InitState()
{
  m_tds_done.resize(StationSettings::instance()->nrRspBoards());
  m_rsu_done.resize(StationSettings::instance()->nrRspBoards());
  m_bs_done.resize(StationSettings::instance()->nrBlps());
  m_rcuenable_done.resize(StationSettings::instance()->nrBlps());

  init();
}

void InitState::init(CurrentState state)
{
  m_tds_done       = false;
  m_rsu_done       = false;
  m_bs_done        = false;
  m_rcuenable_done = false;
  m_state          = state;
}

void InitState::setTDSDone(int boardid, bool value)
{
  ASSERT(boardid >= 0 && boardid < StationSettings::instance()->nrRspBoards());
  m_tds_done(boardid) = value;
  if (isTDSDone()) m_state = WRITE_RSU;
}

void InitState::setRSUDone(int boardid, bool value)
{
  ASSERT(boardid >= 0 && boardid < StationSettings::instance()->nrRspBoards());
  m_rsu_done(boardid) = value;
  if (isRSUDone()) m_state = WRITE_BS;
}

void InitState::setBSDone(int blpid, bool value)
{
  ASSERT(blpid >= 0 && blpid < StationSettings::instance()->nrBlps());
  m_bs_done(blpid) = value;
  if (isBSDone()) m_state = WRITE_RCUENABLE;
}

void InitState::setRCUEnableDone(int blpid, bool value)
{
  ASSERT(blpid >= 0 && blpid < StationSettings::instance()->nrBlps());
  m_rcuenable_done(blpid) = value;
  if (isRCUEnableDone()) {
    init();
  }
}

bool InitState::isTDSDone() const
{
  return (m_state == WRITE_TDS) && (sum(where(m_tds_done == true, 1, 0)) == StationSettings::instance()->nrRspBoards());
}

bool InitState::isRSUDone() const
{
  return (m_state == WRITE_RSU) && (sum(where(m_rsu_done == true, 1, 0)) == StationSettings::instance()->nrRspBoards());
}

bool InitState::isBSDone() const
{
  return (m_state == WRITE_BS) && (sum(where(m_bs_done == true, 1, 0)) == StationSettings::instance()->nrBlps());
}

bool InitState::isRCUEnableDone() const
{
  return (m_state == WRITE_RCUENABLE) && (sum(where(m_rcuenable_done == true, 1, 0)) == StationSettings::instance()->nrBlps());
}
