#include "OctoMultiplexer.h"
#include "OctoAgent.h"
#include "OCTOPUSSY/WPInterface.h"

//##ModelId=3E26BE240137
OctoMultiplexer::OctoMultiplexer (AtomicID wpid)
    : proxy(wpid)
{
  nevents = 0;
}

//##ModelId=3E26BE6701CD
OctoMultiplexer::OctoMultiplexer (const OctoMultiplexer& right)
    : proxy(right.proxy)
{
  nevents = 0;
}

//##ModelId=3E26BE670240
OctoMultiplexer::~OctoMultiplexer ()
{
}

//##ModelId=3E26BE670273
OctoMultiplexer& OctoMultiplexer::operator= (const OctoMultiplexer& right)
{
  if( this != &right )
  {
    proxy = right.proxy;
    // pointers to agents are not copied over
    agents.clear();
    nevents = 0;
  }
  return *this;
}

//##ModelId=3E26BE760036
void OctoMultiplexer::addAgent (OctoAgent* agent)
{
  agents.push_back(agent);
}

//##ModelId=3E26D2D6021D
int OctoMultiplexer::pollEvents (bool wait)
{
  // nevents is the number of agents that have an event pending.
  // If >0, then we can't poll for more events until all these events have
  // been claimed by the app.
  FailWhen(nevents<0,"unexpected nevents value");
  if( nevents )
    return AppAgent::OUTOFSEQ;
  
  WPInterface &wp = proxy.wp();
  if( !wp.isRunning() )
    return AppAgent::CLOSED;
  
  // lock the queue mutex
  Thread::Mutex::Lock lock( wp.queueCondition() );
  // if !wait and queue is empty, return the WAIT code
  if( !wait && wp.queue().empty() )
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
    // dequeue first message
    WPInterface::QueueEntry qe = wp.queue().front();
    wp.queue().pop_front();
    const Message &msg = qe.mref.deref();
    cdebug(3)<<"received message "<<msg.sdebug(2)<<endl;
    // check against input map of every agent?
    for( AgentList::const_iterator iter = agents.begin(); iter != agents.end(); iter++ )
    {
      HIID id;
      if( (*iter)->mapReceiveEvent(id,msg.id()) )
      {
        (*iter)->cacheEvent(id,msg.payload());
        cdebug(3)<<"maps to "<<(*iter)->sdebug(1)<<", event "<<id<<endl;
        nevents++;
      }
    }
    if( nevents )
      return AppAgent::SUCCESS;
    // no, so ignore this message and wait for another one
    cdebug(3)<<"unmapped message, ignoring\n";
  }
}

//##ModelId=3E26D2D602E5
void OctoMultiplexer::claimEvent ()
{
  FailWhen(nevents<=0,"unexpected claimEvent call");
  nevents--;
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

