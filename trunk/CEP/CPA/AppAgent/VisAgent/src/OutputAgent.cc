#include "OutputAgent.h"

namespace VisAgent
{
using namespace AppEvent;  
  
static int dum = aidRegistry_VisAgent();

//##ModelId=3E28276A0257
int OutputAgent::putHeader(DataRecord::Ref &hdr)
{
  sink().postEvent(HeaderEvent,hdr);
  return SUCCESS;
}

//##ModelId=3E28276D022A
int OutputAgent::putNextTile(VisTile::Ref &tile)
{
  ObjRef ref = tile.ref_cast<BlockableObject>();
  sink().postEvent(TileEvent,ref);
  return SUCCESS;
}

//##ModelId=3E41144B0245
string OutputAgent::sdebug(int detail, const string &prefix, const char *name) const
{
  return AppEventAgentBase::sdebug(detail,prefix,name?name:"VisAgent::Output");
}


} // namespace VisAgent
