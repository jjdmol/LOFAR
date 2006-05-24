//#  -*- mode: c++ -*-
//#
//#  RegisterState.cc: implementation of RegisterState class.
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

#include <APL/RTCCommon/RegisterState.h>

using namespace std;
using namespace LOFAR;
using namespace RTC;

void RegisterState::tran(State source, State target, int i)
{
  int lb = 0, ub = 0;
  if (i < 0) {
    lb = 0;
    ub = m_state.extent(blitz::firstDim);
  } else {
    ASSERT(i >= 0 && i < m_state.extent(blitz::firstDim));
    lb = i;
    ub = i + 1;
  }

  for (int j = lb; j < ub; j++) {
    if (source == m_state(j)) {
      m_state(j) = target;
    } else if (target != m_state(j)) {
      LOG_ERROR_STR("tran(" << source << ", " << target << ") failed, current state = " << m_state(j));
    }
  }
}

// transition from DONE -> IDLE
// from READ_ERROR  -> READ
// from WRITE_ERROR -> WRITE
void RegisterState::clear(int i)
{
  int lb = 0, ub = 0;
  if (i < 0) {
    lb = 0;
    ub = m_state.extent(blitz::firstDim);
  } else {
    ASSERT(i >= 0 && i < m_state.extent(blitz::firstDim));
    lb = i;
    ub = i + 1;
  }
   
  for (int j = lb; j < ub; j++) {
    if (DONE == m_state(j)) {
      m_state(j) = IDLE;
    } else if (READ_ERROR == m_state(j)) {
      m_state(j) = READ;
    } else if (WRITE_ERROR == m_state(j)) {
      m_state(j) = WRITE;
    } else {
      LOG_ERROR_STR("clear tran from " << m_state(j) << " failed");
    }
  }
}

void RegisterState::reset(int i)
{
  if (i < 0) {
    for (int j = 0; j < m_state.extent(blitz::firstDim); j++) {
      m_state(j) = IDLE;
    }
    return;
  }
  ASSERT(i >= 0 && i < m_state.extent(blitz::firstDim));
  m_state(i) = IDLE;
}

