//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3C95AADB010A.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3C95AADB010A.cm

//## begin module%3C95AADB010A.cp preserve=no
//## end module%3C95AADB010A.cp

//## Module: GWServerWP%3C95AADB010A; Package body
//## Subsystem: PSCF%3C5A73670223
//## Source file: F:\lofar8\oms\LOFAR\CEP\CPA\PSCF\src\pscf\GWServerWP.cc

//## begin module%3C95AADB010A.additionalIncludes preserve=no
//## end module%3C95AADB010A.additionalIncludes

//## begin module%3C95AADB010A.includes preserve=yes
//## end module%3C95AADB010A.includes

// GWServerWP
#include "GWServerWP.h"
//## begin module%3C95AADB010A.declarations preserve=no
//## end module%3C95AADB010A.declarations

//## begin module%3C95AADB010A.additionalDeclarations preserve=yes
// timeout value for a retry of bind, in seconds
const Timeval Timeout_Retry(10.0);
// max retries
const int MaxOpenRetries = 10;
//## end module%3C95AADB010A.additionalDeclarations


// Class GWServerWP 

GWServerWP::GWServerWP (const string &port1)
  //## begin GWServerWP::GWServerWP%3C8F95710177.hasinit preserve=no
  //## end GWServerWP::GWServerWP%3C8F95710177.hasinit
  //## begin GWServerWP::GWServerWP%3C8F95710177.initialization preserve=yes
  : WorkProcess(AidGWServerWP),port(port1),sock(0)
  //## end GWServerWP::GWServerWP%3C8F95710177.initialization
{
  //## begin GWServerWP::GWServerWP%3C8F95710177.body preserve=yes
  //## end GWServerWP::GWServerWP%3C8F95710177.body
}


GWServerWP::~GWServerWP()
{
  //## begin GWServerWP::~GWServerWP%3C8F942502BA_dest.body preserve=yes
  if( sock )
    delete sock;
  //## end GWServerWP::~GWServerWP%3C8F942502BA_dest.body
}



//## Other Operations (implementation)
void GWServerWP::start ()
{
  //## begin GWServerWP::start%3C90BE4A029B.body preserve=yes
  WorkProcess::start();
  open_retries = 0;
  tryOpen();
  //## end GWServerWP::start%3C90BE4A029B.body
}

void GWServerWP::stop ()
{
  //## begin GWServerWP::stop%3C90BE880037.body preserve=yes
  if( sock )
    delete sock;
  sock = 0;
  //## end GWServerWP::stop%3C90BE880037.body
}

int GWServerWP::timeout (const HIID &)
{
  //## begin GWServerWP::timeout%3C90BE8E000E.body preserve=yes
  // (since we only have one active timeout, we don't need no arguments)
  tryOpen();
  return Message::ACCEPT;
  //## end GWServerWP::timeout%3C90BE8E000E.body
}

int GWServerWP::input (int , int )
{
  //## begin GWServerWP::input%3C95B4DC031C.body preserve=yes
  // This is called when an incoming connection is requested
  // (since we only have one active input, we don't need no arguments)
  if( !sock ) // sanity check
    return Message::CANCEL;
  // do an accept on the socket
  Socket *newsock = sock->accept();
  if( newsock ) // success? Launch a Gateway WP to manage it
  {
    dprintf(1)("accepted new connection, launching gateway\n");
    attachWP(new GatewayWP(newsock),DMI::ANON);
    return Message::ACCEPT;
  }
  else
  { 
    dprintf(1)("accept error: %s. Closing and retrying\n",sock->errstr().c_str());
    // just to be anal, close the socket and retry binding it
    open_retries = 0;
    tryOpen();
    return Message::CANCEL;
  }
  //## end GWServerWP::input%3C95B4DC031C.body
}

// Additional Declarations
  //## begin GWServerWP%3C8F942502BA.declarations preserve=yes
void GWServerWP::tryOpen ()
{
  // Try to start a server socket
  if( sock )
    delete sock;
  sock = new Socket("sock/"+wpname(),port,Socket::TCP,10);
  dprintf(1)("opening server socket: result %d\n",sock->errcode());
  if( !sock->ok() )
  {
    if( sock->errcode() == Socket::BIND )
    {
      // if bind error, assume another process already has it open,
      // so launch a GWClientWP to attach to it, and commit harakiri
      dprintf(1)("socket bind error, launching client mode\n");
      attachWP(new GWClientWP("localhost",port),DMI::ANON);
      detachMyself();
    }
    else // some other error
    {
      dprintf(1)("fatal error: %s\n",sock->errstr().c_str());
      delete sock; sock=0;
      if( open_retries++ > MaxOpenRetries )
      {
        dprintf(1)("too many retries, giving up\n");
        detachMyself();
      }
      else // retry later - schedule a timeout
      {
        dprintf(1)("will retry later\n");
        addTimeout(Timeout_Retry,0,EV_ONESHOT);
      }
    }
    return;
  }
  // since we got here, the socket is OK. Add an input on it
  addInput(sock->getSid(),EV_FDREAD);
}
  //## end GWServerWP%3C8F942502BA.declarations
//## begin module%3C95AADB010A.epilog preserve=yes
//## end module%3C95AADB010A.epilog


