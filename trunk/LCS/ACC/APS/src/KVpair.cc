//#  KVpair.cc: one line description
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include<Common/LofarLogger.h>
#include<ACC/KVpair.h>

namespace LOFAR {
  namespace ACC {

KVpair::KVpair(const string&	aKey, const string&		aValue) :
	pair<string, string> (aKey, aValue)
{}

KVpair::KVpair(const string&	aKey, bool				aValue) :
	pair<string, string> (aKey, formatString("%s", aValue ? "True" : "False"))
{}

KVpair::KVpair(const string&	aKey, int				aValue) :
	pair<string, string> (aKey, formatString("%d", aValue))
{}

KVpair::KVpair(const string&	aKey, double			aValue) :
	pair<string, string> (aKey, formatString("%lg", aValue))
{}

KVpair::KVpair(const string&	aKey, float				aValue) :
	pair<string, string> (aKey, formatString("%e", aValue))
{}

KVpair::KVpair(const string&	aKey, time_t			aValue) :
	pair<string, string> (aKey, formatString("%ld", aValue))
{}


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


  } // namespace ACC
} // namespace LOFAR
