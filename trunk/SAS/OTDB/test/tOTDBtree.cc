//#  tOTDBtree: test the OTDBtree class
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
#include <Common/lofar_datetime.h>
#include <OTDB/OTDBtree.h>

using namespace LOFAR;
using namespace LOFAR::OTDB;

//
// show the result
//
void showList(const vector<OTDBnode>&	items) {


	cout << "paramID|parentID|name           |Index|Type|Unit|Description" << endl;
	cout << "-------+--------+---------------+-----+----+----+------------------" << endl;
	for (uint32	i = 0; i < items.size(); ++i) {
		string row(formatString("%7d|%8d|%-15.15s|%5d|%4d|%4d|%s",
			items[i].ID,
			items[i].parentID,
			items[i].name.c_str(),
			items[i].index,
			items[i].type,
			items[i].unit,
			items[i].description.c_str()));
		cout << row << endl;
	}

	cout << items.size() << " records" << endl << endl;
}

int main (int	argc, char*	argv[]) {

	INIT_LOGGER(basename(argv[0]));
	LOG_INFO_STR("Starting " << argv[0]);

	OTDBconnection conn("paulus", "boskabouter", "overeem");

	try {

		LOG_DEBUG("Trying to connect to the database");
		ASSERTSTR(conn.connect(), "Connnection failed");
		LOG_INFO_STR("Connection succesful: " << conn);

		LOG_DEBUG("Trying to construct a tree object");
		OTDBtree	tree(&conn, 25);

		LOG_INFO("getItemList(15,2) of tree 25");
		vector<OTDBnode>	itemList = tree.getItemList(15,2);
		if (itemList.size() == 0) {
			LOG_INFO_STR("Error:" << conn.errorMsg());
		}
		else {
			showList(itemList);
		}

	}
	catch (std::exception&	ex) {
		LOG_FATAL_STR("Unexpected exception: " << ex.what());
		return (1);		// return !0 on failure
	}

	LOG_INFO_STR ("Terminated succesfully: " << argv[0]);

	return (0);		// return 0 on succes
}
