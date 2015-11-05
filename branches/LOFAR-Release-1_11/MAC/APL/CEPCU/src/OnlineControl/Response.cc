//#  Reponse.cc: Dummy class for handlind responses of PVSS
//#
//#  Copyright (C) 2006-2012
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
//#  $Id: OnlineControl.cc 22253 2012-10-08 14:53:35Z overeem $
#include <lofar_config.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/PVSS/PVSSinfo.h>
#include <Common/StringUtil.h>
#include "Response.h"

namespace LOFAR {
	using namespace GCF::PVSS;
	namespace CEPCU {

void Response::dpCreated(const string& propName, PVSSresult		result)
{
	LOG_DEBUG(formatString("RESPONSE:Property '%s' created (err=%d)", propName.c_str(), result));
}

void Response::dpDeleted(const string& propName, PVSSresult		result)
{
	LOG_DEBUG(formatString("RESPONSE:Property '%s' deleted (err=%d)", propName.c_str(), result));
}

void Response::dpeSubscribed(const string& propName, PVSSresult		result)
{
	LOG_DEBUG(formatString("RESPONSE:Property '%s' subscribed (err=%d)", propName.c_str(), result));
}

void Response::dpeUnsubscribed(const string& propName, PVSSresult		result)
{
	LOG_DEBUG(formatString("RESPONSE:Property '%s' unsubscribed (err=%d)", propName.c_str(), result));
}

void Response::dpeValueGet(const string& propName, PVSSresult		result, const GCFPValue& value)
{
	if (result == SA_NO_ERROR) {
		LOG_DEBUG_STR("RESPONSE:Valueget of property " << propName << " : " << 
					((GCFPVInteger *)&value)->getValue() << " (err=" << result << ")");
	}
	else {
		LOG_DEBUG_STR("RESPONSE: error " << result << " while getting the value of " << propName);
	}
}

void Response::dpeValueChanged(const string& propName, PVSSresult		result, const GCFPValue& /*value*/)
{
	LOG_DEBUG(formatString("RESPONSE:Value of property '%s' changed (err=%d)", propName.c_str(), result));
}

void Response::dpeValueSet(const string& propName, PVSSresult		result)
{
	LOG_DEBUG(formatString("RESPONSE:Value of property '%s' is set (err=%d)", propName.c_str(), result));
}

void Response::dpeSubscriptionLost(const string& propName, PVSSresult		result)
{
	LOG_DEBUG(formatString("RESPONSE:Lost subscription of %s (err=%d)", propName.c_str(), result));
}

void Response::dpQuerySubscribed(uint32 queryId, PVSSresult		result)
{
	LOG_DEBUG(formatString("RESPONSE:dpQuerySubscribed: id=%d (err=%d)", queryId, result));
}

void Response::dpQueryUnsubscribed(uint32 queryId, PVSSresult		result)
{
	LOG_DEBUG(formatString("RESPONSE:dpQueryUnsubscribed: id=%d (err=%d)", queryId, result));
}

void Response::dpQueryChanged(uint32 queryId, PVSSresult result,
							  const GCFPVDynArr&	DPnames,
							  const GCFPVDynArr&	DPvalues,
							  const GCFPVDynArr&	DPtimes)
{
	LOG_DEBUG(formatString("RESPONSE:dpQueryChanges: id=%d (err=%d)", queryId, result));
	int		nrElems = DPnames.getValue().size();
	for (int idx = 0; idx < nrElems; ++idx) {
		LOG_DEBUG_STR(formatString("%s | %s | %s", 
						DPnames.getValue() [idx]->getValueAsString().c_str(), 
						DPvalues.getValue()[idx]->getValueAsString().c_str(), 
						DPtimes.getValue() [idx]->getValueAsString().c_str()));
	}
}

 } // namespace CEPCU
} // namespace LOFAR
