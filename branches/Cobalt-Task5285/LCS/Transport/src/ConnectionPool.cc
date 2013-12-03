//# ConnectionPool.cc: Manages a collection of connections.
//#
//# Copyright (C) 2002-2004
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Transport/ConnectionPool.h>


namespace LOFAR {

ConnectionPool::ConnectionPool() :
	itsHighestID(0),
	itsLowestID (0)
{
	FD_ZERO(&itsConnSet);
}

ConnectionPool::~ConnectionPool()
{}


bool ConnectionPool::add   (int32	aConnID, Connection*	aConn)
{
	LOG_TRACE_RTTI_STR("ConnectionPool:add(" << aConnID << ")");

	pair<iterator, bool>	result;
	result = itsConnPool.insert(std::make_pair(aConnID, aConn));

	if (!result.second) {
		return (false);
	}

	FD_SET(aConnID, &itsConnSet);
	if (aConnID > itsHighestID) {
		itsHighestID = aConnID;
	}
	return (true);
}

bool ConnectionPool::remove(int32		aConnID)
{
	LOG_TRACE_RTTI_STR("ConnectionPool:remove(" << aConnID << ")");

	FD_CLR(aConnID, &itsConnSet);

	// update highestID number
	if (aConnID == itsHighestID) {
		while (itsHighestID > itsLowestID && 
									!FD_ISSET(itsHighestID, &itsConnSet)) {
			itsHighestID--;
		}
	}
	return (itsConnPool.erase(aConnID));
}

Connection*	ConnectionPool::getConn(int32	aConnID) const
{
	LOG_TRACE_RTTI_STR("getConn:" << aConnID);

	const_iterator	iter = itsConnPool.find(aConnID);

	if (iter == itsConnPool.end()) {
		return (0);
	}

	return (iter->second);
}


} // namespace LOFAR
