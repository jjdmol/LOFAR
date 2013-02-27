#include <lofar_config.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/PVSS/PVSSinfo.h>
#include <Common/StringUtil.h>
#include "PerformanceResponse.h"

int		gDeleteCounter = 0;
int		gCreateCounter = 0;
int		gSetCounter = 0;
int		gGetCounter = 0;
int		NR_OF_DPS = 10;

namespace LOFAR {
  namespace GCF {
	namespace PVSS {
   
void PerformanceResponse::dpCreated(const string& /*propName*/, PVSSresult /*result*/)
{
	gCreateCounter--;
}

void PerformanceResponse::dpDeleted(const string& /*propName*/, PVSSresult /*result*/)
{
	gDeleteCounter--;
}

void PerformanceResponse::dpeSubscribed(const string& propName, PVSSresult /*result*/)
{
	LOG_DEBUG(formatString("RESPONSE:Property '%s' subscribed", propName.c_str()));
}

void PerformanceResponse::dpeUnsubscribed(const string& propName, PVSSresult /*result*/)
{
	LOG_DEBUG(formatString("RESPONSE:Property '%s' unsubscribed", propName.c_str()));
}

void PerformanceResponse::dpeValueGet(const string& /*propName*/, PVSSresult /*result*/, const GCFPValue& /*value*/)
{
	//	GCFPVInteger	theValue = ((GCFPVInteger *)&value)->getValue();
	//	if (value < 23 || value > (23 + NR_OF_DPS)) {
	//		LOG_DEBUG_STR (propName << " returned value " << theValue);
	//	}
	gGetCounter--;
}

void PerformanceResponse::dpeValueChanged(const string& propName, PVSSresult /*result*/, const GCFPValue& /*value*/)
{
	LOG_DEBUG(formatString("RESPONSE:Value of property '%s' changed", propName.c_str()));
}

void PerformanceResponse::dpeValueSet(const string& /*propName*/, PVSSresult /*result*/)
{
	gSetCounter--;
}

void PerformanceResponse::dpeSubscriptionLost(const string& propName, PVSSresult /*result*/)
{
	LOG_DEBUG(formatString("RESPONSE:Lost subscription of %s", propName.c_str()));
}

void PerformanceResponse::dpQuerySubscribed(uint32 queryId, PVSSresult /*result*/)
{
	LOG_DEBUG(formatString("RESPONSE:dpQuerySubscribed: id=", queryId));
}

void PerformanceResponse::dpQueryUnsubscribed(uint32 queryId, PVSSresult /*result*/)
{
	LOG_DEBUG(formatString("RESPONSE:dpQueryUnsubscribed: id=", queryId));
}

void PerformanceResponse::dpQueryChanged (uint32 queryId, PVSSresult result,
										  const GCFPVDynArr&	/*DPnames*/,
										  const GCFPVDynArr&	/*DPvalues*/,
										  const GCFPVDynArr&	/*DPtimes*/)
{
	LOG_DEBUG(formatString("RESPONSE:dpQueryChanges: id=%d (err=%d)", queryId, result));
}

  } // namespace PVSS
 } // namespace GCF
} // namespace LOFAR
