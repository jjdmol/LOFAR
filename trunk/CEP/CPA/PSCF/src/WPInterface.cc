//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3C8F26A30123.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3C8F26A30123.cm

//## begin module%3C8F26A30123.cp preserve=no
//## end module%3C8F26A30123.cp

//## Module: WPInterface%3C8F26A30123; Package body
//## Subsystem: PSCF%3C5A73670223
//## Source file: F:\lofar8\oms\LOFAR\CEP\CPA\PSCF\src\pscf\WPInterface.cc

//## begin module%3C8F26A30123.additionalIncludes preserve=no
//## end module%3C8F26A30123.additionalIncludes

//## begin module%3C8F26A30123.includes preserve=yes
//## end module%3C8F26A30123.includes

// Dispatcher
#include "Dispatcher.h"
// WPInterface
#include "WPInterface.h"
//## begin module%3C8F26A30123.declarations preserve=no
//## end module%3C8F26A30123.declarations

//## begin module%3C8F26A30123.additionalDeclarations preserve=yes
//## end module%3C8F26A30123.additionalDeclarations


// Class WPInterface 

WPInterface::WPInterface (AtomicID wpc)
  //## begin WPInterface::WPInterface%3C7CBB10027A.hasinit preserve=no
  //## end WPInterface::WPInterface%3C7CBB10027A.hasinit
  //## begin WPInterface::WPInterface%3C7CBB10027A.initialization preserve=yes
    : address_(wpc),dsp_(0),queue_(0),wpid_(wpc)
  //## end WPInterface::WPInterface%3C7CBB10027A.initialization
{
  //## begin WPInterface::WPInterface%3C7CBB10027A.body preserve=yes
  full_lock = receive_lock = started = False;
  //## end WPInterface::WPInterface%3C7CBB10027A.body
}


WPInterface::~WPInterface()
{
  //## begin WPInterface::~WPInterface%3C7B6A3702E5_dest.body preserve=yes
  //## end WPInterface::~WPInterface%3C7B6A3702E5_dest.body
}



//## Other Operations (implementation)
void WPInterface::attach (Dispatcher* pdsp)
{
  //## begin WPInterface::attach%3C7CBAED007B.body preserve=yes
  dsp_ = pdsp;
  //## end WPInterface::attach%3C7CBAED007B.body
}

void WPInterface::do_init ()
{
  //## begin WPInterface::do_init%3C99B0070017.body preserve=yes
  setNeedRepoll(False);
  full_lock = receive_lock = started = False;
  init();
  //## end WPInterface::do_init%3C99B0070017.body
}

void WPInterface::do_start ()
{
  //## begin WPInterface::do_start%3C99B00B00D1.body preserve=yes
  MessageRef ref(new Message(AidMsgHello|address(),Message::PRI_EVENT),DMI::ANON|DMI::WRITE);
  publish(ref);
  start();
  // if we now have any subscriptions, publish them all
  started = True;
  if( subscriptions.size() )
    publishSubs();
  //## end WPInterface::do_start%3C99B00B00D1.body
}

void WPInterface::do_stop ()
{
  //## begin WPInterface::do_stop%3C99B00F0254.body preserve=yes
  MessageRef ref(new Message(AidMsgBye|address(),Message::PRI_EVENT),DMI::ANON|DMI::WRITE);
  publish(ref);
  stop();
  //## end WPInterface::do_stop%3C99B00F0254.body
}

void WPInterface::init ()
{
  //## begin WPInterface::init%3C7F882B00E6.body preserve=yes
  //## end WPInterface::init%3C7F882B00E6.body
}

void WPInterface::start ()
{
  //## begin WPInterface::start%3C7E4A99016B.body preserve=yes
  //## end WPInterface::start%3C7E4A99016B.body
}

void WPInterface::stop ()
{
  //## begin WPInterface::stop%3C7E4A9C0133.body preserve=yes
  //## end WPInterface::stop%3C7E4A9C0133.body
}

bool WPInterface::poll ()
{
  //## begin WPInterface::poll%3C8F13B903E4.body preserve=yes
  if( !queue().size() )
    return setNeedRepoll(False);
  
  int res = Message::ACCEPT;
  // remove message from queue
  MessageRef mref = queue().back();
  queue().pop_back();
  
  const Message &msg = mref.deref();
  const HIID &id = msg.id();
  FailWhen( !id.size(),"null message ID" );
  dprintf1(3)("%s: receiving %s\n",sdebug(1).c_str(),msg.debug(1));
  // is it a system event message?
  if( id[0] == AidMsgEvent ) 
  {
    if( full_lock ) 
      return False;
    if( id[1] == AidMsgTimeout ) // deliver timeout message
    {
      FailWhen( id.size() < 2,"malformed "+id.toString()+" message" );
      HIID to_id = id.subId(2);
      res = timeout(to_id);
      if( res == Message::CANCEL )
        dsp()->removeTimeout(this,to_id);
      res = Message::ACCEPT;
    }
    else if( id[1] == AidMsgInput ) // deliver input message
    {
      FailWhen( id.size() != 3,"malformed "+id.toString()+" message" );
      int fd=id[2],flags=msg.state();
      if( flags )  // no flags? Means the input has been already removed. Ignore
      {
        res = input(fd,flags);
        if( res == Message::CANCEL )
          dsp()->removeInput(this,fd,flags);
        res = Message::ACCEPT;
      }
    }
    else if( id[1] == AidMsgSignal ) // deliver input message
    {
      FailWhen( id.size() != 3,"malformed "+id.toString()+" message" );
      int signum = id[2];
      res = signal(signum);
      if( res == Message::CANCEL )
        dsp()->removeSignal(this,signum);
      res = Message::ACCEPT;
    }
    else
      Throw("unexpected event" + id.toString());
    // Once the event has been delivered, reset its state to 0.
    // This helps the dispatcher keep track of when a new event message is
    // required (as opposed to updating a previous message that's still
    // undelivered). See Dispatcher::checkEvents() for details.
    if( mref.isWritable() )
      mref().setState(0);
  }
  else // deliver regular message
  {
    if( receive_lock || full_lock )
      return False;
    // lock 
    receive_lock = True;
    res = receive(mref);
    receive_lock = False;
  }
  // dispence of queue() accordingly
  if( res == Message::ACCEPT )   // message accepted, remove from queue
  {
    dprintf(3)("result code: OK, de-queuing\n");
  }
  else      // message not accepted, stays in queue
  {
    FailWhen( !mref.valid(),"message not accepted but its ref was detached or xferred" );
    if( res == Message::HOLD )
    {
      dprintf(3)("result code: HOLD, leaving at head of queue\n");
      queue().push_back(mref);
      setNeedRepoll(False);
    }
    else if( res == Message::REQUEUE )
    {
      dprintf(3)("result code: REQUEUE\n");
      // requeue - re-insert into queue() according to priority
      enqueue(mref);
      // repoll if head of queue has changed
    }
  }
  // ask for repoll if head of queue has changed
  return setNeedRepoll( queue().size() && peekAtQueue() != &msg );
  //## end WPInterface::poll%3C8F13B903E4.body
}

bool WPInterface::enqueue (const MessageRef &msg)
{
  //## begin WPInterface::enqueue%3C8F204A01EF.body preserve=yes
  int pri = msg->priority();
  // iterate from end of queue() as long as msg priority is lower
  MQI iter = queue().begin();
  int count = queue().size();
  while( iter != queue().end() && (*iter)->priority() < pri )
    iter++,count--;
  // if inserting at head of queue (which is end), then raise the repoll flag
  if( iter == queue().end() )
  {
    dprintf(3)("queueing [%s] at head of queue\n",msg->debug(1));
    setNeedRepoll(True);
  }
  else
    dprintf(3)("queueing [%s] at H-%d\n",msg->debug(1),count);
  queue().insert(iter,msg);
  return needRepoll();
  //## end WPInterface::enqueue%3C8F204A01EF.body
}

bool WPInterface::dequeue (const HIID &id, MessageRef *ref)
{
  //## begin WPInterface::dequeue%3C8F204D0370.body preserve=yes
  bool erased_head = True;
  for( MQRI iter = queue().rbegin(); iter != queue().rend(); )
  {
    if( id.matches( (*iter)->id() ) )
    {
      // is this the head of the queue? 
      erased_head |= ( iter == queue().rbegin() );
      if( ref )
        *ref = *iter; // xfer the reference
      queue().erase((iter++).base());
      // we're done if a ref was asked for
      if( ref )
        break;
    }
    else
      iter++;  
  }
  return setNeedRepoll( erased_head && queue().size() );
  //## end WPInterface::dequeue%3C8F204D0370.body
}

bool WPInterface::dequeue (int pos, MessageRef *ref)
{
  //## begin WPInterface::dequeue%3C8F205103D0.body preserve=yes
  FailWhen( (uint)pos >= queue().size(),"dequeue: illegal position" );
  setNeedRepoll( !pos && queue().size()>1 );
  // iterate to the req. position
  MQRI iter = queue().rbegin();
  while( pos-- )
    iter++;
  if( ref )
    *ref = *iter;
  queue().erase(iter.base());
  return needRepoll();
  //## end WPInterface::dequeue%3C8F205103D0.body
}

int WPInterface::searchQueue (const HIID &id, int pos, MessageRef *ref)
{
  //## begin WPInterface::searchQueue%3C8F205601EC.body preserve=yes
  FailWhen( (uint)pos >= queue().size(),"dequeue: illegal position" );
  // iterate to the req. position
  MQRI iter = queue().rbegin();
  for( int i=0; i<pos; i++ )
    iter++;
  // start searching
  for( ; iter != queue().rend(); iter++,pos++ )
    if( id.matches( (*iter)->id() ) )
    {
      if( ref )
        *ref = iter->copy(DMI::PRESERVE_RW);
      return pos;
    }
  // not found
  return -1;
  //## end WPInterface::searchQueue%3C8F205601EC.body
}

const Message * WPInterface::peekAtQueue () const
{
  //## begin WPInterface::peekAtQueue%3C8F206C0071.body preserve=yes
  return queue().size() 
    ? queue().back().deref_p() 
    : 0;
  //## end WPInterface::peekAtQueue%3C8F206C0071.body
}

bool WPInterface::queueLocked () const
{
  //## begin WPInterface::queueLocked%3C8F207902AB.body preserve=yes
  if( full_lock )
    return True;
  if( receive_lock && queue().size() )
  {
    const HIID &id = queue().back()->id();
    if( id[0] != AidMsgEvent )
      return True;
  }
  return False;
  //## end WPInterface::queueLocked%3C8F207902AB.body
}

bool WPInterface::subscribe (const HIID &id, const MsgAddress &scope)
{
  //## begin WPInterface::subscribe%3C99AB6E0187.body preserve=yes
  // If something has changed in the subs, _and_ WP has been started,
  // then re-publish the whole thing.
  // (If not yet started, then everything will be eventually published 
  // by do_start(), above)
  bool change = subscriptions.add(id,scope);
  if( change  && started )
    publishSubs();
  return change;
  //## end WPInterface::subscribe%3C99AB6E0187.body
}

bool WPInterface::unsubscribe (const HIID &id)
{
  //## begin WPInterface::unsubscribe%3C7CB9C50365.body preserve=yes
  // If something has changed in the subs, _and_ WP has been started,
  // then re-publish the whole thing.
  // (If not yet started, then everything will be eventually published 
  // by do_start(), above)
  bool change = subscriptions.remove(id);
  if( change && started )
    publishSubs();
  return change;
  //## end WPInterface::unsubscribe%3C7CB9C50365.body
}

int WPInterface::receive (MessageRef &mref)
{
  //## begin WPInterface::receive%3C7CC0950089.body preserve=yes
  dprintf(1)("unhandled receive(%s)\n",mref->sdebug(1).c_str());
  return Message::ACCEPT;
  //## end WPInterface::receive%3C7CC0950089.body
}

int WPInterface::timeout (const HIID &id)
{
  //## begin WPInterface::timeout%3C7CC2AB02AD.body preserve=yes
  dprintf(1)("unhandled timeout(%s)\n",id.toString().c_str());
  return Message::ACCEPT;
  //## end WPInterface::timeout%3C7CC2AB02AD.body
}

int WPInterface::input (int fd, int flags)
{
  //## begin WPInterface::input%3C7CC2C40386.body preserve=yes
  dprintf(1)("unhandled input(%d,%x)\n",fd,flags);
  return Message::ACCEPT;
  //## end WPInterface::input%3C7CC2C40386.body
}

int WPInterface::signal (int signum)
{
  //## begin WPInterface::signal%3C7DFD240203.body preserve=yes
  dprintf(1)("unhandled signal(%s)\n",sys_siglist[signum]);
  return Message::ACCEPT;
  //## end WPInterface::signal%3C7DFD240203.body
}

int WPInterface::send (MessageRef msg, MsgAddress to)
{
  //## begin WPInterface::send%3C7CB9E802CF.body preserve=yes
  FailWhen( !isAttached(),"unattached wp");
  // if not writable, privatize for writing (but not deeply)
  if( !msg.isWritable() )
    msg.privatize(DMI::WRITE);
  msg().setFrom(address());
  msg().setState(state());
  dprintf(2)("send [%s] to %s\n",msg->sdebug(1).c_str(),to.toString().c_str());
  // substitute 'Local' for actual addresses
  if( to.host() == AidLocal )
    to.host() = address().host();
  if( to.process() == AidLocal )
    to.process() = address().process();
  return dsp()->send(msg,to); 
  //## end WPInterface::send%3C7CB9E802CF.body
}

int WPInterface::publish (MessageRef msg, int scope)
{
  //## begin WPInterface::publish%3C7CB9EB01CF.body preserve=yes
  FailWhen( !isAttached(),"unattached wp");
  // if not writable, privatize for writing (but not deeply)
  if( !msg.isWritable() )
    msg.privatize(DMI::WRITE);
  msg().setFrom(address());
  msg().setState(state());
  dprintf(2)("publish [%s] scope %d\n",msg->sdebug(1).c_str(),scope);
  AtomicID host = (scope < Message::GLOBAL) ? address().host() : AidAny;
  AtomicID process = (scope < Message::HOST) ? address().process() : AidAny;
  return dsp()->send(msg,MsgAddress(AidPublish,AidPublish,process,host));
  //## end WPInterface::publish%3C7CB9EB01CF.body
}

// Additional Declarations
  //## begin WPInterface%3C7B6A3702E5.declarations preserve=yes

void WPInterface::publishSubs ()
{
  // pack subscriptions into a block
  SmartBlock *block = new SmartBlock(subscriptions.packSize());
  subscriptions.pack(block->data());
  // publish it
  MessageRef ref(
    new Message(AidMsgSubscribe|address(),
                block,DMI::ANON|DMI::WRITE,
                Message::PRI_EVENT),
    DMI::ANON|DMI::WRITE);
  publish(ref);
}


string WPInterface::sdebug ( int detail,const string &,const char *nm ) const
{
  string out;
  if( detail>=0 ) // basic detail
  {
    if( nm )
      out = string(nm) + "/";
    out += address().toString();
    if( detail>3 )
      out += Debug::ssprintf("/%08x",this);
  }
  if( detail >= 1 || detail == -1 )   // normal detail
  {
    Debug::appendf(out,"Q:%d",queue().size());
    Debug::appendf(out,"st:%d",state_);
  }
  return out;
}
  //## end WPInterface%3C7B6A3702E5.declarations
//## begin module%3C8F26A30123.epilog preserve=yes
//## end module%3C8F26A30123.epilog


// Detached code regions:
#if 0
//## begin WPInterface::subscribe%3C7CB9B70120.body preserve=yes
  subscribe(id,MsgAddress(
      AidAny,AidAny,
      scope < Message::PROCESS ? address().process() : AidAny,
      scope < Message::GLOBAL ?  address().host() : AidAny));
//## end WPInterface::subscribe%3C7CB9B70120.body

//## begin WPInterface::publish%3C99BFA502B0.body preserve=yes
  FailWhen( !isAttached(),"unattached wp");
  // if not writable, privatize for writing (but not deeply)
  if( !msg.isWritable() )
    msg.privatize(DMI::WRITE);
  msg().setFrom(address());
  msg().setState(state());
  dprintf(2)("publish [%s] scope %d\n",msg->sdebug(1).c_str(),scope);
  AtomicID host = (scope < Message::GLOBAL) ? dsp()->hostId() : AidAny;
  AtomicID process = (scope < Message::HOST) ? dsp()->processId() : AidAny;
  return dsp()->send(msg,MsgAddress(AidPublish,AidPublish,process,host));
//## end WPInterface::publish%3C99BFA502B0.body

//## begin WPInterface::subscribesTo%3C8F14310315.body preserve=yes
  // determine minimum subscription scope
  int minscope = Message::PROCESS;
  if( msg.from().process() != address().process() )
    minscope = Message::HOST;
  if( msg.from().host() != address().host() )
    minscope = Message::GLOBAL;
  // look thru map
  for( CSSI iter = subs.begin(); iter != subs.end(); iter++ )
    if( iter->first.matches(msg.id()) && iter->second >= minscope )
      return True;
  return False;
//## end WPInterface::subscribesTo%3C8F14310315.body

//## begin WPInterface::initSubsIterator%3C98D8090048.body preserve=yes
  return subs.begin();
//## end WPInterface::initSubsIterator%3C98D8090048.body

//## begin WPInterface::iterateSubs%3C98D81C0077.body preserve=yes
  if( iter == 
//## end WPInterface::iterateSubs%3C98D81C0077.body

//## begin WPInterface::wpclass%3C905E8B000E.body preserve=yes
  // default wp class is 0
  return 0;
//## end WPInterface::wpclass%3C905E8B000E.body

#endif
