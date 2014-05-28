#include <lofar_config.h>
#include <GCF/GCF_PVTypes.h>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include "Service.h"

namespace LOFAR {
  namespace GCF {
    using namespace Common;
	namespace PAL {
   
TSAResult Service::dpCreate(const string& propName, 
                            const string& macType)
{
	return GSAService::dpCreate(propName, macType);
}

TSAResult Service::dpDelete(const string& propName)
{
	return GSAService::dpDelete(propName);
}

TSAResult Service::dpeSubscribe(const string& propName)
{
	return GSAService::dpeSubscribe(propName);
}

TSAResult Service::dpeUnsubscribe(const string& propName)
{
	return GSAService::dpeUnsubscribe(propName);
}

TSAResult Service::dpeGet(const string& propName)
{
	return GSAService::dpeGet(propName);
}

TSAResult Service::dpeSet(const string& propName, const GCFPValue& value, double timestamp, bool wantAnswer)
{
	return GSAService::dpeSet(propName, value, timestamp, wantAnswer);
}

bool Service::dpeExists(const string& propName)
{
	return GCFPVSSInfo::propExists(propName);
}

    
void Service::dpCreated(const string& propName)
{
	LOG_DEBUG(formatString("SERVICE:Property '%s' created", propName.c_str()));
}

void Service::dpDeleted(const string& propName)
{
	LOG_DEBUG(formatString("SERVICE:Property '%s' deleted", propName.c_str()));
}

void Service::dpeSubscribed(const string& propName)
{
	LOG_DEBUG(formatString("SERVICE:Property '%s' subscribed", propName.c_str()));
}

void Service::dpeUnsubscribed(const string& propName)
{
	LOG_DEBUG(formatString("SERVICE:Property '%s' unsubscribed", propName.c_str()));
}

void Service::dpeValueGet(const string& propName, const GCFPValue& value)
{
	LOG_DEBUG_STR("SERVICE:Valueget of property " << propName << " : " << 
								((GCFPVInteger *)&value)->getValue());
}

void Service::dpeValueChanged(const string& propName, const GCFPValue& /*value*/)
{
	LOG_DEBUG(formatString("SERVICE:Value of property '%s' changed", propName.c_str()));
}

void Service::dpeValueSet(const string& propName)
{
	LOG_DEBUG(formatString("SERVICE:Value of property '%s' is set", propName.c_str()));
}

  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
