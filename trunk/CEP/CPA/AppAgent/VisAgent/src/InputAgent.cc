#include "InputAgent.h"

// ensures initialization of the VisAgent datamaps
void VisAgent::datamap_VisAgent_init ()
{
  Thread::Mutex::Lock lock;
  if( !datamap_VisAgent.isInitialized(lock) )
  {
    datamap_VisAgent.initialize(DataEventMask);
    datamap_VisAgent
            .add(HEADER,HeaderEvent,TpDataRecord,False)
            .add(DATA,TileEvent,TpVisTile,True)
            .add(FOOTER,FooterEvent,TpDataRecord,False);
  }
}

namespace VisAgent 
{

using namespace AppEvent;
  
static int dum = aidRegistry_VisAgent();

DataStreamMap datamap_VisAgent;

//##ModelId=3EB2434C03A0
DataStreamMap & InputAgent::datamap = datamap_VisAgent;

//##ModelId=3E41433F01DD
InputAgent::InputAgent (const HIID &initf)
  : AppEventAgentBase(initf)
{
  datamap_VisAgent_init();
}

//##ModelId=3E41433F037E
InputAgent::InputAgent (AppEventSink &sink, const HIID &initf)
  : AppEventAgentBase(sink,initf)
{
  datamap_VisAgent_init();
}

//##ModelId=3E50FAB702CB
InputAgent::InputAgent (AppEventSink *sink, int dmiflags, const HIID &initf)
  : AppEventAgentBase(sink,dmiflags,initf)
{
  datamap_VisAgent_init();
}

//##ModelId=3E42350F01EB
bool InputAgent::init (const DataRecord &data)
{
  suspended_ = False;
  return AppEventAgentBase::init(data);
}


//##ModelId=3EB242F5014F
int InputAgent::getNext (HIID &id,ObjRef &ref,int expect_type,int wait)
{
  int res;
  // get any object
  if( expect_type <= 0 )
  {
    // loop forever until we get a valid data event 
    while( (res = sink().getEvent(id,ref,datamap.eventMask(),wait)) == SUCCESS )
    {
      // find this event in the data map
      const DataStreamMap::Entry &entry = datamap.find(id);
      if( entry.code ) // found valid entry?
      {
        // no data checking for this event? Return success already
        if( entry.datatype == TpNull )
          return entry.code;
        if( ref.valid() ) // got data with event? check type 
        {
          if( ref->objectType() == entry.datatype )
            return entry.code;
          // else datatype mismatch, fall through for another event
          cdebug(2)<<"datatype mismatch in event "<<id<<", dropping\n";
        }
        else // no data with event
        {
          if( !entry.data_required ) // ... and none required, OK
            return entry.code;
          // else fall through for another go
          cdebug(2)<<"missing data in event "<<id<<", dropping\n";
        }
      }
      else
      {
        cdebug(2)<<"unmapped event: "<<id<<", dropping\n";
      }
      // loop back for another try
    }
  }
  else // get a specific kind of object
  {
    const DataStreamMap::Entry &entry = datamap.find(expect_type);
    FailWhen( !entry.code,"unmapped data category "+codeToString(expect_type) );
    while( (res = sink().getEvent(id,ref,entry.event,wait)) == SUCCESS )
    {
      // no data checking for this event? Return success already
      if( entry.datatype == TpNull )
        return entry.code;
      if( ref.valid() ) // got data with event? check type 
      {
        if( ref->objectType() == entry.datatype )
          return entry.code;
        // else datatype mismatch, fall through for another event
        cdebug(2)<<"datatype mismatch in event "<<id<<", dropping\n";
      }
      else // no data with event
      {
        if( !entry.data_required ) // ... and none required, OK
          return entry.code;
        // else fall through for another go
        cdebug(2)<<"missing data in event "<<id<<", dropping\n";
      }
      // loop back for another try
    }
  }
  return res;
}

//##ModelId=3EB242F5031B
int InputAgent::hasNext () const
{
  HIID id;
  int res = sink().hasEvent(datamap.eventMask(),id);
  if( res != SUCCESS )
    return res;
  const DataStreamMap::Entry &entry = datamap.find(id);
  if( !entry.code )
    return OUTOFSEQ;
  return entry.code;
}

//##ModelId=3EB2434D0054
void InputAgent::suspend ()
{
  if( !suspended_ )
  {
    sink().postEvent(SuspendEvent);
    suspended_ = True; 
  }
}

//##ModelId=3EB2434D008D
void InputAgent::resume ()
{
  sink().postEvent(ResumeEvent);
  suspended_ = False;
}

//##ModelId=3EB2434D014F
string InputAgent::sdebug (int detail, const string &prefix, const char *name) const
{
  string out = AppEventAgentBase::sdebug(detail,prefix,name?name:"VisAgent::Input");
  if( suspended_ && ( detail > 1 || detail == -1 ) )
    out += " (suspended)";
  return out;
}


} // namespace VisAgent
