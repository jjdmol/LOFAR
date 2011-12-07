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

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <Common/ParameterSet.h>
#include <MACIO/GCF_Event.h>
#include <GCF/TM/GCF_Port.h>
#include <GCF/TM/GCF_RawPort.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Protocols.h>
#include <GTM_Defines.h>

#include <GCF/TM/GCF_TCPPort.h>
#include <GCF/TM/GCF_ETHRawPort.h>
#include <GCF/TM/GCF_DevicePort.h>

namespace LOFAR {
 namespace GCF {
  namespace TM {

//
// GCFPort(task, name, type, protocol, raw)
//
GCFPort::GCFPort(GCFTask& 		task, 
                 const string&	name, 
                 TPortType 		type, 
                 int 			protocol, 
                 bool 			transportRawData) : 
	GCFPortInterface(&task, name, type, protocol, transportRawData), _pSlave(0)
{
	// When modern name is used this name contains the complete servicename
	// Store this info in the remotexxx variables.
	string::size_type 	colon = name.find(":",0);
	if (colon != string::npos) {
		_remotetask = name.substr(0,colon);
		_remoteport = name.substr(colon+1);
	}
}

//
// GCFPort()
//
GCFPort::GCFPort() :
    GCFPortInterface(0, "", SAP, 0, false), _pSlave(0)
{
}

//
// ~GCFPort()
//
GCFPort::~GCFPort()
{
	//
	// if the open method on the port has been called, there will be
	// a slave port which needs to be cleaned up.
	//
	if (_pSlave)  {
		delete _pSlave;
	}
	_pSlave = 0;
}

//
// init (task, name, type, protocol, raw)
//
void GCFPort::init(GCFTask& 	 task,
				   const string& name,
				   TPortType 	 type,
				   int 			 protocol, 
				   bool 		 transportRawData)
{
	GCFPortInterface::init(task, name, type, protocol, transportRawData);
	if (_pSlave) {
		delete _pSlave;
	}
	_pSlave = 0;

	// When modern name is used this name contains the complete servicename
	// Store this info in the remotexxx variables.
	string::size_type 	colon = name.find(":",0);
	if (colon != string::npos) {
		_remotetask = name.substr(0,colon);
		_remoteport = name.substr(colon+1);
	}
}

//
// open()
//
bool GCFPort::open()
{
	if (_state != S_DISCONNECTED && _state != S_CONNECTED) {
		return (false);
	}

	if (getType() == MSPP) {		// listener
		LOG_ERROR("Ports of type MSPP should not be initialised directly via this common port");
		return (false);
	}
	_state = S_CONNECTING;

	//
	// If the port has been openend before then the _pSlave port has already
	// been connected, and we don't need to connect it again. We can simply
	// call the ::open() method on the slave and return.
	//
	if (_pSlave) {
		return (_pSlave->open());
	}

	//
	// This is the first call to open.
	// Determine what KIND OF SLAVE PORT to create and open it.
	//
	string 		typeParam = formatString (PARAM_PORT_PROT_TYPE,
									_pTask->getName().c_str(), _name.c_str());
	string 		protType("");
	TPeerAddr 	addr;
	// try mac.ns.<taskname>.<name>.type in parameterSet
	if (globalParameterSet()->isDefined(typeParam)) {
		protType = globalParameterSet()->getString(typeParam);
	}
	else {
		// remote service is set by user?
		if (_remotetask.length() > 0 && _remoteport.length() > 0) {
			addr.taskname = _remotetask;
			addr.portname = _remoteport;
		}
		else {
			// try to retrieve remote service addr from parameter set
			// mac.top.<taskname>.<name>.remoteservice
			string remoteServiceNameParam = 
								formatString(PARAM_SERVER_SERVICE_NAME,
								_pTask->getName().c_str(), _name.c_str());
			string remoteAddr;
			if (globalParameterSet()->isDefined(remoteServiceNameParam)) {
				remoteAddr = globalParameterSet()->getString(remoteServiceNameParam);
				// format is: "<taskname>:<portname>"
				string::size_type 	colon = remoteAddr.find(":",0);
				if (colon != string::npos) {
					addr.taskname   = remoteAddr.substr(0,colon);
					addr.portname   = remoteAddr.substr(colon+1);
					_deviceNameMask = remoteAddr;
				}
			}
		}

		// found addr information?
		if (addr.taskname.length() > 0 && addr.portname.length() > 0) {
			// now the protType of the remote service port can be 
			// retrieved from the parameter set
			// try mac.ns.<taskname>.<name>.type in parameterSet
			typeParam = formatString (PARAM_PORT_PROT_TYPE,
							addr.taskname.c_str(), addr.portname.c_str());
			if (globalParameterSet()->isDefined(typeParam)) {
				protType = globalParameterSet()->getString(typeParam);
			}
		}
	}

	// When protType is not set after this effort use TCP.
	if (protType.length() <= 1) {
		LOG_DEBUG(formatString ("No port info for port '%s', using TCP", 
								makeServiceName().c_str()));
		protType = "TCP";
	}

	// 'protType' is now set, check for the various port types
	if (protType == "TCP") {
		GCFTCPPort* 	pNewPort(0);
//		if (usesModernServiceName()) {
			pNewPort = new GCFTCPPort(*_pTask, makeServiceName(), _type, 
									  _protocol, _transportRawData);
//		}
//		else {
//			pNewPort = new GCFTCPPort(*_pTask, _name+"_TCP", _type, 
//									  _protocol, _transportRawData);
//		}

		if (_remotetask.length() > 0 && _remoteport.length() > 0) {
			addr.taskname = _remotetask;
			addr.portname = _remoteport;
			pNewPort->setAddr(addr);    
		}
		pNewPort->setMaster(this);    
		pNewPort->setInstanceNr(getInstanceNr());

		_pSlave = pNewPort;
	}
	else if (protType == "ETH") {
		GCFETHRawPort* 	pNewPort(0);
		string 			pseudoName = _name + "_ETH";
		pNewPort = new GCFETHRawPort(*_pTask, pseudoName, _type, _transportRawData);
		pNewPort->setMaster(this);    

		_pSlave = pNewPort;
	}
	else if (protType == "DEV") {
		GCFDevicePort* 	pNewPort(0);
		string 			pseudoName = _name + "_DEV";
		pNewPort = new GCFDevicePort(*_pTask, pseudoName, 0, "", _transportRawData);
		pNewPort->setMaster(this);    

		_pSlave = pNewPort;
	}
	else  {
		ASSERTSTR(false, "No implementation found for port protocol type '" <<
						protType << "'. Only TCP, DEV and ETH are supported!");
		return (false);
	}

	return (_pSlave->open());
}

//
// close()
//
bool GCFPort::close()
{
	_state = S_CLOSING;
	if (!_pSlave) {
		LOG_DEBUG("Port was not opened!!!");
		return (false);
	}

	return (_pSlave->close());
}

//
// setRemoteAddr(task, port)
//
bool GCFPort::setRemoteAddr(const string& remotetask, const string& remoteport)
{
	if (_type == SAP) {
		_remotetask = remotetask;
		_remoteport = remoteport;
		return (_remotetask.length() > 0 && _remoteport.length() > 0);
	}
	return false;
}

//
// send(event)
//
ssize_t GCFPort::send(GCFEvent& e)
{
	if (!_pSlave || !isConnected()) {
		LOG_ERROR(formatString ("Port '%s' of task '%s' not connected! Event not sent!",
								makeServiceName().c_str(), getTask()->getName().c_str()));
		return (0);
	}

	// 170507: This IN/OUT stuff is not very handy in the control-chains.
	//		   It is also not neccesary to check this because signals will always
	//		   be catched in switches. It is much more usefull to use these two bits
	//		   for marking whether a message is an order/request or an acknowlegdement.
	//		   For now these checks are disabled.
#if 0
	if (SPP == _type) {
		if (!(F_EVT_INOUT(e) & F_OUT)) {
			LOG_ERROR(formatString ("Trying to send IN event '%s' on SPP "
									"port '%s'; discarding this event.",
							getTask()->eventName(e), makeServiceName().c_str()));

			return (-1);
		}
	}
	else if (SAP == _type) {
		if (!(F_EVT_INOUT(e) & F_IN)) {
			LOG_ERROR(formatString ("Trying to send OUT event '%s' on SAP "
									"port '%s'; discarding this event.",
							getTask()->eventName(e), makeServiceName().c_str()));

			return (-1);
		}
	}
	else if (MSPP == _type) {
		LOG_ERROR(formatString ("Trying to send event '%s' by means of "
								"the portprovider: %s (MSPP). "
								"Not supported yet",
							getTask()->eventName(e), makeServiceName().c_str()));

		return (-1);
	}
#endif

	return (_pSlave->send(e));
}

//
// recv (buf, count)
//
ssize_t GCFPort::recv(void* buf, size_t count)
{
	if (!_pSlave) {
		return (-1);
	}

	return (_pSlave->recv(buf, count));
}

//
// setTimer(sec, usec, itvsec, itvusec, arg)
//
long GCFPort::setTimer(long delay_sec,    long delay_usec,
					   long interval_sec, long interval_usec,
					   void* arg)
{
	if (!_pSlave) {
		return (-1);
	}

	return (_pSlave->setTimer(delay_sec, delay_usec, 
							  interval_sec, interval_usec, arg));
}

//
// setTimer(seconds, interval, args)
//
long GCFPort::setTimer(double delay_seconds, 
					   double interval_seconds,
					   void* arg)
{
	if (!_pSlave) {
		return (-1);
	}

	return (_pSlave->setTimer(delay_seconds, interval_seconds, arg));
}

//
// cancelTimer(timerid, arg)
//
int GCFPort::cancelTimer(long timerid, void **arg)
{
	if (!_pSlave) {
		return (-1);
	}

	return (_pSlave->cancelTimer(timerid, arg));
}

//
// cancelAllTimers
//
int GCFPort::cancelAllTimers()
{
	if (!_pSlave) {
		return (-1);
	}

	return (_pSlave->cancelAllTimers());
}

//
// timeLeft(timerid)
//
double GCFPort::timeLeft(long timerID)
{
	return (_pSlave ? _pSlave->timeLeft(timerID) : 0);
}

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
