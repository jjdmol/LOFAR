//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3C95AADB0170.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3C95AADB0170.cm

//## begin module%3C95AADB0170.cp preserve=no
//## end module%3C95AADB0170.cp

//## Module: GWClientWP%3C95AADB0170; Package body
//## Subsystem: PSCF%3C5A73670223
//## Source file: F:\lofar8\oms\LOFAR\CEP\CPA\PSCF\src\GWClientWP.cc

//## begin module%3C95AADB0170.additionalIncludes preserve=no
//## end module%3C95AADB0170.additionalIncludes

//## begin module%3C95AADB0170.includes preserve=yes
//## end module%3C95AADB0170.includes

// GWClientWP
#include "GWClientWP.h"
//## begin module%3C95AADB0170.declarations preserve=no
//## end module%3C95AADB0170.declarations

//## begin module%3C95AADB0170.additionalDeclarations preserve=yes
const Timeval ReconnectTimeout(.2),
              ReopenTimeout(2.0),
              FailConnectTimeout(10.0);
              
const HIID    BYE(AidBye|AidGatewayWP|AidWildcard),
              REMOTE_UP(AidRemote|AidUp|AidWildcard),
              SERVER_LIST(AidServer|AidList);
//## end module%3C95AADB0170.additionalDeclarations


// Class GWClientWP 

GWClientWP::GWClientWP (const vector<string> &connlist)
  //## begin GWClientWP::GWClientWP%3C95A9410081.hasinit preserve=no
  //## end GWClientWP::GWClientWP%3C95A9410081.hasinit
  //## begin GWClientWP::GWClientWP%3C95A9410081.initialization preserve=yes
  : WorkProcess(AidGWClientWP)
  //## end GWClientWP::GWClientWP%3C95A9410081.initialization
{
  //## begin GWClientWP::GWClientWP%3C95A9410081.body preserve=yes
  // parse the connection list
  for( vector<string>::const_iterator iter = connlist.begin(); iter != connlist.end(); iter++ )
  {
    string::size_type pos = iter->find(':');
    if( pos < iter->length()-1 )
    {
      string host = pos>0 ? iter->substr(0,pos) : "localhost";
      string port = iter->substr(pos+1);
      if( !find(host,port) )
      {
        Connection cx;
        cx.host = pos>0 ? iter->substr(0,pos) : "localhost";
        cx.port = iter->substr(pos+1);
        cx.sock = 0;
        cx.state = STOPPED;
        conns.push_back(cx);
      }
    }
  }
  setState(STOPPED);
  //## end GWClientWP::GWClientWP%3C95A9410081.body
}


GWClientWP::~GWClientWP()
{
  //## begin GWClientWP::~GWClientWP%3C95A941002E_dest.body preserve=yes
  for( CCLI iter = conns.begin(); iter != conns.end(); iter++ )
    if( iter->sock )
      delete iter->sock;
  //## end GWClientWP::~GWClientWP%3C95A941002E_dest.body
}



//## Other Operations (implementation)
void GWClientWP::init ()
{
  //## begin GWClientWP::init%3CA1C0C300FA.body preserve=yes
  // Bye messages from child gateways tell us when they have closed
  // (and thus need to be reopened)
  subscribe(BYE,Message::LOCAL);
  // Remote.Up messages tell us when a new remote node has been spotted
  subscribe(REMOTE_UP,Message::LOCAL);
  //## end GWClientWP::init%3CA1C0C300FA.body
}

void GWClientWP::start ()
{
  //## begin GWClientWP::start%3C95A941008B.body preserve=yes
  WorkProcess::start();
  Timestamp::now(&now);
  setState(CONNECTING);
  // initiate connection on every socket
  for( CLI iter = conns.begin(); iter != conns.end(); iter++ )
    tryConnect(*iter);
  // add a re-open timeout
  addTimeout(ReopenTimeout,AidReopen);
  //## end GWClientWP::start%3C95A941008B.body
}

void GWClientWP::stop ()
{
  //## begin GWClientWP::stop%3C95A9410092.body preserve=yes
  setState(STOPPED);
  for( CLI iter = conns.begin(); iter != conns.end(); iter++ )
  {
    if( iter->sock )
      delete iter->sock;
    iter->sock = 0;
    iter->state = STOPPED;
  }
  //## end GWClientWP::stop%3C95A9410092.body
}

int GWClientWP::timeout (const HIID &id)
{
  //## begin GWClientWP::timeout%3C95A9410093.body preserve=yes
  bool connecting = False;
  Timestamp::now(&now);
  // go thru connection list and figure out what to do
  for( CLI iter = conns.begin(); iter != conns.end(); iter++ )
  {
    switch( iter->state )
    {
      case CONNECTED:   // do nothing
        break;
      
      case CONNECTING:  // connecting?
        if( now >= iter->fail ) // check for timeout
        {
          lprintf(1,"error: timeout connecting to %s:%s\n",
              iter->host.c_str(),iter->port.c_str());
          delete iter->sock; iter->sock = 0;
          iter->state = WAITING;
          iter->retry = now + ReopenTimeout;
          break;
        }
        
      case STOPPED:
      case WAITING:    // not connecting - is it time for a retry?
        if( now >= iter->retry )
          tryConnect(*iter);
        break;
      
      default:
        lprintf(1,"error: unexpected state %d for %s:%s\n",iter->state,
            iter->host.c_str(),iter->port.c_str());
    }
    if( iter->state == CONNECTING )
      connecting = True;
  }
  // if reconnecting timeout (i.e. short one) is set, and no-one
  // seems to be connecting, then cancel it
  if( id == AidReconnect && !connecting )
  {
    reconnect_timeout_set = False;
    return Message::CANCEL;
  }
  if( connecting && !reconnect_timeout_set )
  {
    addTimeout(ReconnectTimeout,AidReconnect);
    reconnect_timeout_set = True;
  }
  
  return Message::ACCEPT;
  //## end GWClientWP::timeout%3C95A9410093.body
}

int GWClientWP::receive (MessageRef& mref)
{
  //## begin GWClientWP::receive%3C95A9410095.body preserve=yes
  const Message &msg = mref.deref();
  const HIID &id = msg.id();
  // bye message from child? Have to reopen its gateway then
  if( id.matches(BYE) )
  {
    Connection *cx = find(msg.from());
    if( cx )
    {
      lprintf(1,"caught Bye from child gateway for %s:%s, waking up\n",
          cx->host.c_str(),cx->port.c_str());
      if( cx->state == CONNECTED ) // just to make sure it's not a stray message
      {
        cx->state = WAITING;
        tryConnect(*cx);
      }
    }
    else
    {
      lprintf(1,"error: caught Bye from unknown child gateway %s\n",
          msg.from().toString().c_str());
    }
  }
  return Message::ACCEPT;
  //## end GWClientWP::receive%3C95A9410095.body
}

GWClientWP::Connection * GWClientWP::find (const string &host, const string &port)
{
  //## begin GWClientWP::find%3CA1C0030307.body preserve=yes
  for( CLI iter = conns.begin(); iter != conns.end(); iter++ )
    if( iter->host == host && iter->port == port )
      return &(*iter);
  return 0;
  //## end GWClientWP::find%3CA1C0030307.body
}

GWClientWP::Connection * GWClientWP::find (const MsgAddress &gw)
{
  //## begin GWClientWP::find%3CA1C52E0108.body preserve=yes
  for( CLI iter = conns.begin(); iter != conns.end(); iter++ )
    if( iter->gw == gw )
      return &(*iter);
  return 0;
  //## end GWClientWP::find%3CA1C52E0108.body
}

// Additional Declarations
  //## begin GWClientWP%3C95A941002E.declarations preserve=yes
void GWClientWP::tryConnect (Connection &cx)
{
  if( cx.state == CONNECTED ) // ignore if connected
    return;
  if( cx.state != CONNECTING ) // try to reconnect completely
  {
    if( cx.sock )
      delete cx.sock;
    lprintf(1,"creating client socket for %s:%s\n",
              cx.host.c_str(),cx.port.c_str());
    cx.sock = new Socket("cx.sock/"+wpname(),cx.host,cx.port,Socket::TCP,-1);
    cx.state = CONNECTING;
    cx.fail = now  + FailConnectTimeout; 
    // fall thru to connection attempt, below
  }
  if( !cx.sock )  // sanity check
    return;
  // [re]try connection attempt
  int res = cx.sock->connect(0);
  dprintf(3)("connect(%s:%s): %d (%s)\n",cx.host.c_str(),cx.port.c_str(),
                  res,cx.sock->errstr().c_str());
  if( res > 0 ) // connection established
  {
    lprintf(1,"connected to %s:%s, spawning child gateway\n",
                      cx.host.c_str(),cx.port.c_str());
    cx.state = CONNECTED;
    // spawn a new child gateway, subscribe to its Bye message
    cx.gw   = attachWP( new GatewayWP(cx.sock),DMI::ANON );
    cx.sock = 0; // socket is taken over by child
  }
  else if( !res )  // in progress
  {
    dprintf(3)("connect still in progress\n");
    if( !reconnect_timeout_set )
    {
      addTimeout(ReconnectTimeout,AidReconnect);
      reconnect_timeout_set = True;
    }
  }
  else // else it is a fatal error, close and retry
  {
    lprintf(2,"error: %s\nwill retry later",cx.sock->errstr().c_str());
    delete cx.sock; cx.sock = 0;
    cx.state = WAITING;
    cx.retry = now + ReopenTimeout;
  }
}
  //## end GWClientWP%3C95A941002E.declarations
//## begin module%3C95AADB0170.epilog preserve=yes
//## end module%3C95AADB0170.epilog
