#include "OctoMultiplexer.h"
#include "OctoAgent.h"
#include "OCTOPUSSY/WPInterface.h"

//##ModelId=3E26BE240137
OctoMultiplexer::OctoMultiplexer (AtomicID wpid)
    : proxy(wpid)
{
  nassign = 0;
}

//##ModelId=3E26BE6701CD
OctoMultiplexer::OctoMultiplexer (const OctoMultiplexer& right)
    : proxy(right.proxy)
{
  nassign = 0;
}

//##ModelId=3E26BE670240
OctoMultiplexer::~OctoMultiplexer ()
{
  for( uint i=0; i<agents.size(); i++ )
    delete agents[i];
  agents.clear();
}

//##ModelId=3E26BE670273
OctoMultiplexer& OctoMultiplexer::operator= (const OctoMultiplexer& right)
{
  if( this != &right )
  {
    proxy = right.proxy;
    // pointers to agents are not copied over
    for( uint i=0; i<agents.size(); i++ )
      delete agents[i];
    agents.clear();
    event_assignment.clear();
    event_id.clear();
    nassign = 0;
  }
  return *this;
}

//##ModelId=3E26BE760036
OctoMultiplexer & OctoMultiplexer::addAgent (OctoAgent* agent)
{
  agent->setMultiplexId(agents.size());
  agents.push_back(agent);
  event_assignment.resize(agents.size());
  event_id.resize(agents.size());
  return *this;
}


//##ModelId=3E3FC3A7000B
int OctoMultiplexer::checkQueue (const HIID& mask,int wait,int agent_id)
{
  WPInterface &wp = proxy.wp();
  if( !wp.isRunning() )
    return AppAgent::CLOSED;
  
//  // lock the queue mutex
//  Thread::Mutex::Lock lock( wp.queueCondition() );
  
  // if !wait and queue is empty, return the WAIT code
  if( wait == AppAgent::NOWAIT && wp.queue().empty() )
    return AppAgent::WAIT;
  // loop until something is received (will return from loop below)
  for(;;)
  {
    // wait until something gets into the queue
    while( wp.queue().empty() && wp.isRunning() )
      wp.queueCondition().wait();
    // have we stopped? return False
    if( !wp.isRunning() )
      return AppAgent::CLOSED;
    // check first message in queue and see which agents it should
    // be assigned to. If headmsg is valid and points
    // to the same mesage object, then we've already done the assignment
    // check previously.
    WPInterface::QueueEntry &qe = wp.queue().front();
    if( !headmsg.valid() || nassign == 0 || headmsg != qe.mref )
    {
      headmsg.copy(qe.mref,DMI::PRESERVE_RW);
      const Message &msg = headmsg.deref();
      cdebug(3)<<"checking message "<<msg.sdebug(2)<<endl;
      // check against input map of every agent
      for( uint i=nassign=0; i<agents.size(); i++ )
      {
        event_assignment[i] = agents[i]->mapReceiveEvent(event_id[i],msg.id());
        if( event_assignment[i] )
        {
          cdebug(3)<<"maps to "<<agents[i]->sdebug(1)<<", event "<<event_id[i]<<endl;
          nassign++;
        }
      }
    }
    // message assigned to some agents?
    if( nassign )
    {
      if( event_assignment[agent_id] &&
          ( !mask.length() || mask.matches(event_id[agent_id]) ) ) // to requested agent? success.
      {
        return AppAgent::SUCCESS;
      }
      else if( wait != AppAgent::BLOCK ) // return out-of-seq code unless blocking mode is requested
      {
        return AppAgent::OUTOFSEQ;
      }
      else
      { // block until something happens to queue, then go back up for another check
        wp.queueCondition().wait();
      }
    }
    else
    {
      // no, so discard this message and go back to wait for another one
      cdebug(3)<<"unmapped message, discarding\n";
      wp.queue().pop_front();
    }
  }
}

//##ModelId=3E26D2D6021D
int OctoMultiplexer::getEvent (HIID& id,ObjRef& data,const HIID& mask,int wait,int agent_id)
{
  // return CLOSED if WP is stopped
  WPInterface &wp = proxy.wp();
  if( !wp.isRunning() )
    return AppAgent::CLOSED;
  // lock the queue
  Thread::Mutex::Lock lock(wp.queueCondition());
  // check the queue, and if not successful, return the code immediately
  int res = checkQueue(mask,wait,agent_id);
  if( res != AppAgent::SUCCESS )
    return res;
  // sanity checks
  FailWhen(wp.queue().empty(),"checkQueue() failure: queue empty");
  FailWhen(!headmsg.valid(),"checkQueue() failure: headmsg not valid");
  FailWhen(nassign<=0 || !event_assignment[agent_id],
           "checkQueue() failure: event not assigned to this agent");
  // return the message as event to the agent
  id = event_id[agent_id];
  if( headmsg->payload().valid() )
    data.copy(headmsg->payload(),DMI::PRESERVE_RW);
  cdebug(2)<<"returning event "<<id<<" to agent "<<agents[agent_id]->sdebug(2)<<endl;
  // clear event assignment for this agent, and delete message if it is
  // not assigned to anyone else
  event_assignment[agent_id] = False;
  nassign--;
  if( !nassign )
  {
    cdebug(3)<<"message completely assigned: dequeueing"<<endl;
    headmsg.detach();
    wp.queue().pop_front();
  }
  return AppAgent::SUCCESS;
}

//##ModelId=3E3FC3A601B0
int OctoMultiplexer::hasEvent (const HIID& mask,int agent_id)
{
  // return CLOSED if WP is stopped
  WPInterface &wp = proxy.wp();
  if( !wp.isRunning() )
    return AppAgent::CLOSED;
  // lock the queue
  Thread::Mutex::Lock lock(wp.queueCondition());
  // check the queue and return the code
  return checkQueue(mask,AppAgent::NOWAIT,agent_id);
}


//##ModelId=3E26D91A02E3
bool OctoMultiplexer::subscribe (const HIID &id, int scope)
{
  return proxy.wp().subscribe(id,scope);
}

//##ModelId=3E26D7CA01DA
int OctoMultiplexer::publish (MessageRef &mref, int scope)
{
  if( !proxy.isRunning() )
    return 0;
  return proxy.wp().publish(mref,scope);
}


string OctoMultiplexer::sdebug (int detail, const string &prefix, const char *name) const
{
  using Debug::append;
  using Debug::appendf;
  using Debug::ssprintf;
  
  string out;
  if( detail >= 0 ) // basic detail
  {
    appendf(out,"%s/%08x",name?name:"OctoMUX",(int)this);
  }
  if( detail >= 1 || detail == -1 )
  {
    appendf(out,"agents:%d",agents.size());
    appendf(out,"wp:%s",proxy.wp().debug(abs(detail),prefix));
  }
  return out;
}

