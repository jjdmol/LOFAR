//#  OTDBvalue.cc: For retrieving and modifying OTDB trees.
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
#include<OTDB/OTDBvalue.h>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::posix_time;
using namespace pqxx;

namespace LOFAR {
  namespace OTDB {

//
// Construct OTDBvalue from tuple
//
OTDBvalue::OTDBvalue(const result::tuple&	row)
{
	row["paramid"].to(itsNodeID);
	row["name"].to(name);
	row["value"].to(value);
	string	aTime;
	row["time"].to(aTime);
	if (!aTime.empty()) {
		time = time_from_string(aTime);
	}
}

//
// print(ostream&): os&
//
// Show Tree charateristics.
ostream& OTDBvalue::print (ostream& os) const
{
	os << "paramID: " << itsNodeID << endl;
	os << "name   : " << name << endl;
	os << "value  : " << value << endl;
	os << "time   : " << time << endl;

	return (os);
}

  } // namespace OTDB
} // namespace LOFAR
