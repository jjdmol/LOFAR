//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3C90D43B0096.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3C90D43B0096.cm

//## begin module%3C90D43B0096.cp preserve=no
//## end module%3C90D43B0096.cp

//## Module: Socket%3C90D43B0096; Package body
//## Subsystem: Networking%3C90D442031F
//## Source file: F:\lofar8\oms\LOFAR\src-links\PSCF\Socket.cc

//## begin module%3C90D43B0096.additionalIncludes preserve=no
//## end module%3C90D43B0096.additionalIncludes

//## begin module%3C90D43B0096.includes preserve=yes
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include "PSCFDebugContext.h"
//## end module%3C90D43B0096.includes

// Socket
#include "PSCF/Socket.h"
//## begin module%3C90D43B0096.declarations preserve=no
//## end module%3C90D43B0096.declarations

//## begin module%3C90D43B0096.additionalDeclarations preserve=yes
InitDebugSubContext(Socket,PSCFDebugContext,"Socket");
//## end module%3C90D43B0096.additionalDeclarations


// Class Socket 

Socket::Socket (const string &sname)
  //## begin Socket::Socket%3C91BA4300F6.hasinit preserve=no
  //## end Socket::Socket%3C91BA4300F6.hasinit
  //## begin Socket::Socket%3C91BA4300F6.initialization preserve=yes
    : name(sname),errcode_(Socket::NOINIT),sid(-1),bound(False)
  //## end Socket::Socket%3C91BA4300F6.initialization
{
  //## begin Socket::Socket%3C91BA4300F6.body preserve=yes
  //## end Socket::Socket%3C91BA4300F6.body
}

Socket::Socket (const string &sname, const string &serv, int proto, int backlog)
  //## begin Socket::Socket%9FD2BC39FEED.hasinit preserve=no
  //## end Socket::Socket%9FD2BC39FEED.hasinit
  //## begin Socket::Socket%9FD2BC39FEED.initialization preserve=yes
    : name(sname),port_(serv),bound(False)
  //## end Socket::Socket%9FD2BC39FEED.initialization
{
  //## begin Socket::Socket%9FD2BC39FEED.body preserve=yes
  initServer(serv,proto,backlog);
  //## end Socket::Socket%9FD2BC39FEED.body
}

Socket::Socket (const string &sname, const string &host, const string &serv, int proto, int wait_ms)
  //## begin Socket::Socket%C15CE2A5FEED.hasinit preserve=no
  //## end Socket::Socket%C15CE2A5FEED.hasinit
  //## begin Socket::Socket%C15CE2A5FEED.initialization preserve=yes
    : name(sname),host_(host),port_(serv),bound(False)
  //## end Socket::Socket%C15CE2A5FEED.initialization
{
  //## begin Socket::Socket%C15CE2A5FEED.body preserve=yes
  initClient(host,serv,proto,wait_ms);
  //## end Socket::Socket%C15CE2A5FEED.body
}

Socket::Socket (int id, struct sockaddr_in &sa)
  //## begin Socket::Socket%4760B82BFEED.hasinit preserve=no
  //## end Socket::Socket%4760B82BFEED.hasinit
  //## begin Socket::Socket%4760B82BFEED.initialization preserve=yes
    : name("client"),sid(id),type(TCP),host_(">tcp"),port_("0"),bound(False)
  //## end Socket::Socket%4760B82BFEED.initialization
{
  //## begin Socket::Socket%4760B82BFEED.body preserve=yes
  dprintf(1)("creating connected socket\n");
  // constructs a generic socket (used by accept(), below)
  rmt_addr = sa;
  if( setDefaults()<0 )
  {
    close(sid);
    sid = -1;
    return;
  }
// Set non-blocking mode
  if( fcntl(sid,F_SETFL,FNDELAY)<0 )
    set_errcode(SOCKOPT);
  //## end Socket::Socket%4760B82BFEED.body
}

Socket::Socket (int id, struct sockaddr_un &sa)
  //## begin Socket::Socket%3CC95D6E032A.hasinit preserve=no
  //## end Socket::Socket%3CC95D6E032A.hasinit
  //## begin Socket::Socket%3CC95D6E032A.initialization preserve=yes
    : name("client"),sid(id),type(UNIX),host_(">unix"),port_("0"),bound(False)
  //## end Socket::Socket%3CC95D6E032A.initialization
{
  //## begin Socket::Socket%3CC95D6E032A.body preserve=yes
  dprintf(1)("creating connected socket\n");
  unix_addr = sa;
  if( setDefaults()<0 )
  {
    close(sid);
    sid = -1;
    return;
  }
// Set non-blocking mode
  if( fcntl(sid,F_SETFL,FNDELAY)<0 )
    set_errcode(SOCKOPT);
  //## end Socket::Socket%3CC95D6E032A.body
}


Socket::~Socket()
{
  //## begin Socket::~Socket%3C90CE58024E_dest.body preserve=yes
  if( sid >=0 )
  {
    dprintf(1)("close(fd)\n");
    close(sid);
  }
  if( bound && type == Socket::UNIX && unix_addr.sun_path[0] )
  {
    int res = unlink(unix_addr.sun_path);
    dprintf(1)("unlink(%s) = %d (%s)\n",unix_addr.sun_path,res<0?errno:res,res<0?strerror(errno):"OK");
  }
  dprintf(1)("destroying socket\n");
  //## end Socket::~Socket%3C90CE58024E_dest.body
}



//## Other Operations (implementation)
int Socket::initServer (const string &serv, int proto, int backlog)
{
  //## begin Socket::initServer%3C91B9FC0130.body preserve=yes
  // open a server socket
  errcode_ = errno_sys_ = 0;
  sid = -1;
  server = True;
  connected = bound = False;
  type = proto;
  
  if( type == UNIX )
  {
    host_ = "unix";
    port_ = serv;
    // setup socket address
    unix_addr.sun_family = AF_UNIX;
    FailWhen( serv.length() >= sizeof(unix_addr.sun_path),"socket name too long");
    if( serv[0] == '=' ) // abstract socket name
    {
      memset(unix_addr.sun_path,0,sizeof(unix_addr.sun_path));
      serv.substr(1).copy(unix_addr.sun_path+1,sizeof(unix_addr.sun_path)-1);
    }
    else // socket in filesystem
      serv.copy(unix_addr.sun_path,sizeof(unix_addr.sun_path));
    // create socket
    sid = socket(PF_UNIX,SOCK_STREAM,0);
    if( sid < 0 )
      return set_errcode(SOCKET);
    dprintf(1)("creating unix socket %s\n",serv.c_str());
    if( setDefaults()<0 )
    {
      close(sid);
      sid = -1;
      return errcode_;
    }
    // bind the socket
    int res = bind(sid,(struct sockaddr*)&unix_addr,sizeof(unix_addr));
    dprintf(1)("bind()=%d (%s)\n",res<0?errno:res,res<0?strerror(errno):"OK");
    if( res<0 )
    {
      dprintf(1)("close(fd)\n");
      close(sid);
      sid = -1;
      return set_errcode(BIND);
    }
    bound = True;
    // start listening for connections
    res = listen(sid,backlog);
    dprintf(1)("listen()=%d (%s)\n",res<0?errno:res,res<0?strerror(errno):"OK");
    if( res<0 )
    {
      dprintf(1)("close(fd)\n");
      close(sid);
      sid=-1;
      return set_errcode(LISTEN);
    }
  } // endif type UNIX
  else // networked socket (type TCP or UDP)
  {
    host_ = "localhost";
    port_ = serv;
    
    struct servent     *pse;    // service info entry
    struct protoent    *ppe;    // protocol entry

    const char *prot = ( type == UDP ? "udp" : "tcp" );

    memset(&rmt_addr,0,sizeof(rmt_addr));
    rmt_addr.sin_family = AF_INET;
    rmt_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // try to get service by name or port
    if( (pse = getservbyname(serv.c_str(),prot)) != 0 )
      rmt_addr.sin_port = pse->s_port;
    else if( !(rmt_addr.sin_port = htons((u_short)atoi(serv.c_str()))) )
      return errcode_ = PORT;
    // Map protocol name to protocol number
    if( !(ppe = getprotobyname(prot)) )
      return errcode_ = PROTOCOL;
    // open the socket fd
    int soktype = type == TCP ? SOCK_STREAM : SOCK_DGRAM;
    sid = socket(PF_INET, soktype, ppe->p_proto);
    if( sid < 0 )
      return set_errcode(SOCKET);
    dprintf(1)("created server socket, port %d, protocol %d\n",
                ntohs((ushort)rmt_addr.sin_port),(int)ppe->p_proto);
    // set default options
    if( setDefaults()<0 )
    {
      close(sid);
      sid = -1;
      return errcode_;
    }
    // bind to the socket
    int res = bind(sid,(struct sockaddr*)&rmt_addr,sizeof(rmt_addr));
    dprintf(1)("bind()=%d (%s)\n",res<0?errno:res,res<0?strerror(errno):"OK");
    if( res<0 )
    {
      dprintf(1)("close(fd)\n");
      close(sid);
      sid = -1;
      return set_errcode(BIND);
    }
    bound = True;
    // start listening on the socket
    if( type == TCP )
    {
      res = listen(sid,backlog);
      dprintf(1)("listen()=%d (%s)\n",res<0?errno:res,res<0?strerror(errno):"OK");
      if( res<0 )
      {
        dprintf(1)("close(fd)\n");
        close(sid);
        sid=-1;
        return set_errcode(LISTEN);
      }
    }
    else
    {
      memset(&rmt_addr,0,sizeof(rmt_addr));
    }
  } // end else TCP/UDP
  
  // set non-blocking mode
  if( fcntl(sid,F_SETFL,FNDELAY)<0 )
    return set_errcode(SOCKOPT);
  
  return 0;
  //## end Socket::initServer%3C91B9FC0130.body
}

int Socket::initClient (const string &host, const string &serv, int proto, int wait_ms)
{
  //## begin Socket::initClient%3C91BA16008E.body preserve=yes
  errcode_ = errno_sys_ = 0;
  sid = -1;
  server = False;
  connected = False;
  type = proto;
  host_ =  host;
  port_ = serv;
  if( type == UNIX )
  {
    // setup socket address
    string path = host +":" +serv;
    unix_addr.sun_family = AF_UNIX;
    FailWhen(path.length() >= sizeof(unix_addr.sun_path),"socket name too long");
    if( path[0] == '=' ) // abstract socket name
    {
      memset(unix_addr.sun_path,0,sizeof(unix_addr.sun_path));
      path.substr(1).copy(unix_addr.sun_path+1,sizeof(unix_addr.sun_path)-1);
    }
    else // socket in filesystem
      path.copy(unix_addr.sun_path,sizeof(unix_addr.sun_path));
    // create socket
    sid = socket(PF_UNIX,SOCK_STREAM,0);
    if( sid < 0 )
      return set_errcode(SOCKET);
    dprintf(1)("connecting unix socket to %s\n",path.c_str());
    if( setDefaults()<0 )
    {
      close(sid);
      sid = -1;
      return errcode_;
    }
  } // endif type UNIX
  else // networked socket (type TCP or UDP)
  {
    // opens a client socket
    struct servent     *pse;    // service info entry
    struct hostent     *phe;    // server host entry
    struct protoent    *ppe;    // protocol entry
    string host_addr;           // address of server host

    memset(&rmt_addr,0,sizeof(rmt_addr));
    rmt_addr.sin_family = AF_INET;

    const char *prot = ( proto == UDP ? "udp" : "tcp" );

    // try to get service by name or port
    if( (pse = getservbyname(serv.c_str(),prot)) != 0 )
      rmt_addr.sin_port = pse->s_port;
    else if( !(rmt_addr.sin_port = htons((u_short)atoi(serv.c_str()))) )
      return errcode_ = PORT;
    // try to get host by name
    if( (phe = gethostbyname(host.c_str())) != 0 )
    {
      if( phe->h_addrtype != AF_INET )
        return errcode_ = BADADDRTYPE; 
      host_addr = inet_ntoa(*((struct in_addr*)*(phe->h_addr_list)));
    }
    else
      host_addr = host;
    // try using dot notation
    if( (rmt_addr.sin_addr.s_addr = inet_addr(host_addr.c_str())) == INADDR_NONE )
      return errcode_ = BADHOST;
    // Map protocol name to protocol number
    if( !(ppe = getprotobyname(prot)) )
      return errcode_ = PROTOCOL;
    dprintf(1)("connecting client socket to %s:%s, protocol %d\n",
      host_addr.c_str(),port_.c_str(),ppe->p_proto);
    // open the socket fd
    int soktype = type == TCP ? SOCK_STREAM : SOCK_DGRAM;
    sid = socket(PF_INET,soktype,ppe->p_proto);
    if( sid < 0 )
      return set_errcode(SOCKET);
    // set default options
    if( setDefaults()<0 )
    {
      close(sid);
      sid = -1;
      return errcode_;
    }
  }
  
  // set non-blocking mode
  if( fcntl(sid,F_SETFL,FNDELAY)<0 )
    return set_errcode(SOCKOPT);
  // try to connect
  if( wait_ms >= 0 )
    return connect(wait_ms);
  return 0;
  //## end Socket::initClient%3C91BA16008E.body
}

string Socket::errstr () const
{
  //## begin Socket::errstr%F1A741D4FEED.body preserve=yes
  static char const *s_errstr[] = {
     "OK",
     "Can't create socket (%d: %s)",
     "Can't bind local address (%d: %s)",
     "Can't connect to server (%d: %s)",
     "Can't accept client socket (%d: %s)",
     "Bad server host name given",
     "Bad address type",
     "Read error (%d: %s)",
     "Write error (%d: %s)",
     "Remote client closed connection (%d: %s)",
     "Couldn't read/write whole message (%d: %s)",
     "Invalid operation",
     "setsockopt() or getsockopt() failure (%d: %s)",
     "wrong port/service specified (%d: %s)",
     "invalid protocol (%d: %s)",
     "listen() error (%d: %s)",
     "timeout (%d: %s)",
     "connect in progress (%d: %s)",
     "No more clients (%d: %s)",
     "General failure",
     "Uninitialized socket" 
  };  
  if( errcode_ < NOINIT || errcode_ > 0 )
    return "";
  return Debug::ssprintf(s_errstr[-errcode_],errno,strerror(errno));
  //## end Socket::errstr%F1A741D4FEED.body
}

int Socket::connect (int wait_ms)
{
  //## begin Socket::connect%6AE5AA36FEED.body preserve=yes
  if( isServer() )
    return errcode_=INVOP;
  for(;;)
  {
    int res;
    if( type == UNIX )
       res = ::connect(sid,(struct sockaddr*)&unix_addr,sizeof(unix_addr));
    else
       res = ::connect(sid,(struct sockaddr*)&rmt_addr,sizeof(rmt_addr));
    if( !res )
      break; // connected? break out
    else 
    {
      dprintf(2)("connect() failed: errno=%d (%s)\n",errno,strerror(errno));
      if( errno == EINPROGRESS || errno == EALREADY )
      {
        {
          errcode_ = INPROGRESS;
          return 0;
        }
      }
      close(sid);
      sid = -1;
      return set_errcode(CONNECT);
    }
  }
  dprintf(1)("connect() successful\n");
  connected = True;
  errcode_ = 0;
  return 1;
  //## end Socket::connect%6AE5AA36FEED.body
}

Socket* Socket::accept ()
{
  //## begin Socket::accept%1357FC75FEED.body preserve=yes
  if( !isServer() )   
    { errcode_=INVOP; return 0; }
  if( sid<0 ) 
    { errcode_=NOINIT; return 0; }
  if( type == UDP ) 
    return this;
  int id; 
  if( type == UNIX )
  {
    size_t len = sizeof(unix_addr);
    id = ::accept(sid,(struct sockaddr*)&unix_addr,&len);
  }
  else
  {
    size_t len = sizeof(rmt_addr);
    id = ::accept(sid,(struct sockaddr*)&rmt_addr,&len);
  }
  if( id < 0 )
  {
    dprintf(1)("accept() failed, errno=%d (%s)\n",errno,strerror(errno));
    set_errcode(ACCEPT);
    return 0;
  }
  else
  {
    dprintf(1)("accept() successful\n");
    errcode_=0;
    return type == UNIX 
        ? new Socket(id,unix_addr) 
        : new Socket(id,rmt_addr);
  }
  //## end Socket::accept%1357FC75FEED.body
}

int Socket::read (void *buf, int maxn)
{
  //## begin Socket::read%5264A6A9FEED.body preserve=yes
  if( sid<0 ) 
    return errcode_=NOINIT; 
  FailWhen(!buf,"null buffer");
  errcode_ = 0;
  if( !maxn ) 
    return 0;
  bool sigpipe = False;
  
  int nread,nleft=maxn;
  if( type != UDP )
  {
    while( nleft>0 && !errcode_ && !sigpipe )
    {
      errno = 0;
      int old_counter = sigpipe_counter;
      nread = ::read( sid,buf,nleft ); // try to read something
      sigpipe = old_counter != sigpipe_counter; // check for SIGPIPE
      dprintf(3)("read(%d)=%d%s, errno=%d (%s)\n",nleft,nread,
            sigpipe?" SIGPIPE":"",errno,strerror(errno));
      if( Debug(10) && nread>0 )
        printData(buf,min(nread,200));
      if( nread<0 ) // error?
      {
        if( errno == EWOULDBLOCK || errno == EAGAIN ) 
        { // if refuses to block, that's OK, return 0
          errcode_ = TIMEOUT;
          return maxn - nleft;
        }
        else // else a real error
          return set_errcode(READERR);
      }
      else if( nread == 0 )
        return errcode_ = PEERCLOSED;
      else
      {
        buf = nread + (char*)buf;
        nleft -= nread;
      }
    }
  }
  else // UDP socket
  {
    errno = 0;
//    if ((wres=Wait(rtimeout,SWAIT_READ))==0)
//      errcode_=SK_TIMEOUT;
//    else 
    socklen_t alen = sizeof(rmt_addr);
    if( (nread=recvfrom(sid,(char*)buf,maxn,0,
                        (struct sockaddr*)&rmt_addr,&alen))<=0 ||
         errno )
      return set_errcode(READERR);
    else
    {
      nleft = 0;
      connected = True;
    }
  }
  
  if( sigpipe )
    return errcode_ = PEERCLOSED;

  return maxn-nleft;
  //## end Socket::read%5264A6A9FEED.body
}

int Socket::write (const void *buf, int n)
{
  //## begin Socket::write%139EF112FEED.body preserve=yes
  if( sid<0 ) 
    return errcode_=NOINIT; 
  FailWhen(!buf,"null buffer");
  errcode_ = 0;
  if( !n ) 
    return 0;
  bool sigpipe = False;

  int nleft=n,nwr;
  if( type != UDP ) // TCP or UNIX: write to stream
  {
    while( nleft>0 && !errcode_ && !sigpipe)
    {
      errno = 0;
      int old_counter = sigpipe_counter;
      nwr = ::write(sid,buf,nleft);
      sigpipe = old_counter != sigpipe_counter; // check for SIGPIPE
      dprintf(3)("write(%d)=%d%s, errno=%d (%s)\n",nleft,nwr,
            sigpipe?" SIGPIPE":"",errno,strerror(errno));
      if( Debug(10) && nwr>0 )
        printData(buf,min(nwr,200));
      if( nwr<0 )
      {
        if( errno == EWOULDBLOCK || errno == EAGAIN ) 
        { // if refuses to block, that's OK, return 0
          errcode_ = TIMEOUT;
          return n - nleft;
        }
        else // else a real error
          return set_errcode(WRITERR);
      }
      else if( nwr==0 )
        return errcode_ = PEERCLOSED;
      else
      {
        buf = nwr + (char*)buf;
        nleft -= nwr;
      }
    }
  }
  else // UDP
  {
    errno = 0;
    if( !connected )
      return errcode_ = WRITERR;
    if( (nwr = sendto(sid,(char*)buf,n,0,(struct sockaddr*)&rmt_addr,
                         sizeof(rmt_addr)))<=0 || errno )
      return set_errcode(WRITERR);
    else
      nleft=0;
  }
  
  if( sigpipe )
    return errcode_ = PEERCLOSED;
  
  return n - nleft;
  //## end Socket::write%139EF112FEED.body
}

int Socket::shutdown (bool receive, bool send)
{
  //## begin Socket::shutdown%890ACD77FEED.body preserve=yes
  FailWhen(!receive && !send,"neither receive nor send specified");
  if( sid<0 ) 
    return errcode_ = NOINIT; 
  errcode_ = 0;
  int how = receive ? (send ? 2 : 0) : 1;
  int res = shutdown(sid,how);
  dprintf(1)("shutdown(%d)=%d",how,res);
  if( res<0 )
    return set_errcode(SHUTDOWN);
  return 0;
  //## end Socket::shutdown%890ACD77FEED.body
}

int Socket::setDefaults ()
{
  //## begin Socket::setDefaults%3EE80597FEED.body preserve=yes
  if( sid<0 ) 
    return errcode_ = NOINIT;
  
  uint val=1;
  struct linger lin = { 1,1 };

  
  if( setsockopt(sid,SOL_SOCKET,SO_REUSEADDR,(char*)&val,sizeof(val))<0 )
    return set_errcode(SOCKOPT);
  
  // no more defaults for UNIX sockets
  if( type == UNIX )
    return 0;

  if( setsockopt(sid,SOL_SOCKET,SO_KEEPALIVE,(char*)&val,sizeof(val) )<0)
    return set_errcode(SOCKOPT);
  
  if( setsockopt(sid,SOL_SOCKET,SO_LINGER,(const char*)&lin,sizeof(lin))<0 )
    return set_errcode(SOCKOPT);
  
  if( getsockopt(sid,SOL_SOCKET,SO_SNDBUF,(char*)&sbuflen,&val)<0 )
    return set_errcode(SOCKOPT);
  
  if( getsockopt(sid,SOL_SOCKET,SO_RCVBUF,(char*)&rbuflen,&val)<0 )
    return set_errcode(SOCKOPT);
  
  return 0;
  //## end Socket::setDefaults%3EE80597FEED.body
}

// Additional Declarations
  //## begin Socket%3C90CE58024E.declarations preserve=yes

void Socket::printData (const void *buf,int n)
{
  char hex[64],chars[32];
  chars[20] = 0;
  const unsigned char *ch=(const unsigned char*)buf;
  for( int i=0; i<n; i++,ch++ )
  {
    int pos = i%20;
    if( i && !pos )
    {
      printf("%-60s%-20s\n",hex,chars);
      hex[0] = chars[0] = 0;
    }
    sprintf(hex+pos*3,"%02x ",(int)*ch);
    chars[pos] = (*ch>=32 && *ch<=127) ? *ch : '.';
  }
  if( strlen(hex) )
  {
    if( n%20 )
      chars[n%20] = 0;
    printf("%-60s%-20s\n",hex,chars);
  }
}


string Socket::sdebug ( int detail,const string &,const char *name ) const
{
  string out;
  if( detail>=0 ) // basic detail
  {
    out = Debug::ssprintf("%s/%d",name?name:"Socket",sid);
    if( server )
      Debug::append(out,"S");
    if( connected )
      Debug::append(out,"c");
  }
  if( detail >= 1 || detail == -1 )   // normal detail
  {
    Debug::appendf(out,"err:%d",errcode_);
    if( errcode_ )
      Debug::append(out,errstr());
  }
  return out;
}
  //## end Socket%3C90CE58024E.declarations
//## begin module%3C90D43B0096.epilog preserve=yes
//## end module%3C90D43B0096.epilog
