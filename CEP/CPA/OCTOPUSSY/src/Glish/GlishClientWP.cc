#include <sys/time.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <aips/Glish.h>
#include <aips/Arrays/Array.h>
#include <aips/Arrays/ArrayMath.h>

#include <DMI/DataRecord.h>
#include <DMI/DataField.h>
#include <DMI/DataArray.h>
#include <DMI/DynamicTypeManager.h>
#include <DMI/NCIter.h>

// GlishClientWP
#include "OCTOPUSSY/Glish/GlishClientWP.h"

static int dum = aidRegistry_Glish();
//##ModelId=3CB562BB0226
//##ModelId=3DB9369503DE
//##ModelId=3DB9369600AA


// Class GlishClientWP 

GlishClientWP::GlishClientWP (GlishSysEventSource *src, bool autostp, AtomicID wpc)
  : WorkProcess(wpc),evsrc(src),autostop_(autostp)
{
  connected = evsrc->connected();
  has_events = False;
}


//##ModelId=3DB9369201C7
GlishClientWP::~GlishClientWP()
{
  if( evsrc )
    delete evsrc;
}

void GlishClientWP::init ()
{
  dprintf(2)("init: waiting for glish start event\n");
  glish_started = False;
  while( !glish_started )
  {
    GlishSysEvent event = evsrc->nextGlishEvent();
    handleEvent(event);
  }
  dprintf(2)("init complete\n");
}

void GlishClientWP::handleEvent (GlishSysEvent &event)
{
  dprintf(2)("got event '%s'\n", event.type().c_str());
  Bool result = True;       // AIPS++ Bool for event result

  if( event.type() == "shutdown" ) // shutdown event
  {
    shutdown();
  }
  else 
  {
    try // catch all event processing exceptions
    {
      // all other events must carry a GlishRecord
      FailWhen(event.valType() != GlishValue::RECORD,"event value not a record");
      // get the record out and process stuff
      GlishRecord rec = event.val();
      GlishArray tmp;
      if( event.type() == "start" )
      {
        FailWhen( glish_started,"unexpected start event" );
        glish_started = True;
      }
      else if( event.type() == "subscribe" )
      {
        FailWhen( rec.nelements() != 2,"illegal event value" );
        String idstr; int scope;
        tmp = rec.get(0); tmp.get(idstr);
        tmp = rec.get(1); tmp.get(scope);
        HIID id(idstr);
        FailWhen( !id.size(),"null HIID in subscribe" );
        subscribe(id,scope);
      }
      else if( event.type() == "unsubscribe" )
      {
        FailWhen( rec.nelements() != 1,"illegal event value" );
        String idstr; 
        tmp = rec.get(0); tmp.get(idstr);
        HIID id(idstr);
        FailWhen( !id.size(),"null HIID in unsubscribe" );
        unsubscribe(id);
      }
      else if( event.type() == "send" )
      {
        FailWhen(!glish_started,"got send event before start event");
        String tostr; 
        FailWhen(!rec.attributeExists("to"),"missing 'to' attribute");
        tmp = rec.getAttribute("to"); tmp.get(tostr);
        HIID to(tostr);
        FailWhen(!to.size(),"bad 'to' attribute");
        AtomicID wpi,process=AidLocal,host=AidLocal;
        if( to.size() > 1 )  wpi = to[1];
        if( to.size() > 2 )  process = to[2];
        if( to.size() > 3 )  host = to[3];
        MessageRef ref = glishRecToMessage(rec);
        setState(ref->state());
        dprintf(4)("sending message: %s\n",ref->sdebug(10).c_str());
        send(ref,MsgAddress(to[0],wpi,process,host));
      }
      else if( event.type() == "publish" )
      {
        FailWhen(!glish_started,"got publish event before start event");
        int scope;
        FailWhen( !rec.attributeExists("scope"),"missing 'scope' attribute");
        tmp = rec.getAttribute("scope"); tmp.get(scope);
        MessageRef ref = glishRecToMessage(rec);
        setState(ref->state());
        dprintf(4)("publishing message: %s\n",ref->sdebug(10).c_str());
        publish(ref,scope);
      }
      else if( event.type() == "log" )
      {
        FailWhen(!glish_started,"got log event before start event");
        FailWhen( rec.nelements() != 3,"illegal event value" );
        String msg,typestr; int level;
        tmp = rec.get(0); tmp.get(msg);
        tmp = rec.get(1); tmp.get(level);
        tmp = rec.get(2); tmp.get(typestr);
        AtomicID type(typestr);
        log(msg,level,type);
      }
      else
        Throw("unknown event");
    } // end try 
    catch ( std::exception &exc ) 
    {
      dprintf(1)("error processing glish event, ignoring: %s\n",exc.what());
      result = False;
    }
  }
  // if we fell through to here, return the reply
  if( evsrc->replyPending() )
    evsrc->reply(GlishArray(result));
}

//##ModelId=3CBA97E70232
bool GlishClientWP::start ()
{
  fd_set fdset;
  FD_ZERO(&fdset);
  if( evsrc->addInputMask(&fdset) )
  {
    for( int fd=0; fd<FD_SETSIZE; fd++ )
      if( FD_ISSET(fd,&fdset) )
      {
        dprintf(2)("adding input for fd %d\n",fd);
        addInput(fd,EV_FDREAD);
      }
  }
  else
  {
    dprintf(2)("no input fds indicated by GlishEventSource\n");
  }
  // add a timeout to keep checking for connectedness
  addTimeout(2.0,HIID(),EV_CONT);
  
  return False;
}

//##ModelId=3CBABEA10165
void GlishClientWP::stop ()
{
  if( evsrc && connected )
    evsrc->postEvent("exit",GlishValue());
}

//##ModelId=3CBACB920259
int GlishClientWP::input (int , int )
{
  if( !evsrc->connected() )
  {
    // got disconnected?
    if( connected )
      dprintf(1)("disconnected from Glish process\n");
    shutdown();
  }
  else
  {
    GlishSysEvent event;
    // The event loop
    // loop until the mex # of events is reached, or no more events
    for( int i=0; i < MaxEventsPerPoll; i++ )
      if( !evsrc->nextGlishEvent(event,0) )
      {
        has_events=False; // no events? reset flag and exit
        break;
      }
      else   // else process the event
      {
        handleEvent(event);
      } // end of event loop
  }
  return Message::ACCEPT;
}

//##ModelId=3CBACFC6013D
int GlishClientWP::timeout (const HIID &)
{
  // fake an input all to check for connectedness, etc.
  return input(0,0);
}

//##ModelId=3CB5622B01ED
int GlishClientWP::receive (MessageRef &mref)
{
  // if no connection, then just ignore it
  if( !evsrc->connected() )
  {
    dprintf(2)("not connected, ignoring [%s]\n",mref->sdebug(1).c_str());
    return Message::ACCEPT;
  }
  // wrap the message into a record and post it
  GlishRecord rec;
  if( messageToGlishRec(mref.deref(),rec) )
  {
    evsrc->postEvent("receive",rec);
  }
  else
  {
    dprintf(1)("unable to convert [%s] to glish record\n",mref->sdebug(1).c_str());
  }
  return Message::ACCEPT;
}

//##ModelId=3CB57C8401D6
MessageRef GlishClientWP::glishRecToMessage (const GlishRecord &glrec)
{
  // get message attributes
  FailWhen( !glrec.attributeExists("id") ||
            !glrec.attributeExists("priority"),"missing 'id' or 'priority' attribute");
  String idstr; 
  int priority,state=0;
  GlishArray tmp;
  tmp = glrec.getAttribute("id"); tmp.get(idstr);
  tmp = glrec.getAttribute("priority"); tmp.get(priority);
  if( glrec.attributeExists("state") )
  {
    tmp = glrec.getAttribute("state"); tmp.get(state);
  }
  // setup message & ref
  HIID id(idstr);
  Message &msg = *new Message(id,priority);
  MessageRef ref(msg,DMI::ANON|DMI::WRITE);
  ref().setState(state);
  // do we have a payload?
  if( glrec.attributeExists("payload") )
  {
    String typestr; 
    tmp = glrec.getAttribute("payload"); tmp.get(typestr);
    TypeId tid(typestr);
    // data record is unwrapped explicitly
    if( tid == TpDataRecord )
    {
      DataRecord *rec = new DataRecord;
      msg <<= rec;
      glishToRec(glrec,*rec);
    }
    else // else try to unblock the object
    {
      msg <<= blockRecToObject(glrec);
    }
  }
  // do we have a data block as well?
  if( glrec.attributeExists("datablock") )
  {
    Array<uChar> data;
    tmp = glrec.getAttribute("datablock"); tmp.get(data);
    size_t sz = data.nelements();
    SmartBlock *block = new SmartBlock(sz);
    msg <<= block;
    if( sz )
    {
      bool del;
      const uChar *pdata = data.getStorage(del);
      memcpy(block->data(),pdata,sz);
      data.freeStorage(pdata,del);
    }
  }
  return ref;
}

//##ModelId=3CB57CA00280
bool GlishClientWP::messageToGlishRec (const Message &msg, GlishRecord &glrec)
{
  glrec.addAttribute("id",GlishArray(msg.id().toString()));
  glrec.addAttribute("to",GlishArray(msg.id().toString()));
  glrec.addAttribute("from",GlishArray(msg.from().toString()));
  glrec.addAttribute("priority",GlishArray(msg.priority()));
  glrec.addAttribute("state",GlishArray(msg.state()));
  // convert payload
  if( msg.payload().valid() )
  {
    TypeId tid = msg.payload()->objectType();
    glrec.addAttribute("payload",GlishArray(tid.toString()));
    // records are converted
    if( tid == TpDataRecord )
    {
      const DataRecord *rec = dynamic_cast<const DataRecord *>(msg.payload().deref_p());
      Assert(rec);
      recToGlish(*rec,glrec);
    }
    else
    {
      objectToBlockRec(msg.payload().deref(),glrec);
    }
  }
  // copy data block, if any
  if( msg.block().valid() )
  {
    size_t sz = msg.datasize();
    glrec.addAttribute("datasize",GlishArray((int)sz));
    if( sz )
      glrec.addAttribute("data",GlishArray(Array<uChar>(IPosition(1,sz),
		static_cast<uChar*>(const_cast<void*>(msg.data())),COPY)));
  }
  
  return True;
}

// Additional Declarations
//##ModelId=3DB9369202CC
void GlishClientWP::recToGlish (const DataRecord &rec, GlishRecord& glrec)
{
  DataRecord::Iterator iter = rec.initFieldIter();
  HIID id;
  TypeId type;
  int size;
  while( rec.getFieldIter(iter,id,type,size) )
  {
    #ifdef USE_THREADS
    // obtain mutex on the datafield
    Thread::Mutex::Lock lock(rec[id].as_DataField().mutex());
    #endif
    string name = id.toString();
    GlishRecord subrec;
    bool mapped = True;
    // subrecords are recursively expanded
    if( type == TpDataRecord )
    {
      if( size == 1 )  // one record mapped directly
      {
        recToGlish(rec[id].as_DataRecord(),subrec);
      }
      else // array of records mapped as record of records
      {
        subrec.addAttribute("fieldsize",GlishArray(size));
        for( int i=0; i<size; i++ )
        {
          GlishRecord subsubrec;
          recToGlish(rec[id][i].as_DataRecord(),subsubrec);
          char num[32];
          sprintf(num,"%d",i);
          subrec.add(num,subsubrec);
        }
      }
      glrec.add(name,subrec);
    }
    else if( TypeInfo::isNumeric(type) )
    {
      switch( type.id() )
      {
        // primitive numeric types passed as arrays
        // (we cast away const, but that's OK since Array is constructed with COPY storage)
        case Tpbool_int:
            glrec.add(name,Array<Bool>(IPosition(1,size),const_cast<Bool*>(rec[id].as_bool_p()),COPY));
            break;
        case Tpuchar_int:
            glrec.add(name,Array<uChar>(IPosition(1,size),const_cast<uChar*>(rec[id].as_uchar_p()),COPY));
            break;
        case Tpshort_int:
            glrec.add(name,Array<Short>(IPosition(1,size),const_cast<Short*>(rec[id].as_short_p()),COPY));
            break;
        case Tpint_int:
        {
            // adjust indices for Index fields
            Array<Int> arr(IPosition(1,size),const_cast<Int*>(rec[id].as_int_p()),COPY);
            if( id[id.size()-1] == AidIndex )
              arr += 1;
            glrec.add(name,arr);
            break;
        }
        case Tpfloat_int:
            glrec.add(name,Array<Float>(IPosition(1,size),const_cast<Float*>(rec[id].as_float_p()),COPY));
            break;
        case Tpdouble_int:
            glrec.add(name,Array<Double>(IPosition(1,size),const_cast<Double*>(rec[id].as_double_p()),COPY));
            break;
        case Tpfcomplex_int:
            glrec.add(name,Array<Complex>(IPosition(1,size),const_cast<Complex*>(rec[id].as_fcomplex_p()),COPY));
            break;
        case Tpdcomplex_int:
            glrec.add(name,Array<DComplex>(IPosition(1,size),const_cast<DComplex*>(rec[id].as_dcomplex_p()),COPY));
            break;
        default:
            mapped = False; // non-supported type
      }
    } // endif( isNumeric(type) )
    else if( TypeInfo::isArray(type) )
    {
      if( size == 1 )
      {
        switch( type.id() )
        {
          // primitive numeric types passed as arrays
          // (we cast away const, but that's OK since Array is constructed with COPY storage)
          case Tpbool_int:
          {
              Array<Bool> arr;
              arr = rec[id].as_Array_bool(); // make copy of array
              glrec.add(name,arr);
              break;
          }
          case Tpuchar_int:
          {
              Array<Int> arr;
              arr = rec[id].as_Array_int(); // make copy of array
              glrec.add(name,arr);
              break;
          }
          case Tpshort_int:
          {
              Array<Short> arr;
              arr = rec[id].as_Array_short(); // make copy of array
              glrec.add(name,arr);
              break;
          }
          case Tpint_int:
          {
              Array<int> arr;
              arr = rec[id].as_Array_int(); // make copy of array
              // adjust indices for Index fields
              if( id[id.size()-1] == AidIndex )
                arr += 1;
              glrec.add(name,arr);
              break;
          }
          case Tpfloat_int:
          {
              Array<Float> arr;
              arr = rec[id].as_Array_float(); // make copy of array
              glrec.add(name,arr);
              break;
          }
          case Tpdouble_int:
          {
              Array<Double> arr;
              arr = rec[id].as_Array_double(); // make copy of array
              glrec.add(name,arr);
              break;
          }
          case Tpfcomplex_int:
          {
              Array<Complex> arr;
              arr = rec[id].as_Array_fcomplex(); // make copy of array
              glrec.add(name,arr);
              break;
          }
          case Tpdcomplex_int:
          {
              Array<DComplex> arr;
              arr = rec[id].as_Array_dcomplex(); // make copy of array
              glrec.add(name,arr);
              break;
          }
//string        case Tpstring_int:
//string        {
//string          // special case: convert string field to AIPS++ Strings
//string          Array_string arr = rec[id].as_Array_string();
//string          IPosition shape = arr.shape();
//string          int n = shape.product();
//string          bool del;
//string          const string *from = arr.getStorage(del);
//string          String *to = new String[n];
//string          for( int i=0; i<n; i++ )
//string            to[i] = from[i];
//string          glrec.add(name,Array<String>(shape,to,TAKE_OVER));
//string          arr.freeStorage(from,del);
//string        }
          default:
            mapped = False; // non-supported type
        }
      }
      else
      { // multi-array fields not yet supported
        mapped = False;
      }
    } // endif( isArray(type) )
    else if( type == Tpstring )
    {
      // convert vector of strings into Array of Strings
      NCConstIter_string iter = rec[id];
      Array<String> arr(IPosition(1,size));
      for( int i=0; i<size; i++ )
        arr(IPosition(1,i)) = iter.next();
      glrec.add(name,arr);
      // special case: convert string field to AIPS++ Strings
//      String str = rec[id][0].as_string();
//      glrec.add(name,str);
//string    String *storage = new String[size];
//string    const string *ptr = &rec[id];
//string    for( int i=0; i<size; i++ )
//string      storage[i] = *ptr++;
//string    glrec.add(name,Array<String>(IPosition(1,size),storage,TAKE_OVER));
    }
    else
      mapped = False;
    
    // if field was not mapped, pass it in as a block rec
    if( !mapped )
    {
      objectToBlockRec(rec[id].as_DataField(),subrec);
      glrec.add(name,subrec);
    }
  } // end of iteration over fields
}

// helper function to create a DataField from a GlishArray
template<class T> 
static void newDataField( const GlishArray &arr,DataField &fld,TypeId tid,int n)
{
  Array<T> array;
  arr.get(array);
  bool del;
  const T * data = array.getStorage(del);
  fld.init(tid,n,static_cast<const void *>(data));
  array.freeStorage(data,del);
}

template<class T> 
static void newDataArray( const GlishArray &arr,DataField &fld)
{
  Array<T> array;
  arr.get(array);
  fld[0] <<= new DataArray(array);
}


//##ModelId=3DB936940034
void GlishClientWP::glishToRec (const GlishRecord &glrec, DataRecord& rec)
{
  for( uint i=0; i < glrec.nelements(); i++ )
  {
    HIID id = glrec.name(i);
    GlishValue val = glrec.get(i);
    // arrays mapped to DataFields (NB: should also be DataArrays, once
    // those are ready)
    if( val.type() == GlishValue::ARRAY )
    {
      GlishArray arr = val;
      IPosition shape = arr.shape();
      DataField *fld = new DataField;
      rec[id] <<= fld;
      
      if( shape.nelements() == 1 ) // 1D array converted to a DataField
      {
        int n = arr.shape().product();
        switch( arr.elementType() )
        {
          case GlishArray::BOOL:      
              newDataField<Bool>(val,*fld,Tpbool,n);
              break;
          case GlishArray::BYTE:
              newDataField<uChar>(val,*fld,Tpuchar,n);
              break;
          case GlishArray::SHORT:
              newDataField<Short>(val,*fld,Tpshort,n);
              break;
          case GlishArray::INT:
          {
              // INT handled separately to adjust indexing
              Array<Int> array;
              arr.get(array);
              bool del;
              if( id[id.size()-1] == AidIndex )
                array -= 1;
              const int * data = array.getStorage(del);
              fld->init(Tpint,n,static_cast<const void *>(data));
              array.freeStorage(data,del);
              break;
          }
          case GlishArray::FLOAT:
              newDataField<Float>(val,*fld,Tpfloat,n);
              break;
          case GlishArray::DOUBLE:
              newDataField<Double>(val,*fld,Tpdouble,n);
              break;
          case GlishArray::COMPLEX:
              newDataField<Complex>(val,*fld,Tpfcomplex,n);
              break;
          case GlishArray::DCOMPLEX:
              newDataField<DComplex>(val,*fld,Tpdcomplex,n);
              break;
          case GlishArray::STRING: 
          {
              // special case: convert AIPS++ Strings to strings
              Array<String> array;
              arr.get(array);
              bool del; const String *data = array.getStorage(del);
              fld->init(Tpstring,n);
              NCIter_string iter = (*fld)[HIID()];
              for( int i=0; i<n; i++ )
                iter.next(data[i]);
              array.freeStorage(data,del);
              break;
          }
          default:
              Throw("unsupported array element type");
        }
      } // end of 1D array
      else // n-D arrays map to DataArrays
      {
        switch( arr.elementType() )
        {
          case GlishArray::BOOL:      
              newDataArray<Bool>(val,*fld);
              break;
          case GlishArray::BYTE:
              newDataArray<uChar>(val,*fld);
              break;
          case GlishArray::SHORT:
              newDataArray<Short>(val,*fld);
              break;
          case GlishArray::INT:
              newDataArray<Int>(val,*fld);
              if( id[id.size()-1] == AidIndex )
              {
                Array_int arr = (*fld)[0].as_Array_int();
                arr -= 1;
              }
              break;
          case GlishArray::FLOAT:
              newDataArray<Float>(val,*fld);
              break;
          case GlishArray::DOUBLE:
              newDataArray<Double>(val,*fld);
              break;
          case GlishArray::COMPLEX:
              newDataArray<Complex>(val,*fld);
              break;
          case GlishArray::DCOMPLEX:
              newDataArray<DComplex>(val,*fld);
              break;
          case GlishArray::STRING: 
          {
//string            // special case: convert AIPS++ Strings to strings
//string            Array<String> array;
//string            arr.get(array);
//string            bool del; const String *from = array.getStorage(del);
//string            (*fld)[0] <<= new DataArray(Tpstring,arr.shape());
//string            string * ptr = (*fld)[0].as_string_wp();
//string            int n = array.shape().product();
//string            for( int i=0; i<n; i++ )
//string              *ptr++ = from[i];
//string            array.freeStorage(from,del);
//string            break;
          }
          default:
              Throw("unsupported array element type");
        }
      }
    }
    else //  initialize a record
    {
      GlishRecord glsubrec = val;
      // interpret as field of several sub-records
      if( glsubrec.attributeExists("fieldsize")  )
      {
        int nrec;
        GlishArray tmp;
        tmp = glsubrec.getAttribute("fieldsize"); tmp.get(nrec);
        rec[id] <<= new DataField(TpDataRecord,nrec);
        for( int i=0; i<nrec; i++ )
        {
          GlishValue val = glsubrec.get(i);
          if( val.type() != GlishValue::RECORD )
          {
            dprintf(2)("warning: field is not a sub-record, ignoring\n");
            continue;
          }
          glishToRec(val,rec[id][i]);
        }
      }
      else // interpret as single record
      {
        DataRecord *subrec = new DataRecord;
        rec[id] <<= subrec;
        glishToRec(glsubrec,*subrec);
      }
    }
  }
}

//##ModelId=3DB93695024D
BlockableObject * GlishClientWP::blockRecToObject (const GlishRecord &rec )
{
  FailWhen( !rec.attributeExists("blocktype"),"missing 'blocktype' attribute" );
  String typestr;
  GlishArray tmp = rec.getAttribute("blocktype"); tmp.get(typestr);
  TypeId tid(typestr);
  FailWhen( !tid,"illegal blocktype "+static_cast<string>(typestr) );
  // extract blockset form record
  BlockSet set;
  for( uint i=0; i<rec.nelements(); i++ )
  {
    Array<uChar> arr;
    tmp = rec.get(i); tmp.get(arr);
    // create SmartBlock and copy data from array
    size_t sz = arr.nelements();
    SmartBlock *block = new SmartBlock(sz);
    set.pushNew().attach(block,DMI::WRITE|DMI::ANON);
    if( sz )
    {
      Bool del;
      const uChar * data = arr.getStorage(del);
      memcpy(block->data(),data,sz);
      arr.freeStorage(data,del);
    }
  }
  // create object & return
  return DynamicTypeManager::construct(tid,set);
}

//##ModelId=3DB936930231
void GlishClientWP::objectToBlockRec (const BlockableObject &obj,GlishRecord &rec )
{
  BlockSet set;
  obj.toBlock(set);
  rec.addAttribute("blocktype",GlishArray(obj.objectType().toString()));
  int i=0;
  while( set.size() )
  {
    char num[32];
    sprintf(num,"%d",i++);
    rec.add("num",Array<uChar>(IPosition(1,set.front()->size()),
                static_cast<uChar*>(const_cast<void*>(set.front()->data())),COPY));
    set.pop();
  }
}

//##ModelId=3DB9369900E1
void GlishClientWP::shutdown ()
{
  dprintf(1)("shutting down\n");
  connected = False;
  setState(-1);
  removeInput(-1);
  removeTimeout("*");
  if( autostop() )
  {
    dprintf(1)("autostop is on: stopping the system\n");
    dsp()->stopPolling();
  }
  else
  {
    dprintf(1)("detaching\n");
    detachMyself();
  }
}

GlishClientWP * makeGlishClientWP (int argc,const char *argv[] )
{
  // stupid glish wants non-const argv
  GlishSysEventSource *evsrc = 
      new GlishSysEventSource(argc,const_cast<char**>(argv));
  AtomicID wpc = AidGlishClientWP;
  // scan arguments for an override
  string wpcstr;
  for( int i=1; i<argc; i++ )
  {
    if( string(argv[i]) == "-wpc" && i < argc-1 )
    {
      wpcstr = argv[i+1];
      break;
    }
  }
  if( wpcstr.length() )
    wpc = AtomicID(wpcstr);
  return new GlishClientWP(evsrc,wpc);
}


