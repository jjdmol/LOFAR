#include <lofar_config.h>
#include <GCF/GCF_PVTypes.h>
#include <GCF/PVSS/PVSSinfo.h>
#include "RTDB_Property.h"
#include "PropResponse.h"

namespace LOFAR {
  namespace GCF {
    using namespace Common;
	namespace RTDB {
   
void PropResponse::dpCreated(const string& /*propName*/, PVSSresult		/*result*/)
{
	ASSERT(false);
}

void PropResponse::dpDeleted(const string& /*propName*/, PVSSresult		/*result*/)
{
	ASSERT(false);
}

void PropResponse::dpeSubscribed(const string& /*propName*/, PVSSresult		/*result*/)
{
	// TODO: look at PVSSresult
	itsProperty->setSubscription(true);
}

void PropResponse::dpeUnsubscribed(const string& /*propName*/, PVSSresult		/*result*/)
{
	itsProperty->setSubscription(false);
}

void PropResponse::dpeValueGet(const string& /*propName*/, PVSSresult		result, const GCFPValue& value)
{
	itsProperty->valueGetAck(result, value);
}

void PropResponse::dpeValueChanged(const string& /*propName*/, PVSSresult		result, const GCFPValue& value)
{
	itsProperty->valueChangedAck(result, value);
}

void PropResponse::dpeValueSet(const string& /*propName*/, PVSSresult		result)
{
	itsProperty->valueSetAck(result);
}

void PropResponse::dpeSubscriptionLost(const string& /*propName*/, PVSSresult		/*result*/)
{
	itsProperty->setSubscription(false);
}

void PropResponse::dpQuerySubscribed(uint32 /*queryId*/, PVSSresult		/*result*/)
{
	ASSERT(false);
}

  } // namespace RTDB
 } // namespace GCF
} // namespace LOFAR
