//#  ParameterTemplate.cc: Implements a map of Key-Value pairs.
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

#include <ACC/ParameterTemplate.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_fstream.h>

namespace LOFAR {
  namespace ACC {

//#-------------------------- creation and destroy ---------------------------
//#
//# Default constructor
//#
ParameterTemplate::ParameterTemplate()
{}

//#
//# ParameterTemplate(filename)
//#
ParameterTemplate::ParameterTemplate(const string&	filename) :
	ParameterCollection(filename)
{}

//#
//# ParameterTemplate (ParameterCollection)
//#
ParameterTemplate::ParameterTemplate(const ParameterCollection& that) :
	ParameterCollection(that)
{}

//#
//# Copying is allowed.
//#
ParameterTemplate::ParameterTemplate(const ParameterTemplate& that)
  : ParameterCollection (that)
{}

//#
//# operator= copying
//#
ParameterTemplate& ParameterTemplate::operator=(const ParameterTemplate& that)
{
	if (this != &that) {
		ParameterCollection::operator= (that);
	}
	return (*this);
}

//#
//#	Destructor
//#
ParameterTemplate::~ParameterTemplate()
{}

//#
//# operator<<
//#
std::ostream&	operator<< (std::ostream& os, const ParameterTemplate &thePS)
{
	return os << static_cast<ParameterCollection> (thePS);
}

//#
//# getQualification()
//#
string	ParameterTemplate::getQualification() const
{
	const_iterator iter = find(getName()+"."+PC_KEY_QUAL);
	if (iter == end()) {
		return (PC_QUAL_DEVELOP);
	}

	return (iter->second);
}

//#
//# check()
//#
//# 1) Versionnumber must be present
//# 2) A Qualification key should be available (not fatal)
//# 3) All keys must start with the same keypart
//# 4) All versionnumber references should be valid
//#
bool	ParameterTemplate::check(string&	errorReport) const
{
	bool	isOk = true;
	errorReport  = "";

	//# [1] must have valid versionnumber
	if (!isValidVersionNr(getVersionNr())) {	
		errorReport += "Versionnumber not found or not valid.\n";
		isOk = false;
	}

	//# [2] should have qualification (not fatal)
	if (find(getName()+"."+PC_KEY_QUAL) == end()) {
		errorReport += "Qualification key is missing.\n";
	}

	//# [3] all keys should belong to same module
	const_iterator		iter = end();
	iter--;
	string fullKeyName = iter->first;
	char*	firstPoint = strchr (fullKeyName.c_str(), '.');

	if (fullKeyName.substr(0, firstPoint - fullKeyName.c_str()) != getName()) {
		errorReport += "Not all keys belong to the same module.\n";
		isOk = false;
	}

	//# [4] all versionnr keys must be valid
	iter = begin();
	while (iter != end()) {
		if ((keyName(iter->first) == PC_KEY_VERSIONNR)  &&
									(!isValidVersionNrRef(iter->second))) {
			errorReport += iter->first + " has not a valid versionnumber reference.\n";
			isOk = false;
		}
		iter++;
	}

	return (isOk);
}

} // namespace ACC
} // namespace LOFAR
