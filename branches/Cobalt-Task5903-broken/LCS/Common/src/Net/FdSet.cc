//# FdSet.cc: C++ version of fd_set
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
#include<Common/Net/FdSet.h>

namespace LOFAR {

//
// FdSet()
//
FdSet::FdSet():
	itsHighest(0),
	itsLowest(0),
	itsFdCount(0)
{
	FD_ZERO(&itsSet);
}

//
// ~FdSet()
//
FdSet::~FdSet()
{}

//
// operator=
//
FdSet&	FdSet::operator= (const FdSet& that)
{
	if (this != &that) {
		itsSet     = that.itsSet;
		itsHighest = that.itsHighest;
		itsLowest  = that.itsLowest;
		itsFdCount = that.itsFdCount;
	}

	return (*this);
}

//
// add(fID)
//
void FdSet::add(int32	fID)
{
	// Range must be valid and bit must be cleared
	if ((fID < 0) || (fID > FD_SETSIZE) || FD_ISSET(fID, &itsSet)) {
		return;
	}

	itsFdCount++;
	FD_SET(fID, &itsSet);

	// Do bookkeeping
	if (fID > itsHighest) {
		itsHighest = fID;
	}
	if (fID < itsLowest || (itsLowest == 0)) {
		itsLowest = fID;
	}
}

//
// remove(fID)
//
void FdSet::remove (int32 fID)
{
	// Range must be valid and bit must be set
	if ((fID < 0) || (fID > FD_SETSIZE) || !FD_ISSET(fID, &itsSet)) {
		return;
	}

	itsFdCount--;
	FD_CLR(fID, &itsSet);
	
	// Do bookkeeping
	if (fID == itsLowest) {
		itsLowest  = 0;
		itsHighest = 0;
		return;
	} 
	
	if (fID == itsHighest) {
		while ((itsHighest > itsLowest) && !FD_ISSET(itsHighest, &itsSet)) {
			itsHighest--;
		}
	}
}

//
// clear()
//
void FdSet::clear()
{
	FD_ZERO(&itsSet);
	itsLowest  = 0;
	itsHighest = 0;
	itsFdCount = 0;
}

} // namespace LOFAR
