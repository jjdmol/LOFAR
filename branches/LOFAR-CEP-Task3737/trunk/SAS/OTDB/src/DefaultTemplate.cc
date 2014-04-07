//#  DefaultTemplate.cc: Structure that describes the tree information.
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
//#  $Id: DefaultTemplate.cc 7762 2006-03-09 15:12:58Z overeem $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include<Common/LofarLogger.h>
#include<Common/lofar_datetime.h>
#include<OTDB/DefaultTemplate.h>

using namespace pqxx;

namespace LOFAR {
  namespace OTDB {

DefaultTemplate::DefaultTemplate(const result::tuple&		row) 
{
	// construct DefaultTemplate class with right ID
	// Note: names refer to SQL DefaultTemplate type
	row["treeid"].to(itsTreeID);
	row["name"].to(name);
}

//
// print(ostream&): os&
//
// Show Tree charateristics.
ostream& DefaultTemplate::print (ostream& os) const
{
	os << "treeID : " << itsTreeID << endl;
	os << "name   : " << name      << endl;

	return (os);
}

  } // namespace OTDB
} // namespace LOFAR
