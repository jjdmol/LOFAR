#include "OctoMultiplexer.h"
#include "OctoAgent.h"
#include "DMI/DataRecord.h"

InitDebugContext(OctoAgent,"OctoAgent");
    
using namespace OctoAgentVocabulary;

DataRecord OctoAgent::_dum;

int _dum2 = aidRegistry_OctoAgent();
    
//##ModelId=3E26DA36005C
OctoAgent::OctoAgent(const HIID &mapfield)
    : proxy(0),map_field_name(mapfield)
{
}

//##ModelId=3E091DDB02F2
OctoAgent::OctoAgent (OctoMultiplexer &pxy,const HIID &mapfield)
    : proxy(0),map_field_name(mapfield)
{
  attach(pxy,mapfield);
}

//##ModelId=3E26DA370170
OctoAgent::~OctoAgent()
{
}
  
//##ModelId=3E091DDD02B8
void OctoAgent::attach (OctoMultiplexer &pxy,const HIID &mapfield)
{
  FailWhen(proxy,"already attached to multiplexer");
  cdebug(2)<<"attaching to multiplexer "<<proxy->sdebug(1)<<endl;
  proxy = &pxy;
  map_field_name = mapfield;
}


//##ModelId=3E27F30B019B
bool OctoAgent::init (const DataRecord::Ref &dataref)
{
  const DataRecord &data = *dataref;
  if( data[map_field_name].exists() )
  {
    if( data[map_field_name][FMapReceive].exists() )
      setReceiveMap(data[map_field_name][FMapReceive]);
    if( data[map_field_name][FMapPost].exists() )
      setPostMap(data[map_field_name][FMapPost]);
  }
  return True;
}


//##ModelId=3E26CAE001BA
bool OctoAgent::mapReceiveEvent(HIID &out, const HIID &in) const
{
  EMCI iter = receive_map.find(in);
  // is it in the input map?
  if( iter == receive_map.end() )
    return False;
  out = iter->second.id;
  return True;
}

//##ModelId=3E2FEAD10188
bool OctoAgent::mapPostEvent(HIID &out, const HIID &in) const
{
  EMCI iter = post_map.find(in);
  // is it in the input map?
  if( iter == post_map.end() )
    return False;
  out = iter->second.id;
  return True;
}

//##ModelId=3E096F2103B3
int OctoAgent::getEvent(HIID &id, ObjRef &data, const HIID &mask, bool wait)
{
  if( !pending_event )
  {
    // no event pending? Try polling the multiplexer
    int res = proxy->pollEvents(wait);
    // not successful? Return the code
    if( res != SUCCESS )
      return res;
    // successful? Check that it got an event for us and not someone else...
    else if( !pending_event )
      return OUTOFSEQ;
  }
  // does it match the mask? 
  if( mask.empty() || mask.matches(pending_event_id) )
  {
    // tell the proxy that we've got the event
    pending_event = False;
    proxy->claimEvent();
    // return the event
    id = pending_event_id;
    data = pending_event_data;
    cdebug(2)<<"getEvent(): "<<id<<", "<<data->sdebug(2,"  ")<<endl;
    return SUCCESS;
  }
  else
  // no, does not match the mask: retrun OUTOFSEQ then
    return OUTOFSEQ;
}

//##ModelId=3E0918BF0299
int OctoAgent::getEvent (HIID &id,DataRecord::Ref &data, const HIID &mask, bool wait)
{
  ObjRef objref;
  data.detach();
  // call generic getEvent() for all payload types
  if( getEvent(id,objref,mask,wait) )
  {
    // if payload is DataRecord, attach it to data ref, else discard
    if( objref.valid() )
    {
      if( objref->objectType() == TpDataRecord )
        data = objref.ref_cast<DataRecord>();
      else
      {
        cdebug(3)<<"discarded unsupported event payload ("<<objref->objectType()<<")\n";
      }
    }
    return True;
  }
  else
    return False;
}

//##ModelId=3E0918BF02F0
int OctoAgent::hasEvent (const HIID &mask,bool)
{
  if( !pending_event )
  {
    // no event pending? Try polling the multiplexer
    int res = proxy->pollEvents(False);
    // not successful? Return the code
    if( res != SUCCESS )
      return res;
    // successful? Check that it got an event for us and not someone else...
    else if( !pending_event )
      return OUTOFSEQ;
  }
  // does it not match the mask? If not, return OUTOFSEQ code
  if( mask.empty() || mask.matches(pending_event_id) )
    return SUCCESS;
  else
    return OUTOFSEQ;
}

//##ModelId=3E2FD67D0246
void OctoAgent::postEvent(const HIID &id, const ObjRef &data)
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
  mref() <<= data.copy();
  cdebug(3)<<"publishing as "<<mref->sdebug(2)<<", scope "<<scope<<endl;
  proxy->publish(mref,scope);
}

//##ModelId=3E0918BF034D
void OctoAgent::postEvent (const HIID &id,const DataRecord::Ref &data)
{
  postEvent(id,data.ref_cast<BlockableObject>());
}

//##ModelId=3E0A34E7020E
int OctoAgent::getDefaultScope (const DataRecord &map)
{
  // looks at map to determine the default publish or subscribe scope
  if( map[FDefaultScope].exists() )
  {
    // publish scope can be specified as string or int
    if( map[FDefaultScope].type() == Tpstring )
    {
      string str = map[FDefaultScope].as_string(); // .upcase();
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
      return map[FDefaultScope].as_int(); 
  }
  return Message::GLOBAL;
}

//##ModelId=3E0A34E801D6
int OctoAgent::resolveScope (HIID &id,int scope)
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
int OctoAgent::resolvePriority (HIID &id,int priority)
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
int OctoAgent::getDefaultPriority (const DataRecord &map)
{
  // looks at map to determine the default priority
  if( !map[FDefaultPriority].exists() )
    return Message::PRI_NORMAL;
  // if specified as a HIID, AtomicID or string, then interpret as a HIID
  // (has to end up as a single-element HIID suitable for resolvePriority(),
  // above)
  HIID id; 
  if( map[FDefaultPriority].type() == TpHIID )
    id = map[FDefaultPriority].as_HIID();
  else if( map[FDefaultPriority].type() == TpAtomicID )
    id = map[FDefaultPriority].as_AtomicID();
  else if( map[FDefaultPriority].type() == Tpstring )
    id = map[FDefaultPriority].as_string();
  // if none of those, then try to interpret as an integer
  else
    return map[FDefaultPriority].as_int(Message::PRI_NORMAL);
  // use resolvePriority() above to map to a priority constant
  int pri = resolvePriority(id,Message::PRI_NORMAL);
  // ID must be empty now, else error
  FailWhen(!id.empty(),"illegal "+FDefaultPriority.toString());
  return pri;
}

//##ModelId=3E0A295102A5
void OctoAgent::setReceiveMap (const DataRecord &map)
{
  receive_map.clear();
  // interpret Default.Scope argument
  int defscope = getDefaultScope(map);
  // translate map, and subscribe proxy to all input events
  DataRecord::Iterator iter = map.initFieldIter();
  HIID id; TypeId type; int size;
  int nevents = 0;
  while( map.getFieldIter(iter,id,type,size) )
  {
    // ignore Default.Scope argument
    if( id == FDefaultScope )
      continue;
    // all other fields must be event IDs (HIIDs)
    FailWhen(type != TpHIID || size != 1,"illegal input map entry "+id.toString());
    const HIID &event = map[id];
    int scope = resolveScope(id,defscope);
    // add to event map and subscribe
    EventMapEntry &ee = receive_map[id];
    ee.id = event; ee.scope = scope; 
    proxy->subscribe(id,scope);
    nevents++;
    cdebug(2)<<"subscribing to "<<id<<" (scope "<<scope<<") for event "<<event<<endl;
  }
  dprintf(1)("subscribed to %d input events\n",nevents);
}

//##ModelId=3E0A296A02C8
void OctoAgent::setPostMap (const DataRecord &map)
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
    HIID msgid = map[id].as_HIID();
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


//##ModelId=3E26D4A80155
void OctoAgent::cacheEvent (const HIID &id, const ObjRef &data)
{
  FailWhen(pending_event,"can't cache event while an older event is pending");
  pending_event_id = id;
  if( data.valid() )
    pending_event_data.copy(data,DMI::PRESERVE_RW);
  else
    pending_event_data.detach();
  pending_event = True;  
}


string OctoAgent::sdebug (int detail, const string &prefix, const char *name) const
{
  using Debug::append;
  using Debug::appendf;
  using Debug::ssprintf;
  
  string out;
  if( detail >= 0 ) // basic detail
  {
    appendf(out,"%s/%08x",name?name:"OctoAgent",(int)this);
  }
  if( detail >= 1 || detail == -1 )
  {
    if( pending_event )
      appendf(out,"ev:%s (%s)",pending_event_id.toString().c_str(),
          pending_event_data.valid() 
          ? pending_event_data->debug(1) : "-" );
  }
  if( detail >= 2 || detail == -2 )
  {
    appendf(out,"proxy:%s",proxy->debug(abs(detail)-1,prefix));
  }
  return out;
}
