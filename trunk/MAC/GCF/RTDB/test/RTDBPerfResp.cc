#include <lofar_config.h>
#include <GCF/PVSS/GCF_PValue.h>
#include "RTDBPerfResp.h"

int		gDeleteCounter = 0;
int		gCreateCounter = 0;
int		gSetCounter = 0;
int		gGetCounter = 0;
int		NR_OF_DPS = 10;

namespace LOFAR {
  namespace GCF {
	namespace RTDB {
   
void RTDBPerfResp::dpCreated(const string& /*propName*/, PVSSresult /*result*/)
{
}

void RTDBPerfResp::dpDeleted(const string& /*propName*/, PVSSresult /*result*/)
{
	gDeleteCounter--;
}

void RTDBPerfResp::dpeSubscribed(const string& /*propName*/, PVSSresult /*result*/)
{
}

void RTDBPerfResp::dpeUnsubscribed(const string& /*propName*/, PVSSresult /*result*/)
{
}

void RTDBPerfResp::dpeValueGet(const string& /*propName*/, PVSSresult /*result*/, const PVSS::GCFPValue& /*value*/)
{
}

void RTDBPerfResp::dpeValueChanged(const string& /*propName*/, PVSSresult /*result*/, const PVSS::GCFPValue& /*value*/)
{
}

void RTDBPerfResp::dpeValueSet(const string& /*propName*/, PVSSresult /*result*/)
{
}

void RTDBPerfResp::dpeSubscriptionLost(const string& /*propName*/, PVSSresult /*result*/)
{
}

void RTDBPerfResp::dpQuerySubscribed(uint32 /*queryId*/, PVSSresult /*result*/)
{
}

  } // namespace RTDB
 } // namespace GCF
} // namespace LOFAR
