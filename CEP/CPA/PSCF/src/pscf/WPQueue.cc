//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3C7B7F300007.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3C7B7F300007.cm

//## begin module%3C7B7F300007.cp preserve=no
//## end module%3C7B7F300007.cp

//## Module: WPQueue%3C7B7F300007; Package body
//## Subsystem: PSCF%3C5A73670223
//## Source file: f:\lofar8\oms\LOFAR\cep\cpa\pscf\src\pscf\WPQueue.cc

//## begin module%3C7B7F300007.additionalIncludes preserve=no
//## end module%3C7B7F300007.additionalIncludes

//## begin module%3C7B7F300007.includes preserve=yes
//## end module%3C7B7F300007.includes

// Dispatcher
#include "Dispatcher.h"
// WPQueue
#include "WPQueue.h"
//## begin module%3C7B7F300007.declarations preserve=no
//## end module%3C7B7F300007.declarations

//## begin module%3C7B7F300007.additionalDeclarations preserve=yes
//## end module%3C7B7F300007.additionalDeclarations


// Class WPQueue 

WPQueue::WPQueue()
  //## begin WPQueue::WPQueue%3C7B72F901E6_const.hasinit preserve=no
  //## end WPQueue::WPQueue%3C7B72F901E6_const.hasinit
  //## begin WPQueue::WPQueue%3C7B72F901E6_const.initialization preserve=yes
  : wp(0),dsp(0)
  //## end WPQueue::WPQueue%3C7B72F901E6_const.initialization
{
  //## begin WPQueue::WPQueue%3C7B72F901E6_const.body preserve=yes
  //## end WPQueue::WPQueue%3C7B72F901E6_const.body
}

WPQueue::WPQueue (Dispatcher* pdsp, WorkProcess* pwp)
  //## begin WPQueue::WPQueue%3C7E301D03D7.hasinit preserve=no
  //## end WPQueue::WPQueue%3C7E301D03D7.hasinit
  //## begin WPQueue::WPQueue%3C7E301D03D7.initialization preserve=yes
  : wp(0),dsp(0)
  //## end WPQueue::WPQueue%3C7E301D03D7.initialization
{
  //## begin WPQueue::WPQueue%3C7E301D03D7.body preserve=yes
  attach(pdsp,pwp);
  //## end WPQueue::WPQueue%3C7E301D03D7.body
}


WPQueue::~WPQueue()
{
  //## begin WPQueue::~WPQueue%3C7B72F901E6_dest.body preserve=yes
  if( wp )
    delete wp;
  //## end WPQueue::~WPQueue%3C7B72F901E6_dest.body
}



//## Other Operations (implementation)
void WPQueue::attach (Dispatcher* pdsp, WorkProcess *pwp)
{
  //## begin WPQueue::attach%3C7B92AA007F.body preserve=yes
  if( wp )
    delete wp;
  dsp = pdsp;
  wp = pwp;
  subscriptions.clear();
  queue.clear();
  wp->attach(this);
  setRepoll(False);
  full_lock = receive_lock = False;
  //## end WPQueue::attach%3C7B92AA007F.body
}

bool WPQueue::subscribe (const HIID &id)
{
  //## begin WPQueue::subscribe%3C7B935E03B3.body preserve=yes
  dprintf(2)("subscribing to %s\n",id.toString().c_str());
  subscriptions.add(id);
  return True;
  //## end WPQueue::subscribe%3C7B935E03B3.body
}

bool WPQueue::unsubscribe (const HIID &id)
{
  //## begin WPQueue::unsubscribe%3C7B9367008A.body preserve=yes
  dprintf(2)("unsubscribing from %s\n",id.toString().c_str());
  subscriptions.remove(id);
  return True;
  //## end WPQueue::unsubscribe%3C7B9367008A.body
}

bool WPQueue::subscribesTo (const HIID &id)
{
  //## begin WPQueue::subscribesTo%3C7B945D028D.body preserve=yes
  return subscriptions.contains(id);
  //## end WPQueue::subscribesTo%3C7B945D028D.body
}

bool WPQueue::place (const MessageRef &msg)
{
  //## begin WPQueue::place%3C7B94670179.body preserve=yes
  int pri = msg->priority();
  // iterate from end of queue as long as msg priority is lower
  MLI iter = queue.begin();
  int count = 0;
  while( iter != queue.end() && (*iter)->priority() < pri )
    iter++,count++;
  // if inserting at beginning of queue, then raise the repoll flag
  if( iter == queue.end() )
  {
    dprintf(3)("queueing [%s] at #%d (head of queue)\n",msg->debug(1),count);
    setRepoll(True);
  }
  else
    dprintf(3)("queueing [%s] at #%d\n",msg->debug(1),count);
  MessageRef newref(msg,DMI::COPYREF|DMI::PRESERVE_RW);
  queue.insert(iter,newref);
  return repoll();
  //## end WPQueue::place%3C7B94670179.body
}

int WPQueue::topPriority ()
{
  //## begin WPQueue::topPriority%3C7CCD05016E.body preserve=yes
  if( queue.size() )
    return queue.front()->priority();
  else
    return Message::MIN_PRIORITY;
  //## end WPQueue::topPriority%3C7CCD05016E.body
}

bool WPQueue::queueLocked ()
{
  //## begin WPQueue::queueLocked%3C7D255F0153.body preserve=yes
  if( full_lock )
    return True;
  if( receive_lock && queue.size() )
  {
    const HIID &id = queue.back()->id();
    if( id[0] != AidMsgEvent )
      return True;
  }
  return False;
  //## end WPQueue::queueLocked%3C7D255F0153.body
}

bool WPQueue::performDelivery ()
{
  //## begin WPQueue::performDelivery%3C7CCD0A014D.body preserve=yes
  setRepoll(False);
  if( !queue.size() )
    return False;
  int res;
  const Message &msg = queue.back().deref();
  const HIID &id = msg.id();
  FailWhen( !id.size(),"null message ID" );
  dprintf(3)("delivering %s\n",msg.debug(1));
  // is it a  system event message?
  if( id[0] == AidMsgEvent ) 
  {
    if( full_lock ) 
      return False;
    if( id[1] == AidMsgTimeout ) // deliver timeout message
    {
      FailWhen( id.size() != 3,"malformed "+id.toString()+" message" );
      int handle = id[2];
      Message::TimeoutData *tos = static_cast<Message::TimeoutData*>(msg.data());
      res = wp->timeout(handle,tos->id,tos->data);
      if( res == Message::CANCEL )
        dsp->removeTimeout(this,handle);
      res = Message::MSG_OK;
    }
    else if( id[1] == AidMsgInput ) // deliver input message
    {
      FailWhen( id.size() != 5,"malformed "+id.toString()+" message" );
      int handle = id[2],fd=id[3],flags=id[4];
      res = wp->input(handle,fd,flags);
      if( res == Message::CANCEL )
        dsp->removeInput(this,handle);
      res = Message::MSG_OK;
    }
    else if( id[1] == AidMsgSignal ) // deliver input message
    {
      FailWhen( id.size() != 3,"malformed "+id.toString()+" message" );
      int signum = id[2];
      res = wp->signal(signum);
      if( res == Message::CANCEL )
        dsp->removeSignal(this,signum);
      res = Message::MSG_OK;
    }
    else
      Throw("unexpected message" + id.toString());
  }
  else // deliver regular message
  {
    if( receive_lock || full_lock )
      return False;
    // lock 
    receive_lock = True;
    res = wp->receive(queue.back());
    receive_lock = False;
  }
  // dispence of queue accordingly
  if( res == Message::MSG_OK )
  {
    dprintf(3)("result code: OK, de-queuing\n");
    queue.pop_front();
    if( queue.size() ) // something else in queue - mark for repoll
      setRepoll(True);
  }
  else if( res == Message::HOLD )
  {
    dprintf(3)("result code: HOLD\n");
    // hold - so do nothing
  }
  else if( res == Message::REQUEUE )
  {
    dprintf(3)("result code: REQUEUE\n");
    // requeue - remove message from front and re-insert into queue
    MessageRef ref( queue.front(),DMI::COPYREF|DMI::PRESERVE_RW );
    queue.pop_front();
    place(ref);
    // head of queue is now different? mark for repoll
    if( &queue.front().deref() != &msg )
      setRepoll(True);
  }
  return repoll();
  //## end WPQueue::performDelivery%3C7CCD0A014D.body
}

// Additional Declarations
  //## begin WPQueue%3C7B72F901E6.declarations preserve=yes
string WPQueue::sdebug ( int detail,const string &prefix,const char *name ) const
{
  string out = name?name:"WPQ";
  if( detail )
    out += Debug::ssprintf("(%d)",queue.size());
  if( detail>3 )
    out += Debug::ssprintf("/%08x",this);
  out += "/"+wp->sdebug(detail,prefix);
  return out;
}
  //## end WPQueue%3C7B72F901E6.declarations

//## begin module%3C7B7F300007.epilog preserve=yes
//## end module%3C7B7F300007.epilog
