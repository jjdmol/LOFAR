//#  GCF_TCPPort.cc: connection to a remote process
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/SystemUtil.h>

#include <MACIO/SB_Protocol.ph>
#include <GCF/TM/GCF_TCPPort.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Protocols.h>
#include <Common/ParameterSet.h>
#include <Timer/GTM_TimerHandler.h>
#include <ServiceBroker/ServiceBrokerTask.h>
#include <GTM_Defines.h>
#include "GTM_TCPServerSocket.h"
#include <errno.h>

namespace LOFAR {
  namespace GCF {
    using namespace SB;
    namespace TM {


//
// GCFTCPPort(task, servicename, type, protocol, raw)
//
GCFTCPPort::GCFTCPPort(GCFTask& 	 task, 
                       const string& name, 
                       TPortType 	 type, 
                       int 			 protocol, 
                       bool 		 transportRawData) 
  : GCFRawPort(task, name, type, protocol, transportRawData),
    _pSocket		  (0),
    _addrIsSet		  (false),
	_addr			  (),
	_host			  (myHostname(false)),
    _portNumber		  (0),
	itsFixedPortNr	  (false),
	itsAutoOpen		  (false),
	itsAutoOpenTimer  (0),
	itsAutoRetryTimer (0),
	itsAutoRetries	  (0),
	itsAutoRetryItv	  (0.0),
	itsConnectTimer	  (0),
    _broker			  (0)
{
	if (SPP == getType() || MSPP == getType()) {
		_pSocket = new GTMTCPServerSocket(*this, (MSPP == type));
	}
	else if (SAP == getType()) {
		_pSocket = new GTMTCPSocket(*this);
	}  
}

//
// GCFTCPPort()
//
GCFTCPPort::GCFTCPPort()
    : GCFRawPort(),
    _pSocket		  (0),
    _addrIsSet		  (false),
	_addr			  (),
	_host			  (myHostname(false)),
    _portNumber		  (0),
	itsFixedPortNr	  (false),
	itsAutoOpen		  (false),
	itsAutoOpenTimer  (0),
	itsAutoRetryTimer (0),
	itsAutoRetries	  (0),
	itsAutoRetryItv	  (0.0),
	itsConnectTimer	  (0),
    _broker			  (0)
{
}

//
// ~GCFTCPPort()
//
GCFTCPPort::~GCFTCPPort()
{
	if (_pSocket) {
		delete _pSocket;
		_pSocket = 0;    
	}

	if (_broker) {
		_broker->deletePort(*this);
		ServiceBrokerTask::release();
		_broker = 0;
	}
}

//
// init (task, servicename, type, protocol, raw)
//
void GCFTCPPort::init(GCFTask& 		task, 
                      const string&	name, 
                      TPortType 	type, 
                      int 			protocol,
                      bool 			transportRawData)
{
    _state = S_DISCONNECTED;
    GCFRawPort::init(task, name, type, protocol, transportRawData);
    _portNumber 	= 0;
    _host       	= myHostname(false);
    _addrIsSet  	= false;
	itsFixedPortNr	= false;
    if (_pSocket) {
		delete _pSocket;
		_pSocket = 0;
	}
}

//
// Try to open the socket
// Remember that this routine may be called many times before a socket is realy open.
//
// NOTE: the return state reflects whether or not an answer is already send to the caller.
//
bool GCFTCPPort::open()
{
	// already connect?
	if (isConnected()) {
		LOG_WARN(formatString("Port %s already open.", makeServiceName().c_str()));
		return (false);
	}

	if (getState() == S_CONNECTING) {
		LOG_DEBUG(formatString("Opening of port %s already in progress", makeServiceName().c_str()));
		return (false);
	}

	// allocate a TCP socket when not done before.
	if (!_pSocket) {
		if (isSlave()) {
			LOG_ERROR(formatString ("Port %s not initialised.", makeServiceName().c_str()));
			return (false);
		}

		if ((getType() == SPP) || (getType() == MSPP)) {
			_pSocket = new GTMTCPServerSocket(*this, (MSPP == getType()));
		}
		else {
			ASSERTSTR (SAP == getType(), "Unknown TPCsocket type " << getType());
			_pSocket = new GTMTCPSocket(*this);
			_pSocket->setBlocking(false);
		}
	}

	setState(S_CONNECTING);
	LOG_TRACE_COND("open: state = CONNECTING");

	if (getType() == SAP) {						// client socket?
		if (_portNumber != 0) {					// dest. overruled by user?
												// or answer already received before
			// Try to 'open' en 'connect' to port
			serviceInfo(SB_NO_ERROR, _portNumber, _host);		// sends a F_CONNECTED
			return (true);
		}

		string	remoteServiceName;
		// If service name is not set yet try to resolve it.
		if (usesModernServiceName()) {
			remoteServiceName = makeServiceName();
		}
		else {
			if (!_addrIsSet) {
				TPeerAddr fwaddr;
				// try mac.top.<taskname>.<name>.remoteservice in gblPS
				if (findAddr(fwaddr)) {
					setAddr(fwaddr);
					remoteServiceName = formatString("%s:%s", 
							fwaddr.taskname.c_str(), fwaddr.portname.c_str());
				}
				else {
					// No information available to connect.
					setState(S_DISCONNECTED);
					LOG_TRACE_COND("open: state = DISCONNECTED");
					ASSERTSTR(false, "No remote address info for port '" <<
									 getRealName() << "' of task " <<
									 _pTask->getName());
				}
			}
		}

		// Service name is set, use it to resolve host+port and connect.
		if (!_broker) {
			_broker = ServiceBrokerTask::instance();
		}
		ASSERT(_broker);
		_broker->getServiceinfo(*this, remoteServiceName, _host);
		// a (dis)connect event will be scheduled
		return (false);	// REO: changed that
	}

	// porttype = MSPP or SPP
	// portnumber overruled by user? try mac.ns.<taskname>.<realname>.port
	string portNumParam = formatString(PARAM_TCP_PORTNR, getTask()->getName().c_str(), getRealName().c_str());

	_portNumber = globalParameterSet()->getInt32(portNumParam, 0);
	if (_portNumber > 0) {					// portnumber hard set by user.
		serviceRegistered(SB_NO_ERROR, _portNumber);	// 'hard' open port
		return (true);
	}

	// portnumber not overruled by user so ask SB for a portnumber
	_broker = ServiceBrokerTask::instance();
	ASSERT(_broker);
	_broker->registerService(*this); // a (dis)connect event will be scheduled
	return (false);	// REO changed that
}

//
// autoOpen(nrRetries, timeout, reconnectInterval)
//	Will generate a F_CONNECTED or F_DISCONNECTED event.
//
void GCFTCPPort::autoOpen(uint	nrRetries, double	timeout, double	reconnectInterval)
{
	itsAutoOpen = true;

	if (open()) {							// first try to open it
		return;
	}

	// It is not open yet. But the call to open() activated the ServiceBroker task to do it job.
	// All we have to do is copy the user settings, (start a timer) and wait.

	itsAutoOpenTimer  = 0;
	itsAutoRetryTimer = 0;
	itsAutoRetries    = nrRetries;
	itsAutoRetryItv   = reconnectInterval;
	if (timeout > 0.0) {		// absolute max auto-open time specified? Set timer for doomsday.
		itsAutoOpenTimer = _pTimerHandler->setTimer(*this, (uint64)(1000000.0*timeout), 0, &itsAutoOpenTimer);
		if (itsAutoRetries == 0) {
			itsAutoRetries = -1;		// to let the timeout timer running
		}
	}
}

//
// _handleDisconnect()
//
void GCFTCPPort::_handleDisconnect()
{
	LOG_TRACE_STAT_STR("_handleDisco:autoOpen=" << (itsAutoOpen ? "Yes" : "No") << 
			", nrRetries=" << itsAutoRetries << ", retryTimer=" << itsAutoRetryTimer << 
			", maxTimer=" << itsAutoOpenTimer << ", connTimer=" << itsConnectTimer);

    setState(S_DISCONNECTED);
	LOG_TRACE_COND_STR("_state=" << _state);
	LOG_TRACE_COND_STR("getState=" << getState());

	// retries left?
	if (itsAutoOpen) {
		if (itsAutoRetries != 0) {
			itsAutoRetryTimer = _pTimerHandler->setTimer(*this, (uint64)(1000000.0*itsAutoRetryItv), 0, &itsAutoRetryTimer);
			return;
		}
	}

	// stop reconnect timer
	_pTimerHandler->cancelTimer(itsConnectTimer);
	itsConnectTimer = 0;

	// stop auto timers
	_pTimerHandler->cancelTimer(itsAutoOpenTimer);
	itsAutoOpenTimer = 0;
	_pTimerHandler->cancelTimer(itsAutoRetryTimer);
	itsAutoRetryTimer = 0;

	// inform user
	_pSocket->setBlocking(true);
	LOG_DEBUG_STR("Scheduling disconnect for port " << getName());
	schedule_disconnected();
}

//
// _handleConnect()
//
void GCFTCPPort::_handleConnect()
{
	LOG_TRACE_STAT_STR("_handleConn:autoOpen=" << (itsAutoOpen ? "Yes" : "No") << 
			", nrRetries=" << itsAutoRetries << ", retryTimer=" << itsAutoRetryTimer << 
			", maxTimer=" << itsAutoOpenTimer << ", connTimer=" << itsConnectTimer);

	// stop all related timers.
	_pTimerHandler->cancelTimer(itsConnectTimer);
	itsConnectTimer = 0;

	// stop auto timers
	_pTimerHandler->cancelTimer(itsAutoOpenTimer);
	itsAutoOpenTimer = 0;
	_pTimerHandler->cancelTimer(itsAutoRetryTimer);
	itsAutoRetryTimer = 0;
	itsAutoOpen = false;

	// inform user
	_pSocket->setBlocking(true);
	schedule_connected();
}

//
// dispatch(event)
//
GCFEvent::TResult	GCFTCPPort::dispatch(GCFEvent&	event)
{
	if (event.signal == F_TIMER) {
		GCFTimerEvent	*TEptr = static_cast<GCFTimerEvent*>(&event);
		if (TEptr->arg == &itsAutoOpenTimer) {	// Max auto open time reached?
			itsAutoRetries   = 0;
			itsAutoOpenTimer = 0;
			_handleDisconnect();
			return (GCFEvent::HANDLED);
		}
		if (TEptr->arg == &itsAutoRetryTimer) {
			if (itsAutoRetries > 0) {		// don't lower counter when it is set by the maxTimer
				itsAutoRetries--;
			}
			itsAutoRetryTimer = 0;
			open();
			return (GCFEvent::HANDLED);
		}
		if (TEptr->arg == &itsConnectTimer) {
		    LOG_INFO_STR("GCFTCPPort:connect(" << _portNumber << "@" << _host << ") still in progress");
			_pSocket->connect(_portNumber, _host);
			return (GCFEvent::HANDLED);
		}
	}

	return(GCFRawPort::dispatch(event));	// call dispatch from parent (RawPort).
}


//
// serviceRegistered(resultToReturn, portNr)
//
// Log what port we use and try to open and connect to that port.
// Note: Is also called by the ServiceBrokerTask
//
void GCFTCPPort::serviceRegistered(unsigned int result, unsigned int portNumber)
{
	LOG_TRACE_FLOW_STR("serviceRegistered(" << result << ", " << portNumber << "): " << getRealName());

	ASSERT(MSPP == getType() || SPP == getType());
	if (result != SB_NO_ERROR) {
		_handleDisconnect();
		return;
	}

	LOG_DEBUG(formatString ( "Starting service '%s' for task '%s' on portnumber %d.",
				getRealName().c_str(), _pTask->getName().c_str(), portNumber));
	_portNumber = portNumber;
	if (!_pSocket->open(portNumber)) {
		_handleDisconnect();
		return;
	}

	if (getType() == MSPP) {
		_handleConnect();
	}
}


//
// serviceInfo (result, portNumber)
//
// Ask servicebroker for a portnumber and try to open the port(listener)
//
// Note: Is also called by the ServiceBrokerTask
void GCFTCPPort::serviceInfo(unsigned int result, unsigned int portNumber, const string& host)
{
	LOG_TRACE_FLOW_STR("serviceInfo(" << result << ", " << portNumber << ", " << host << ")");

	ASSERTSTR(getType() == SAP, "Programming error: serviceInfo called on port of type " << getType() << " iso " << SAP);

	if (result != SB_NO_ERROR) {
		// Errors are already printed in the ServiceBrokerTask
		_handleDisconnect();
		return;
	}

	// Service name was resolved succesfully.
	_portNumber = portNumber;
	_host 		= host;
	LOG_DEBUG(formatString ("Can now connect '%s' to remote SPP [%s:%s@%s:%d].",
							makeServiceName().c_str(), _addr.taskname.c_str(), _addr.portname.c_str(),
							host.c_str(), portNumber));

	// Note: _pSocket is of type GTMTCPSocket
	if (!_pSocket->open(portNumber)) {
		_handleDisconnect();
	}

    // Set socket to non-blocking to prevent stalls
    _pSocket->setBlocking(false);

	switch (_pSocket->connect(portNumber, host)) {
	case -1: _handleDisconnect(); break;	// error
	case 0:  
		LOG_INFO_STR("GCFTCPPort:connect(" << portNumber << "@" << host << ") still in progress");
		// start 1 second interval timer to poll connect result
		if (!itsConnectTimer) {
			itsConnectTimer = _pTimerHandler->setTimer(*this, (uint64)(1000000.0), (uint64)(1000000.0), &itsConnectTimer);
		}
		break;							// in progress
	case 1:  _handleConnect(); break;		// successfull
	}
}

// Note: Is also called by the ServiceBrokerTask
void GCFTCPPort::serviceGone()
{
	_host = myHostname(false);
	_portNumber = 0;
}

//
// send(event)
//
ssize_t GCFTCPPort::send(GCFEvent& e)
{
	ssize_t written = 0;

	ASSERT(_pSocket);

	if (!isConnected()) {
		LOG_ERROR(formatString (
					"Port '%s' on task '%s' not connected! Event not sent!",
					getRealName().c_str(), getTask()->getName().c_str()));
		return 0;
	}

	if (MSPP == getType())   {
		return 0; // no messages can be send by this type of port
	}

#if 0
	unsigned int packSize;
	void* buf = e.pack(packSize);
#else
	e.pack();
	char*	buf      = e.packedBuffer();
	uint	packSize = e.bufferSize();
#endif

	LOG_TRACE_STAT(formatString (
						"Sending event '%s' for task '%s' on port '%s'",
						eventName(e).c_str(), 
						getTask()->getName().c_str(), 
						getRealName().c_str()));

	if ((written = _pSocket->send(buf, packSize)) != (ssize_t) packSize) {  
		LOG_ERROR_STR("Could only send " << written << " of " << packSize << " bytes");
		setState(S_DISCONNECTING);     
		LOG_TRACE_COND("write: state = DISCONNECTING");
		_handleDisconnect();

		written = 0;
	}

	return (written);
}


//
// recv(buf, count)
//
ssize_t GCFTCPPort::recv(void* buf, size_t count)
{
	ASSERT(_pSocket);

	if (!isConnected()) {
		LOG_ERROR(formatString (
					"Port '%s' on task '%s' not connected! Can't read device!",
					getRealName().c_str(), getTask()->getName().c_str()));
		return 0;
	}

	ssize_t	btsRead = _pSocket->recv(buf, count, isTransportRawData());
	if (btsRead == 0) {
		setState(S_DISCONNECTING);     
		LOG_TRACE_COND("recv: state = DISCONNECTING");
		_handleDisconnect();
	}

	return (btsRead);
}

//
// close()
//
bool GCFTCPPort::close()
{
	setState(S_CLOSING);  
	LOG_TRACE_COND("close: state = CLOSING");
	_pSocket->close();
	if (!itsFixedPortNr) {	// when portnumber was resolved clear it, so that on
		_portNumber = 0;	// the next open it will be resolved again.
	}
	// make sure a single server port is unregistered to that is can be 'connect'ed again.
	// NOTE: 050308 this is neccesary for EventPort but conflicts with the ServiceBroker
	//				functionality in CUDaemons/ServiveBroker.cc.	 this is then 0x0.
//	if (getType() == SPP) {
//		_broker->deletePort(*this);
//	}

	if (_broker && getType() != SAP) {
		_broker->deletePort(*this);
	}

	// return success when port is still connected
	// scheduled close will only occur later
	return isConnected();
}

//
// setAddr(addr)
//
// Depricated: this mechanisme should not be used anymore, use modern task-masks instead.
//
void GCFTCPPort::setAddr(const TPeerAddr& addr)
{
	LOG_DEBUG_STR("set service at " << addr.taskname << ":" << addr.portname);

	// Is new address different from current address?
	if (_addr.taskname != addr.taskname || _addr.portname != addr.portname) {
		_host = myHostname(false);					// clear current settings
		_portNumber = 0;
	}
	_addr = addr;
	_addrIsSet = ((_addr.taskname != "") && (_addr.portname != ""));
	_deviceNameMask = formatString("%s:%s", _addr.taskname.c_str(), 
											_addr.portname.c_str());
}

//
// accept(port)
//
bool GCFTCPPort::accept(GCFTCPPort& port)
{
	bool result(false);

	// NOTE: MSPP is check against getType and SPP against PORT.getType() !!!!
	if (getType() != MSPP || port.getType() != SPP) {
		return (false);
	}

	GTMTCPServerSocket* pProvider = (GTMTCPServerSocket*)_pSocket;
	if (port._pSocket == 0) {
		port._pSocket = new GTMTCPSocket(port);
	}

	if (pProvider->accept(*port._pSocket)) {
		setState(S_CONNECTING);        
		LOG_TRACE_COND("accept: state = CONNECTING");
		port.schedule_connected();		// NO _handleConnect()  !!!!
		result = true;
	}
	return result;
}
  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
