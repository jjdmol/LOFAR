//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3C7B7F3000C5.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3C7B7F3000C5.cm

//## begin module%3C7B7F3000C5.cp preserve=no
//## end module%3C7B7F3000C5.cp

//## Module: WorkProcess%3C7B7F3000C5; Package body
//## Subsystem: PSCF%3C5A73670223
//## Source file: f:\lofar8\oms\LOFAR\cep\cpa\pscf\src\pscf\WorkProcess.cc

//## begin module%3C7B7F3000C5.additionalIncludes preserve=no
//## end module%3C7B7F3000C5.additionalIncludes

//## begin module%3C7B7F3000C5.includes preserve=yes
//## end module%3C7B7F3000C5.includes

// WPQueue
#include "WPQueue.h"
// Dispatcher
#include "Dispatcher.h"
// WorkProcess
#include "WorkProcess.h"
//## begin module%3C7B7F3000C5.declarations preserve=no
//## end module%3C7B7F3000C5.declarations

//## begin module%3C7B7F3000C5.additionalDeclarations preserve=yes
//## end module%3C7B7F3000C5.additionalDeclarations


// Class WorkProcess 

WorkProcess::WorkProcess()
  //## begin WorkProcess::WorkProcess%3C7B6A3702E5_const.hasinit preserve=no
  //## end WorkProcess::WorkProcess%3C7B6A3702E5_const.hasinit
  //## begin WorkProcess::WorkProcess%3C7B6A3702E5_const.initialization preserve=yes
  : id(0),queue(0),dsp(0)
  //## end WorkProcess::WorkProcess%3C7B6A3702E5_const.initialization
{
  //## begin WorkProcess::WorkProcess%3C7B6A3702E5_const.body preserve=yes
  //## end WorkProcess::WorkProcess%3C7B6A3702E5_const.body
}

WorkProcess::WorkProcess (AtomicID wpid)
  //## begin WorkProcess::WorkProcess%3C7CBB10027A.hasinit preserve=no
  //## end WorkProcess::WorkProcess%3C7CBB10027A.hasinit
  //## begin WorkProcess::WorkProcess%3C7CBB10027A.initialization preserve=yes
    : id(wpid),queue(0),dsp(0)
  //## end WorkProcess::WorkProcess%3C7CBB10027A.initialization
{
  //## begin WorkProcess::WorkProcess%3C7CBB10027A.body preserve=yes
  //## end WorkProcess::WorkProcess%3C7CBB10027A.body
}


WorkProcess::~WorkProcess()
{
  //## begin WorkProcess::~WorkProcess%3C7B6A3702E5_dest.body preserve=yes
  //## end WorkProcess::~WorkProcess%3C7B6A3702E5_dest.body
}



//## Other Operations (implementation)
void WorkProcess::attach (WPQueue* pq)
{
  //## begin WorkProcess::attach%3C7CBAED007B.body preserve=yes
  queue = pq;
  dsp = pq->getDsp();
  address = MsgAddress(id,dsp->processId(),dsp->hostId());
  //## end WorkProcess::attach%3C7CBAED007B.body
}

void WorkProcess::init ()
{
  //## begin WorkProcess::init%3C7F882B00E6.body preserve=yes
  //## end WorkProcess::init%3C7F882B00E6.body
}

void WorkProcess::start ()
{
  //## begin WorkProcess::start%3C7E4A99016B.body preserve=yes
  //## end WorkProcess::start%3C7E4A99016B.body
}

void WorkProcess::stop ()
{
  //## begin WorkProcess::stop%3C7E4A9C0133.body preserve=yes
  //## end WorkProcess::stop%3C7E4A9C0133.body
}

bool WorkProcess::subscribe (const HIID &id)
{
  //## begin WorkProcess::subscribe%3C7CB9B70120.body preserve=yes
  FailWhen( !isAttached(),"unattached wp");
  return queue->subscribe(id);
  //## end WorkProcess::subscribe%3C7CB9B70120.body
}

bool WorkProcess::unsubscribe (const HIID &id)
{
  //## begin WorkProcess::unsubscribe%3C7CB9C50365.body preserve=yes
  FailWhen( !isAttached(),"unattached wp");
  return queue->unsubscribe(id);
  //## end WorkProcess::unsubscribe%3C7CB9C50365.body
}

int WorkProcess::addTimeout (int ms, int flags, const HIID &id, void *data)
{
  //## begin WorkProcess::addTimeout%3C7D285803B0.body preserve=yes
  FailWhen( !isAttached(),"unattached wp");
  return dsp->addTimeout(queue,ms,flags,id,data);
  //## end WorkProcess::addTimeout%3C7D285803B0.body
}

int WorkProcess::addInput (int fd, int flags)
{
  //## begin WorkProcess::addInput%3C7D2874023E.body preserve=yes
  FailWhen( !isAttached(),"unattached wp");
  return dsp->addInput(queue,fd,flags);
  //## end WorkProcess::addInput%3C7D2874023E.body
}

int WorkProcess::addSignal (int signum)
{
  //## begin WorkProcess::addSignal%3C7DFE520239.body preserve=yes
  FailWhen( !isAttached(),"unattached wp");
  return dsp->addSignal(queue,signum);
  //## end WorkProcess::addSignal%3C7DFE520239.body
}

bool WorkProcess::removeTimeout (int handle)
{
  //## begin WorkProcess::removeTimeout%3C7D287F02C6.body preserve=yes
  FailWhen( !isAttached(),"unattached wp");
  return dsp->removeTimeout(queue,handle);
  //## end WorkProcess::removeTimeout%3C7D287F02C6.body
}

bool WorkProcess::removeInput (int handle)
{
  //## begin WorkProcess::removeInput%3C7D28A30141.body preserve=yes
  FailWhen( !isAttached(),"unattached wp");
  return dsp->removeInput(queue,handle);
  //## end WorkProcess::removeInput%3C7D28A30141.body
}

bool WorkProcess::removeSignal (int signum)
{
  //## begin WorkProcess::removeSignal%3C7DFE480253.body preserve=yes
  FailWhen( !isAttached(),"unattached wp");
  return dsp->removeSignal(queue,signum);
  //## end WorkProcess::removeSignal%3C7DFE480253.body
}

int WorkProcess::send (MessageRef &msg, MsgAddress to)
{
  //## begin WorkProcess::send%3C7CB9E802CF.body preserve=yes
  FailWhen( !isAttached(),"unattached wp");
  msg().setFrom(getAddress());
  msg().setState(getState());
  dprintf(2)("send [%s] to %s\n",msg->sdebug(1).c_str(),to.toString().c_str());
  return dsp->send(msg,to); 
  //## end WorkProcess::send%3C7CB9E802CF.body
}

int WorkProcess::publish (MessageRef &msg, int scope)
{
  //## begin WorkProcess::publish%3C7CB9EB01CF.body preserve=yes
  FailWhen( !isAttached(),"unattached wp");
  msg().setFrom(getAddress());
  msg().setState(getState());
  dprintf(2)("publish [%s] scope %d\n",msg->sdebug(1).c_str(),scope);
  return dsp->publish(msg,scope); 
  //## end WorkProcess::publish%3C7CB9EB01CF.body
}

int WorkProcess::timeout (int handle, const HIID &id, void* data)
{
  //## begin WorkProcess::timeout%3C7CC2AB02AD.body preserve=yes
  return Message::CANCEL;
  //## end WorkProcess::timeout%3C7CC2AB02AD.body
}

int WorkProcess::input (int handle, int fd, int flags)
{
  //## begin WorkProcess::input%3C7CC2C40386.body preserve=yes
  return Message::CANCEL;
  //## end WorkProcess::input%3C7CC2C40386.body
}

int WorkProcess::signal (int signum)
{
  //## begin WorkProcess::signal%3C7DFD240203.body preserve=yes
  return Message::CANCEL;
  //## end WorkProcess::signal%3C7DFD240203.body
}

// Additional Declarations
  //## begin WorkProcess%3C7B6A3702E5.declarations preserve=yes
string WorkProcess::sdebug ( int detail,const string &prefix,const char *nm ) const
{
  string out;
  if( detail>=0 ) // basic detail
  {
    out = Debug::ssprintf("%s/%s",nm?nm:name(),address.toString().c_str());
    if( detail>3 )
      out += Debug::ssprintf("/%08x",this);
  }
  if( detail >= 1 || detail == -1 )   // normal detail
  {
    Debug::appendf(out,"state %d",state);
  }
  return out;
}
  //## end WorkProcess%3C7B6A3702E5.declarations
//## begin module%3C7B7F3000C5.epilog preserve=yes
//## end module%3C7B7F3000C5.epilog
