#include <lofar_config.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/PVSS/PVSSinfo.h>
#include "DPresponse.h"

namespace LOFAR {
  namespace GCF {
	using namespace PVSS;
	namespace RTDB {
   
int		gCreateCounter = 0;
int		gSetCounter = 0;
int		gGetCounter = 0;
int		gQryCounter = 0;
int		gQueryID = 0;

void DPresponse::dpCreated(const string& propName, PVSSresult		result)
{
	LOG_DEBUG(formatString("RESPONSE:Property '%s' created (err=%d)", 
				propName.c_str(), result));
	gCreateCounter--;
}

void DPresponse::dpDeleted(const string& propName, PVSSresult		result)
{
	LOG_DEBUG(formatString("RESPONSE:Property '%s' deleted (err=%d)", 
				propName.c_str(), result));
}

void DPresponse::dpeSubscribed(const string& propName, PVSSresult		result)
{
	LOG_DEBUG(formatString("RESPONSE:Property '%s' subscribed (err=%d)", 
				propName.c_str(), result));
}

void DPresponse::dpeUnsubscribed(const string& propName, PVSSresult		result)
{
	LOG_DEBUG(formatString("RESPONSE:Property '%s' unsubscribed (err=%d)", 
				propName.c_str(), result));
}

void DPresponse::dpeValueGet(const string& propName, PVSSresult		result, const GCFPValue& value)
{
	LOG_DEBUG_STR("RESPONSE: error " << result << " while getting the value of " << propName);
	gGetCounter--;

	if (propName.find("test_int.theValue") != string::npos) {
		GCFPVInteger	dbValue;
		dbValue.copy(value);
		LOG_DEBUG_STR("Value of test_int.theValue = " << dbValue.getValue());
	}
}

void DPresponse::dpeValueChanged(const string& propName, PVSSresult		result, const GCFPValue& /*value*/)
{
	LOG_DEBUG(formatString("RESPONSE:Value of property '%s' changed (err=%d)", 
				propName.c_str(), result));
}

void DPresponse::dpeValueSet(const string& propName, PVSSresult		result)
{
	LOG_DEBUG(formatString("RESPONSE:Value of property '%s' is set (err=%d)", 
				propName.c_str(), result));
	gSetCounter--;
}

void DPresponse::dpeSubscriptionLost(const string& propName, PVSSresult		result)
{
	LOG_DEBUG(formatString("RESPONSE:Lost subscription of %s (err=%d)", 
				propName.c_str(), result));
}

void DPresponse::dpQuerySubscribed(uint32 queryId, PVSSresult		result)
{
	LOG_DEBUG(formatString("RESPONSE:dpQuerySubscribed: id=%d (err=%d)", 
				queryId, result));
	gQueryID = queryId;
	gQryCounter--;
}

void DPresponse::dpQueryUnsubscribed(uint32 queryId, PVSSresult		result)
{
	LOG_DEBUG(formatString("RESPONSE:dpQueryUnsubscribed: id=%d (err=%d)", 
				queryId, result));
	gQueryID = 0;
	gQryCounter--;
}

void DPresponse::dpQueryChanged(uint32 queryId, PVSSresult		result,
									  const GCFPVDynArr&	/*DPnames*/,
									  const GCFPVDynArr&	/*DPvalues*/,
									  const GCFPVDynArr&	/*DPtypes*/)
{
	LOG_DEBUG(formatString("RESPONSE:dpQueryChanged: id=%d (err=%d)", 
				queryId, result));
	gQueryID = queryId;
	gQryCounter--;
}

  } // namespace RTDB
 } // namespace GCF
} // namespace LOFAR
