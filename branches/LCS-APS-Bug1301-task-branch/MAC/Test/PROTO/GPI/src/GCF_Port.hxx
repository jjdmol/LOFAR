#ifndef GCF_PORT_HXX
#define GCF_PORT_HXX

#define MAX_HOSTNAME 256

#include <Types.hxx>
#include <CharString.hxx>
#include <DynPtrArray.hxx>
#include <TASK/GCF_PortInterface.hxx>

class GCFFsm;
class GCFPortManager;
class Socket;

/** this structure holds all the host information which is read from the config file.
    additional data elements are used for status and connection purposes */
class GCFPort : public GCFPortInterface
{
  public:
 
    friend class GCFPortManager;
    
    /// constructor for passive socket entries
    GCFPort() : GCFPortInterface(SPP), _port(0), _pSocket(0)  {};
    GCFPort(const char *netaddr, const char *clientName = 0);
 
    /// destructor
    ~GCFPort();

    void close();
    ssize_t send(const GCFEvent& event, void* buf = 0, size_t count = 0);
    ssize_t sendv(const GCFEvent& event, const iovec buffers[], int n);

    ssize_t recv(void* buf, size_t count);
    ssize_t recvv(iovec buffers[], int n);
  
  private:
 
    /// the port number we want to connect
    unsigned short int _port;     
 
    Socket *_pSocket;
    /// the hostname as supported by the config file; the port number is stripped
    CharString _hostName;
    GCFFsm *_pFsm;

    void connected(Socket *pSocket, unsigned short int port);
    void eventReceived(GCFEvent &e);
    void workProc();
};
#endif /* GPI_CONNMANAGER_HXX */
