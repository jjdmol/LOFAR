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

#include <GCF/GCF_Port.h>
#include <GCF/GCF_RawPort.h>
#include <GCF/GCF_Task.h>
#include <GCF/GCF_Event.h>
#include <GCF/GCF_TMProtocols.h>
#include <PortInterface/GTM_NameService.h>
#include <PortInterface/GTM_TopologyService.h>
#include <GTM_Defines.h>

// all possible implementations are included here
#include <GCF/GCF_TCPPort.h>

/**
 * ::GCFPort constructor
 */
GCFPort::GCFPort(GCFTask& task, 
                 string& name, 
                 TPortType type, 
                 int protocol, 
                 bool transportRawData) : 
    GCFPortInterface(&task, name, type, protocol, transportRawData), _pSlave(0)
{
}

/**
 * ::GCFPort default constructor
 */
GCFPort::GCFPort() :
    GCFPortInterface(0, "", SAP, 0, false), _pSlave(0)
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
		 string name,
		 TPortType type,
		 int protocol, 
     bool transportRawData)
{
    GCFPortInterface::init(task, name, type, protocol, transportRawData);
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
      LOFAR_LOG_DEBUG(TM_STDOUT_LOGGER, (
          "No address found for port '%s' of task '%s'",
          _name.c_str(), _pTask->getName().c_str()));
    
      return -1;
    }
  }
  
  // find my own address in the nameservice
  if (GTMNameService::instance()->query(_pTask->getName(),
				      _localAddr) < 0)
  {
    LOFAR_LOG_ERROR(TM_STDOUT_LOGGER, (
        "Could not find own address for task '%s'.", 
        _pTask->getName().c_str()));
    
    return -1;
  }
  if (GTMNameService::instance()->queryPort(_pTask->getName(),
					  getName(),
					  _localAddr))
  {
    LOFAR_LOG_ERROR(TM_STDOUT_LOGGER, (
        "Could not find port info for port '%s' of task '%s'.", 
        _name.c_str(), _pTask->getName().c_str()));
    
    return -1;
  }

  if (SAP == _type)
  {
    LOFAR_LOG_DEBUG(TM_STDOUT_LOGGER, (
        "Connecting local SAP [%s:%s] to remote SPP [%s(%s,%d):%s].",
        _pTask->getName().c_str(),
        _name.c_str(),
        _remoteAddr.getTaskname().c_str(),
        _remoteAddr.getHost().c_str(),
        _remoteAddr.getPortnumber(),
        _remoteAddr.getPortname().c_str()));
  }
  else if (SPP == _type && MSPP == _type)
  {
    LOFAR_LOG_DEBUG(TM_STDOUT_LOGGER, (
        "Local SPP [%s:%s] listening on port %d for connections.",
        _localAddr.getTaskname().c_str(),
        _localAddr.getPortname().c_str(),
        _localAddr.getPortnumber()));
  }

  // Check for the various port types
  if (_localAddr.getPorttype() == "TCP")
  {
    GCFTCPPort* pNewPort(0);
    string pseudoName = _name + "_TCP";
    pNewPort = new GCFTCPPort(*_pTask, pseudoName, _type, _protocol);
    pNewPort->setMaster(this);

    if (SAP == _type) pNewPort->setAddr(_remoteAddr);
    else pNewPort->setAddr(_localAddr);

    _pSlave = pNewPort;
  }
  else
  {
    LOFAR_LOG_ERROR(TM_STDOUT_LOGGER, (
        "no implementation found for port type '%s'",
	      _localAddr.getPorttype().c_str()));
    
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
    LOFAR_LOG_ERROR(TM_STDOUT_LOGGER, (
        "GCFPort::close: _pSlave == 0"));
    
    return -1;
  }

  return _pSlave->close();
}

/**
 * ::send
 */
ssize_t GCFPort::send(const GCFEvent& e, void* buf, size_t count)
{
  if (F_RAW_SIG != e.signal)
  {
    LOFAR_LOG_TRACE(TM_STDOUT_LOGGER, (
      "Sending event '%s' for task %s on port %s",
      getTask()->evtstr(e),
      getTask()->getName().c_str(), 
      getName().c_str()));
  }

  if (SPP == _type)
  {
    if (F_EVT_INOUT(e) & F_IN)
    {
      LOFAR_LOG_ERROR(TM_STDOUT_LOGGER, (
          "Trying to send IN event '%s' on SPP "
		      "port '%s'; discarding this event.",
		      getTask()->evtstr(e), _name.c_str()));
         
      return -1; // RETURN
    }
  }
  else if (SAP == _type)
  {
    if (F_EVT_INOUT(e) & F_OUT)
    {
      LOFAR_LOG_ERROR(TM_STDOUT_LOGGER, (
          "Trying to send OUT event '%s' on SAP "
		      "port '%s'; discarding this event.",
		      getTask()->evtstr(e), _name.c_str()));
      return -1; // RETURN
    }
  }
  else if (MSPP == _type)
  {
     LOFAR_LOG_ERROR(TM_STDOUT_LOGGER, (
        "Trying to send event '%s' by means of the portprovider: %s (MSPP). "
        "Not supported yet",
         getTask()->evtstr(e), _name.c_str()));
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
  if (F_RAW_SIG != e.signal)
  {
    LOFAR_LOG_TRACE(TM_STDOUT_LOGGER, (
      "Sending event '%s' for task %s on port %s",
      getTask()->evtstr(e),
      getTask()->getName().c_str(), 
      getName().c_str()));
  }

  if (SPP == getType())
  {
    if (F_EVT_INOUT(e) & F_IN)
    {
      LOFAR_LOG_ERROR(TM_STDOUT_LOGGER, (
          "Trying to send IN event '%s' on SPP "
		      "port '%s'; discarding this event.",
		      getTask()->evtstr(e), _name.c_str()));
      return -1; // RETURN
    }
  }
  else if (SAP == getType())
  {
    if (F_EVT_INOUT(e) & F_OUT)
    {
      LOFAR_LOG_ERROR(TM_STDOUT_LOGGER, (
          "Trying to send OUT event '%s' on SAP "
		      "port '%s'; discarding this event.",
		      getTask()->evtstr(e), _name.c_str()));
      return -1; // RETURN
    }
  }
  else if (MSPP == _type)
  {
     LOFAR_LOG_ERROR(TM_STDOUT_LOGGER, (
        "Trying to send event '%s' by means of the portprovider: %s (MSPP). "
        "Not supported yet",
         getTask()->evtstr(e), _name.c_str()));
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
		     void* arg)
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
		     void* arg)
{
  if (!_pSlave) return -1;
  return _pSlave->setTimer(delay_seconds,
			  interval_seconds,
			  arg);
}

/**
 * ::cancelTimer
 */
int GCFPort::cancelTimer(long timerid, void **arg)
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
