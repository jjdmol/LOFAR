//#  CampaignInfo.cc: Structure containing the node info.
//#
//#  Copyright (C) 2010
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
#include<Common/lofar_datetime.h>
#include<OTDB/CampaignInfo.h>

using namespace pqxx;

namespace LOFAR {
  namespace OTDB {

CampaignInfo::CampaignInfo(const result::tuple&	row)
{
	// construct a node class with the right database keys
	row["id"].to(itsID);
	row["name"].to(name);
	row["title"].to(title);
	row["PI"].to(PI);
	row["CO_I"].to(CO_I);
	row["contact"].to(contact);
}

//
// print(ostream&): os&
//
// Show Tree charateristics.
ostream& CampaignInfo::print (ostream& os) const
{
	os << "ID     : " << itsID << endl;
	os << "name   : " << name << endl;
	os << "title  : " << title << endl;
	os << "PI     : " << PI << endl;
	os << "CO_I   : " << CO_I << endl;
	os << "contact: " << contact << endl;

	return (os);
}

  } // namespace OTDB
} // namespace LOFAR
