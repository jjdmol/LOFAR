//#  CmdStack.cc: Implements time ordered map of DH_ApplControl structs
//#
//#  Copyright (C) 2004
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

//# Includes
#include <Common/LofarLogger.h>
#include <Common/Exception.h>
#include <Common/StringUtil.h>
#include "CmdStack.h"

namespace LOFAR {
  namespace ACC {

CmdStack::CmdStack()
{
}

CmdStack::~CmdStack()
{
	itsStack.clear();		// in case the destructor does not do this
}

void CmdStack::add(time_t				scheduleTime,
				   DH_ApplControl*		aDHAC) 
{
	LOG_TRACE_RTTI_STR("CmdStack::add: " << timeString(scheduleTime) <<
					   ", " << aDHAC);

	pair < iterator, bool>	result;
	DH_ApplControl*			newDHAC = aDHAC->makeDataCopy();
	LOG_TRACE_RTTI_STR("newDHAC=" << newDHAC);

	result = itsStack.insert(std::make_pair(scheduleTime, newDHAC));
	if (!result.second) {
		THROW (Exception, "insert in CmdStack failed");
	}

	LOG_DEBUG_STR("CmdStack:Cmd added with timestamp = " << 
													timeString(scheduleTime));
}
 
DH_ApplControl*	CmdStack::pop()
{
	ASSERTSTR(!itsStack.empty(), "Cmdstack is empty");

//	DH_ApplControl*		theCmd = itsStack.begin()->second;
	DH_ApplControl*		theCmd = (itsStack.begin()->second)->makeDataCopy();
	itsStack.erase(itsStack.begin());

	LOG_DEBUG("CmdStack: pop()");

	return (theCmd);
}

bool CmdStack::timeExpired()
{
	if (itsStack.empty()) {
		return (false);
	}

	if (itsStack.begin()->first <= time(0)) {
		LOG_DEBUG("CmdStack: time expired, returning command");
		return (true);
	}

	return (false);
}

} // namespace ACC
} // namespace LOFAR

