//# Socket.h: Class for socket conections
//#
//# Copyright (C) 2002
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#if !defined(COMMON_SOCKET_H)
#define COMMON_SOCKET_H

#include <Common/Debug.h>

#include <resolv.h>
#include <errno.h>
#include <sys/un.h>

namespace LOFAR
{

  //##ModelId=3C90CE58024E
  class Socket 
  {
    //##ModelId=3DB936CC0314
    LocalDebugContext;

  public:
    //##ModelId=3C91BA4300F6
    explicit Socket (const string &sname = "");

    //##ModelId=9FD2BC39FEED
    //##Documentation
    //## Creates server socket
    Socket (
            //##Documentation
            //## service name/port number
            const string &sname, const string &serv,
            int proto = Socket::TCP, int backlog = 5);

    //##ModelId=C15CE2A5FEED
    //##Documentation
    //## Creates client socket
    Socket (
            //##Documentation
            //## remote host
            const string &sname, const string &host,
            //##Documentation
            //## service name/port number
            const string &serv,
            int proto = Socket::TCP, int wait_ms = -1);

    //##ModelId=3DB936D00067
    ~Socket();


    //##ModelId=3C91B9FC0130
    int initServer (const string &serv, int proto = Socket::TCP, int backlog = 5);

    //##ModelId=3C91BA16008E
    int initClient (const string &host, const string &serv, int proto = Socket::TCP, int wait_ms = -1);

    //##ModelId=F1A741D4FEED
    string errstr () const;

    //##ModelId=3A99B058FEED
    bool ok ();

    //##ModelId=6AE5AA36FEED
    //##Documentation
    //## Attempts to connect to a server socket.
    int connect (int wait_ms = 0);

    //##ModelId=1357FC75FEED
    //##Documentation
    //## Tries to accept an incoming connection on a server socket.
    Socket* accept ();

    //##ModelId=5264A6A9FEED
    //##Documentation
    //## Reads up to maxn bytes from socket.
    int read (void *buf, int maxn);

    //##ModelId=139EF112FEED
    //##Documentation
    //## Writes n bytes to socket.
    int write (const void *buf, int n);

    //##ModelId=890ACD77FEED
    //##Documentation
    //## Shuts down the socket for receive and/or send
    int shutdown (bool receive, bool send);

    //##ModelId=3DB936D000E9
    const string& getName () const;
    //##ModelId=3DB936D001E4
    void setName (const string& value);

    //##ModelId=3DB936D003C4
    int errcode () const;

    //##ModelId=3DB936D100CD
    int errno_sys () const;

    //##ModelId=3DB936D101DB
    int getSid () const;

    //##ModelId=3DB936D102D5
    int getType () const;

    //##ModelId=3DB936D20006
    bool isServer () const;

    //##ModelId=3DB936D20128
    bool isConnected () const;

    //##ModelId=3DB936D20255
    const string& host () const;

    //##ModelId=3DB936D20377
    const string& port () const;

    // Data Members for Class Attributes

    // Additional Public Declarations
    //##ModelId=3DB936520032
    typedef enum { 
      UDP,         // UDP datagram socket
      TCP,         // TCP (stream) socket over network
      UNIX,        // unix socket (local)
      LOCAL=UNIX 
    } SocketTypes;
    //##ModelId=3DB9365200B4
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

    // sets blocking mode on socket if block is true
    // (default sockets are non-blocking)
    //##ModelId=3DB936D300BC
    int setBlocking (bool block=true);
      
    // interrupts readblock / writeblock calls (for multithreaded sockets)
    //##ModelId=3DB936D3037B
    void interrupt (bool intr=true);
      
    // points socket at SIGPIPE counter
    //##ModelId=3DB936D401F6
    void setSigpipeCounter (const volatile int *counter);
      
    //	Reads maxn bytes from socket (in blocking mode)
    //##ModelId=3DB936D50067
    int readblock (void *buf, int maxn);
    //	Writes n bytes to socket (in blocking mode)
    //##ModelId=3DB936D6007C
    int writeblock (const void *buf, int n);

    // helper function: prints formatted data from buffer
    //##ModelId=3DB936D700EC
    static void printData (const void *buf,int n);
      
    //##ModelId=3DB936D80210
    Declare_sdebug( );
    //##ModelId=3DB936D802BA
    Declare_debug( );
  protected:
    //##ModelId=4760B82BFEED
    //##Documentation
    //## Constructs a generic socket for an incoming connection on a server
    //## socket.
    Socket (int id, struct sockaddr_in &sa);

    //##ModelId=3CC95D6E032A
    Socket (int id, struct sockaddr_un &sa);


    //##ModelId=3EE80597FEED
    //##Documentation
    //## Sets default socket options
    int setDefaults ();

    // Additional Protected Declarations
    //##ModelId=3DB936D8036E
    int set_errcode (int n);
  private:
    //##ModelId=3DB936D902EE
    Socket(const Socket &right);

    //##ModelId=3DB936DA0277
    Socket & operator=(const Socket &right);

  private:
    // Data Members for Class Attributes

    //##ModelId=3C90CE5803BA
    string name;

    //##ModelId=3C90CE5803C1
    int errcode_;

    //##ModelId=3C91B661029C
    int errno_sys_;

    //##ModelId=3C90CE590017
    int sid;

    //##ModelId=3C90CE590038
    int type;

    //##ModelId=3C90CE590039
    bool server;

    //##ModelId=3C90CE59009B
    bool connected;

    //##ModelId=3CC00CDC00EA
    string host_;

    //##ModelId=3CC00CE902E8
    string port_;

    //##ModelId=3C90CE59009C
    int sbuflen;

    //##ModelId=3C90CE5900A1
    int rbuflen;

    // Additional Implementation Declarations
    //##ModelId=3DB936CE001E
    bool do_intr;  // flag: interrupt readblock/writeblock call
    //##ModelId=3DB936CE01B9
    bool bound;
    //##ModelId=3DB936CE0321
    struct sockaddr_in rmt_addr;  // connected client address (TCP)
    //##ModelId=3DB936CF00CA
    struct sockaddr_un unix_addr; // connected client address (UNIX)
      
    //##ModelId=3C90CE5803B3
    const volatile int *sigpipe_counter;
    //##ModelId=3DB936CF02C9
    static int default_sigpipe_counter;
  };

  // Class Socket 


  //##ModelId=3A99B058FEED
  inline bool Socket::ok ()
  {
    return sid >= 0 && !errcode_;
  }

  //##ModelId=3DB936D000E9
  inline const string& Socket::getName () const
  {
    return name;
  }

  //##ModelId=3DB936D001E4
  inline void Socket::setName (const string& value)
  {
    name = value;
  }

  //##ModelId=3DB936D003C4
  inline int Socket::errcode () const
  {
    return errcode_;
  }

  //##ModelId=3DB936D100CD
  inline int Socket::errno_sys () const
  {
    return errno_sys_;
  }

  //##ModelId=3DB936D101DB
  inline int Socket::getSid () const
  {
    return sid;
  }

  //##ModelId=3DB936D102D5
  inline int Socket::getType () const
  {
    return type;
  }

  //##ModelId=3DB936D20006
  inline bool Socket::isServer () const
  {
    return server;
  }

  //##ModelId=3DB936D20128
  inline bool Socket::isConnected () const
  {
    return connected;
  }

  //##ModelId=3DB936D20255
  inline const string& Socket::host () const
  {
    return host_;
  }

  //##ModelId=3DB936D20377
  inline const string& Socket::port () const
  {
    return port_;
  }

  //##ModelId=3DB936D8036E
  inline int Socket::set_errcode ( int n )
  {
    errno_sys_ = errno;
    return errcode_ = n;
  }

  //##ModelId=3DB936D401F6
  inline void Socket::setSigpipeCounter (const volatile int *counter)
  {
    sigpipe_counter = counter;
  }

} // namespace LOFAR

#endif

using LOFAR::Socket;

// Detached code regions:
#if 0
sigpipe = True;

#endif
