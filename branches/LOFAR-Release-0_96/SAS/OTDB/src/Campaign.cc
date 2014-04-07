//#  Campaign.cc: Interface for access to the tree (KVT) values
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
#include <OTDB/Campaign.h>

namespace LOFAR {
  namespace OTDB {

//
// Campaign()
//
Campaign::Campaign (OTDBconnection* 	aConn) :
	itsConn  (aConn),
	itsError ("")
{
	ASSERTSTR(aConn, "Null pointer for connection not allowed");
}

//
// ~Campaign()
//
Campaign::~Campaign()
{
	// Do not delete the connection, we just borrowed it.
}

//
// getCampaign(name)
//
CampaignInfo Campaign::getCampaign(const string&	name)
{
	CampaignInfo	empty;

	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (empty);
	}

	LOG_TRACE_FLOW_STR("getCampaign(" << name << ")");

	work	xAction(*(itsConn->getConn()), "getCampaign");
	try {
		result	res = xAction.exec("SELECT * FROM getCampaign('" + name + "')");
		return (CampaignInfo(res[0]));
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during getCampaign:") + ex.what();
	}
	return (empty);
}

//
// getCampaign(ID)
//
CampaignInfo Campaign::getCampaign(int32	ID)
{
	CampaignInfo	empty;

	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (empty);
	}

	LOG_TRACE_FLOW_STR("getCampaign(" << ID << ")");

	work	xAction(*(itsConn->getConn()), "getCampaign");
	try {
		result	res = xAction.exec("SELECT * FROM getCampaign('" + toString(ID) + "')");
		return (CampaignInfo(res[0]));
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during getCampaign:") + ex.what();
	}
	return (empty);
}

//
// getCampaignList()
//
vector<CampaignInfo> Campaign::getCampaignList()
{
	vector<CampaignInfo>	resultVec;

	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (resultVec);
	}

	LOG_TRACE_FLOW("getCampaignList()");

	work	xAction(*(itsConn->getConn()), "getCampaignList");
	try {
		result	res = xAction.exec("SELECT * FROM getCampaignList()");
		result::size_type	nrRecords = res.size();
		for (result::size_type i = 0; i < nrRecords; ++i) {
			resultVec.push_back(CampaignInfo(res[i]));
		}
		return (resultVec);
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during getCampaignList:") + ex.what();
	}

	return (resultVec);
}

//
// saveCampaign(ID)
//
int32 Campaign::saveCampaign(const CampaignInfo&	aCampaign)
{
	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (0);
	}

	LOG_TRACE_FLOW_STR("saveCampaign(" << aCampaign.ID() << "," << aCampaign.name << ")");

	work	xAction(*(itsConn->getConn()), "saveCampaign");
	try {
		result	res = xAction.exec(formatString(
			"SELECT saveCampaign(%d,'%s','%s','%s','%s','%s')", 
					aCampaign.ID(), 	  aCampaign.name.c_str(), aCampaign.title.c_str(), 
					aCampaign.PI.c_str(), aCampaign.CO_I.c_str(), aCampaign.contact.c_str()));

		// Analyze result
		int32	newID;
		res[0]["savecampaign"].to(newID);
		if (!newID) {
			itsError = "Unable tp update the campaign";
			return (0);
		}
		xAction.commit();
		return (newID);
		
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during getCampaignList:") + ex.what();
		LOG_FATAL(itsError);
	}

	return (0);
}

//


  } // namespace OTDB
} // namespace LOFAR
