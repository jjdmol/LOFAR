#include <lofar_config.h>
#include <GCF/GCF_PVTypes.h>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include "PerformanceService.h"

int		gDeleteCounter = 0;
int		gCreateCounter = 0;
int		gSetCounter = 0;
int		gGetCounter = 0;
int		NR_OF_DPS = 10;
bool	gValidate = false;

namespace LOFAR {
  namespace GCF {
    using namespace Common;
	namespace PAL {
   
TSAResult PerformanceService::dpCreate(const string& propName, 
                            const string& macType)
{
	return GSAService::dpCreate(propName, macType);
}

TSAResult PerformanceService::dpDelete(const string& propName)
{
	return GSAService::dpDelete(propName);
}

TSAResult PerformanceService::dpeSubscribe(const string& propName)
{
	return GSAService::dpeSubscribe(propName);
}

TSAResult PerformanceService::dpeUnsubscribe(const string& propName)
{
	return GSAService::dpeUnsubscribe(propName);
}

TSAResult PerformanceService::dpeGet(const string& propName)
{
	return GSAService::dpeGet(propName);
}

TSAResult PerformanceService::dpeSet(const string& propName, const GCFPValue& value, double timestamp, bool wantAnswer)
{
	return GSAService::dpeSet(propName, value, timestamp, wantAnswer);
}

bool PerformanceService::dpeExists(const string& propName)
{
	return GCFPVSSInfo::propExists(propName);
}

    
void PerformanceService::dpCreated(const string& /*propName*/)
{
	gCreateCounter--;
}

void PerformanceService::dpDeleted(const string& /*propName*/)
{
	gDeleteCounter--;
}

void PerformanceService::dpeSubscribed(const string& propName)
{
	LOG_DEBUG(formatString("SERVICE:Property '%s' subscribed", propName.c_str()));
}

void PerformanceService::dpeUnsubscribed(const string& propName)
{
	LOG_DEBUG(formatString("SERVICE:Property '%s' unsubscribed", propName.c_str()));
}

void PerformanceService::dpeValueGet(const string& propName, const GCFPValue& value)
{
	if (gValidate) {
	//	GCFPVInteger	theValue = ((GCFPVInteger *)&value)->getValue();
	//	if (value < 23 || value > (23 + NR_OF_DPS)) {
	//		LOG_DEBUG_STR (propName << " returned value " << theValue);
	//	}
	}
	gGetCounter--;
}

void PerformanceService::dpeValueChanged(const string& propName, const GCFPValue& /*value*/)
{
	gSetCounter--;
//	LOG_DEBUG(formatString("SERVICE:Value of property '%s' changed", propName.c_str()));
}

void PerformanceService::dpeValueSet(const string& /*propName*/)
{
	gSetCounter--;
}

  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
