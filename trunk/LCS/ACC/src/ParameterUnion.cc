//#  ParameterUnion.cc: Implements a map of Key-Value pairs.
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

#include <ACC/ParameterUnion.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_fstream.h>

namespace LOFAR {
  namespace ACC {

//#-------------------------- creation and destroy ---------------------------
//#
//# Default constructor
//#
ParameterUnion::ParameterUnion()
{}

//#
//# ParameterUnion(filename)
//#
ParameterUnion::ParameterUnion(const string&	filename) :
	ParameterCollection(filename)
{}

//#
//# ParameterUnion (ParameterCollection)
//#
ParameterUnion::ParameterUnion(const ParameterCollection& that) :
	ParameterCollection(that)
{}

//#
//# Copying is allowed.
//#
ParameterUnion::ParameterUnion(const ParameterUnion& that)
  : ParameterCollection (that)
{}

//#
//# operator= copying
//#
ParameterUnion& ParameterUnion::operator=(const ParameterUnion& that)
{
	if (this != &that) {
		ParameterCollection::operator= (that);
	}
	return (*this);
}

//#
//#	Destructor
//#
ParameterUnion::~ParameterUnion()
{}

//#
//# operator<<
//#
std::ostream&	operator<< (std::ostream& os, const ParameterUnion &thePS)
{
	return os << static_cast<ParameterCollection> (thePS);
}

//#
//# check()
//#
//# 1) Versionnumber must be present
//# 2) All versionnumber references should be valid versionnumbers
//#
bool	ParameterUnion::check(string&	errorReport) const
{
	bool	isOk = true;
	errorReport  = "";

	//# [1] must have valid versionnumber
	if (!isValidVersionNr(getVersionNr())) {	
		errorReport += "Versionnumber not found or not valid.\n";
		isOk = false;
	}

	//# [2] all versionnr keys must be valid
	const_iterator iter = begin();
	while (iter != end()) {
		if ((keyName(iter->first) == PC_KEY_VERSIONNR)  &&
									(!isValidVersionNr(iter->second))) {
			errorReport += iter->first + " has not a valid versionnumber.\n";
			isOk = false;
		}
		iter++;
	}

	return (isOk);
}

} // namespace ACC
} // namespace LOFAR
