#include "InputAgent.h"

namespace VisAgent {

using namespace AppEvent;
  
static int dum = aidRegistry_VisAgent();
    
//##ModelId=3E42350F01EB
bool InputAgent::init (const DataRecord &data)
{
  suspended_ = False;
  return AppEventAgentBase::init(data);
}


//##ModelId=3DF9FECF0310
int InputAgent::getHeader (DataRecord::Ref &hdr, int wait)
{
  HIID id;
  return sink().getEvent(id,hdr,HeaderEvent,wait);
}

//##ModelId=3DF9FECF03A9
int InputAgent::getNextTile (VisTile::Ref &tile, int wait)
{
  HIID id;
  ObjRef ref;
  int res;
  while( (res = sink().getEvent(id,ref,TileEvent,wait)) == SUCCESS )
  {
    // if object is tile, return it, else loop to try again
    if( ref.valid() && ref->objectType() == TpVisTile )
    {
      tile.xfer(ref.ref_cast<VisTile>());
      return SUCCESS;
    }
  }
  return res;
}

//##ModelId=3DF9FED0005A
int InputAgent::hasHeader () const
{
  return sink().hasEvent(HeaderEvent);
}

//##ModelId=3DF9FED00076
int InputAgent::hasTile () const
{
  return sink().hasEvent(TileEvent);
}

//##ModelId=3DF9FED00090
void InputAgent::suspend ()
{
  if( !suspended_ )
  {
    sink().postEvent(SuspendEvent);
    suspended_ = True; 
  }
}

//##ModelId=3DF9FED000AA
void InputAgent::resume ()
{
  sink().postEvent(ResumeEvent);
  suspended_ = False;
}

//##ModelId=3E41141D029F
string InputAgent::sdebug (int detail, const string &prefix, const char *name) const
{
  string out = AppEventAgentBase::sdebug(detail,prefix,name?name:"VisAgent::Input");
  if( suspended_ && ( detail > 1 || detail == -1 ) )
    out += " (suspended)";
  return out;
}


} // namespace VisAgent
