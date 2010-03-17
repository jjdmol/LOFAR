//#  CampaignInfo.h: Structure containing a campaign
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

#ifndef LOFAR_OTDB_CAMPAIGNINFO_H
#define LOFAR_OTDB_CAMPAIGNINFO_H

// \file
// Structure containing a campaign

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <OTDB/OTDBconstants.h>
#include <OTDB/OTDBtypes.h>
#include <pqxx/pqxx>

using namespace boost::posix_time;

namespace LOFAR {
  namespace OTDB {

// \addtogroup OTDB
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.


// A CampaignInfo struct describes one item/element of the OTDB. An item can
// be node or an parameter.
// Note: it does NOT contain the value of the item.
class CampaignInfo {
public:
	CampaignInfo() : itsID(0) {};
	CampaignInfo(const string&	name, const string& title, const string& PI,
				 const string&  CO_I, const string& contact) :
			name(name), title(title), PI(PI), CO_I(CO_I), contact(contact), itsID(0) {};
	~CampaignInfo() {};

	int32		ID()	 const	{ return (itsID); }
	string		name;
	string		title;
	string		PI;
	string		CO_I;
	string		contact;

	// Show campaign
	ostream& print (ostream& os) const;

	friend class Campaign;
private:
	//# Prevent changing the database keys
	CampaignInfo(int32	ID): itsID(ID) {};
	CampaignInfo(const pqxx::result::tuple&	row);

	int32		itsID;
};

//#
//# operator<<
//#
inline ostream& operator<< (ostream&			os,
							const CampaignInfo		aCampaignInfo)
{
	return (aCampaignInfo.print(os));
}


// @}
  } // namespace OTDB
} // namespace LOFAR

#endif
