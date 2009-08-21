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

#include <lofar_config.h>
#include <Common/StringUtil.h>
#include <APL/RTCCommon/RegisterState.h>

using namespace std;
using namespace LOFAR;
using namespace RTC;
using namespace blitz;

RegisterState::State RegisterState::get(int i) const
{
	ASSERTSTR(i >= 0 && i < m_state.extent(blitz::firstDim), "i=" << i << ", firstDim=" << m_state.extent(blitz::firstDim));
	return m_state(i);
}

int RegisterState::getMatchCount(State matchstate) const
{
	return sum(where(m_state == matchstate, 1, 0));
}

bool RegisterState::isMatchAll(State matchstate) const
{
	return (getMatchCount(matchstate) == m_state.extent(blitz::firstDim));
}

void RegisterState::print(std::ostream& out) const
{
	for (int i = 0; i < m_state.extent(blitz::firstDim); i++) {
	switch (m_state(i)) {
		case UNDEFINED:     out << "? "; break;
		case IDLE:          out << ". "; break;
		case CHECK:         out << "C "; break;
		case WRITE:         out << "W "; break;
		case READ:          out << "R "; break;
		case READ_ERROR:    out << "ER"; break;
		case WRITE_ERROR:   out << "EW"; break;
		case DONE:          out << "* "; break;
		case FAIL:          out << "F "; break;
		default:            out << formatString("%02X", m_state(i)); break;
//		default:            out << "X "; break;
		}
	}
	out << "$" << endl;
}

RegisterState& RegisterState::operator=(const RegisterState& state)
{
	if (this != &state) {
		m_state.resize(state.m_state.shape());
		m_state = state.m_state;
		m_error.resize(state.m_error.shape());
		m_error = state.m_error;
	}
	return (*this);
}

RegisterState::RegisterState(const RegisterState& state)
{
	m_state.resize(state.m_state.shape());
	m_state = state.m_state;
	m_error.resize(state.m_error.shape());
	m_error = state.m_error;
}

void RegisterState::tran(State source, State target, int i)
{
	int lb = 0, ub = 0;
	if (i < 0) {
		lb = 0;
		ub = m_state.extent(blitz::firstDim);
	} else {
		ASSERTSTR(i >= 0 && i < m_state.extent(blitz::firstDim), "i=" << i << ", firstDim=" << m_state.extent(blitz::firstDim));
		lb = i;
		ub = i + 1;
	}

	for (int j = lb; j < ub; j++) {
		if (source == m_state(j)) {
			m_state(j) = target;
		} else if (target != m_state(j)) {
			LOG_DEBUG_STR("tran(" << source << ", " << target << ") failed, current state = " << m_state(j));
		}
	}
}

// transition from DONE -> IDLE
// from READ_ERROR  -> READ | FAIL
// from WRITE_ERROR -> WRITE | FAIL
// from FAIL -> DONE
void RegisterState::clear(int i)
{
	int lb = 0, ub = 0;
	if (i < 0) {
		lb = 0;
		ub = m_state.extent(blitz::firstDim);
	} else {
		ASSERTSTR(i >= 0 && i < m_state.extent(blitz::firstDim), "i=" << i << ", firstDim=" << m_state.extent(blitz::firstDim));
		lb = i;
		ub = i + 1;
	}

	for (int j = lb; j < ub; j++) {
		if (m_state(j) == DONE || m_state(j) == IDLE) {
			m_state(j) = IDLE;

		} else if (m_state(j) == READ_ERROR) {
			m_state(j) = (m_error(j) < MAX_REGISTER_ERROR) ? READ : FAIL;

		} else if (m_state(j) == WRITE_ERROR) {
			m_state(j) = (m_error(j) < MAX_REGISTER_ERROR) ? WRITE : FAIL;

		} else if (m_state(j) == FAIL) {
			m_state(j) = DONE;

		} else {
			LOG_DEBUG_STR("clear tran from " << m_state(j) << " failed");
		}
	}
}

// transition unconditionally, write must always be noted
void RegisterState::write(int i)
{
	int lb = 0, ub = 0;
	if (i < 0) {
		lb = 0;
		ub = m_state.extent(blitz::firstDim);
	} else {
		ASSERTSTR(i >= 0 && i < m_state.extent(blitz::firstDim), "i=" << i << ", firstDim=" << m_state.extent(blitz::firstDim));
		lb = i;
		ub = i + 1;
	}

	for (int j = lb; j < ub; j++) {
		m_state(j) = WRITE;
	}
}

void RegisterState::reset(int i)
{
	if (i < 0) {
		for (int j = 0; j < m_state.extent(blitz::firstDim); j++) {
			m_state(j) = IDLE;
			m_error(j) = 0;
		}
		return;
	}

	ASSERTSTR(i >= 0 && i < m_state.extent(blitz::firstDim), "i=" << i << ", firstDim=" << m_state.extent(blitz::firstDim));
	m_state(i) = IDLE;
	m_error(i) = 0;
}

