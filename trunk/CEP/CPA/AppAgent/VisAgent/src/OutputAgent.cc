#include "OutputAgent.h"

namespace VisAgent
{
using namespace AppEvent;  
  
static int dum = aidRegistry_VisAgent();

DataStreamMap & OutputAgent::datamap = datamap_VisAgent;

//##ModelId=3E4143600221
OutputAgent::OutputAgent (const HIID &initf)
  : AppEventAgentBase(initf) 
{
  datamap_VisAgent_init();
}

//##ModelId=3E41436101A6
OutputAgent::OutputAgent (AppEventSink &sink, const HIID &initf)
  : AppEventAgentBase(sink,initf) 
{
  datamap_VisAgent_init();
}

//##ModelId=3E50FAF103A1
OutputAgent::OutputAgent(AppEventSink *sink, int dmiflags, const HIID &initf)
  : AppEventAgentBase(sink,dmiflags,initf) 
{
  datamap_VisAgent_init();
}

//##ModelId=3EB244510086
int OutputAgent::put (int type, const ObjRef::Xfer &ref)
{
  const DataStreamMap::Entry &entry = datamap.find(type);
  FailWhen(!entry.code,"unknown data object code");
  sink().postEvent(entry.event,ref);
  return SUCCESS;
}

//##ModelId=3E28276A0257
int OutputAgent::putHeader (const DataRecord::Ref::Xfer &hdr)
{
  return put(HEADER,hdr);
}

//##ModelId=3E28276D022A
int OutputAgent::putNextTile (const VisTile::Ref::Xfer &tile)
{
  return put(DATA,tile);
}

int OutputAgent::putFooter (const DataRecord::Ref::Xfer &ftr)
{
  return put(FOOTER,ftr);
}

//##ModelId=3E41144B0245
string OutputAgent::sdebug(int detail, const string &prefix, const char *name) const
{
  return AppEventAgentBase::sdebug(detail,prefix,name?name:"VisAgent::Output");
}


} // namespace VisAgent
