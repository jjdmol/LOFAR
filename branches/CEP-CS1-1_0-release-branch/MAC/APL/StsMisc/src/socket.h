/*   Socket.h

Deze class wordt vervangen door de lofar socket class

*/

#ifndef SOCKET_H
#define SOCKET_H

#include <WinSock2.h>
//#include "TypeNames.h"

#include <string>

enum TypeSocket {BlockingSocket, NonBlockingSocket};

class Socket {
public:

//	Socket();
  virtual ~Socket();
  Socket(const Socket&);
  Socket& operator=(Socket&);

  std::string ReceiveLine();
  std::string ReceiveBytes();
  

  

  // The parameter of SendLine is not a const reference
  // because SendLine modifes the std::string passed.
  void   SendLine (std::string);

  // The parameter of SendBytes is a const reference
  // because SendBytes does not modify the std::string passed 
  // (in contrast to SendLine).
  void   SendBytes(const std::string&);

//protected:
	  friend class SocketServer;
	  friend class SocketSelect;

//  Socket(SOCKET s);
	Socket(const std::string& socketname, const std::string& hostname, int service, int protocol);
	int initClient (const std::string& hostname, const int	service, int protocol);
	int connection();
	int open_connection();
	void Close();

  SOCKET s_;

  int* refCounter_;


private:
  static void Start();
  static void End();
  static int  nofSockets_;

          // Name of socket given by user
		 std::string                   itsSocketname;
        // Own error number
        int                           itsErrno;
        // System error number
        int                          itsSysErrno;
        // File descriptor of the socket
        int                           itsSocketID;
        // Socket type
        int                          itsType;
        // Server or Client Socket
        bool                         itsIsServer;
        // Connected or not
        bool                         itsIsConnected;
        // Name of host at other side
		std::string                     itsHost;
        // Portnr of server
		int								itsPort;
        // Interrupt read/write block call
        bool                            itsAllowIntr;
        // Socket is initialized
        bool                            itsIsInitialized;
        // Blocking mode or not
        bool                            itsIsBlocking;
};

class SocketClient : public Socket {
public:
  SocketClient(const std::string& host, int port);
};


class SocketServer : public Socket {
public:
  SocketServer(int port, int connections, TypeSocket type=BlockingSocket);

  Socket* Accept();
};

// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winsock/wsapiref_2tiq.asp
class SocketSelect {
  public:
    SocketSelect(Socket const * const s1, Socket const * const s2=NULL, TypeSocket type=BlockingSocket);

    bool Readable(Socket const * const s);

  private:
    fd_set fds_;
}; 



#endif
