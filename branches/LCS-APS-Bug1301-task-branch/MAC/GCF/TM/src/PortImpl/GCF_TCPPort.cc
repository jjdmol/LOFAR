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
#include "GTM_TCPServerSocket.h"
#include <ServiceBroker/ServiceBrokerTask.h>
//#include <ServiceBroker/GSB_Defines.h>
#include <GTM_Defines.h>
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
    _pSocket(0),
    _addrIsSet(false),
	_addr(),
	_host(myHostname(false)),
    _portNumber(0),
	itsFixedPortNr(false),
    _broker(0)
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
    _pSocket(0),
    _addrIsSet(false),
	_addr(),
	_host(myHostname(false)),
    _portNumber(0),
	itsFixedPortNr(false),
    _broker(0)
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
// try to open the socket
// Remember that this routine may be called many times before a socket is
// really open.
//
bool GCFTCPPort::open()
{
	// already connect?
	if (isConnected()) {
		LOG_ERROR(formatString("Port %s already open.", makeServiceName().c_str()));
		return (false);
	}

	// allocate a TCP socket when not done before.
	if (!_pSocket) {
		if (isSlave()) {
			LOG_ERROR(formatString ("Port %s not initialised.",
									makeServiceName().c_str()));
			return (false);
		}

		if ((getType() == SPP) || (getType() == MSPP)) {
			_pSocket = new GTMTCPServerSocket(*this, (MSPP == getType()));
		}
		else {
			ASSERTSTR (SAP == getType(), "Unknown TPCsocket type " << getType());
			_pSocket = new GTMTCPSocket(*this);
		}
	}

	setState(S_CONNECTING);

	if (getType() == SAP) {						// client socket?
		if (_portNumber != 0) {					// dest. overruled by user?
												// or answer already received before
			// Try to 'open' en 'connect' to port
			serviceInfo(SB_NO_ERROR, _portNumber, _host);
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
		return (true);
	}

	// porttype = MSPP or SPP
	// portnumber overruled by user? try mac.ns.<taskname>.<realname>.port
	string portNumParam = formatString(PARAM_TCP_PORTNR, 
						getTask()->getName().c_str(), getRealName().c_str());
	if (globalParameterSet()->isDefined(portNumParam)) {
		_portNumber = globalParameterSet()->getInt32(portNumParam);
	}
	if (_portNumber > 0) {					// portnumber hard set by user.
		serviceRegistered(SB_NO_ERROR, _portNumber);	// 'hard' open port
		// a (dis)connect event will be scheduled
		// Note: service is NOT registered at service broker!!!
	}
	else {	// portnumber not overruled by user so ask SB for a portnumber
		_broker = ServiceBrokerTask::instance();
		ASSERT(_broker);
		_broker->registerService(*this);
		// a (dis)connect event will be scheduled
	}

	return (true);
}


//
// serviceRegistered(resultToReturn, portNr)
//
// Log what port we use and try to open and connect to that port.
// Note: Is also called by the ServiceBrokerTask
//
void GCFTCPPort::serviceRegistered(unsigned int result, unsigned int portNumber)
{
	ASSERT(MSPP == getType() || SPP == getType());
	if (result != SB_NO_ERROR) {
		schedule_disconnected();
		return;
	}

	LOG_DEBUG(formatString (
				"(M)SPP port '%s' in task '%s' listens on portnumber %d.",
				getRealName().c_str(), _pTask->getName().c_str(),
				portNumber));
	_portNumber = portNumber;
	if (!_pSocket->open(portNumber)) {
		schedule_disconnected();
		return;
	}

	if (getType() == MSPP) {
		schedule_connected();
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
	ASSERT(SAP == getType());

	if (result == SB_UNKNOWN_SERVICE) {
		LOG_DEBUG(formatString ("Cannot connect the local SAP [%s] "
								"to remote (M)SPP [%s:%s]. Try again!!!",
								makeServiceName().c_str(),
								_addr.taskname.c_str(), _addr.portname.c_str()));

		schedule_disconnected();
		return;
	}

	if (result == SB_NO_ERROR) {
		_portNumber = portNumber;
		_host = host;

		LOG_DEBUG(formatString ("Can now connect the local SAP [%s] "
								"to remote (M)SPP [%s:%s@%s:%d].",
								makeServiceName().c_str(),
								_addr.taskname.c_str(), _addr.portname.c_str(),
								host.c_str(), portNumber));

		if (_pSocket->open(portNumber) && _pSocket->connect(portNumber, host)) {
			schedule_connected();
			return;
		}

		schedule_disconnected();
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

	unsigned int packsize;
	void* buf = e.pack(packsize);

	LOG_TRACE_STAT(formatString (
						"Sending event '%s' for task '%s' on port '%s'",
//						getTask()->eventName(e).c_str(), 
						eventName(e).c_str(), 
						getTask()->getName().c_str(), 
						getRealName().c_str()));

	if ((written = _pSocket->send(buf, packsize)) != (ssize_t) packsize) {  
		setState(S_DISCONNECTING);     
		schedule_disconnected();

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

	ssize_t	btsRead = _pSocket->recv(buf, count);
	if (btsRead == 0) {
		setState(S_DISCONNECTING);     
		schedule_disconnected();
	}

	return (btsRead);
}

//
// close()
//
bool GCFTCPPort::close()
{
	setState(S_CLOSING);  
	_pSocket->close();
	if (!itsFixedPortNr) {	// when portnumber was resolved clear it, so that on
		_portNumber = 0;	// the next open it will be resolved again.
	}
	// make sure a single server port is unregistered to that is can be 'connect'ed again.
	// NOTE: 050308 this is neccesary for EventPort but conflicts with the ServiceBroker
	//				functionality in GSB_Controller.cc.	 this is then 0x0.
//	if (getType() == SPP) {
//		_broker->deletePort(*this);
//	}

	schedule_close();

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

	// TODO: MSPP is check against getType and SPP against PORT.getType() !!!!
	if (getType() != MSPP || port.getType() != SPP) {
		return (false);
	}

	GTMTCPServerSocket* pProvider = (GTMTCPServerSocket*)_pSocket;
	if (port._pSocket == 0) {
		port._pSocket = new GTMTCPSocket(port);
	}

	if (pProvider->accept(*port._pSocket)) {
		setState(S_CONNECTING);        
		port.schedule_connected();
		result = true;
	}
	return result;
}
  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
