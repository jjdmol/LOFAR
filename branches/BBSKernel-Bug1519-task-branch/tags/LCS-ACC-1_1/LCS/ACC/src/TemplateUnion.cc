//#  TemplateUnion.cc: Implements a map of Key-Value pairs.
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

#include <ACC/TemplateUnion.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_fstream.h>

namespace LOFAR {
  namespace ACC {

//#-------------------------- creation and destroy ---------------------------
//#
//# Default constructor
//#
TemplateUnion::TemplateUnion()
{}

//#
//# TemplateUnion(filename)
//#
TemplateUnion::TemplateUnion(const string&	filename) :
	ParameterCollection(filename)
{}

//#
//# TemplateUnion (ParameterCollection)
//#
TemplateUnion::TemplateUnion(const ParameterCollection& that) :
	ParameterCollection(that)
{}

//#
//# Copying is allowed.
//#
TemplateUnion::TemplateUnion(const TemplateUnion& that)
  : ParameterCollection (that)
{}

//#
//# operator= copying
//#
TemplateUnion& TemplateUnion::operator=(const TemplateUnion& that)
{
	if (this != &that) {
		ParameterCollection::operator= (that);
	}
	return (*this);
}

//#
//#	Destructor
//#
TemplateUnion::~TemplateUnion()
{}

//#
//# operator<<
//#
std::ostream&	operator<< (std::ostream& os, const TemplateUnion &thePS)
{
	return os << static_cast<ParameterCollection> (thePS);
}

//#
//# check()
//#
//# 1) Versionnumber must be present
//# 2) All versionnumber references should be valid versionnumbers
//#
bool	TemplateUnion::check(string&	errorReport) const
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
