#include "DataStreamMap.h"

DataStreamMap::Entry DataStreamMap::_dum = { 0,HIID(),0,False };
    
//##ModelId=3EB242EC03D7
//##ModelId=3EB242ED002B
DataStreamMap::DataStreamMap ()
    : initialized(false)
{
}

//##ModelId=3EB242EC03D8
bool DataStreamMap::isInitialized (Thread::Mutex::Lock &lock)
{
  lock.relock(mutex);
  if( initialized )
    lock.release();
  return initialized;
}

//##ModelId=3EB242EC03DC
void DataStreamMap::initialize (const HIID &evmask)
{ 
  initialized = true;
  event_mask = evmask; 
}

//##ModelId=3EB242EC03E1
DataStreamMap & DataStreamMap::add (int code,const HIID &event,TypeId type,bool required)
{
  Entry entry = { code,event,type,required };
  codemap[code] = entry;
  eventmap[event] = entry; 
  return *this;
}

//##ModelId=3EB242ED0010
const DataStreamMap::Entry & DataStreamMap::find (int code) const
{
  std::map<int,Entry>::const_iterator iter = codemap.find(code);
  return iter == codemap.end() ? _dum : iter->second;
}

//##ModelId=3EB242ED001E
const DataStreamMap::Entry & DataStreamMap::find (const HIID &event) const
{
  std::map<HIID,Entry>::const_iterator iter = eventmap.find(event);
  return iter == eventmap.end() ? _dum : iter->second;
}

