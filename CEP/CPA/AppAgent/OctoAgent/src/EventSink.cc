#include "EventMultiplexer.h"
#include "EventSink.h"
#include "DMI/DataRecord.h"

namespace OctoAgent
{
  
InitDebugContext(EventSink,"OctoEventSink");
    
DataRecord EventSink::_dum;

static int _dum2 = aidRegistry_OctoAgent();
    
//##ModelId=3E26DA36005C
EventSink::EventSink()
    : AppEventSink(HIID()),multiplexer(0),my_multiplex_id(-1)
{
}

//##ModelId=3E26DA3602F4
EventSink::EventSink (EventMultiplexer& mux,int flags)
    : AppEventSink(HIID()),multiplexer(0),my_multiplex_id(-1)
{
  attach(mux,flags);
}

//##ModelId=3E26DA370170
EventSink::~EventSink()
{
}
  
//##ModelId=3E091DDD02B8
void EventSink::attach (EventMultiplexer& mux,int flags)
{
  FailWhen(multiplexer,"already attached to multiplexer");
  cdebug(2)<<"attaching to multiplexer "<<mux.sdebug(1)<<endl;
  multiplexer = &mux;
  mux.addSink(this,flags);
}

//##ModelId=3E27F30B019B
bool EventSink::init (const DataRecord &data)
{
  FailWhen(!multiplexer,"no mux attached");
  cdebug(1)<<"initializing"<<endl;
  multiplexer->init();
// setup input event map
  if( data[FEventMapIn].exists() )
    setReceiveMap(data[FEventMapIn]);
  else
  {
    cdebug(1)<<"no input event map\n";
  }
// setup output event map
  if( data[FEventMapOut].exists() )
    setPostMap(data[FEventMapOut]);
  else
  {
    cdebug(1)<<"no output event map\n";
  }
  return True;
}

//##ModelId=3E50E88903AD
void EventSink::close ()
{
  if( multiplexer )
    multiplexer->close();
}

//##ModelId=3E26CAE001BA
bool EventSink::mapReceiveEvent(HIID &out, const HIID &in) const
{
  EMCI iter = receive_map.find(in);
  // is it in the input map?
  if( iter == receive_map.end() )
    return False;
  out = iter->second.id;
  return True;
}

//##ModelId=3E2FEAD10188
bool EventSink::mapPostEvent(HIID &out, const HIID &in) const
{
  EMCI iter = post_map.find(in);
  // is it in the input map?
  if( iter == post_map.end() )
    return False;
  out = iter->second.id;
  return True;
}

//##ModelId=3E096F2103B3
int EventSink::getEvent (HIID &id,ObjRef &data, const HIID &mask, int wait)
{
  FailWhen(!multiplexer,"no mux attached");
  return multiplexer->getEvent(id,data,mask,wait,my_multiplex_id);
}

//##ModelId=3E0918BF02F0
int EventSink::hasEvent (const HIID &mask)
{
  FailWhen(!multiplexer,"no mux attached");
  return multiplexer->hasEvent(mask,my_multiplex_id);
}

//##ModelId=3E2FD67D0246
void EventSink::postEvent (const HIID &id,const ObjRef::Xfer &data)
{
  cdebug(3)<<"postEvent("<<id<<")\n";
  // find event in output map
  EMCI iter = post_map.find(id);
  MessageRef mref;
  int scope;
  if( iter == post_map.end() )
  {
    // not found - do we publish with a default prefix then?
    if( !publish_unmapped_events )
    {
      cdebug(3)<<"unmapped event posted, dropping\n";
      return;
    }
    mref <<= new Message(unmapped_prefix|id,unmapped_priority);
    scope = unmapped_scope;
  }
  else // get ID and scope from map
  {
    // is it specifically mapped to null ID? ignore then
    if( iter->second.id.empty() )
    {
      cdebug(3)<<"event disabled in output map, dropping"<<endl;
      return;
    }
    mref <<= new Message(iter->second.id,iter->second.priority);
    scope = iter->second.scope;
  }
  // attach payload to message
  mref().payload() = data;
  cdebug(3)<<"publishing as "<<mref->sdebug(2)<<", scope "<<scope<<endl;
  multiplexer->publish(mref,scope);
}

//##ModelId=3E0A34E7020E
int EventSink::getDefaultScope (const DataRecord &map)
{
  // looks at map to determine the default publish or subscribe scope
  if( map[FDefaultScope].exists() )
  {
    // publish scope can be specified as string or int
    if( map[FDefaultScope].type() == Tpstring )
    {
      string str = map[FDefaultScope].as<string>(); // .upcase();
      if( str == "LOCAL" )
        return Message::LOCAL;
      else if( str == "HOST" )
        return Message::HOST;
      else if( str == "GLOBAL" )
        return Message::GLOBAL;
      else
      {
        Throw("illegal "+FDefaultScope.toString()+" value: "+str);
      }
    }
    else
      return map[FDefaultScope].as<int>(); 
  }
  return Message::GLOBAL;
}

//##ModelId=3E0A34E801D6
int EventSink::resolveScope (HIID &id,int scope)
{
  // determine scope by looking at first element of ID. Strip off the specifier
  // if it is valid; if not, return default scope value.
  AtomicID first = id.front();
  if( first == AidGlobal )
  {
    scope = Message::GLOBAL; id.pop_front();
  }
  else if( first == AidHost )
  {
    scope = Message::HOST; id.pop_front();
  }
  else if( first == AidLocal )
  {
    scope = Message::LOCAL; id.pop_front();
  }
  return scope;
}

//##ModelId=3E0A4C7D007A
int EventSink::resolvePriority (HIID &id,int priority)
{
  // determine scope by looking at first element of ID. Strip off the specifier
  // if it is valid; if not, return default scope value.
  AtomicID first = id.front();
  switch( first.id() )
  {
    case AidLowest_int:   priority = Message::PRI_LOWEST;  break;
    case AidLower_int:    priority = Message::PRI_LOWER;   break;
    case AidLow_int:      priority = Message::PRI_LOW;     break;
    case AidNormal_int:   priority = Message::PRI_NORMAL;  break;
    case AidHigh_int:     priority = Message::PRI_HIGH;    break;
    case AidHigher_int:   priority = Message::PRI_HIGHER;  break;
    default:              return priority;
  }
  id.pop_front();
  return priority;
}

//##ModelId=3E0A4C7B0006
int EventSink::getDefaultPriority (const DataRecord &map)
{
  // looks at map to determine the default priority
  if( !map[FDefaultPriority].exists() )
    return Message::PRI_NORMAL;
  // if specified as a HIID, AtomicID or string, then interpret as a HIID
  // (has to end up as a single-element HIID suitable for resolvePriority(),
  // above)
  HIID id; 
  if( map[FDefaultPriority].type() == TpHIID )
    id = map[FDefaultPriority].as<HIID>();
  else if( map[FDefaultPriority].type() == TpAtomicID )
    id = map[FDefaultPriority].as<AtomicID>();
  else if( map[FDefaultPriority].type() == Tpstring )
    id = map[FDefaultPriority].as<string>();
  // if none of those, then try to interpret as an integer
  else
    return map[FDefaultPriority].as<int>(Message::PRI_NORMAL);
  // use resolvePriority() above to map to a priority constant
  int pri = resolvePriority(id,Message::PRI_NORMAL);
  // ID must be empty now, else error
  FailWhen(!id.empty(),"illegal "+FDefaultPriority.toString());
  return pri;
}

//##ModelId=3E0A295102A5
void EventSink::setReceiveMap (const DataRecord &map)
{
  receive_map.clear();
  // interpret Default.Scope argument
  int defscope = getDefaultScope(map);
  // translate map, and subscribe multiplexer to all input events
  DataRecord::Iterator iter = map.initFieldIter();
  HIID event; TypeId type; int size;
  int nevents = 0;
  while( map.getFieldIter(iter,event,type,size) )
  {
    // ignore Default.Scope argument
    if( event == FDefaultScope )
      continue;
    // all other fields must be event IDs (HIIDs)
    FailWhen(type != TpHIID,"illegal input map entry "+event.toString());
    for( int i=0; i<size; i++ )
    {
      HIID msgid = map[event][i];
      int scope = resolveScope(msgid,defscope);
      // add to event map and subscribe
      EventMapEntry &ee = receive_map[msgid];
      ee.id = event; ee.scope = scope; 
      multiplexer->subscribe(msgid,scope);
      nevents++;
      cdebug(2)<<"subscribing to "<<msgid<<" (scope "<<scope<<") for event "<<event<<endl;
    }
  }
  dprintf(1)("subscribed to %d input events\n",nevents);
}

//##ModelId=3E0A296A02C8
void EventSink::setPostMap (const DataRecord &map)
{
  post_map.clear();
  // interpret Default.Scope argument
  int defscope = getDefaultScope(map);
  // interpret Default.Priority argument
  int defpri = getDefaultPriority(map);
  // translate the map
  publish_unmapped_events = False;
  DataRecord::Iterator iter = map.initFieldIter();
  HIID id; TypeId type; int size;
  int nevents = 0;
  while( map.getFieldIter(iter,id,type,size) )
  {
    // ignore Default.Scope argument
    if( id == FDefaultScope || id == FDefaultPriority )
      continue;
    FailWhen(type != TpHIID || size != 1,"illegal output map entry "+id.toString());
    HIID msgid = map[id].as<HIID>();
    int scope = resolveScope(msgid,defscope);
    int priority = resolvePriority(msgid,defpri);
    // interpret Unmapped.Prefix argument
    if( id == FUnmappedPrefix )
    {
      unmapped_prefix = msgid;
      unmapped_scope = scope;
      unmapped_priority = priority;
      publish_unmapped_events = True;
    }
    else
    {
      EventMapEntry &ee = post_map[id];
      ee.id = msgid; ee.scope = scope; ee.priority = priority;
      nevents++;
      if( msgid.empty() )
      {
        cdebug(2)<<"disabling output event "<<id<<endl;
      }
      else
      {
        cdebug(2)<<"mapping event "<<id<<" to message "<<msgid
                 <<" (S"<<scope<<"P"<<priority<<")\n";
      }
    }
  }
  dprintf(1)("mapped %d output events\n",nevents);
}

string EventSink::sdebug (int detail, const string &prefix, const char *name) const
{
  using Debug::append;
  using Debug::appendf;
  using Debug::ssprintf;
  
  string out;
  if( detail >= 0 ) // basic detail
  {
    appendf(out,"%s/%08x",name?name:"OctoEventSink",(int)this);
  }
  if( detail >= 1 || detail == -1 )
  {
    out += "mux:" + multiplexer->sdebug(abs(detail)-1,prefix);
  }
  return out;
}


} // namespace OctoAgent
