#include <lofar_config.h>
#include <GCF/GCF_PVTypes.h>
#include <GCF/PVSS/PVSSinfo.h>
#include "Response.h"

namespace LOFAR {
  namespace GCF {
    using namespace Common;
	namespace PVSS {
   
void Response::dpCreated(const string& propName)
{
	LOG_DEBUG(formatString("RESPONSE:Property '%s' created", propName.c_str()));
}

void Response::dpDeleted(const string& propName)
{
	LOG_DEBUG(formatString("RESPONSE:Property '%s' deleted", propName.c_str()));
}

void Response::dpeSubscribed(const string& propName)
{
	LOG_DEBUG(formatString("RESPONSE:Property '%s' subscribed", propName.c_str()));
}

void Response::dpeUnsubscribed(const string& propName)
{
	LOG_DEBUG(formatString("RESPONSE:Property '%s' unsubscribed", propName.c_str()));
}

void Response::dpeValueGet(const string& propName, const GCFPValue& value)
{
	LOG_DEBUG_STR("RESPONSE:Valueget of property " << propName << " : " << 
								((GCFPVInteger *)&value)->getValue());
}

void Response::dpeValueChanged(const string& propName, const GCFPValue& /*value*/)
{
	LOG_DEBUG(formatString("RESPONSE:Value of property '%s' changed", propName.c_str()));
}

void Response::dpeValueSet(const string& propName)
{
	LOG_DEBUG(formatString("RESPONSE:Value of property '%s' is set", propName.c_str()));
}

void Response::dpeSubscriptionLost(const string& propName)
{
	LOG_DEBUG(formatString("RESPONSE:Lost subscription of %s", propName.c_str()));
}

void Response::dpQuerySubscribed(uint32 queryId)
{
	LOG_DEBUG(formatString("RESPONSE:dpQuerySubscribed: id=", queryId));
}

  } // namespace PVSS
 } // namespace GCF
} // namespace LOFAR
