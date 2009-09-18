//# ConnectionPool.h: Manages a collection of connections.
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

#ifndef LOFAR_TRANSPORT_CONNECTIONPOOL_H
#define LOFAR_TRANSPORT_CONNECTIONPOOL_H

// \file
// Manages a collection of Connections for easy use by servers.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/lofar_map.h>
#include <Transport/Connection.h>

namespace LOFAR {
// \addtogroup LOFAR
// @{

// A connectionPool is a collection of Connections that releases servers
// from a lot of administrative tasks.
class ConnectionPool
{
	typedef map<int32, Connection*>		ConnMap;

public:
	ConnectionPool();
	~ConnectionPool();
	
	bool add   (int32	aConnID, Connection*	aConn);
	bool remove(int32	aConnID);
	Connection*	getConn(int32	ConnID) const;

private:
	typedef ConnMap::iterator			iterator;
	typedef ConnMap::const_iterator		const_iterator;

	// Copying is not allowed
	ConnectionPool(const ConnectionPool&	that);
	ConnectionPool& operator=(const ConnectionPool& that);

	//# Datamembers	
	ConnMap		itsConnPool;
	fd_set		itsConnSet;
	int32		itsHighestID;
	int32		itsLowestID;
};

// @}	addtogroup
} // namespace LOFAR

#endif
