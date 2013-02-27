#include "GCF_Port.hxx"
#include "GCF_PortManager.hxx"

#include <TASK/GCF_Event.hxx>
#include <TASK/GCF_Fsm.hxx>

#include <Resources.hxx>
#include <DynPtrArray.cxx>
#include <Socket.hxx>

#include <netdb.h>

static GCFEvent disconnected_event(F_DISCONNECTED_SIG);
static GCFEvent connected_event   (F_CONNECTED_SIG);
static GCFEvent closed_event      (F_CLOSED_SIG);

//------------------------------------------------------------------------------
GCFPort::GCFPort(const char *netaddr, const char *clientName) :
  GCFPortInterface(SPP, clientName), _port(0), _pSocket(0)
{
  struct hostent *hostinfo;
  hostinfo = gethostbyname(netaddr);
  if (hostinfo != 0)
    _hostName = hostinfo->h_name;    
}

GCFPort::~GCFPort()
{
  close();
  if (_pSocket != 0)
  {
    delete _pSocket;
  }
}

void GCFPort::close()
{
  if (_pSocket && _isConnected)
  {
    if (Resources::isDbgFlag(Resources::DBG_API_USR1) && _name != 0 && strlen(_name) > 0)
      cerr << "Closing connection to " << _name << endl;
    if (_pSocket->isValid())
    {
      _pSocket->close();
      _isConnected = PVSS_FALSE;
    }
  }
}

void GCFPort::workProc()
{
  if (_isConnected)
  {
    Socket::SelectMasks masks(Socket::AllMask);

    if (_pSocket->select(masks, 5))
    {
      ssize_t bytesReceived;
      GCFEvent e;
      //cerr << "Conn mask: " << masks << endl;
      switch (masks)
      {
        case Socket::NullMask:
          cerr << "NullMask: Valid " << _pSocket->isValid() << " CommpletionCode " << _pSocket->getCompletionCode() << endl;
          break;
        case Socket::ReadMask:
          //_pSocket->recv(buf, 200, bytesReceived);
          cerr << "Bytes recv: " << bytesReceived << endl;
          break;
        case Socket::WriteMask:
          break;
        case Socket::ErrorMask:
          if (Resources::isDbgFlag(Resources::DBG_API_USR1))
            cerr << "ErrorMask: Valid " << _pSocket->isValid() << " CommpletionCode " << _pSocket->getCompletionCode() << endl;
          break;
        case Socket::ReadWriteMask:
          bytesReceived = recv(&e, sizeof(GCFEvent));
          if (Resources::isDbgFlag(Resources::DBG_API_USR1))
            cerr << "Bytes recv: " << bytesReceived << endl;
          if (bytesReceived < (ssize_t) sizeof(GCFEvent))
          {
            if (Resources::isDbgFlag(Resources::DBG_API_USR1))
              cerr << "Disconneted by peer: ";
              
            close();
            _pFsm->dispatch(disconnected_event, *this);
          }
          else
          {
            if (F_EVT_INOUT(e) & F_OUT) // we are always a SPP, so only IN events are valid
            {
              if (Resources::isDbgFlag(Resources::DBG_API_USR1))
                cerr << "Received IN event '%s' on SAP port '%s'; ignoring this event.\n";
            }
            else
            {
              eventReceived(e);
            }
          }
          break;
        case Socket::AllMask:
          cerr << "AllMask: Valid " << _pSocket->isValid() << " CommpletionCode " << _pSocket->getCompletionCode() << endl;
          break;
      }
    }
  }
}

ssize_t GCFPort::send(const GCFEvent& event, void* buf, size_t count)
{
  ssize_t bytesReceived;
  ssize_t totalBytesReceived = 0;
  if (event.signal != F_RAW_SIG)
  {
    _pSocket->send((char*)&event, event.length - count, bytesReceived);
    totalBytesReceived += bytesReceived;
  }
  if (count > 0 && buf != 0)
  {
    _pSocket->send((char*)buf, count, bytesReceived);
    totalBytesReceived += bytesReceived;
  }
  return totalBytesReceived;
}

ssize_t GCFPort::sendv(const GCFEvent& event, const iovec buffers[], int n)
{
  ssize_t bytesReceived = 0;

  bytesReceived += send(event);
  for (int i = 0; i < n; i++)
  {
    bytesReceived += send(GCFEvent(F_RAW_SIG), buffers[i].iov_base, buffers[i].iov_len);
  }
  return bytesReceived;
}

ssize_t GCFPort::recv(void* buf, size_t count)
{
  ssize_t bytesReceived;
  _pSocket->recv((char*)buf, count, bytesReceived);
  return bytesReceived;
}

ssize_t GCFPort::recvv(iovec buffers[], int n)
{
  ssize_t bytesReceived = 0;
  
  for (int i = 0; i < n; i++)
  {
    bytesReceived += recv(buffers[i].iov_base, buffers[i].iov_len);
  }
  return bytesReceived;
}

void GCFPort::connected(Socket *pSocket, unsigned short int port)
{
  if (_pSocket)
  {
     delete _pSocket;
  }
  _pSocket = pSocket;
  _port = port;
  _isConnected = PVSS_TRUE;
  _pFsm->dispatch(connected_event, *this);
}

void GCFPort::eventReceived(GCFEvent &e)
{
  char*   event_buf  = 0;
  GCFEvent* full_event = 0;

  event_buf = (char*)malloc(e.length);
  full_event = (GCFEvent*)event_buf;

  memcpy(event_buf, &e, sizeof(GCFEvent));
  if (e.length - sizeof(GCFEvent) > 0)
  {
    // recv the rest of the message (payload)
    ssize_t count = recv(event_buf + sizeof(GCFEvent),
                          e.length - sizeof(GCFEvent));

    if ((ssize_t)(e.length - sizeof(GCFEvent)) != count)
    {
	    fprintf(stderr, "truncated recv (count=%d)\n", count);
      exit(1);
    }
  }

  _pFsm->dispatch(*full_event, *this);
  free(event_buf);
}