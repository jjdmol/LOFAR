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
//## Source file: F:\lofar8\oms\LOFAR\CEP\CPA\PSCF\src\pscf\GWClientWP.cc

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
//## end module%3C95AADB0170.additionalDeclarations


// Class GWClientWP 

GWClientWP::GWClientWP (const string &host1, const string &port1)
  //## begin GWClientWP::GWClientWP%3C95A9410081.hasinit preserve=no
  //## end GWClientWP::GWClientWP%3C95A9410081.hasinit
  //## begin GWClientWP::GWClientWP%3C95A9410081.initialization preserve=yes
  : WorkProcess(AidGWClientWP),host(host1),port(port1),sock(0)
  //## end GWClientWP::GWClientWP%3C95A9410081.initialization
{
  //## begin GWClientWP::GWClientWP%3C95A9410081.body preserve=yes
  setState(STOPPED);
  //## end GWClientWP::GWClientWP%3C95A9410081.body
}


GWClientWP::~GWClientWP()
{
  //## begin GWClientWP::~GWClientWP%3C95A941002E_dest.body preserve=yes
  if( sock )
    delete sock;
  //## end GWClientWP::~GWClientWP%3C95A941002E_dest.body
}



//## Other Operations (implementation)
void GWClientWP::start ()
{
  //## begin GWClientWP::start%3C95A941008B.body preserve=yes
  WorkProcess::start();
  tryConnect();
  //## end GWClientWP::start%3C95A941008B.body
}

void GWClientWP::stop ()
{
  //## begin GWClientWP::stop%3C95A9410092.body preserve=yes
  setState(STOPPED);
  if( sock )
    delete sock;
  sock = 0;
  //## end GWClientWP::stop%3C95A9410092.body
}

int GWClientWP::timeout (const HIID &id)
{
  //## begin GWClientWP::timeout%3C95A9410093.body preserve=yes
  // fail timeout?
  if( id[0] == AidFailConnect )
  {
    if( state() == CONNECTING ) // still connecting? Forget it
    {
      dprintf(1)("connect() failed: timeout\n");
      delete sock; sock = 0;
      setState(WAITING);
      addTimeout(ReopenTimeout,AidReopen,EV_ONESHOT);
    }
  }
  else // let tryConnect() figure out what to do, depending on state
    tryConnect();
  return Message::ACCEPT;
  //## end GWClientWP::timeout%3C95A9410093.body
}

int GWClientWP::receive (MessageRef& mref)
{
  //## begin GWClientWP::receive%3C95A9410095.body preserve=yes
  if( mref->id() == child_bye )
  {
    dprintf(1)("caught Bye from child gateway, waking up\n");
    unsubscribe(mref->id());
    if( state() == CONNECTED ) // just to make sure it's not a stray message
    {
      setState(WAITING);
      tryConnect();
    }
  }
  return Message::ACCEPT;
  //## end GWClientWP::receive%3C95A9410095.body
}

// Additional Declarations
  //## begin GWClientWP%3C95A941002E.declarations preserve=yes
void GWClientWP::tryConnect ()
{
  if( state() == CONNECTED ) // ignore if connected
    return;
  else if( state() != CONNECTING ) // try to reconnect completely
  {
    if( sock )
      delete sock;
    dprintf(1)("creating client socket\n");
    sock = new Socket("sock/"+wpname(),host,port,Socket::TCP,-1);
    setState(CONNECTING);
    // schedule a fail timeout in case connect() hangs
    addTimeout(FailConnectTimeout,AidFailConnect,EV_ONESHOT);
    // fall thru to connection attempt, below
  }
  if( state() == CONNECTING )  // retry a connect() call?
  {
    if( !sock )  // sanity check
      return;
    int res = sock->connect(0);
    dprintf(3)("connect: %d (%s)\n",res,sock->errstr().c_str());
    if( res > 0 ) // connection established
    {
      dprintf(1)("connected, spawning child gateway\n");
      setState(CONNECTED);
      removeTimeout(AidWildcard); // remove all timeouts
      reconnect_timeout_set = False;
      // spawn a new child gateway, subscribe to its Bye message
      child_bye = AidMsgBye | 
          attachWP( new GatewayWP(sock),DMI::ANON );
      subscribe(child_bye);
      sock = 0; // socket is taken over by child
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
      dprintf(1)("fatal error, closing and retrying later\n");
      delete sock; sock = 0;
      setState(WAITING);
      addTimeout(ReopenTimeout,AidReopen,EV_ONESHOT);
    }
  }
}
  //## end GWClientWP%3C95A941002E.declarations
//## begin module%3C95AADB0170.epilog preserve=yes
//## end module%3C95AADB0170.epilog
