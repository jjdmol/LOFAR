//#  setStatus.cc: Force the status of an observation or pipeline
//#
//#  Copyright (C) 2004-2012
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
//#  $Id: MACScheduler.cc 27964 2014-01-17 11:04:09Z mol $
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/ParameterSet.h>
#include "TreeStateConv.h"

using namespace LOFAR;
using namespace LOFAR::OTDB;

int main()
{
	std::string obsID;
	std::string status; // queued, active, completing, finished, aborted

	// Try to connect to the SAS database.
	ParameterSet* pParamSet = globalParameterSet();
	string username	= pParamSet->getString("OTDBusername");
	string DBname 	= pParamSet->getString("OTDBdatabasename");
	string password	= pParamSet->getString("OTDBpassword");
	string hostname	= pParamSet->getString("OTDBhostname");

	LOG_INFO_STR ("Trying to connect to the OTDB on " << hostname);
	conn = new OTDBconnection(username, password, DBname, hostname);

	LOG_INFO_STR ("Setting status of observation " << obsID << " to " << status);
	OTDB::TreeMaintenance tm(conn);
	TreeStateConv tsc(conn);
	tm.setTreeState(obsID, tsc.get(status));

	LOG_INFO_STR ("Disconnecting from OTDB");
	delete conn;
}
