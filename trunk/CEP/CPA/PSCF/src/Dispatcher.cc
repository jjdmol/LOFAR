//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3C7B7F30004B.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3C7B7F30004B.cm

//## begin module%3C7B7F30004B.cp preserve=no
//## end module%3C7B7F30004B.cp

//## Module: Dispatcher%3C7B7F30004B; Package body
//## Subsystem: PSCF%3C5A73670223
//## Source file: F:\lofar8\oms\LOFAR\CEP\CPA\PSCF\src\pscf\Dispatcher.cc

//## begin module%3C7B7F30004B.additionalIncludes preserve=no
//## end module%3C7B7F30004B.additionalIncludes

//## begin module%3C7B7F30004B.includes preserve=yes
#include <sys/time.h>
#include <errno.h>
#include <string.h>
//## end module%3C7B7F30004B.includes

// WPInterface
#include "WPInterface.h"
// Dispatcher
#include "Dispatcher.h"
//## begin module%3C7B7F30004B.declarations preserve=no
//## end module%3C7B7F30004B.declarations

//## begin module%3C7B7F30004B.additionalDeclarations preserve=yes
// pulls in registry definitions
static int dum = aidRegistry_PSCF();

Dispatcher * Dispatcher::dispatcher = 0;
sigset_t Dispatcher::raisedSignals,Dispatcher::allSignals;
struct sigaction * Dispatcher::orig_sigaction[_NSIG];
      
// static signal handler
void Dispatcher::signalHandler (int signum,siginfo_t *,void *)
{
  sigaddset(&raisedSignals,signum);
  // if this signal is in the maps, increment its counters
  pair<CSMI,CSMI> rng = dispatcher->signals.equal_range(signum);
  for( CSMI iter = rng.first; iter != rng.second; iter++ )
    if( iter->second.counter )
      (*iter->second.counter)++;
}
//## end module%3C7B7F30004B.additionalDeclarations


// Class Dispatcher 

Dispatcher::Dispatcher (AtomicID process, AtomicID host, int hz)
  //## begin Dispatcher::Dispatcher%3C7CD444039C.hasinit preserve=no
  //## end Dispatcher::Dispatcher%3C7CD444039C.hasinit
  //## begin Dispatcher::Dispatcher%3C7CD444039C.initialization preserve=yes
    : heartbeat_hz(hz)
  //## end Dispatcher::Dispatcher%3C7CD444039C.initialization
{
  //## begin Dispatcher::Dispatcher%3C7CD444039C.body preserve=yes
  if( dispatcher )
    Throw("multiple Dispatchers instantiated");
  dispatcher = this;
  address = MsgAddress(AidDispatcher,0,process,host);
  running = False;
  
  memset(orig_sigaction,0,sizeof(orig_sigaction));
  
  sigemptyset(&raisedSignals);
  sigemptyset(&allSignals);
  
  dprintf(1)("created\n");
  //## end Dispatcher::Dispatcher%3C7CD444039C.body
}


Dispatcher::~Dispatcher()
{
  //## begin Dispatcher::~Dispatcher%3C7B6A3E00A0_dest.body preserve=yes
  if( running )
    stop();
  wps.clear();
  dispatcher = 0;
  //## end Dispatcher::~Dispatcher%3C7B6A3E00A0_dest.body
}



//## Other Operations (implementation)
const MsgAddress & Dispatcher::attach (WPRef &wpref)
{
  //## begin Dispatcher::attach%3C8CDDFD0361.body preserve=yes
  FailWhen( !wpref.isWritable(),"writable ref required" ); 
  WPInterface & wp = wpref;
  FailWhen( wp.isAttached(),"wp is already attached" );
  // assign instance number to this WP
  AtomicID wpclass = wp.address().wpclass();
  int ninst = wp_instances[wpclass]++;
  WPID wpid(wpclass,ninst);
  wp.setAddress( MsgAddress(wpid,processId(),hostId()) );
  // add to map
  wps[wpid] = wpref;
  dprintf(1)("attaching WP: %s\n",wp.sdebug().c_str());
  wp.attach(this);
  if( running )
  {
    wp.do_init();
    wp.do_start();
  }
  return wp.address();
  //## end Dispatcher::attach%3C8CDDFD0361.body
}

const MsgAddress & Dispatcher::attach (WPInterface* wp, int flags)
{
  //## begin Dispatcher::attach%3C7B885A027F.body preserve=yes
  WPRef ref(wp,flags|DMI::WRITE);
  return attach(ref);
  //## end Dispatcher::attach%3C7B885A027F.body
}

void Dispatcher::detach (WPInterface* wp)
{
  //## begin Dispatcher::detach%3C8CA2BD01B0.body preserve=yes
  FailWhen( wp->dsp() != this,
      "wp '"+wp->sdebug(1)+"' is not attached to this dispatcher");
  detach(wp->address().wpid());
  //## end Dispatcher::detach%3C8CA2BD01B0.body
}

void Dispatcher::detach (const WPID &id)
{
  //## begin Dispatcher::detach%3C8CDE320231.body preserve=yes
  WPI iter = wps.find(id); 
  FailWhen( iter == wps.end(),
      "wpid '"+id.toString()+"' is not attached to this dispatcher");
  iter->second().do_stop();
  wps.erase(iter);
  GWI iter2 = gateways.find(id);
  if( iter2 != gateways.end() )
    gateways.erase(iter2);
  //## end Dispatcher::detach%3C8CDE320231.body
}

void Dispatcher::declareForwarder (WPInterface *wp)
{
  //## begin Dispatcher::declareForwarder%3C95C73F022A.body preserve=yes
  CWPI iter = wps.find(wp->wpid()); 
  FailWhen( iter == wps.end() || iter->second.deref_p() != wp,
      "WP not attached to this dispatcher");
  gateways[wp->wpid()] = wp;
  //## end Dispatcher::declareForwarder%3C95C73F022A.body
}

void Dispatcher::start ()
{
  //## begin Dispatcher::start%3C7DFF770140.body preserve=yes
  running = True;
  inPollLoop = False;
  // say init to all WPs
  dprintf(1)("starting\n");
  dprintf(2)("start: initializing WPs\n");
  for( WPI iter = wps.begin(); iter != wps.end(); iter++ )
    iter->second().do_init();
  // setup signal handler for SIGALRM
  sigemptyset(&raisedSignals);
  rebuildSignals();
  num_active_fds = 0;
  rebuildInputs();
  // setup heartbeat timer
  struct itimerval tval = { {0,1000000/heartbeat_hz},{0,1000000/heartbeat_hz} };
  setitimer(ITIMER_REAL,&tval,0);
  // say start to all WPs
  dprintf(2)("start: starting WPs\n");
  for( WPI iter = wps.begin(); iter != wps.end(); iter++ )
    iter->second().do_start();
  dprintf(2)("start: complete\n");
  //## end Dispatcher::start%3C7DFF770140.body
}

void Dispatcher::stop ()
{
  //## begin Dispatcher::stop%3C7E0270027B.body preserve=yes
  running = False;
  dprintf(1)("stopping\n");
  // stop the heartbeat timer
  struct itimerval tval = { {0,0},{0,0} };
  setitimer(ITIMER_REAL,&tval,0);
  // tell all WPs that we are stopping
  for( WPI iter = wps.begin(); iter != wps.end(); iter++ )
    iter->second().do_stop();
  // clear all event lists
  timeouts.clear();
  inputs.clear();
  signals.clear();
  // this will effectively remove all signal handlers
  rebuildSignals();
  //## end Dispatcher::stop%3C7E0270027B.body
}

int Dispatcher::send (MessageRef &msg, const MsgAddress &to)
{
  //## begin Dispatcher::send%3C7B8867015B.body preserve=yes
  dprintf(2)("send(%s,%s)\n",msg->sdebug().c_str(),to.toString().c_str());
  int ndeliver = 0;
  // set the message to-address
  msg().setTo(to);
  // local delivery: check that host/process spec matches ours
  if( to.host().matches(hostId()) && to.process().matches(processId()) )
  {
    // addressed to specific wp class/instance?
    AtomicID wpc = to.wpclass(),wpi=to.inst();
    if( !wildcardAddr(wpc) && !wildcardAddr(wpi) )
    {
      WPI iter = wps.find(to.wpid()); // do we have that ID?
      if( iter != wps.end() )
      {
        dprintf(2)("  queing for: %s\n",iter->second->debug(1));
        // for first delivery, privatize the whole message & payload as read-only
        if( !ndeliver )
          msg.privatize(DMI::READONLY|DMI::DEEP);
        repoll |= iter->second().enqueue(msg.copy());
        ndeliver++;
      }
    }
    else // else it's a publish/b-cast: scan thru all WPs
    {
      for( WPI iter = wps.begin(); iter != wps.end(); iter++ )
      {
        const MsgAddress & wpaddr = iter->second->address();
        if( wpc.matches(wpaddr.wpclass()) && wpi.matches(wpaddr.inst()) )
        {
          dprintf(2)("  b-casting to %s\n",iter->second->debug(1));
        }
        else if( wpc == AidPublish && 
                 iter->second->getSubscriptions().matches(msg.deref()) )
        {
          dprintf(2)("  publishing to %s\n",iter->second->debug(1));
        }
        else // else continue, so as to skip the deligvery below
          continue;
        // for first delivery, privatize the whole message & payload as read-only
        if( !ndeliver )
          msg.privatize(DMI::READONLY|DMI::DEEP);
        repoll |= iter->second().enqueue(msg.copy());
        ndeliver++;
      }
    }
  }
  else
    dprintf(2)("  destination address is not local\n");
  // If the message has a remote/broadcast destination, then deliver it to
  // all gateways. The only exception is Publish, which is already taken
  // care of (at least for gateways) in the loop above.
  if( to.wpclass() != AidPublish && 
      to.host() != hostId()  && to.process() != processId() )
  {
    for( GWI iter = gateways.begin(); iter != gateways.end(); iter++ )
      if( iter->second->willForward(msg.deref()) )
      {
        dprintf(2)("  forwarding via %s\n",iter->second->debug(1));
        if( !ndeliver )
          msg.privatize(DMI::READONLY|DMI::DEEP);
        repoll |= iter->second->enqueue(msg.copy());
        ndeliver++;
      }
  }
  if( !ndeliver )
    dprintf(2)("not delivered anywhere\n");
  return ndeliver;
  //## end Dispatcher::send%3C7B8867015B.body
}

void Dispatcher::poll ()
{
  //## begin Dispatcher::poll%3C7B888E01CF.body preserve=yes
  FailWhen(!running,"not running");
  // main polling loop
  while( repoll || checkEvents() )
  {
    int maxpri = Message::PRI_LOWEST;
    WPInterface *pwp = 0;
    int num_repoll = 0; // # of WPs needing a repoll
    // count num_repoll, and find queue with maximum priority
    for( WPI wpi = wps.begin(); wpi != wps.end(); wpi++ )
    {
      if( wpi->second().needRepoll() && !wpi->second().queueLocked() )
      {
        num_repoll++;
        int pri = Message::PRI_LOWEST;
        if( wpi->second().peekAtQueue() )
          pri = max(wpi->second().peekAtQueue()->priority(),Message::PRI_LOWEST);
        if( pri >= maxpri )
        {
          pwp = wpi->second;
          maxpri = pri;
        }
      }
    }
    // if more than 1 WP needs a repoll, force another loop
    repoll = ( num_repoll > 1 );
    // deliver message, if a queue was found
    if( pwp )
    {
      dprintf(3)("poll: max priority %d in %s\n",maxpri,pwp->debug(1));
      repoll |= pwp->poll();
    }
  }
  //## end Dispatcher::poll%3C7B888E01CF.body
}

void Dispatcher::pollLoop ()
{
  //## begin Dispatcher::pollLoop%3C8C87AF031F.body preserve=yes
  FailWhen(!running,"not running");
  FailWhen(inPollLoop,"already in pollLoop()");
  inPollLoop = True;
  for(;;)
  {
    poll();
    // pause until next heartbeat (SIGALRM), or until an fd is active
    if( max_fd >= 0 ) // use select(2) 
    {
      fds_active = fds_watched;
      num_active_fds = select(max_fd,&fds_active.r,&fds_active.w,&fds_active.x,NULL);
      // we don't expect any errors except perhaps EINTR (interrupted by signal)
      if( num_active_fds < 0 && errno != EINTR )
        dprintf(0)("select(): unexpected error %d (%s)\n",errno,strerror(errno));
    }
    else
      pause();       // use plain pause(2)
  }
  inPollLoop = False;
  //## end Dispatcher::pollLoop%3C8C87AF031F.body
}

void Dispatcher::addTimeout (WPInterface* pwp, const Timestamp &period, const HIID &id, int flags, int priority)
{
  //## begin Dispatcher::addTimeout%3C7D28C30061.body preserve=yes
  FailWhen(!period,"addTimeout: null period");
  // setup a new timeout structure
  TimeoutInfo ti(pwp,id,priority);
  ti.period = period;
  ti.next = Timestamp() + period;
  if( !next_to || ti.next < next_to )
    next_to = ti.next;
  ti.flags = flags;
  ti.id = id;
  // add to list
  timeouts.push_front(ti);
  //## end Dispatcher::addTimeout%3C7D28C30061.body
}

void Dispatcher::addInput (WPInterface* pwp, int fd, int flags, int priority)
{
  //## begin Dispatcher::addInput%3C7D28E3032E.body preserve=yes
  FailWhen(fd<0,Debug::ssprintf("addInput: invalid fd %d",fd));
  FailWhen(!(flags&EV_FDALL),"addInput: no fd flags specified");
  // check if perhaps this fd is already being watched, then we only need to 
  // add to the flags
  for( IILI iter = inputs.begin(); iter != inputs.end(); iter++ )
    if( iter->pwp == pwp && iter->fd == fd )
    {
      iter->flags = flags | (iter->flags&EV_FDALL);
      iter->msg().setPriority(priority);
      return;
    }
  // else setup a new input structure
  InputInfo ii(pwp,AtomicID(fd),priority);
  ii.fd = fd;
  ii.flags = flags;
  // add to list
  inputs.push_front(ii);
  // rebuild input sets
  rebuildInputs();
  //## end Dispatcher::addInput%3C7D28E3032E.body
}

void Dispatcher::addSignal (WPInterface* pwp, int signum, int flags, volatile int* counter, int priority)
{
  //## begin Dispatcher::addSignal%3C7DFF4A0344.body preserve=yes
  FailWhen(signum<0,Debug::ssprintf("addSignal: invalid signal %d",signum));
   // look at map for this signal to see if this WP is already registered
  for( SMI iter = signals.lower_bound(signum); 
       iter->first == signum && iter != signals.end(); iter++ )
  {
    if( iter->second.pwp == pwp )  // found it? change priority & return
    {
      iter->second.msg().setPriority(priority);
      return;
    }
  }
  // insert WP into map
  SignalInfo si(pwp,AtomicID(signum),priority);
  si.signum = signum;
  si.flags = flags;
  si.counter = counter;
  si.msg().setState(0);
  signals.insert( SMPair(signum,si) );
  rebuildSignals();
  //## end Dispatcher::addSignal%3C7DFF4A0344.body
}

bool Dispatcher::removeTimeout (WPInterface* pwp, const HIID &id)
{
  //## begin Dispatcher::removeTimeout%3C7D28F202F3.body preserve=yes
  for( TOILI iter = timeouts.begin(); iter != timeouts.end(); )
  {
    if( iter->pwp == pwp && id.matches(iter->id) )
    {
      // remove any remaining timeout messages from this queue
      repoll |= pwp->dequeue(iter->msg->id());
      timeouts.erase(iter++);
      return True;
    }
    else
      iter++;
  }
  return False;
  //## end Dispatcher::removeTimeout%3C7D28F202F3.body
}

bool Dispatcher::removeInput (WPInterface* pwp, int fd, int flags)
{
  //## begin Dispatcher::removeInput%3C7D2947002F.body preserve=yes
  for( IILI iter = inputs.begin(); iter != inputs.end(); iter++ )
  {
    MessageRef & ref = iter->last_msg;
    // is an input message sitting intill undelivered?
    // (WPInterface::poll() will reset its state to 0 when delivered)
    if( ref.valid() && ref->state() != 0 )
    // input messages are dequeued/modified inside WorkProcess::removeInput.
    if( iter->pwp == pwp && iter->fd == fd ) 
    {   
       // is an input message sitting in the queue, undelivered? Clear its flags
      MessageRef & ref = iter->last_msg;
      if( ref.valid() && ref.isWritable() && ref->state() )
        ref().setState(ref->state()&~flags);
      // clear flags of input
      if( ( iter->flags &= ~flags ) == 0 ) // removed all modes? 
        inputs.erase(iter);
      return True;
    }
  }
  return False;
  //## end Dispatcher::removeInput%3C7D2947002F.body
}

bool Dispatcher::removeSignal (WPInterface* pwp, int signum)
{
  //## begin Dispatcher::removeSignal%3C7DFF57025C.body preserve=yes
  bool res = False;
  pair<SMI,SMI> rng;
  // if signum<0, removes all signals for this WP
  if( signum<0 )
    rng = pair<SMI,SMI>(signals.begin(),signals.end()); // range = all signals
  else 
    rng = signals.equal_range(signum);  // range = this signal's entries
  // iterate over the range
  for( SMI iter = rng.first; iter != rng.second; )
  {
    if( iter->second.pwp == pwp )
    {
      signals.erase(iter++);
      res = True;
    }
    else
      iter++;
  }
  // remove any pending messages from WP's queue
  HIID id = AidMsgEvent|AidMsgSignal|AtomicID(signum);
  if( signum<0 )
    id[2] = AidWildcard;
  repoll |= pwp->dequeue(id);
  
  if( res )
    rebuildSignals();
  
  return res;
  //## end Dispatcher::removeSignal%3C7DFF57025C.body
}

Dispatcher::WPIter Dispatcher::initWPIter ()
{
  //## begin Dispatcher::initWPIter%3C98D4530076.body preserve=yes
  return wps.begin();
  //## end Dispatcher::initWPIter%3C98D4530076.body
}

bool Dispatcher::getWPIter (Dispatcher::WPIter &iter, WPID &wpid, const WPInterface *&pwp)
{
  //## begin Dispatcher::getWPIter%3C98D47B02B9.body preserve=yes
  if( iter == wps.end() )
    return False;
  wpid = iter->first;
  pwp = iter->second.deref_p();
  return True;
  //## end Dispatcher::getWPIter%3C98D47B02B9.body
}

// Additional Declarations
  //## begin Dispatcher%3C7B6A3E00A0.declarations preserve=yes
void Dispatcher::rebuildInputs ()
{
  FD_ZERO(&fds_watched.r);
  FD_ZERO(&fds_watched.w);
  FD_ZERO(&fds_watched.x);
  max_fd=-1;
  for( CIILI iter = inputs.begin(); iter != inputs.end(); iter++ )
  {
    if( iter->flags&EV_FDREAD )
      FD_SET(iter->fd,&fds_watched.r);
    if( iter->flags&EV_FDWRITE )
      FD_SET(iter->fd,&fds_watched.w);
    if( iter->flags&EV_FDEXCEPTION )
      FD_SET(iter->fd,&fds_watched.x);
    if( iter->fd > max_fd )
      max_fd = iter->fd;
  }
  if( max_fd >=0 )
    max_fd++; // this is the 'n' argument to select(2)
}

void Dispatcher::rebuildSignals ()
{
  // rebuild mask of handled signals
  sigset_t newmask;
  sigemptyset(&newmask);
  if( running )
    sigaddset(&newmask,SIGALRM); // ALRM handled when running
  int sig0 = -1;
  for( CSMI iter = signals.begin(); iter != signals.end(); iter++ )
    if( iter->first != sig0 )
      sigaddset(&newmask,sig0=iter->first);
  // init a sigaction structure
  struct sigaction sa;
  sa.sa_handler = 0;
  sa.sa_sigaction = Dispatcher::signalHandler;
  sa.sa_mask = newmask;
  sa.sa_flags = SA_SIGINFO;
  // go thru all signals
  for( int sig = 0; sig < _NSIG; sig++ )
  {
    bool newsig = sigismember(&newmask,sig),
         oldsig = sigismember(&allSignals,sig);
    if( newsig && !oldsig )   // start handling signal?
    {
      Assert(!orig_sigaction[sig]);  // to store old handler
      orig_sigaction[sig] = new struct sigaction;
      sigaction(sig,&sa,orig_sigaction[sig]);
    }
    else if( !newsig && oldsig ) // stop handling?
    {
      Assert(orig_sigaction[sig]);
      sigaction(sig,0,orig_sigaction[sig]); // reset old handler
      delete orig_sigaction[sig];
      orig_sigaction[sig] = 0;
    }
  }
  allSignals = newmask;
}

bool Dispatcher::checkEvents()
{
  // ------ check timeouts
  // next_to gives us the timestamp of the nearest timeout
  Timestamp now;
  if( next_to && next_to <= now )
  {
    next_to = 0;
    // see which timeouts are up, and update next_to as well
    for( TOILI iter = timeouts.begin(); iter != timeouts.end(); )
    {
      if( iter->next <= now ) // this timeout has fired?
      {
        repoll |= iter->pwp->enqueue( iter->msg.copy(DMI::READONLY) );
        if( iter->flags&EV_ONESHOT ) // not continouos? clear it now
        {
          timeouts.erase(iter++);
          continue;
        }
        // else continous, so set the next timestamp
        iter->next = now + iter->period;
      }
      if( !next_to || iter->next < next_to )
        next_to = iter->next;
      iter++;
    }
  }
  // ------ check inputs
  // while in Dispatcher::pollLoop(), select() is already being done for
  // us. Check the fds otherwise
  if( !inPollLoop && inputs.size() )
  {
    struct timeval to = {0,0}; // select will return immediately
    fds_active = fds_watched;
    num_active_fds = select(max_fd,&fds_active.r,&fds_active.w,&fds_active.x,&to);
    // we don't expect any errors except perhaps EINTR (interrupted by signal)
    if( num_active_fds < 0 && errno != EINTR )
      dprintf(0)("select(): unexpected error %d (%s)\n",errno,strerror(errno));
  }
  if( num_active_fds>0 )
  {
    // iterate over all inputs
    for( IILI iter = inputs.begin(); iter != inputs.end(); )
    {
      // set local flags var according to state of fd
      int flags = 0;
      if( iter->flags&EV_FDREAD && FD_ISSET(iter->fd,&fds_active.r) )
        flags |= EV_FDREAD;
      if( iter->flags&EV_FDWRITE && FD_ISSET(iter->fd,&fds_active.w) )
        flags |= EV_FDWRITE;
      if( iter->flags&EV_FDEXCEPTION && FD_ISSET(iter->fd,&fds_active.x) )
        flags |= EV_FDEXCEPTION;
      // anything raised?
      if( flags )
      {
        MessageRef & ref = iter->last_msg;
        // is a previous message still undelivered?
        // (WPInterface::poll() will reset its state to 0 when delivered)
        if( ref.valid() && ref->state() != 0 )
        {
          ref().setState(ref->state()|flags); // update state
          // is this message at the head of the queue? repoll then
          if( iter->pwp->peekAtQueue() == ref.deref_p() )
          {
            repoll = True;     
            iter->pwp->setNeedRepoll(True);
          }
        }
        else // not found, so enqueue a message
        {
          // make a writable copy of the template message, because we set state
          ref = iter->msg.copy().privatize(DMI::WRITE);
          ref().setState(flags);
          repoll |= iter->pwp->enqueue(ref.copy(DMI::WRITE));  
        }
        // if event is one-shot, clear it
        if( iter->flags&EV_ONESHOT )
        {
          inputs.erase(iter++);
          continue;
        }
      }
      iter++;
    }
    // clear active fds 
    num_active_fds = 0;
  }
  // ------ check signals
  // grab & flush the current raised-mask
  sigset_t mask = raisedSignals;
  sigemptyset(&raisedSignals);
  // go through all raised signals
  for( int sig = 0; sig < _NSIG; sig++ )
  {
    if( sigismember(&mask,sig) ) // signal raised? See who wants it
    {
      pair<SMI,SMI> rng = signals.equal_range(sig);
      for( SMI iter = rng.first; iter != rng.second; )
      {
        // no message generated for EV_IGNORE
        if( iter->second.flags&EV_IGNORE )
          continue;
        // discrete signal events requested, so always enqueue a message
        if( iter->second.flags&EV_DISCRETE )
        {
          repoll |= iter->second.pwp->enqueue(iter->second.msg.copy(DMI::WRITE));
        }
        else // else see if event is already enqueued & not delivered
        {
          // is a previous message still undelivered?
          // (WPInterface::poll() will reset its state to 0 when delivered)
          if( iter->second.msg->state() )
          {
            // simply ask for a repoll if the message is at top of queue
            if( iter->second.pwp->peekAtQueue() == iter->second.msg.deref_p() )
              iter->second.pwp->setNeedRepoll(repoll=True);
          }
          else // not in queue anymore, so enqueue a message
          {
            iter->second.msg().setState(1); // state=1 means undelivered
            repoll |= iter->second.pwp->enqueue(iter->second.msg.copy(DMI::WRITE));
          }
        }
        // remove signal if one-shot
        if( iter->second.flags&EV_ONESHOT )
          signals.erase(iter++);
        else
          iter++;
      }
    }
  }
  
  return repoll;
}



string Dispatcher::sdebug ( int detail,const string &,const char *name ) const
{
  string out;
  if( detail>=0 ) // basic detail
  {
    if( name )
      out = string(name) + "/";
    out += address.toString();
    if( detail>3 )
      out += Debug::ssprintf("/%08x",this);
  }
  if( detail >= 1 || detail == -1 )   // normal detail
  {
    Debug::appendf(out,"wps:%d",wps.size());
    if( !running )
      Debug::append(out,"stopped");
  }
  return out;
}
  //## end Dispatcher%3C7B6A3E00A0.declarations
//## begin module%3C7B7F30004B.epilog preserve=yes
//## end module%3C7B7F30004B.epilog
