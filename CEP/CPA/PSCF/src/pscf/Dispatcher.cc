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
//## Source file: f:\lofar8\oms\LOFAR\cep\cpa\pscf\src\pscf\Dispatcher.cc

//## begin module%3C7B7F30004B.additionalIncludes preserve=no
//## end module%3C7B7F30004B.additionalIncludes

//## begin module%3C7B7F30004B.includes preserve=yes
#include <sys/time.h>
//## end module%3C7B7F30004B.includes

// WorkProcess
#include "WorkProcess.h"
// WPQueue
#include "WPQueue.h"
// Dispatcher
#include "Dispatcher.h"
//## begin module%3C7B7F30004B.declarations preserve=no
//## end module%3C7B7F30004B.declarations

//## begin module%3C7B7F30004B.additionalDeclarations preserve=yes
Dispatcher * Dispatcher::dispatcher = 0;
sigset_t Dispatcher::raisedSignals;
      
// static signal handler
void Dispatcher::signalHandler (int signum,siginfo_t *,void *)
{
  sigaddset(&raisedSignals,signum);
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
  address = MsgAddress(MsgAddress::DISPATCHER,process,host);
  sigalrm_handled = False;
  running = False;
  dprintf(1)("created\n");
  //## end Dispatcher::Dispatcher%3C7CD444039C.body
}


Dispatcher::~Dispatcher()
{
  //## begin Dispatcher::~Dispatcher%3C7B6A3E00A0_dest.body preserve=yes
  if( running )
    stop();
  for( WPQI iter = queues.begin(); iter != queues.end(); iter++ )
    if( iter->second )
      delete iter->second;
  dispatcher = 0;
  //## end Dispatcher::~Dispatcher%3C7B6A3E00A0_dest.body
}



//## Other Operations (implementation)
void Dispatcher::attach (WorkProcess *wp)
{
  //## begin Dispatcher::attach%3C7B885A027F.body preserve=yes
  FailWhen( wp->isAttached(),"wp is already attached" );
  FailWhen( queues.find(wp->getId()) != queues.end(),
      "wpid '"+wp->getId().toString()+"' is already in use");
  queues[wp->getId()] = new WPQueue(this,wp);
  dprintf(1)("attaching WP: %s\n",wp->debug());
  if( running )
  {
    wp->init();
    wp->start();
  }
  //## end Dispatcher::attach%3C7B885A027F.body
}

void Dispatcher::start ()
{
  //## begin Dispatcher::start%3C7DFF770140.body preserve=yes
  running = True;
  // say init to all WPs
  dprintf(1)("starting");
  dprintf(2)("start: initializing WPs\n");
  for( WPQI iter = queues.begin(); iter != queues.end(); iter++ )
    iter->second->getWp()->init();
  // setup signal handler for SIGALRM
  sigemptyset(&raisedSignals);
  struct sigaction sa;
  sa.sa_handler = 0;
  sa.sa_sigaction = Dispatcher::signalHandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_SIGINFO;
  sigaction(SIGALRM,&sa,&orig_sigalrm_sa);
  sigalrm_handled = True;
  // setup heartbeat timer
  struct itimerval tval = { {0,1000000/heartbeat_hz},{0,1000000/heartbeat_hz} };
  setitimer(ITIMER_REAL,&tval,0);
  // say start to all WPs
  dprintf(2)("start: starting WPs\n");
  for( WPQI iter = queues.begin(); iter != queues.end(); iter++ )
    iter->second->getWp()->start();
  dprintf(2)("start: complete\n");
  //## end Dispatcher::start%3C7DFF770140.body
}

void Dispatcher::stop ()
{
  //## begin Dispatcher::stop%3C7E0270027B.body preserve=yes
  running = False;
  struct itimerval tval = { {0,0},{0,0} };
  setitimer(ITIMER_REAL,&tval,0);
  if( sigalrm_handled )
  {
    sigaction(SIGALRM,&orig_sigalrm_sa,0);
    sigalrm_handled = False;
  }
  dprintf(1)("stopping\n");
  for( WPQI iter = queues.begin(); iter != queues.end(); iter++ )
    iter->second->getWp()->stop();
  //## end Dispatcher::stop%3C7E0270027B.body
}

int Dispatcher::send (MessageRef &msg, const MsgAddress &to)
{
  //## begin Dispatcher::send%3C7B8867015B.body preserve=yes
  dprintf(2)("send(%s,%s)\n",msg->sdebug().c_str(),to.toString().c_str());
  int ndeliver = 0;
  msg().setTo(to);
  // check that host/process spec matches ourselves;
  if( ( to.host() == MsgAddress::LOCAL || 
        to.host() == MsgAddress::BROADCAST ||
        to.host() == hostId() ) &&
      ( to.process() == MsgAddress::LOCAL || 
        to.process() == MsgAddress::BROADCAST ||
        to.process() == processId() ) )
  {
    // message being broadcast to all WPs -- place in all queues
    if( to.wp() == MsgAddress::BROADCAST )
    {
      dprintf(2)("  broadcasting to all WPs\n");
      for( WPQI iter = queues.begin(); iter != queues.end(); iter++ )
      {
        repoll |= iter->second->place(msg);
        ndeliver++;
      }
    }
    else // addressed to a specific WP 
    {
      WPQI iter = queues.find(to.wp());
      if( iter != queues.end() )
      {
        dprintf(2)("  queing for: %s\n",iter->second->debug(1));
        repoll |= iter->second->place(msg);
        ndeliver++;
      }
    }
  }
  return ndeliver;
  //## end Dispatcher::send%3C7B8867015B.body
}

int Dispatcher::publish (MessageRef &msg, int scope)
{
  //## begin Dispatcher::publish%3C7CAE9B01E8.body preserve=yes
  dprintf(2)("publish(%s,%d)\n",msg->sdebug().c_str(),scope);
  int ndeliver = 0;
  const HIID &id( msg->id() );
  // see who subsribes locally, place messages into their queues
  for( WPQI iter = queues.begin(); iter != queues.end(); iter++ )
    if( iter->second->subscribesTo(id) )
    {
      dprintf(2)("  queing for: %s\n",iter->second->debug(1));
      repoll |= iter->second->place(msg);
      ndeliver++;
    }
  // this code is just a placeholder: in reality, globally-published messages
  // should be communicated to other dispatchers
  if( scope == Message::GLOBAL )
    ndeliver++;
  return ndeliver;
  //## end Dispatcher::publish%3C7CAE9B01E8.body
}

void Dispatcher::poll ()
{
  //## begin Dispatcher::poll%3C7B888E01CF.body preserve=yes
  // main polling loop
  while( repoll )
  {
    repoll = False;
//    checkInputs();
//    checkTimeouts();
    // find queue with maximum priority
    int maxpri = Message::MIN_PRIORITY;
    WPQueue *pq = 0;
    for( WPQI qiter = queues.begin(); qiter != queues.end(); qiter++ )
    {
      if( qiter->second->repoll() && !qiter->second->queueLocked() &&
          qiter->second->topPriority() >= maxpri )
      {
        pq = qiter->second;
        maxpri = pq->topPriority();
      }
    }
    // deliver message, if found
    if( pq )
    {
      dprintf(3)("poll: max priority %d in %s\n",maxpri,pq->debug(1));
      repoll |= pq->performDelivery();
    }
  }
  //## end Dispatcher::poll%3C7B888E01CF.body
}

int Dispatcher::addTimeout (WPQueue *wpq, int ms, int flags, const HIID &id, void* data)
{
  //## begin Dispatcher::addTimeout%3C7D28C30061.body preserve=yes
  //## end Dispatcher::addTimeout%3C7D28C30061.body
}

int Dispatcher::addInput (WPQueue *wpq, int fd, int flags)
{
  //## begin Dispatcher::addInput%3C7D28E3032E.body preserve=yes
  //## end Dispatcher::addInput%3C7D28E3032E.body
}

int Dispatcher::addSignal (WPQueue *wpq, int signum)
{
  //## begin Dispatcher::addSignal%3C7DFF4A0344.body preserve=yes
  //## end Dispatcher::addSignal%3C7DFF4A0344.body
}

bool Dispatcher::removeTimeout (WPQueue *wpq, int handle)
{
  //## begin Dispatcher::removeTimeout%3C7D28F202F3.body preserve=yes
  //## end Dispatcher::removeTimeout%3C7D28F202F3.body
}

bool Dispatcher::removeInput (WPQueue *wpq, int handle)
{
  //## begin Dispatcher::removeInput%3C7D2947002F.body preserve=yes
  //## end Dispatcher::removeInput%3C7D2947002F.body
}

bool Dispatcher::removeSignal (WPQueue *wpq, int signum)
{
  //## begin Dispatcher::removeSignal%3C7DFF57025C.body preserve=yes
  //## end Dispatcher::removeSignal%3C7DFF57025C.body
}

// Additional Declarations
  //## begin Dispatcher%3C7B6A3E00A0.declarations preserve=yes
string Dispatcher::sdebug ( int detail,const string &prefix,const char *name ) const
{
  string out;
  if( detail>=0 ) // basic detail
  {
    out = Debug::ssprintf("%s/%s",name?name:"Dispatcher",address.toString().c_str());
    if( detail>3 )
      out += Debug::ssprintf("/%08x",this);
  }
  if( detail >= 1 || detail == -1 )   // normal detail
  {
    Debug::appendf(out,"wps:%d",queues.size());
    if( !running )
      Debug::append(out,"stopped");
  }
  return out;
}
  //## end Dispatcher%3C7B6A3E00A0.declarations

//## begin module%3C7B7F30004B.epilog preserve=yes
//## end module%3C7B7F30004B.epilog
