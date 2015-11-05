//#  Converter.cc: General base class for database based id-name conversions
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
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <OTDB/OTDBconnection.h>
#include <OTDB/Converter.h>

using namespace pqxx;

namespace LOFAR {
  namespace OTDB {

Converter::Converter(OTDBconnection*	aConn, const string&	aTypeName) :
	itsTypeName (aTypeName)
{ 
	ASSERTSTR(aConn->connect(), "No connection with the database");

	work	xAction(*(aConn->getConn()), itsTypeName);
	try {
		result res = xAction.exec("SELECT id,name from " + itsTypeName);

		// show how many records found
		result::size_type	nrRecords = res.size();
		ASSERTSTR(nrRecords, "No " << itsTypeName << 
									 " conversions in the database");

		// copy information to output vector and construct new nodeList
		pair<string,int16>		aPair;
		for (result::size_type	i = 0; i < nrRecords; ++i) {
			res[i]["id"].to(aPair.second);
			res[i]["name"].to(aPair.first);
			itsMap.insert(aPair);		// assume it goes right
		}
	}
	catch (std::exception&	ex) {
		THROW(Exception, "Exception during construction of " << itsTypeName 
						<< ex.what());
	}

	// init internal iterator
	itsIter = itsMap.begin();
	
}

//
// convert string to id
//
int16	Converter::convert(const string&	aType) const
{
	const_iterator	iter = itsMap.find(aType);
	return ((iter == itsMap.end()) ? -1 : iter->second);
}

//
// convert id to string
//
string	Converter::convert(int16			aTypeID) const
{
	const_iterator	iter = itsMap.begin();

	while(iter != itsMap.end()) {
		if (iter->second == aTypeID) {
			return (iter->first);
		}
		++iter;
	}
	return ("");
}

//
// get pair at inernal iterator
//
bool	Converter::get(int16&	theValue, string&	theName) const
{
	if (itsIter == itsMap.end()) {
		return (false);
	}

	theValue = itsIter->second;
	theName  = itsIter->first;

	return (true);
}


//
// print(ostream&): os&
//
// Show Tree charateristics.
ostream& Converter::print (ostream& os) const
{
	const_iterator	iter = itsMap.begin();

	while(iter != itsMap.end()) {
		os << formatString("%4d : %s\n", iter->second, iter->first.c_str());
		++iter;
	}
	return (os);
}

  } // namespace OTDB
} // namespace LOFAR
