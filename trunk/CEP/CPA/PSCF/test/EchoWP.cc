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
static HIID MsgPing(AidPing),MsgPong(AidPong);
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
  if( !pcount )
    subscribe("Ping");
  else
    subscribe("MsgHello.EchoWP");
  //## end EchoWP::init%3C7F884A007D.body
}

void EchoWP::start ()
{
  //## begin EchoWP::start%3C7E4AC70261.body preserve=yes
  WorkProcess::start();
  if( pcount<0 )
    addTimeout(5.0,0);
  else if( pcount>0 )
    sendPing();
  //## end EchoWP::start%3C7E4AC70261.body
}

int EchoWP::receive (MessageRef& mref)
{
  //## begin EchoWP::receive%3C7E49AC014C.body preserve=yes
  dprintf(2)("received %s\n",mref.debug(10));
  if( mref->id() == MsgPing )
  {
    // privatize message & payload
    Message & msg = mref.privatize(DMI::WRITE,1);
    dprintf(2)("with ping count=%d\n",msg["Count"].as_int());
    // timestamp the reply
    msg["Reply.Timestamp"] = Timestamp();
    // invert the data block if it's there
    if( msg["Invert"].as_bool() )
    {
      msg["Data"].privatize(DMI::WRITE);
      int sz = msg["Data"].size();
      int *data = &msg["Data"];
      dprintf(2)("inverting %d ints at %x\n",sz,(int)data);
      for( int i=0; i<sz; i++,data++ )
        *data = ~*data;
    }
    msg.setId(MsgPong);
    dprintf(2)("replying with %s\n",msg.debug(1));
    send(mref,msg.from());
  }
  else if( mref->id() == MsgPong )
  {
    if( !pcount )
      exit(0);
    sendPing();
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
void EchoWP::sendPing ()
{
  if( pcount )
  {
    Message &msg = *new Message(MsgPing,new DataRecord,DMI::ANON|DMI::WRITE);
    msg["Timestamp"] = Timestamp();
    msg["Invert"] = (bool)True;
    msg["Data"] <<= new DataField(Tpint,0x100000);
    msg["Count"] = pcount;
    int sz = msg["Data"].size();
    int *data = &msg["Data"];
    dprintf(2)("filling %d ints at %x\n",sz,(int)data);
    for( int i=0; i<sz; i++ )
      data[i] = 0x07070707;
    dprintf(2)("ping %d, publishing %s\n",pcount,msg.debug(1));
    dprintf(2)("ping count in message is %d\n",msg["Count"].as_int());
    MessageRef ref(msg,DMI::ANON|DMI::WRITE);
    publish(ref);
    pcount--;
  }
}
  //## end EchoWP%3C7E498E00D1.declarations
//## begin module%3C7E49E90399.epilog preserve=yes
//## end module%3C7E49E90399.epilog
