#include "GCF_PortManager.hxx"
#include <GCF_Port.hxx>
#include <TASK/GCF_Fsm.hxx>
#include <TASK/GCF_Event.hxx>
#include <Resources.hxx>
#include <DynPtrArray.cxx>

template class DynPtrArray<GCFPort>;

GCFPortManager* GCFPortManager::_pInstance = 0;

GCFPortManager* GCFPortManager::getInstance()
{
  if (_pInstance == 0)
  {
    _pInstance = new GCFPortManager();
  }
  return _pInstance;
}

GCFPortManager::GCFPortManager() : _servSockReady(PVSS_FALSE), _serverPort(0)
{
  _connByCl = _connections;
  _connections.setCompareFunction(GCFPortManager::sortFunc);
  _connByCl.setCompareFunction(GCFPortManager::sortClFunc);
}

GCFPortManager::~GCFPortManager()
{
  _serverSocket.close();
}

PVSSboolean GCFPortManager::init(GCFFsm *pFsm, unsigned short serverPort)
{
  PVSSboolean result(PVSS_TRUE);
  GCFPort *item;
  DynPtrArrayIndex i;

  if (_serverPort == 0)
  {
    _serverPort = serverPort;
  }
  if (_pFsm == 0)
  {
    _pFsm = pFsm;
  }
  for (i = 0; i < _connections.nofItems(); i++)
  {
    item = _connections.getAt(i);
    if (!item->_pFsm) item->_pFsm = pFsm;
  }
  
  if (_serverSocket.socket(AF_INET, SOCK_STREAM, 0))
  {
    _serverSocket.setReuseAddress(PVSS_TRUE);
    if (Resources::isDbgFlag(Resources::DBG_API_USR1))
      cerr << "Server socket created." << endl;
    if (_serverSocket.bind("sun", _serverPort))
    {
      if (Resources::isDbgFlag(Resources::DBG_API_USR1))
        cerr << "Server socket bind." << endl;
      if (_serverSocket.listen(5))
      {
        if (Resources::isDbgFlag(Resources::DBG_API_USR1))
          cerr << "Server socket listen..." << endl;
        _servSockReady = PVSS_TRUE;
      }
    }
  }
  if (Socket::SockErr() <= 0 || !_servSockReady)
  {
    if (Resources::isDbgFlag(Resources::DBG_API_USR1))
      cerr << "Server socket error:" << Socket::SockStringErr(Socket::SockErr()) << endl;
    result = PVSS_FALSE;
  }
  else if (Resources::isDbgFlag(Resources::DBG_API_USR1))
    cerr << "Server socket opened!!!" << endl;

  return result;
}

void GCFPortManager::workProc()
{
  if (_servSockReady)
  {
    GCFPort *item;
    DynPtrArrayIndex i;

    Socket::SelectMasks masks(Socket::AllMask);

    if (_serverSocket.select(masks, 5))
    {
      if (masks == Socket::ReadWriteMask || masks == Socket::ReadMask)
      {
        acceptNewConnection();
      }
    }
    for (i = 0; i < _connections.nofItems(); i++)
    {
      item = _connections.getAt(i);
      item->workProc();
    }
  }
  else
  {
    init();
  }
}

int GCFPortManager::sortFunc(const GCFPort *e1, const GCFPort *e2)
{
  if (e1->_hostName < e2->_hostName) return -1;
  if (e1->_hostName > e2->_hostName) return  1;

  return 0;
}

int GCFPortManager::sortClFunc(const GCFPort *e1, const GCFPort *e2)
{
  if (!e1->_name) return 1;
  if (!e2->_name) return -1;

  if (strcmp(e1->_name, e2->_name) < 0) return -1;
  if (strcmp(e1->_name, e2->_name) > 0) return 1;
  
  return 0;
}

int GCFPortManager::addHost(const char *netaddr, const char *clientName)
{
  GCFPort *newEntry = new GCFPort(netaddr, clientName);

  newEntry->_pFsm = _pFsm;
  newEntry->_isConnected = PVSS_FALSE;

  if (_connections.updateOrInsertAndMaintainSort(newEntry) == DYNPTRARRAY_INVALID)
    return -1;

  return 0;
}

int GCFPortManager::removeHost(CharString &host)
{
  GCFPort searchit(host);
  GCFPort *item;

  if ((item = _connections.cut(&searchit)) != 0)
  {
    delete item;
    return 0;
  }

  return -1;
}

GCFPort *GCFPortManager::getPort(const char *clientName)
{
  GCFPort searchit("", clientName);

  return _connByCl.findAndGetItem(&searchit);
}

Socket *GCFPortManager::getSocket(CharString &netaddr)
{
  GCFPort searchit(netaddr);
  GCFPort *item;

  if ((item = _connections.findAndGetItem(&searchit)) != 0)
  {
    return item->_pSocket;
  }

  return 0;
}

Socket *GCFPortManager::getSocketOfClient(const char *clientName)
{
  GCFPort searchit("", clientName);
  GCFPort *item;

  if ((item = _connByCl.findAndGetItem(&searchit)) != 0)
  {
    return item->_pSocket;
  }

  return 0;
}

int GCFPortManager::getFile(CharString &netaddr)
{
  GCFPort searchit(netaddr);
  GCFPort *item;

  if ((item = _connections.findAndGetItem(&searchit)) != 0)
  {
    if (item->_pSocket)
      if (item->_pSocket->isValid())
        return item->_pSocket->getFd();
  }

  return -1;
}

int GCFPortManager::getFileOfClient(const char *clientName)
{
  GCFPort searchit("", clientName);
  GCFPort *item;

  if ((item = _connByCl.findAndGetItem(&searchit)) != 0)
  {
    if (item->_pSocket)
      if (item->_pSocket->isValid())
        return item->_pSocket->getFd();
  }

  return -1;
}

PVSSboolean GCFPortManager::isConnected(CharString &netaddr)
{
  GCFPort searchit(netaddr);
  GCFPort *item;

  if ((item = _connections.findAndGetItem(&searchit)) != 0)
  {
    return item->_isConnected;
  }

  return PVSS_FALSE;
}

PVSSboolean GCFPortManager::isClientConnected(const char *clientName)
{
  GCFPort searchit("", clientName);
  GCFPort *item;

  if ((item = _connByCl.findAndGetItem(&searchit)) != 0)
  {
    return item->_isConnected;
  }

  return PVSS_FALSE;
}

void GCFPortManager::closeAllConnections()
{
  GCFPort *item;
  DynPtrArrayIndex i;

  for (i = 0; i < _connections.nofItems(); i++)
  {
    item = _connections.getAt(i);
    item->close();
  }
}

void GCFPortManager::closeConnection(const char *clientName)
{
  GCFPort searchit("", clientName);
  GCFPort *item;

  if ((item = _connByCl.findAndGetItem(&searchit)) != 0)
  {
    item->close();
  }
}

void GCFPortManager::acceptNewConnection()
{
  CharString peerHost;
  unsigned short peerPoort;
  Socket *pNewSocket = new Socket();

  if (_serverSocket.accept(peerHost, peerPoort, *pNewSocket))
  {
    GCFPort searchit(peerHost);
    GCFPort *item;

    if ((item = _connections.findAndGetItem(&searchit)) != 0)
    {
      if (Resources::isDbgFlag(Resources::DBG_API_USR1))
      {
        cerr << "Connection to " << item->_name << " established" << endl;
        cerr << "Socket to " << peerHost << " " << peerPoort << " accepted." << endl;
      }
      item->close(); // close first if necessary on an old socket
      item->connected(pNewSocket, peerPoort);
    }
    else
    {
      delete pNewSocket;
      if (Resources::isDbgFlag(Resources::DBG_API_USR1))
        cerr << "ConnManger does not accepted " << peerHost << " " << peerPoort << endl;
    }    
  }
  else
  {
    delete pNewSocket;
    if (Resources::isDbgFlag(Resources::DBG_API_USR1))
      cerr << "Server socket accept error:" << Socket::SockStringErr(Socket::SockErr()) << endl;
  }  
}
