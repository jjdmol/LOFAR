//#  -*- mode: c++ -*-
//#  RequestPool.cc: implementation of the SubArray class
//#
//#  Copyright (C) 2010
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
//#  $Id: RequestPool.cc 12256 2008-11-26 10:54:14Z overeem $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include "RequestPool.h"

namespace LOFAR {
  namespace ICAL {
    
//
// RequestPool()
//
RequestPool::RequestPool(int	poolsize) :
	itsPoolSize(poolsize)
{
}

//
// ~RequestPool()
//
RequestPool::~RequestPool()
{
	itsPool.clear();
}

//
// add(subbandNr, timestamp)
//
void RequestPool::add(int subbandNr, const Timestamp&	ts)
{
	LOG_DEBUG_STR("add(" << subbandNr << "," << ts << ")");

	itsPool.push_back(Request(subbandNr, ts));
}

//
// remove(subbandNr)
//
void RequestPool::remove(int	subbandNr)
{
	list<Request>::iterator	iter = itsPool.begin();
	list<Request>::iterator	end  = itsPool.end();
	while (iter != end) {
		if (iter->subband == subbandNr) {
			LOG_DEBUG_STR("remove(" << subbandNr << "," << iter->time << ")");
			itsPool.erase(iter);
			return;
		}
		iter++;
	}
}

//
// clearBeforeTime(timestamp)
//
void RequestPool::clearBeforeTime(const Timestamp&	aTime)
{
	list<Request>::iterator	iter = itsPool.begin();
	list<Request>::iterator	end  = itsPool.end();
	while (iter != end) {
		if (iter->time < aTime) {
			LOG_WARN_STR("Removing unhandled request for subband " << iter->subband << "@" << iter->time);
			itsPool.erase(iter);
			iter = itsPool.begin();
		}
		else {
			iter++;
		}
	}
}

//
// findOnTimestamp(timestamp):subbandnr or -1
//
int RequestPool::findOnTimestamp(const Timestamp&	ts) const
{
	list<Request>::const_iterator	iter = itsPool.begin();
	list<Request>::const_iterator	end  = itsPool.end();
	while (iter != end) {
		if (iter->time == ts) {
			return (iter->subband);
		}
		iter++;
	}
	return (-1);
}

  } // namespace ICAL
} // namespace LOFAR
