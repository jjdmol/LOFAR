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
    setReceiveMap(data[FEventMapIn].as<DataRecord>());
  else
  {
    cdebug(1)<<"no input event map\n";
  }
// setup output event map
  if( data[FEventMapOut].exists() )
    setPostMap(data[FEventMapOut].as<DataRecord>());
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
bool EventSink::mapReceiveEvent (HIID &out, const HIID &in) const
{
  // check the receive map
  EMCI iter = receive_map.find(in);
  if( iter != receive_map.end() )
  {
    out = iter->second.id; // found it
    return True;
  }
  // not explicitly mapped -- have we got a matching default prefix then
  if( default_receive_prefix.size() && in.prefixedBy(default_receive_prefix) )
  {
    // strip off prefix and return
    out = in.subId(default_receive_prefix.size());
    return True;
  }
  else
    return False;
}

//##ModelId=3E2FEAD10188
bool EventSink::mapPostEvent (HIID &out, const HIID &in) const
{
  // is the event explicitly in the post map?
  EMCI iter = post_map.find(in);
  if( iter != post_map.end() )
  {
    out = iter->second.id;
    return True;
  }
  // have we got a default prefix?
  if( default_post_prefix.size() )
  {
    out = default_post_prefix | in;
    return True;
  }
  else
    return False;
}

//##ModelId=3E8C47930062
void EventSink::solicitEvent (const HIID &mask)
{
  // check if mask is a real 
}

//##ModelId=3E096F2103B3
int EventSink::getEvent (HIID &id,ObjRef &data,const HIID &mask,int wait,HIID &source)
{
  FailWhen(!multiplexer,"no mux attached");
  return multiplexer->getEvent(id,data,mask,wait,source,my_multiplex_id);
}

//##ModelId=3E0918BF02F0
int EventSink::hasEvent (const HIID &mask)
{
  FailWhen(!multiplexer,"no mux attached");
  return multiplexer->hasEvent(mask,my_multiplex_id);
}

//##ModelId=3E2FD67D0246
void EventSink::postEvent (const HIID &id,const ObjRef::Xfer &data,
                           const HIID &dest)
{
  cdebug(3)<<"postEvent("<<id<<")\n";
  // find event in output map
  EMCI iter = post_map.find(id);
  MessageRef mref;
  int scope;
  if( iter == post_map.end() )
  {
    // not found - do we publish with a default prefix then?
    if( !default_post_prefix.size() )
    {
      cdebug(3)<<"unmapped event posted, dropping\n";
      return;
    }
    mref <<= new Message(default_post_prefix|id,default_post_priority);
    scope = default_post_scope;
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
  if( dest.size() )
  {
    cdebug(3)<<"sending "<<mref->sdebug(2)<<", to "<<dest<<endl;
    multiplexer->send(mref,dest);
  }
  else
  {
    cdebug(3)<<"publishing as "<<mref->sdebug(2)<<", scope "<<scope<<endl;
    multiplexer->publish(mref,scope);
  }
}

//##ModelId=3E8C47930088
bool EventSink::isEventBound (const HIID &id)
{
  HIID out;
  // event not mapped at all? Return False
  if( !mapPostEvent(out,id) )
    return False;
  // else true
  // BUG! actually we need a haveSubscribers() call here
  return True;
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
  // check for a default subscribe prefix
  default_receive_prefix = map[FDefaultPrefix].as<HIID>(HIID());
  default_receive_scope  = resolveScope(default_receive_prefix,defscope); 
  if( default_receive_prefix.size() )
  {
    cdebug(1)<<"subscribing with default receive prefix: "<<default_receive_prefix<<".*\n";
    multiplexer->subscribe(default_receive_prefix|AidWildcard,default_receive_scope);
  }
  // translate map, and subscribe multiplexer to all input events
  DataRecord::Iterator iter = map.initFieldIter();
  HIID event; TypeId type; int size;
  int nevents = 0;
  NestableContainer::Ref ncref;
  while( map.getFieldIter(iter,event,ncref) )
  {
    // ignore Default.Scope and Prefix arguments
    if( event == FDefaultScope || event == FDefaultPrefix )
      continue;
    const NestableContainer &nc = *ncref;
    // all other fields must be event IDs (HIIDs)
    FailWhen(nc.type() != TpHIID,"illegal input map entry "+event.toString());
    for( int i=0; i<nc.size(); i++ )
    {
      HIID msgid = nc[i];
      int scope = resolveScope(msgid,defscope);
      // add to event map and subscribe
      EventMapEntry &ee = receive_map[msgid];
      ee.id = event; ee.scope = scope; 
      multiplexer->subscribe(msgid,scope);
      nevents++;
      cdebug(2)<<"subscribing to "<<msgid<<" (scope "<<scope<<") for event "<<event<<endl;
    }
  }
  dprintf(1)("subscribed to %d explicit input events\n",nevents);
}

//##ModelId=3E0A296A02C8
void EventSink::setPostMap (const DataRecord &map)
{
  post_map.clear();
  // interpret Default.Scope argument
  int defscope = getDefaultScope(map);
  // interpret Default.Priority argument
  int defpri = getDefaultPriority(map);
  // check for a default posting prefix
  default_post_prefix   = map[FDefaultPrefix].as<HIID>(HIID());
  default_post_scope    = resolveScope(default_post_prefix,defscope); 
  default_post_priority = resolvePriority(default_post_prefix,defpri); 
  if( default_post_prefix.size() )
  {
    cdebug(1)<<"default posting prefix: "<<default_post_prefix<<endl;
  }
  // translate the map
  publish_unmapped_events = False;
  DataRecord::Iterator iter = map.initFieldIter();
  HIID id; TypeId type; int size;
  int nevents = 0;
  NestableContainer::Ref ncref;
  while( map.getFieldIter(iter,id,ncref) )
  {
    // ignore Default.Scope argument
    if( id == FDefaultScope || id == FDefaultPriority || id == FDefaultPrefix )
      continue;
    const NestableContainer &nc = *ncref;
    FailWhen(nc.type() != TpHIID || nc.size() != 1,"illegal output map entry "+id.toString());
    HIID msgid = nc[HIID()].as<HIID>();
    int scope = resolveScope(msgid,defscope);
    int priority = resolvePriority(msgid,defpri);
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
