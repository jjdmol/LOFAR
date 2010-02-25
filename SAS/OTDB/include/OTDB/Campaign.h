//#  Campaign.h: Interface for access to the campaign record
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

#ifndef OTDB_CAMPAIGN_H
#define OTDB_CAMPAIGN_H

// \file
// Interface for access to the campaign record

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <OTDB/OTDBconnection.h>
#include <OTDB/CampaignInfo.h>
#include <Common/ParameterSet.h>


namespace LOFAR {
  namespace OTDB {

// \addtogroup OTDB
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.


// Interface for access to the campaign record.
// ...
class Campaign
{
public:
	Campaign(OTDBconnection*	aConn);
	~Campaign();

	// Get one campaign record.
	CampaignInfo	getCampaign(const string&	name);
	CampaignInfo	getCampaign(int32			ID);

	// Get all campaign records
	vector<CampaignInfo> getCampaignList(); 

	// Update or insert a campaign record
	int32	saveCampaign(const CampaignInfo&	aCampaign);

	// Whenever an error occurs in one the OTDB functions the message can
	// be retrieved with this function.
	string	errorMsg();

private:
	// Copying is not allowed
	Campaign();
	Campaign(const Campaign&	that);
	Campaign& operator=(const Campaign& that);

	//# --- Datamembers ---
	OTDBconnection*		itsConn;
	string				itsError;
};

//# --- Inline functions ---



// @}
  } // namespace OTDB
} // namespace LOFAR

#endif
