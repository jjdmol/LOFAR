//#  GCF_Port.cc: connection to a remote process
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include "GCF_Port.h"
#include "GCF_RawPort.h"
#include <GCF_Task.h>
#include <GCF_Event.h>
#include <GCF_TMProtocols.h>
#include "GTM_NameService.h"
#include "GTM_TopologyService.h"

// all possible implementations are included here
#include <Socket/GCF_TCPPort.h>
//#include <Socket/GCF_TCPPortProvider.h>

// debug macro's
#ifdef DEBUG_SIGNAL
#define F_DEBUG_SEND(e) do     { debug_send(e);     } while (0)
#define F_DEBUG_DISPATCH(e) do { debug_dispatch(e); } while (0)
#else
#define F_DEBUG_SEND(e)
#define F_DEBUG_DISPATCH(e)
#endif

/**
 * ::GCFPort constructor
 */
GCFPort::GCFPort(GCFTask& task, string& name, TPortType type, int protocol) : 
    GCFPortInterface(&task, name, type, protocol), _pSlave(0)
{
}

/**
 * ::GCFPort default constructor
 */
GCFPort::GCFPort() :
    GCFPortInterface(0, "", SAP, 0), _pSlave(0)
{
}

/**
 * ::~GCFPort destructor
 */
GCFPort::~GCFPort()
{
    //
    // if the open method on the port has been called, there will be
    // a slave port which needs to be cleaned up.
    //
    if (_pSlave) delete _pSlave;
    _pSlave = 0;
}

/**
 * ::init
 */
void GCFPort::init(GCFTask& task,
		 string& name,
		 TPortType type,
		 int protocol)
{
    GCFPortInterface::init(task, name, type, protocol);
    _pSlave = 0;
}

/**
 * ::open
 */
int GCFPort::open()
{
  //
  // If the port has been openend before then the _pSlave port has already
  // been connected, and we don't need to connect it again. We can simply
  // call the ::open() method on the slave and return.
  //
  if (_pSlave) return _pSlave->open();
  
  //
  // This is the first call to open.
  // Determine what kind of slave port to create, get the necessary information
  // to connect it to its peer and open it.
  //
  if (SAP == _type)
  {  
      // find local and remote address
      if (GTMTopologyService::instance()->getPeerAddr(_pTask->getName(),
						    _name, _remoteAddr) < 0)
      {
	  LOG_DEBUG(("No address found for port '%s' of task '%s'\n",
		     _name, _pTask->getName()));
	  
	  return -1;
      }
  }
  
  // find my own address in the nameservice
  if (GTMNameService::instance()->query(_pTask->getName(),
				      _localAddr) < 0)
  {
    LOG_ERROR(("Could not find own address for "
	       "task '%s'.\n", _pTask->getName()));
    
    return -1;
  }
  if (GTMNameService::instance()->queryPort(_pTask->getName(),
					  getName(),
					  _localAddr))
  {
    LOG_ERROR(("Could not find port info for port '%s' of "
	       "task '%s'.\n", getName(), _pTask->getName()));
    
    return -1;
  }

  if (SAP == _type)
  {
    LOG_DEBUG((
	       "Connecting local SAP [%s:%s] "
	       "to remote SPP [%s(%s,%d):%s].\n",
	       _pTask->getName(),
	       getName(),
	       _remoteAddr.getTaskname(),
	       _remoteAddr.getHost(),
	       _remoteAddr.getPortnumber(),
	       _remoteAddr.getPortname()));
  }
  else if (SPP == _type && MSPP == _type)
  {
    LOG_DEBUG((
	       "Local SPP [%s:%s] listening on port %d for connections.\n",
	       _localAddr.getTaskname(),
	       _localAddr.getPortname(),
	       _localAddr.getPortnumber()));
  }

  // Check for the various port types
  if (!strcmp(_localAddr.getPorttype(), "TCP"))
  {
    GCFRawPort* pNewPort(0);
    string pseudoName = _name + "_TCP";
    if (MSPP == _type)
    {
      //pNewPort = new GCFTCPPortProvider(*_pTask, pseudoName, _type, _protocol);
    }
    else
    {
      pNewPort = new GCFTCPPort(*_pTask, pseudoName, _type, _protocol);
    }  
    pNewPort->setMaster(this);

    //if (SAP == _type) pNewPort->setAddr(_remoteAddr);
    //else pNewPort->setAddr(_localAddr);

    _pSlave = pNewPort;
  }
  else
  {
    LOG_ERROR(("no implementation found for port type '%s'",
	       _localAddr.getPorttype()));
    
    return -1;
  }

  return _pSlave->open();
}

/**
 * ::close
 */
int GCFPort::close()
{
  if (!_pSlave)
  {
    LOG_ERROR(("GCFPort::close: _pSlave == 0\n"));
    
    return -1;
  }

  return _pSlave->close();
}

/**
 * ::send
 */
ssize_t GCFPort::send(const GCFEvent& e, void* buf, size_t count)
{
  if (SPP == _type)
  {
    if (F_EVT_INOUT(e) & F_IN)
    {
      LOG_ERROR(("Trying to send IN event '%s' on SPP "
		 "port '%s'; discarding this event.\n",
		 getTask()->evtstr(e), getName()));
      return -1; // RETURN
    }
  }
  else if (SAP == _type)
  {
    if (F_EVT_INOUT(e) & F_OUT)
    {
      LOG_ERROR(("Trying to send OUT event '%s' on SAP "
		 "port '%s'; discarding this event.\n",
		 getTask()->evtstr(e), getName()));
      return -1; // RETURN
    }
  }
  else if (MSPP == _type)
  {
     LOG_ERROR(("Trying to send events by means of the portprovider. Not supported yet",
         getTask()->evtstr(e), getName()));
      return -1; // RETURN
  }

  iovec buffers[2];
  buffers[0].iov_base = (void*)&e;
  buffers[0].iov_len = e.length - count;
  buffers[1].iov_base  = buf;
  buffers[1].iov_len = count;
  
  return _pSlave->sendv(GCFEvent(F_RAW_SIG), buffers, (count > 0 ? 2 : 1));

}

/**
 * ::sendv
 */
ssize_t GCFPort::sendv(const GCFEvent& e, const iovec buffers[], int n)
{
  if (SPP == getType())
  {
    if (F_EVT_INOUT(e) & F_IN)
    {
      LOG_ERROR(("Trying to send IN event '%s' on SPP "
		 "port '%s'; discarding this event.\n",
		 getTask()->evtstr(e), getName()));
      return -1; // RETURN
    }
  }
  else if (SAP == getType())
  {
    if (F_EVT_INOUT(e) & F_OUT)
    {
      LOG_ERROR(("Trying to send OUT event '%s' on SAP "
		 "port '%s'; discarding this event.\n",
		 getTask()->evtstr(e), getName()));
      return -1; // RETURN
    }
  }
  else if (MSPP == _type)
  {
     LOG_ERROR(("Trying to send events by means of the portprovider. Not supported yet",
         getTask()->evtstr(e), getName()));
      return -1; // RETURN
  }

  iovec* newbufs = new iovec[n+1];
  newbufs[0].iov_base = (void*)&e;
  newbufs[0].iov_len  = e.length;
  for (int i = 1; i < n+1; i++)
  {
    newbufs[i].iov_base  = buffers[i-1].iov_base;
    newbufs[i].iov_len = buffers[i-1].iov_len;
  }

  int status = _pSlave->sendv(GCFEvent(F_RAW_SIG), newbufs, n+1);
  
  delete [] newbufs;

  return status;
}

/**
 * ::recv
 */
ssize_t GCFPort::recv(void* buf, size_t count)
{
  if (!_pSlave) return -1;
  return _pSlave->recv(buf, count);
}

/**
 * ::recvv
 */
ssize_t GCFPort::recvv(iovec buffers[], int n)
{
  if (!_pSlave) return -1;
  return _pSlave->recvv(buffers, n);
}

/**
 * ::setTimer
 */
long GCFPort::setTimer(long delay_sec,    long delay_usec,
		     long interval_sec, long interval_usec,
		     const void* arg)
{
  if (!_pSlave) return -1;
  return _pSlave->setTimer(delay_sec, delay_usec,
			  interval_sec, interval_usec,
			  arg);
}

/**
 * ::setTimer
 */
long GCFPort::setTimer(double delay_seconds, 
		     double interval_seconds,
		     const void* arg)
{
  if (!_pSlave) return -1;
  return _pSlave->setTimer(delay_seconds,
			  interval_seconds,
			  arg);
}

/**
 * ::cancelTimer
 */
int GCFPort::cancelTimer(long timerid, const void **arg)
{
  if (!_pSlave) return -1;
  return _pSlave->cancelTimer(timerid, arg);
}

/**
 * ::cancelAllTimers
 */
int GCFPort::cancelAllTimers()
{
  if (!_pSlave) return -1;
  return _pSlave->cancelAllTimers();
}

/**
 * resetTimerInterval
 */
int GCFPort::resetTimerInterval(long timerid,
			      long interval_sec,
			      long interval_usec)
{
  if (!_pSlave) return -1;
  return _pSlave->resetTimerInterval(timerid,
				    interval_sec,
				    interval_usec);
}

/**
 * ::debug_signal
 */
void GCFPort::debug_signal(const GCFEvent& /*e*/)
{
  LOG_DEBUG(("[%s:%s] %s %s\n",
	     getName(), p.getName(),
	     ((F_EVT_INOUT(e) & F_IN) ? "<-" : "->"), evtstr(e)));
}

/**
 * ::debug_send
 */
void GCFPort::debug_send(const GCFEvent& e)
{
  if (SAP == _type)
  {      
    LOG_DEBUG(("%s: port=%s event=%s ====> [%s:%s]\n",
	       _pTask->getName(), getName(),
	       _pTask->evtstr(e),
	       _remoteAddr.getTaskname(),
	       _remoteAddr.getPortname()
	       ));
  }
  else
  {
    LOG_DEBUG(("%s: port=%s event=%s ====>\n",
	       _pTask->getName(), getName(),
	       _pTask->evtstr(e)));
  }
}

/**
 * ::debug_dispatch
 */
void GCFPort::debug_dispatch(const GCFEvent& e)
{
  if (SAP == _type)
  {
    LOG_DEBUG(("%s: port=%s <**** event=%s [%s:%s]\n",
	       _pTask->getName(), getName(),
	       _pTask->evtstr(e),
	       _remoteAddr.getTaskname(),
	       _remoteAddr.getPortname()));
  }
  else
  {
    LOG_DEBUG(("%s: port=%s <**** event=%s\n",
	       _pTask->getName(), getName(),
	       _pTask->evtstr(e)));
  }
}

/**
 * ::dispatch
 */
/*int GCFPort::dispatch(GCFEvent& event)
{
  if (F_DATAIN_SIG != event.signal)
    {
      // only F_DATAIN_SIG is handled by this method

      F_DEBUG_DISPATCH(event);

      return getTask()->dispatch(event, *this);     // RETURN
    }

  static  GCFEvent e;
  char*   event_buf  = 0;
  GCFEvent* full_event = 0;
  int     n          = 0;
  int     status     = GCFEvent::HANDLED;

  //ACE_DEBUG((LM_DEBUG, "GCFPort::dispatch\n"));

  // receive the event header and check for errors
  n = _pSlave->recv(&e, sizeof(GCFEvent));

  if (n == 0) return GCFEvent::ERROR; // connection closed

  if (n < 0)
    {
      perror("GCFPort::dispatch");
      return GCFEvent::ERROR; 
    }

  //ACE_DEBUG((LM_DEBUG, "event length=%d\n", e.length));

  // check if the whole header was received
  if (sizeof(GCFEvent) == n)
    {
      // check for correct event direction
      // SAP only receives OUT events
      // SPP only receives IN events
      if (SAP == getType())
	{
	  if (F_EVT_INOUT(e) & F_IN)
	    {
	      ACE_ERROR_RETURN((LM_ERROR, "Received IN event '%s' on SAP "
				"port '%s'; ignoring this event.\n",
				getTask()->evtstr(e), getName()), GCFEvent::HANDLED);
	    }
	}
      else if (SPP == getType())
	{
	  if (F_EVT_INOUT(e) & F_OUT)
	    {
	      ACE_ERROR_RETURN((LM_ERROR, "Received OUT event '%s' on SPP "
				"port '%s'; ignoring this event.\n",
				getTask()->evtstr(e), getName()), GCFEvent::HANDLED);
	    }
	}
    
      // allocate space for the full message
      event_buf = (char*)malloc(e.length);
      full_event = (GCFEvent*)event_buf;
      if (!event_buf)
	{
	  ACE_ERROR_RETURN((LM_ERROR, "failed malloc\n"), GCFEvent::ERROR);
	}

      memcpy(event_buf, &e, sizeof(GCFEvent));
      if (e.length - sizeof(GCFEvent) > 0)
	{
	  // recv the rest of the message (payload)
	  ssize_t count = _pSlave->recv(event_buf + sizeof(GCFEvent),
				       e.length - sizeof(GCFEvent));

	  if ((ssize_t)(e.length - sizeof(GCFEvent)) != count)
	    {
	      ACE_ERROR_RETURN((LM_ERROR, "truncated recv (count=%d)\n", count), GCFEvent::ERROR);
	    }
	}

      //
      // DISPATCH the event to the task
      //
      F_DEBUG_DISPATCH(*full_event);
      status = getTask()->dispatch(*full_event, *this);
      if (GCFEvent::NOT_HANDLED == status)
	{
	  ACE_DEBUG((LM_DEBUG, "Unhandled event in GCFPort::dispatch\n"
		     "'%s' on port '%s'\n",
		     getTask()->evtstr(e), getName()));
	}
      free(event_buf);

      return status;
    }
  else
    {
      // handle_close send F_D_isConnectedSIG to the task
      return GCFEvent::ERROR; // cause close
    }

  return status;
}*/

