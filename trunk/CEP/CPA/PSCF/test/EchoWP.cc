//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3C7E49E90399.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3C7E49E90399.cm

//## begin module%3C7E49E90399.cp preserve=no
//## end module%3C7E49E90399.cp

//## Module: EchoWP%3C7E49E90399; Package body
//## Subsystem: Testing%3C7E494F0184
//## Source file: F:\lofar8\oms\LOFAR\cep\cpa\pscf\test\EchoWP.cc

//## begin module%3C7E49E90399.additionalIncludes preserve=no
//## end module%3C7E49E90399.additionalIncludes

//## begin module%3C7E49E90399.includes preserve=yes
#include <sys/time.h>
#include "DataRecord.h"
//## end module%3C7E49E90399.includes

// EchoWP
#include "EchoWP.h"
//## begin module%3C7E49E90399.declarations preserve=no
//## end module%3C7E49E90399.declarations

//## begin module%3C7E49E90399.additionalDeclarations preserve=yes
const HIID MsgPing("Ping"),MsgPong("Pong"),MsgHelloEchoWP(MsgHello|"EchoWP.*");
//## end module%3C7E49E90399.additionalDeclarations


// Class EchoWP 

EchoWP::EchoWP (int pingcount)
  //## begin EchoWP::EchoWP%3C7E49B60327.hasinit preserve=no
  //## end EchoWP::EchoWP%3C7E49B60327.hasinit
  //## begin EchoWP::EchoWP%3C7E49B60327.initialization preserve=yes
    : WorkProcess(AidEchoWP),pcount(pingcount)
  //## end EchoWP::EchoWP%3C7E49B60327.initialization
{
  //## begin EchoWP::EchoWP%3C7E49B60327.body preserve=yes
  blocksize = 64;
  pipeline = 1;
  invert = 1;
  fill = 0x07070707;
  msgcount = bytecount = 0;
  timecount = 0;
  ts = Timestamp::now();
  //## end EchoWP::EchoWP%3C7E49B60327.body
}


EchoWP::~EchoWP()
{
  //## begin EchoWP::~EchoWP%3C7E498E00D1_dest.body preserve=yes
  //## end EchoWP::~EchoWP%3C7E498E00D1_dest.body
}



//## Other Operations (implementation)
void EchoWP::init ()
{
  //## begin EchoWP::init%3C7F884A007D.body preserve=yes
  WorkProcess::init();
  
  config.get("bs",blocksize);
  lprintf(0,"blocksize = %d KB\n",blocksize);
  blocksize *= 1024/sizeof(int);
 
  config.get("pc",pcount); 
  lprintf(0,"pingcount = %d\n",pcount);

  config.get("pipe",pipeline);  
  lprintf(0,"pipeline = %d\n",pipeline);

  config.get("fill",fill);  
  lprintf(0,"fill = %d\n",fill);
  
  config.get("invert",invert);
  lprintf(0,"invert = %d\n",(int)invert);

  if( !pcount )
    subscribe("Ping");
  else if( pcount<0 )
  {
    subscribe("Ping");
    subscribe(MsgHelloEchoWP);
  }
  //## end EchoWP::init%3C7F884A007D.body
}

bool EchoWP::start ()
{
  //## begin EchoWP::start%3C7E4AC70261.body preserve=yes
  WorkProcess::start();
  if( pcount>0 )
    sendPing();
  return False;
  //## end EchoWP::start%3C7E4AC70261.body
}

int EchoWP::receive (MessageRef& mref)
{
  //## begin EchoWP::receive%3C7E49AC014C.body preserve=yes
  lprintf(4,"received %s\n",mref.debug(10));
  if( mref->id() == MsgPing && mref->from() != address() )
  {
    // privatize message & payload
    Message & msg = mref.privatize(DMI::WRITE,1);
    lprintf(3,"ping(%d) from %s\n",msg["Count"].as_int(),mref->from().toString().c_str());
    // timestamp the reply
    int sz = msg["Data"].size();
    msg["Reply.Timestamp"] = Timestamp();
    // invert the data block if it's there
    if( msg["Invert"].as_bool() )
    {
      int *data = &(msg.setBranch("Data",DMI::WRITE|DMI::PRIVATIZE));
      lprintf(4,"inverting %d ints at %x\n",sz,(int)data);
      for( int i=0; i<sz; i++,data++ )
        *data = ~*data;
    }
    msg.setId(MsgPong);
    lprintf(3,"replying with pong(%d)\n",msg["Count"].as_int());
//    stepCounters(sz*sizeof(int));
    send(mref,msg.from());
  }
  else if( mref->id() == MsgPong )
  {
    lprintf(3,"pong(%d) from %s\n",mref.deref()["Count"].as_int(),mref->from().toString().c_str());
    if( !pcount )
      dsp()->stopPolling();
    else
    {
      const Message &msg = mref.deref();
      stepCounters(msg["Data"].size()*sizeof(int),msg["Timestamp"]);
      sendPing();
    }
  }
  else if( mref->id().matches(MsgHelloEchoWP) )
  {
    if( mref->from() != address() ) // not our own?
    {
      lprintf(0,"found a friend: %s\n",mref->from().toString().c_str());
      bytecount = 0;
      msgcount = 0;
      ts = Timestamp::now();
      for( int i=0; i<pipeline; i++ )
        sendPing();
//      addTimeout(1.0);
    }
  }
  return Message::ACCEPT;
  //## end EchoWP::receive%3C7E49AC014C.body
}

int EchoWP::timeout (const HIID &)
{
  //## begin EchoWP::timeout%3C98CB600343.body preserve=yes
  sendPing();
  return Message::ACCEPT;
  //## end EchoWP::timeout%3C98CB600343.body
}

// Additional Declarations
  //## begin EchoWP%3C7E498E00D1.declarations preserve=yes
void EchoWP::stepCounters ( size_t sz,const Timestamp &stamp )
{
  msgcount++;
  bytecount += sz;
  double ts1 = Timestamp::now();
  timecount += ts1 - (double)stamp;
  if( ts1 - ts > 10 )
  {
    lprintf(0,"%.2f seconds elapsed since last report\n",ts1-ts);
    lprintf(0,"%ld round-trip bytes (%.2f MB/s)\n",
            bytecount,bytecount*2/(1024*1024*(ts1-ts)));
    lprintf(0,"%ld round-trips (%.1f /s)\n",
            msgcount,msgcount/(ts1-ts));
    lprintf(0,"%.3f ms average round-trip time\n",timecount/msgcount*1000);
    bytecount = msgcount = 0;
    ts = ts1;
    timecount = 0;
  }
}
    
    
void EchoWP::sendPing ()
{
  if( pcount )
  {
    Message &msg = *new Message(MsgPing,new DataRecord,DMI::ANON|DMI::WRITE);
    msg["Timestamp"] = Timestamp();
    msg["Invert"] = invert;
    msg["Data"] <<= new DataField(Tpint,blocksize);
    msg["Count"] = pcount;
    if( fill )
    {
      int sz = msg["Data"].size();
      int *data = &msg["Data"];
      lprintf(4,"filling %d ints at %x\n",sz,(int)data);
      for( int i=0; i<sz; i++ )
        data[i] = 0x07070707;
    }
    lprintf(4,"ping %d, publishing %s\n",pcount,msg.debug(1));
    lprintf(3,"sending ping(%d)\n",msg["Count"].as_int());
    MessageRef ref(msg,DMI::ANON|DMI::WRITE);
    publish(ref);
    pcount--;
//    stepCounters(sz*sizeof(int));
  }
}
  //## end EchoWP%3C7E498E00D1.declarations
//## begin module%3C7E49E90399.epilog preserve=yes
//## end module%3C7E49E90399.epilog
