#include <lofar_config.h>
#include <GCF/GCF_PVTypes.h>
#include <GCF/PVSS/PVSSinfo.h>
#include "PerformanceResponse.h"

int		gDeleteCounter = 0;
int		gCreateCounter = 0;
int		gSetCounter = 0;
int		gGetCounter = 0;
int		NR_OF_DPS = 10;
bool	gValidate = false;

namespace LOFAR {
  namespace GCF {
    using namespace Common;
	namespace PVSS {
   
void PerformanceResponse::dpCreated(const string& /*propName*/)
{
	gCreateCounter--;
}

void PerformanceResponse::dpDeleted(const string& /*propName*/)
{
	gDeleteCounter--;
}

void PerformanceResponse::dpeSubscribed(const string& propName)
{
	LOG_DEBUG(formatString("RESPONSE:Property '%s' subscribed", propName.c_str()));
}

void PerformanceResponse::dpeUnsubscribed(const string& propName)
{
	LOG_DEBUG(formatString("RESPONSE:Property '%s' unsubscribed", propName.c_str()));
}

void PerformanceResponse::dpeValueGet(const string& propName, const GCFPValue& value)
{
	if (gValidate) {
	//	GCFPVInteger	theValue = ((GCFPVInteger *)&value)->getValue();
	//	if (value < 23 || value > (23 + NR_OF_DPS)) {
	//		LOG_DEBUG_STR (propName << " returned value " << theValue);
	//	}
	}
	gGetCounter--;
}

void PerformanceResponse::dpeValueChanged(const string& propName, const GCFPValue& /*value*/)
{
	LOG_DEBUG(formatString("RESPONSE:Value of property '%s' changed", propName.c_str()));
}

void PerformanceResponse::dpeValueSet(const string& /*propName*/)
{
	gSetCounter--;
}

void PerformanceResponse::dpeSubscriptionLost(const string& propName)
{
	LOG_DEBUG(formatString("RESPONSE:Lost subscription of %s", propName.c_str()));
}

void PerformanceResponse::dpQuerySubscribed(uint32 queryId)
{
	LOG_DEBUG(formatString("RESPONSE:dpQuerySubscribed: id=", queryId));
}

  } // namespace PVSS
 } // namespace GCF
} // namespace LOFAR
