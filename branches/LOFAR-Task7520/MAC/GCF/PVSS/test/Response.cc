#include <lofar_config.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/PVSS/PVSSinfo.h>
#include <Common/StringUtil.h>
#include "Response.h"

namespace LOFAR {
  namespace GCF {
	namespace PVSS {

uint32	gQueryID = 0;
   
void Response::dpCreated(const string& propName, PVSSresult		result)
{
	LOG_DEBUG(formatString("RESPONSE:Property '%s' created (err=%d)", 
				propName.c_str(), result));
}

void Response::dpDeleted(const string& propName, PVSSresult		result)
{
	LOG_DEBUG(formatString("RESPONSE:Property '%s' deleted (err=%d)", 
				propName.c_str(), result));
}

void Response::dpeSubscribed(const string& propName, PVSSresult		result)
{
	LOG_DEBUG(formatString("RESPONSE:Property '%s' subscribed (err=%d)", 
				propName.c_str(), result));
}

void Response::dpeUnsubscribed(const string& propName, PVSSresult		result)
{
	LOG_DEBUG(formatString("RESPONSE:Property '%s' unsubscribed (err=%d)", 
				propName.c_str(), result));
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
	LOG_DEBUG(formatString("RESPONSE:Value of property '%s' changed (err=%d)", 
				propName.c_str(), result));
}

void Response::dpeValueSet(const string& propName, PVSSresult		result)
{
	LOG_DEBUG(formatString("RESPONSE:Value of property '%s' is set (err=%d)", 
				propName.c_str(), result));
}

void Response::dpeSubscriptionLost(const string& propName, PVSSresult		result)
{
	LOG_DEBUG(formatString("RESPONSE:Lost subscription of %s (err=%d)", 
				propName.c_str(), result));
}

void Response::dpQuerySubscribed(uint32 queryId, PVSSresult		result)
{
	LOG_DEBUG(formatString("RESPONSE:dpQuerySubscribed: id=%d (err=%d)", 
				queryId, result));
	gQueryID = queryId;
}

void Response::dpQueryUnsubscribed(uint32 queryId, PVSSresult		result)
{
	LOG_DEBUG(formatString("RESPONSE:dpQueryUnsubscribed: id=%d (err=%d)", 
				queryId, result));
	gQueryID = 0;
}

void Response::dpQueryChanged(uint32 queryId, PVSSresult result,
							  const GCFPVDynArr&	DPnames,
							  const GCFPVDynArr&	DPvalues,
							  const GCFPVDynArr&	DPtimes)
{
	gQueryID = queryId;
	LOG_DEBUG(formatString("RESPONSE:dpQueryChanges: id=%d (err=%d)", queryId, result));
	int		nrElems = DPnames.getValue().size();
	for (int idx = 0; idx < nrElems; ++idx) {
		LOG_DEBUG_STR(formatString("%s | %s | %s", 
						DPnames.getValue() [idx]->getValueAsString().c_str(), 
						DPvalues.getValue()[idx]->getValueAsString().c_str(), 
						DPtimes.getValue() [idx]->getValueAsString().c_str()));
	}
}

  } // namespace PVSS
 } // namespace GCF
} // namespace LOFAR
