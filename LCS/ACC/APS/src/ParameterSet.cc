//#  ParameterSet.cc: Implements a map of Key-Value pairs.
//#
//#  Copyright (C) 2002-2003
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

#include <ACC/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_fstream.h>

using namespace std;

namespace LOFAR {
  namespace ACC {

//#-------------------------- creation and destroy ---------------------------
//#
//# Default constructor
//#
ParameterSet::ParameterSet()
{}

//#
//# ParameterSet(filename)
//#
ParameterSet::ParameterSet(const string&	filename) :
	ParameterCollection(filename)
{}

//#
//# ParameterSet (ParameterCollection)
//#
ParameterSet::ParameterSet(const ParameterCollection& that) :
	ParameterCollection(that)
{}

//#
//# Copying is allowed.
//#
ParameterSet::ParameterSet(const ParameterSet& that)
  : ParameterCollection (that)
{}

//#
//# operator= copying
//#
ParameterSet& ParameterSet::operator=(const ParameterSet& that)
{
	if (this != &that) {
		ParameterCollection::operator= (that);
	}
	return (*this);
}

//#
//#	Destructor
//#
ParameterSet::~ParameterSet()
{}

//#
//# operator<<
//#
std::ostream&	operator<< (std::ostream& os, const ParameterSet &thePS)
{
	return os << static_cast<ParameterCollection> (thePS);
}

//#
//# check()
//#
//# 1) Versionnumber must be present
//# 2) All keys must be filled.
//#
bool	ParameterSet::check(string&	errorReport) const
{
	bool	isOk = true;
	errorReport  = "";

	if (getVersionNr() == "") {
		errorReport += "No versionnumber found\n";
		isOk = false;
	}

	const_iterator		iter = begin();
	while (iter != end()) {
		if (iter->second == "") {
			errorReport += "Key " + iter->first + " has no value\n";
			isOk = false;
		}
		iter++;
	}

	return (isOk);
}

} // namespace ACC
} // namespace LOFAR
