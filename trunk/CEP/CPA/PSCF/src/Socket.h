//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3C90D43B0094.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3C90D43B0094.cm

//## begin module%3C90D43B0094.cp preserve=no
//## end module%3C90D43B0094.cp

//## Module: Socket%3C90D43B0094; Package specification
//## Subsystem: Networking%3C90D442031F
//## Source file: F:\lofar8\oms\LOFAR\CEP\CPA\PSCF\src\Socket.h

#ifndef Socket_h
#define Socket_h 1

//## begin module%3C90D43B0094.additionalIncludes preserve=no
#include "Common.h"
#include "DMI.h"
//## end module%3C90D43B0094.additionalIncludes

//## begin module%3C90D43B0094.includes preserve=yes
//#include <sys/file.h>
//#include <sys/types.h>
#include <resolv.h>
#include <errno.h>
#include <sys/un.h>
//#include <netinet/in.h>
//#include <netdb.h>
//#include <fcntl.h>
//#include <unistd.h>
//#include <string.h>
//## end module%3C90D43B0094.includes

//## begin module%3C90D43B0094.declarations preserve=no
//## end module%3C90D43B0094.declarations

//## begin module%3C90D43B0094.additionalDeclarations preserve=yes
//## end module%3C90D43B0094.additionalDeclarations


//## begin Socket%3C90CE58024E.preface preserve=yes
//## end Socket%3C90CE58024E.preface

//## Class: Socket%3C90CE58024E
//## Category: Networking%3C90CF69020B
//## Subsystem: Networking%3C90D442031F
//## Persistence: Transient
//## Cardinality/Multiplicity: n



class Socket 
{
  //## begin Socket%3C90CE58024E.initialDeclarations preserve=yes
  LocalDebugSubContext(public);
  //## end Socket%3C90CE58024E.initialDeclarations

  public:
    //## Constructors (specified)
      //## Operation: Socket%3C91BA4300F6
      Socket (const string &sname = "");

      //## Operation: Socket%9FD2BC39FEED; C++
      //	Creates server socket
      Socket (const string &sname, const string &serv, 	// service name/port number
      int proto = Socket::TCP, int backlog = 5);

      //## Operation: Socket%C15CE2A5FEED; C++
      //	Creates client socket
      Socket (const string &sname, const string &host, 	// remote host
      const string &serv, 	// service name/port number
      int proto = Socket::TCP, int wait_ms = -1);

    //## Destructor (generated)
      ~Socket();


    //## Other Operations (specified)
      //## Operation: initServer%3C91B9FC0130
      int initServer (const string &serv, int proto = Socket::TCP, int backlog = 5);

      //## Operation: initClient%3C91BA16008E
      int initClient (const string &host, const string &serv, int proto = Socket::TCP, int wait_ms = -1);

      //## Operation: errstr%F1A741D4FEED; C++
      string errstr () const;

      //## Operation: ok%3A99B058FEED; C++
      bool ok ();

      //## Operation: connect%6AE5AA36FEED; C++
      //	Attempts to connect to a server socket.
      int connect (int wait_ms = 0);

      //## Operation: accept%1357FC75FEED; C++
      //	Tries to accept an incoming connection on a server socket.
      Socket* accept ();

      //## Operation: read%5264A6A9FEED; C++
      //	Reads up to maxn bytes from socket.
      int read (void *buf, int maxn);

      //## Operation: write%139EF112FEED; C++
      //	Writes n bytes to socket.
      int write (const void *buf, int n);

      //## Operation: shutdown%890ACD77FEED; C++
      //	Shuts down the socket for receive and/or send
      int shutdown (bool receive, bool send);

    //## Get and Set Operations for Class Attributes (generated)

      //## Attribute: name%3C90CE5803BA
      const string& getName () const;
      void setName (const string& value);

      //## Attribute: errcode%3C90CE5803C1
      int errcode () const;

      //## Attribute: errno_sys%3C91B661029C
      int errno_sys () const;

      //## Attribute: sid%3C90CE590017
      int getSid () const;

      //## Attribute: type%3C90CE590038
      int getType () const;

      //## Attribute: server%3C90CE590039
      bool isServer () const;

      //## Attribute: connected%3C90CE59009B
      bool isConnected () const;

      //## Attribute: host%3CC00CDC00EA
      const string& host () const;

      //## Attribute: port%3CC00CE902E8
      const string& port () const;

    // Data Members for Class Attributes

      //## Attribute: sigpipe_counter%3C90CE5803B3
      //## begin Socket::sigpipe_counter%3C90CE5803B3.attr preserve=no  public: int {VT} 
      volatile int sigpipe_counter;
      //## end Socket::sigpipe_counter%3C90CE5803B3.attr

    // Additional Public Declarations
      //## begin Socket%3C90CE58024E.public preserve=yes
      typedef enum { 
          UDP,         // UDP datagram socket
          TCP,         // TCP (stream) socket over network
          UNIX,        // unix socket (local)
          LOCAL=UNIX 
      } SocketTypes;
      typedef enum {
          SK_OK         =  0,   // Ok
          SOCKET        = -1,   // Can't create socket
          BIND          = -2,   // Can't bind local address
          CONNECT       = -3,   // Can't connect to server
          ACCEPT        = -4,   // Can't accept client socket
          BADHOST       = -5,   // Bad server host name given
          BADADDRTYPE   = -6,   // Bad address type
          READERR       = -7,   // Read error
          WRITERR       = -8,   // Write error
          PEERCLOSED    = -9,   // Remote client closed connection
          INCOMPLETE    = -10,  // Couldn't read/write whole message
          INVOP         = -11,  // Invalid operation
          SOCKOPT       = -12,  // sockopt() failure
          PORT          = -13,  // wrong port/service specified
          PROTOCOL      = -14,  // invalid protocol
          LISTEN        = -15,  // listen() error
          TIMEOUT       = -16,  // timeout
          INPROGRESS    = -17,  // connect() in progress
          NOMORECLI     = -18,  // No more clients
          SHUTDOWN      = -19,  // shutdown() failure
          NOINIT        = -20   // uninitialized socket
      } ErrorCodes;

      // helper function: prints formatted data from buffer
      static void printData (const void *buf,int n);
      
      Declare_sdebug( );
      Declare_debug( );
      //## end Socket%3C90CE58024E.public
  protected:
    //## Constructors (specified)
      //## Operation: Socket%4760B82BFEED; C++
      //	Constructs a generic socket for an incoming connection on a server
      //	socket.
      Socket (int id, struct sockaddr_in &sa);

      //## Operation: Socket%3CC95D6E032A
      Socket (int id, struct sockaddr_un &sa);


    //## Other Operations (specified)
      //## Operation: setDefaults%3EE80597FEED; C++
      //	Sets default socket options
      int setDefaults ();

    // Additional Protected Declarations
      //## begin Socket%3C90CE58024E.protected preserve=yes
      int set_errcode (int n);
      //## end Socket%3C90CE58024E.protected
  private:
    //## Constructors (generated)
      Socket(const Socket &right);

    //## Assignment Operation (generated)
      Socket & operator=(const Socket &right);

    // Additional Private Declarations
      //## begin Socket%3C90CE58024E.private preserve=yes
      //## end Socket%3C90CE58024E.private

  private: //## implementation
    // Data Members for Class Attributes

      //## begin Socket::name%3C90CE5803BA.attr preserve=no  public: string {V} 
      string name;
      //## end Socket::name%3C90CE5803BA.attr

      //## begin Socket::errcode%3C90CE5803C1.attr preserve=no  public: int {V} 
      int errcode_;
      //## end Socket::errcode%3C90CE5803C1.attr

      //## begin Socket::errno_sys%3C91B661029C.attr preserve=no  public: int {U} 
      int errno_sys_;
      //## end Socket::errno_sys%3C91B661029C.attr

      //## begin Socket::sid%3C90CE590017.attr preserve=no  public: int {V} 
      int sid;
      //## end Socket::sid%3C90CE590017.attr

      //## begin Socket::type%3C90CE590038.attr preserve=no  public: int {V} 
      int type;
      //## end Socket::type%3C90CE590038.attr

      //## begin Socket::server%3C90CE590039.attr preserve=no  public: bool {V} 
      bool server;
      //## end Socket::server%3C90CE590039.attr

      //## begin Socket::connected%3C90CE59009B.attr preserve=no  public: bool {V} 
      bool connected;
      //## end Socket::connected%3C90CE59009B.attr

      //## begin Socket::host%3CC00CDC00EA.attr preserve=no  public: string {U} 
      string host_;
      //## end Socket::host%3CC00CDC00EA.attr

      //## begin Socket::port%3CC00CE902E8.attr preserve=no  public: string {U} 
      string port_;
      //## end Socket::port%3CC00CE902E8.attr

      //## Attribute: sbuflen%3C90CE59009C
      //## begin Socket::sbuflen%3C90CE59009C.attr preserve=no  protected: int {V} 
      int sbuflen;
      //## end Socket::sbuflen%3C90CE59009C.attr

      //## Attribute: rbuflen%3C90CE5900A1
      //## begin Socket::rbuflen%3C90CE5900A1.attr preserve=no  protected: int {V} 
      int rbuflen;
      //## end Socket::rbuflen%3C90CE5900A1.attr

    // Additional Implementation Declarations
      //## begin Socket%3C90CE58024E.implementation preserve=yes
      bool bound;
      struct sockaddr_in rmt_addr;  // connected client address (TCP)
      struct sockaddr_un unix_addr; // connected client address (UNIX)
      //## end Socket%3C90CE58024E.implementation
};

//## begin Socket%3C90CE58024E.postscript preserve=yes
//## end Socket%3C90CE58024E.postscript

// Class Socket 


//## Other Operations (inline)
inline bool Socket::ok ()
{
  //## begin Socket::ok%3A99B058FEED.body preserve=yes
  return sid >= 0 && !errcode_;
  //## end Socket::ok%3A99B058FEED.body
}

//## Get and Set Operations for Class Attributes (inline)

inline const string& Socket::getName () const
{
  //## begin Socket::getName%3C90CE5803BA.get preserve=no
  return name;
  //## end Socket::getName%3C90CE5803BA.get
}

inline void Socket::setName (const string& value)
{
  //## begin Socket::setName%3C90CE5803BA.set preserve=no
  name = value;
  //## end Socket::setName%3C90CE5803BA.set
}

inline int Socket::errcode () const
{
  //## begin Socket::errcode%3C90CE5803C1.get preserve=no
  return errcode_;
  //## end Socket::errcode%3C90CE5803C1.get
}

inline int Socket::errno_sys () const
{
  //## begin Socket::errno_sys%3C91B661029C.get preserve=no
  return errno_sys_;
  //## end Socket::errno_sys%3C91B661029C.get
}

inline int Socket::getSid () const
{
  //## begin Socket::getSid%3C90CE590017.get preserve=no
  return sid;
  //## end Socket::getSid%3C90CE590017.get
}

inline int Socket::getType () const
{
  //## begin Socket::getType%3C90CE590038.get preserve=no
  return type;
  //## end Socket::getType%3C90CE590038.get
}

inline bool Socket::isServer () const
{
  //## begin Socket::isServer%3C90CE590039.get preserve=no
  return server;
  //## end Socket::isServer%3C90CE590039.get
}

inline bool Socket::isConnected () const
{
  //## begin Socket::isConnected%3C90CE59009B.get preserve=no
  return connected;
  //## end Socket::isConnected%3C90CE59009B.get
}

inline const string& Socket::host () const
{
  //## begin Socket::host%3CC00CDC00EA.get preserve=no
  return host_;
  //## end Socket::host%3CC00CDC00EA.get
}

inline const string& Socket::port () const
{
  //## begin Socket::port%3CC00CE902E8.get preserve=no
  return port_;
  //## end Socket::port%3CC00CE902E8.get
}

//## begin module%3C90D43B0094.epilog preserve=yes
inline int Socket::set_errcode ( int n )
{
  errno_sys_ = errno;
  return errcode_ = n;
}
//## end module%3C90D43B0094.epilog


#endif


// Detached code regions:
#if 0
//## begin Socket::setSigpipe%3C9205370382.body preserve=yes
  sigpipe = True;
//## end Socket::setSigpipe%3C9205370382.body

#endif
