//#  -*- mode: c++ -*-
//#  RequestPool.h: Admin class for timestamp/subband pairs
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
//#  $Id: RequestPool.h 11768 2008-09-17 14:18:33Z overeem $

#ifndef REQUEST_POOL_H_
#define REQUEST_POOL_H_

#include <Common/lofar_list.h>
#include <APL/RTCCommon/Timestamp.h>

namespace LOFAR {
  using RTC::Timestamp;
  namespace ICAL {

// Admin class for timestamp/subband pairs
class RequestPool
{
public:
	explicit RequestPool(int		poolsize);
	~RequestPool();

	void	add				(int	subbandNr, const Timestamp&	aTS);
	void	remove			(int	subbandNr);
	void	clear			()			{ itsPool.clear();	}
	void	clearBeforeTime	(const Timestamp&	aTime);
	bool	full			() const	{ return (itsPool.size() >= itsPoolSize); }
	int		findOnTimestamp (const Timestamp& 	aTS) const;

private:
	RequestPool();
	class Request { 
	public:
		Request(int sb, const Timestamp& ts) : subband(sb), time(ts) {};
		int			subband; 
		Timestamp	time; 
	};
	list<Request>    	itsPool; 
	uint				itsPoolSize;
};

  }; // namespace ICAL
}; // namespace LOFAR

#endif 

