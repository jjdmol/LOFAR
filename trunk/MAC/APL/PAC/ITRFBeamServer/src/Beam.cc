//#  Beam.h: implementation of the Beam class
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
#include "Beam.h"

using namespace LOFAR;
using namespace BS;
using namespace IBS_Protocol;
using namespace RTC;
using namespace std;

//
// Beam(name, rcuMask)
//
Beam::Beam(const string& 				name, 
		   const bitset<MAX_RCUS>&		rcuMask) :
	itsName	(name), 
	itsRCUs	(rcuMask)
{}

//
// ~Beam
//
Beam::~Beam()
{
	// clear the pointings
	itsPointings.clear();
}

//
// _resolveGaps()
//
// When there are 'holes' in the pointing sequence, fill them with NIL pointings
//
void Beam::_resolveGaps()
{
	// Note: it is possible that there are already NIL pointings in the list.
	//       To simplify the algorithms we first delete all NIL pointings and than add
	//		 new NIL pointings again.
	list<Pointing>::iterator	iter = itsPointings.begin();
	list<Pointing>::iterator	end  = itsPointings.end();
	while (iter != end) {
		if (iter->getType() == "NONE") {
			iter = itsPointings.erase(iter);
		}
		else {
			++iter;
		}
	}
	
	// Loop over the new list and fill the gaps again.
	iter = itsPointings.begin();
	end  = itsPointings.end();
	Timestamp	endTime = iter->time();
	while (iter != end) {
		// is there a gap between the last endtime and the current begintime add a NIL pointing
		if (iter->time() > endTime) {
			Pointing	nilPointing(0.0 ,0.0 ,endTime, int(iter->time()-endTime), "NONE");
			itsPointings.insert(iter, nilPointing);
		}
			
		if (iter->duration() == 0) {		// current pointings lasts forever?, delete rest of pointings.
			itsPointings.erase(++iter, end);
			return;
		}

		endTime = iter->endTime();
		++iter;
	}

}

//
// _pointingOverlaps(pointing)
//
// Check is the given pointings overlaps with others.
//
bool Beam::_pointingOverlaps(const IBS_Protocol::Pointing& pt) const
{
	list<Pointing>::const_iterator    iter = itsPointings.begin();
	list<Pointing>::const_iterator    end  = itsPointings.end();
	while (iter != end) {
		if ((iter->getType() != "NONE") && (iter->overlap(pt))) {
			return (true);
		}
		++iter;
	}       

	return (false);
}

//
// addPointing(pointing)
//
bool Beam::addPointing(const Pointing& pointing)
{
	// add pointing, sort the list and fill gaps with 'NONE' pointings
	if (_pointingOverlaps(pointing)) {
		LOG_ERROR_STR("Pointing " << pointing << " is NOT added to beam " << itsName << " because it overlaps with existing pointings");
		return (false);
	}

	// it's ok to add it, clean up the admin afterwards.
	itsPointings.push_back(pointing);
	itsPointings.sort();
	_resolveGaps();
	return (true);
}

//
// pointingAtTime(pointing)
//
Pointing Beam::pointingAtTime(const Timestamp& time)
{
	list<Pointing>::const_iterator	iter = itsPointings.begin();
	list<Pointing>::const_iterator	end  = itsPointings.end();
	while (iter != end) {
		if (iter->duration() == 0) {	// forever?
			break;
		}
		if (iter->endTime() < time) {	// already finished? try next
			++iter;
			continue;
		}
	}

	// if nothing found, return NIL pointing
	if (iter == end) {			
		return (Pointing());
	}

	Pointing 	result(*iter);
	result.setTime(time);
	return (result);
}

// Get all pointings in a vector. They are sorted in time order.
vector<Pointing> Beam::getAllPointings() const
{
	vector<Pointing>	result;
	list<Pointing>::const_iterator	iter = itsPointings.begin();
	list<Pointing>::const_iterator	end  = itsPointings.end();
	while (iter != end) {
		result.push_back(*iter);
		++iter;
	}
	return (result);
}

//
// print function for operator<<
//
ostream& Beam::print (ostream& os) const
{
	os << "Beam " << itsName << ": curPt=" << itsCurrentPointing << endl;
	os << "     RCU's= " << itsRCUs << endl;
	return (os);
}


//
// send the pointing administration to the logfile at debug level
//
void Beam::showPointings () const
{
	LOG_DEBUG_STR("Pointings of beam " << itsName);
	list<Pointing>::const_iterator	iter = itsPointings.begin();
	list<Pointing>::const_iterator	end  = itsPointings.end();
	while (iter != end) {
		LOG_DEBUG_STR(*iter);
		++iter;
	}
}

