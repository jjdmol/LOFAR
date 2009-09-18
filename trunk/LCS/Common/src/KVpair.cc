//# KVpair.cc: one line description
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
#include<Common/LofarLogger.h>
#include<Common/StringUtil.h>
#include<Common/KVpair.h>
#include<time.h>

namespace LOFAR {

#define	OPTIONAL_TIMESTAMP		\
	if (genTimestamp) { \
		first.append(formatString("{%d}", time(0))); \
	}

KVpair::KVpair(const string& aKey, const string&  aValue, bool genTimestamp) :
	pair<string, string> (aKey, aValue)
{
	OPTIONAL_TIMESTAMP
}

KVpair::KVpair(const string& aKey, bool			  aValue, bool genTimestamp) :
	pair<string, string> (aKey, formatString("%s", aValue ? "True" : "False"))
{
	OPTIONAL_TIMESTAMP
}

KVpair::KVpair(const string& aKey, int			  aValue, bool genTimestamp) :
	pair<string, string> (aKey, formatString("%d", aValue))
{
	OPTIONAL_TIMESTAMP
}

KVpair::KVpair(const string& aKey, double		  aValue, bool genTimestamp) :
	pair<string, string> (aKey, formatString("%.20lg", aValue))
{
	OPTIONAL_TIMESTAMP
}

KVpair::KVpair(const string& aKey, float		  aValue, bool genTimestamp) :
	pair<string, string> (aKey, formatString("%e", aValue))
{
	OPTIONAL_TIMESTAMP
}

KVpair::KVpair(const string& aKey, time_t		  aValue, bool genTimestamp) :
	pair<string, string> (aKey, formatString("%ld", aValue))
{
	OPTIONAL_TIMESTAMP
}

KVpair::KVpair(const string& aKey, const vector<int>&    aValue, bool genTimestamp)
{
	uint max = aValue.size();
   	string strValue = "[";
   	for (uint i = 0; i < max; i++) {
		strValue += formatString("%d", aValue[i]);
		if (i < max-1) {
			strValue += ",";
		}
	} 
	strValue += "]";
	first = aKey;
	second = strValue;

	OPTIONAL_TIMESTAMP
} 

KVpair::~KVpair()
{}

KVpair::KVpair(const KVpair&	that) : 
	pair<string,string>(that.first, that.second) 
{}

KVpair& KVpair::operator=(const KVpair& that)
{
	if (this != &that) {
		this->first  = that.first;
		this->second = that.second;
	}
	return (*this);
}


} // namespace LOFAR
