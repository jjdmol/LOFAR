#ifndef GCF_PORTMANAGER_HXX
#define GCF_PORTMANAGER_HXX

#define MAX_HOSTNAME 256

#include <Types.hxx>
#include <CharString.hxx>
#include <DynPtrArray.hxx>
#include "GCF_Port.hxx"
#include <Socket.hxx>

class GCFFsm;

typedef DynPtrArray<GCFPort> PortList;


class GCFPortManager
{
  public:
    static GCFPortManager *getInstance();

    /// destructor
    ~GCFPortManager();

    /** sort function by hostname.
        this function sorts the host entries by hostname, port number 
        and network protocoll class. the function is called by the DynPtrArray class.
        @param e1 first hostPort to compare
        @param e2 second hostPort to compare
        @return int: value of comparison: 1 if e1 > e2, -1 if e1 < e2 or 0 if e1 == e2
        */
    static int sortFunc(const GCFPort *e1, const GCFPort *e2);
    
    /** sort function by clientname.
        this function sorts the host entries by client name.
        the function is called by the DynPtrArray class.
        @param e1 first hostPort to compare
        @param e2 second hostPort to compare
        @return int: value of comparison: 1 if e1 > e2, -1 if e1 < e2 or 0 if e1 == e2
        */
    static int sortClFunc(const GCFPort *e1, const GCFPort *e2);
 
    /** add a new host Port.
        this function adds a new host Port to the list. the corresponding GCFPort object
        is created from the parameters passed to this function.
        @param nr the connection reference number
        @param name the hostname:port combination read from the config file
        @return int: 0 on success, -1 on failure
        */ 
    int addHost(const char *name, const char *clientName);
    
    /** remove a host Port.
        you need to know the hostname and the network protocoll class
        @param name the hostname:port combination read from the config file
        @param NetProt the network protocol class; default netTCP
        @return int: 0 on success, -1 on failure
        */         
    int removeHost(CharString &name);
 
    /** get the socket structure for a (UDP) connection.
        @param name the hostname:port combination read from the config file
        @param NetProt the network protocol class; default netTCP
        @return sockaddr_in *: a pointer to the unix socket structure
        */
    Socket *getSocket(CharString &name);
    
    /** get the socket structure for a (UDP) connection.
        @param kr the connection reference number
        @return sockaddr_in *: a pointer to the unix socket structure
        */
    Socket *getSocketOfClient(const char *clientName);
 
    /** get the file handle for a (TCP) connection.
        @param name the hostname:port combination read from the config file
        @param NetProt the network protocol class; default netTCP
        @return int: a file descriptor to an open socket
        */
    int getFile(CharString &name);
    
    /** get the file handle for a (TCP) connection.
        @param kr the connection reference number
        @return int: a file descriptor to an open socket
        */
    int getFileOfClient(const char *clientName);
 
    /** get the connection status connection.
        @param name the hostname:port combination read from the config file
        @param NetProt the network protocol class; default netTCP
        @return PVSSboolean: PVSS_TRUE if connected, otherwise PVSS_FALSE
        NOTE: UDP connections are always "connected"
        */
    PVSSboolean isConnected(CharString &name);
 
    /** get the connection status connection.
        @param kr the connection reference number
        @return PVSSboolean: PVSS_TRUE if connected, otherwise PVSS_FALSE
        NOTE: UDP connections are always "connected"
        */
    PVSSboolean isClientConnected(const char *clientName);
 
    /** get a host Port by kr.
        @param kr the connection reference number
        @return HostPort *: a pointer to the host Port
        */
    GCFPort *getPort(const char *clientName);

    void closeAllConnections();
    void closeConnection(const char *clientName);
    void workProc();
    PVSSboolean init(GCFFsm *pFsm = 0, unsigned short serverPort = 0);
   
  protected:

  private:

    /// constructor
    GCFPortManager();
    void acceptNewConnection();

    static GCFPortManager *_pInstance;
    PortList _connections;
    PortList _connByCl;
    Socket _serverSocket;
    PVSSboolean _servSockReady;
    unsigned short _serverPort;
    GCFFsm *_pFsm;
};

#endif /* GPI_CONNMANAGER_HXX */
