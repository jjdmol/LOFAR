/***************************************************************************
 *   Copyright (C) 2008 by A.R. Offringa   *
 *   offringa@astro.rug.nl   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <AOFlagger/util/stopwatch.h>

#include <boost/date_time/posix_time/posix_time.hpp>

Stopwatch::Stopwatch() : _running(false), _sum(boost::posix_time::seconds(0))
{
}

Stopwatch::Stopwatch(bool start) :
	_running(start),
	_startTime(boost::posix_time::microsec_clock::local_time()),
	_sum(boost::posix_time::seconds(0))
{
}

Stopwatch::~Stopwatch()
{
}

void Stopwatch::Start()
{
	if(!_running) {
		_startTime = boost::posix_time::microsec_clock::local_time();
		_running = true;
	}
}

void Stopwatch::Pause()
{
	if(_running) {
		_sum += (boost::posix_time::microsec_clock::local_time() - _startTime);
		_running = false;
	}
}

void Stopwatch::Reset()
{
	_running = false;
	_sum = boost::posix_time::seconds(0);
}

std::string Stopwatch::ToString() const
{
	if(_running) {
		boost::posix_time::time_duration current = _sum + (boost::posix_time::microsec_clock::local_time() - _startTime);
		return to_simple_string(current);
	} else {
		return to_simple_string(_sum);
	}
}

long double Stopwatch::Seconds() const
{
	if(_running) {
		boost::posix_time::time_duration current = _sum + (boost::posix_time::microsec_clock::local_time() - _startTime);
		return current.seconds();
	} else {
		return _sum.seconds();
	}
}
