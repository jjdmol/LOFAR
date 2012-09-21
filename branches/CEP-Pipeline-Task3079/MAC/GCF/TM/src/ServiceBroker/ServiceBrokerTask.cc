//#  ServiceBrokerTask.cc: 
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
#include <Common/StringUtil.h>
#include <Common/SystemUtil.h>

#include <MACIO/SB_Protocol.ph>
#include <GTM_Defines.h>
#include "ServiceBrokerTask.h"
#include <unistd.h>

namespace LOFAR {
 namespace GCF {
  using namespace TM;
  namespace SB {

//
// Initialize static elements
static string sSBTaskName("GCF-SB");
GTMSBHandler* GTMSBHandler::_pInstance = 0;

//
// GTMSBHandler()
//
GTMSBHandler::GTMSBHandler()
{
}

//
// ServiceBrokerTask()
//
ServiceBrokerTask::ServiceBrokerTask() :
	GCFTask((State)&ServiceBrokerTask::operational, sSBTaskName),
	itsSeqnr		   (0),
//	itsMaxResponse (15),
	itsMaxConnectTime  (1),
	itsMaxResponseTime (5),
	itsTimerPort       (*this, "timerport")
{
	// register the protocol for debugging purposes
	registerProtocol(SB_PROTOCOL, SB_PROTOCOL_STRINGS);
}

//
// ~ServiceBrokerTask()
//
ServiceBrokerTask::~ServiceBrokerTask()
{
}

//
// instance(temp)
//
ServiceBrokerTask* ServiceBrokerTask::instance(bool temporary)
{
	if (!GTMSBHandler::_pInstance) {    
		GTMSBHandler::_pInstance = new GTMSBHandler();
		ASSERT(!GTMSBHandler::_pInstance->mayDeleted());
		GTMSBHandler::_pInstance->_controller.start();
	}

	if (!temporary) { 
		GTMSBHandler::_pInstance->use();
	}

	return (&GTMSBHandler::_pInstance->_controller);
}

//
// release()
//
void ServiceBrokerTask::release()
{
	ASSERT(GTMSBHandler::_pInstance);
	ASSERT(!GTMSBHandler::_pInstance->mayDeleted());
	GTMSBHandler::_pInstance->leave(); 
	if (GTMSBHandler::_pInstance->mayDeleted()) {
		delete GTMSBHandler::_pInstance;
		ASSERT(!GTMSBHandler::_pInstance);
	}
}

// -------------------- USER FUNCTIONS --------------------
//
// registerService(servicePort)
//
void ServiceBrokerTask::registerService(GCFTCPPort& servicePort)
{
	string	servicename = servicePort.makeServiceName();

	SBRegisterServiceEvent request;

	Action action;
	action.type			= request.signal;
	action.pPort		= &servicePort;
	action.servicename	= servicename;
	action.hostname		= myHostname(false);	// always local!
	action.timestamp	= time(0);

	request.seqnr		= _registerAction(action);
	request.servicename = servicename;

	BMiter	serviceBroker = _getBroker(action.hostname);
	if (serviceBroker->second.port->isConnected()) {
		LOG_DEBUG_STR("Sending REGISTER(" << request.servicename << "@" << action.hostname << 
					  "(" << request.seqnr << "))");
		serviceBroker->second.port->send(request);	// will result in SB_SERVICE_REGISTERED
	}
	else {
		LOG_DEBUG_STR("Holding REGISTER(" << request.servicename << "@" << action.hostname << 
					  "(" << request.seqnr << ")) until connection with SB is made");
	}
}

//
// unregisterService(servicePort)
//
void ServiceBrokerTask::unregisterService(GCFTCPPort& servicePort)
{
	string	servicename = servicePort.makeServiceName();
	SBUnregisterServiceEvent request;

	Action action;
	action.type			= request.signal;
	action.pPort		= &servicePort;
	action.servicename	= servicename;
	action.hostname		= myHostname(false);	// always local!
	action.timestamp	= time(0);

	request.seqnr		= _registerAction(action);
	request.servicename = servicename;

	BMiter	serviceBroker = _getBroker(servicePort.getHostName());
	if (serviceBroker->second.port->isConnected()) {
		LOG_DEBUG_STR("Sending UNREGISTER(" << request.servicename << "@" << servicePort.getHostName() << 
					  "(" << request.seqnr << "))");
		serviceBroker->second.port->send(request);	// will result in SB_SERVICE_UNREGISTERED
	}
	else {
		LOG_DEBUG_STR("Holding UNREGISTER(" << request.servicename << "@" << servicePort.getHostName() << 
					  "(" << request.seqnr << ")) until connection with SB is made");
	}
}

//
// getServiceinfo(clientPort, remoteServiceName);
//
void ServiceBrokerTask::getServiceinfo(GCFTCPPort& 	clientPort, 
									  const string& remoteServiceName,
									  const string&	hostname)
{  
	SBGetServiceinfoEvent request;

	Action action;
	action.type			= request.signal;
	action.pPort		= &clientPort;
	action.servicename  = remoteServiceName;
	action.hostname		= hostname;
	action.timestamp	= time(0);

	request.seqnr		= _registerAction(action);
	request.servicename = remoteServiceName;
	request.hostname    = hostname;

	BMiter	serviceBroker = _getBroker(action.hostname);
	if (serviceBroker->second.port->isConnected()) {
		LOG_DEBUG_STR("Sending SERVICEINFO(" << request.servicename << "@" << request.hostname << 
					  "(" << request.seqnr << "))");
		serviceBroker->second.port->send(request);	// will result in SB_SERVICE_INFO
	}
	else {
		LOG_DEBUG_STR("Holding SERVICEINFO(" << request.servicename << "@" << request.hostname << 
					  "(" << request.seqnr << ")) until connection with SB is made");
	}
}

//
// deletePort(port)
//
void ServiceBrokerTask::deletePort(GCFTCPPort& aPort)
{
	// clean up all action that refer to this port
    actionList_t 		tmpActionList;    	// NOTE: 'erase' reorders the elements of a list!!!
	tmpActionList.swap(itsActionList);
	ALiter		end  = tmpActionList.end();
	ALiter		iter = tmpActionList.begin();
	while (iter != end) {
		LOG_TRACE_COND_STR("deletePort checking: " << iter->print());
		if (iter->pPort != &aPort) {			// copy others only
			itsActionList.push_back(*iter);
		}
		iter++;
	}
	tmpActionList.clear();

	// unregister service of this port if any
	_deleteService(aPort);
}

// -------------------- INTERNAL FUNCTIONS --------------------

//
// _deleteService(aClientPort)
//
void ServiceBrokerTask::_deleteService(GCFTCPPort&	aPort)
{
	// its there a service registered at this port?
	SMiter	service = itsServiceMap.find(&aPort);
	if (service != itsServiceMap.end()) {
		unregisterService(aPort);
		itsServiceMap.erase(service);
	}
}

//
// _actionName(type)
//
string ServiceBrokerTask::_actionName(uint16		type) const
{
	switch (type) {
		case SB_REGISTER_SERVICE:	 return ("RegisterService");
		case SB_UNREGISTER_SERVICE:	 return ("UnregisterService");
		case SB_GET_SERVICEINFO:	 return ("GetServiceInfo");
		default:					 return (formatString("%d???", type));
	}
}

//
// _logResult
//
void ServiceBrokerTask::_logResult(uint16	 	result, 
								  const string& servicename, 
								  const string& hostname) const
{
	switch (result) {
		case SB_NO_ERROR:
		break;

		case SB_UNKNOWN_ERROR:
			LOG_FATAL("Unknown error");
		break;

		case SB_SERVICE_ALREADY_EXIST:
			LOG_ERROR_STR("Service " << servicename << " already exist");
		break;

		case SB_NO_FREE_PORTNR:
			LOG_ERROR_STR("No free portnumber for service: " << servicename);
		break;

		case SB_UNKNOWN_SERVICE:
			LOG_WARN_STR("Unknown remote service: "<< servicename << "@" << hostname);
		break;

		case SB_NO_CONNECTION:
			LOG_ERROR_STR("No connection with serviceBroker at " << hostname);
			break;

		case SB_CANT_RECOVER:
			LOG_ERROR_STR("Unable to recover service " << servicename << 
							" at serviceBroker, no new connections can't be made.");
			break;

		default:
		break;
	}
}

//
// _registerAction(action)
//
unsigned short ServiceBrokerTask::_registerAction(Action action)
{
	action.seqnr = ++itsSeqnr;
	itsActionList.push_back(action);
	LOG_TRACE_COND_STR("RegisterAction: " << action.print());
	return (itsSeqnr);
}

//
// _reRegisterServices(hostname)
//
void ServiceBrokerTask::_reRegisterServices(GCFPortInterface*	brokerPort)
{
	// nothing to do?
	if (itsServiceMap.empty()) {
		return;
	}

	SMiter	end  = itsServiceMap.end();
	SMiter	iter = itsServiceMap.begin();
	while (iter != end) {
		SBReregisterServiceEvent	request;
		request.seqnr 	    = 0;
		request.servicename = iter->second.servicename;
		request.portnumber  = iter->second.portNr;
		brokerPort->send(request);

		iter++;
	}
}

//
// _doActionList(hostname)
//
void ServiceBrokerTask::_doActionList(const string&	hostname)
{
	// nothing to do?
	if (itsActionList.empty()) {
		return;
	}
	_printActionList();

	// Note: while processing the list, the list grows. Therefore we use actionsLeft.
    actionList_t 		tmpActionList;    	// NOTE: 'erase' reorders the elements of a list!!!
	tmpActionList.swap(itsActionList);
	ALiter		end  = tmpActionList.end();
	ALiter		iter = tmpActionList.begin();
	while (iter != end) {
		LOG_TRACE_COND_STR("doActionList checking: " << iter->print());
		// only process the actions for this host
		if (iter->hostname != hostname) {
			itsActionList.push_back(*iter);		// restore in original list.
			iter++;
			continue;
		}

		// its an action for this host, process it. action is added to the list again
		switch (iter->type) {
		case SB_REGISTER_SERVICE: 
			registerService(*(iter->pPort)); 
			break;
		case SB_UNREGISTER_SERVICE: 
			unregisterService(*(iter->pPort)); 
			break;
		case SB_GET_SERVICEINFO: 
			getServiceinfo(*(iter->pPort), iter->servicename, iter->hostname); 
			break;
		default: 
			ASSERTSTR(false, "Unknown action in actionlist: " << iter->type
						<< ":" << iter->servicename << "@" << iter->hostname);
		}
		iter++;
	}
	tmpActionList.clear();
}

//
// _lostBroker(hostname)
//
void ServiceBrokerTask::_lostBroker(const string& hostname)
{
	// nothing to do?
	if (itsActionList.empty()) {
		return;
	}

	// Note: while processing the list, the list grows. Therefore we use actionsLeft.
    actionList_t 		tmpActionList;    	// NOTE: 'erase' reorders the elements of a list!!!
	tmpActionList.swap(itsActionList);
	ALiter		end  = tmpActionList.end();
	ALiter		iter = tmpActionList.begin();
	while (iter != end) {
		LOG_TRACE_COND_STR("_lostBroker checking: " << iter->print());
		// only process the actions for this host
		if (iter->hostname != hostname) {
			itsActionList.push_back(*iter);		// restore in original list.
			iter++;
			continue;
		}

		// its an action for this host, process it. action is added to the list again
		switch (iter->type) {
		case SB_REGISTER_SERVICE: {
			SBServiceRegisteredEvent response;
			_logResult(SB_NO_CONNECTION, iter->servicename, iter->hostname);
			iter->pPort->serviceRegistered(SB_NO_CONNECTION, 0);
			break;
		}
		case SB_UNREGISTER_SERVICE: {
			SMiter		service = itsServiceMap.find(iter->pPort);
			if (service != itsServiceMap.end()) {
				itsServiceMap.erase(service);
			}
			break;
		}
		case SB_GET_SERVICEINFO: {
			SBServiceInfoEvent response;
			_logResult(SB_NO_CONNECTION, iter->servicename, iter->hostname);
			// pass response to waiting client
			iter->pPort->serviceInfo(SB_NO_CONNECTION, 0, iter->hostname);
			break;
		}
		default: 
			ASSERTSTR(false, "Unknown action in actionlist: " << iter->type
						<< ":" << iter->servicename << "@" << iter->hostname);
		}
		iter++;
	}
	tmpActionList.clear();
}

//
// _getBroker(hostname)
//
ServiceBrokerTask::BMiter	ServiceBrokerTask::_getBroker(const string&	hostname)
{
	// do we have a connection to this broker already?
	BMiter	serviceBroker = itsBrokerMap.find(hostname);
	if (serviceBroker != itsBrokerMap.end()) {
		return (serviceBroker);			// return pointer to broker.
	}

	// broker to this system not found, create a port to this host.
	GTMSBTCPPort*	aBrokerPort = new GTMSBTCPPort(*this, hostname, GCFPortInterface::SAP, SB_PROTOCOL);
	ASSERTSTR(aBrokerPort, "Unable to allocate a socket to ServiceBroker@" << hostname);

	// Add broker to collection and open connection
	aBrokerPort->setHostName(hostname);
	itsBrokerMap[hostname] = BrokerInfo(aBrokerPort, MAX_RECONNECT_RETRIES);
	itsBrokerMap[hostname].port->open();		// Results in F_CONN or F_DISCO

	return (itsBrokerMap.find(hostname));
}

//
// _findAction(seqnr)
//
ServiceBrokerTask::ALiter	ServiceBrokerTask::_findAction(uint16	seqnr)
{
	ALiter	end  = itsActionList.end();
	ALiter	iter = itsActionList.begin();
	while (iter != end) {
		LOG_TRACE_COND_STR("_findAction checking: " << iter->print());
		if (iter->seqnr == seqnr) {
			return (iter);
		}

		iter++;
	}
	return (iter);
}

//
// _reconnectBrokers()
//
void ServiceBrokerTask::_reconnectBrokers()
{
	LOG_DEBUG("_reconnectBrokers()");

	BMiter	end  = itsBrokerMap.end();
	BMiter	iter = itsBrokerMap.begin();

	while (iter != end) {
		// ready when broker is connected
		if (iter->second.port->isConnected()) {
			iter++;
			continue;
		}

		// broker is not connected, may we retry?
		if (--(iter->second.nRetries) > 0) {
			iter->second.port->open();			// will result in F_CONN or F_DISCONN
			_checkActionList(iter->first);
			iter++;
		}
		else {
			LOG_ERROR_STR("ServiceBroker on host " << iter->first << " is unreachable!");
			_lostBroker(iter->first);
			BMiter	tmp = iter;
			iter++;
			// remove broker except when its the SB on my host and some services were registered there.
			if (tmp->first == myHostname(false) && !itsServiceMap.empty()) {
				tmp->second.nRetries = MAX_RECONNECT_RETRIES;	// keep trying.
				tmp->second.port->open();						// might result in F_CONN or F_DISCONN
			}
			else {
				LOG_DEBUG_STR("Removing servicebroker for " << iter->first << " from brokermap");
				itsBrokerMap.erase(tmp);
			}
		}
	}
}

//
// _printActionList
//
void ServiceBrokerTask::_printActionList()
{
	if (itsActionList.empty()) {
		return;
	}

	ALiter	end  = itsActionList.end();
	ALiter	iter = itsActionList.begin();
	string	typeName;
	LOG_TRACE_FLOW_STR("ActionList at " << time(0));
	while (iter != end) {
		switch (iter->type) {
		case SB_REGISTER_SERVICE:	typeName = "Register";		break;
		case SB_UNREGISTER_SERVICE: typeName = "Unregister";	break;
		case SB_GET_SERVICEINFO: 	typeName = "ServiceInfo";	break;
		default:					typeName = "???";			break;
		}

		LOG_DEBUG_STR(typeName << " " << iter->servicename << "@" << iter->hostname << 
						"(" << iter->timestamp << "(" << iter->seqnr << "))");
		++iter;
	}
}

//
// _checkActionList(hostname);
//
void ServiceBrokerTask::_checkActionList(const string&	hostname)
{
	LOG_DEBUG_STR("_checkActionList(" << hostname <<")");
	_printActionList();

    actionList_t 		tmpActionList;    	// NOTE: 'erase' reorders the elements of a list!!!
	tmpActionList.swap(itsActionList);
	ALiter		end  = tmpActionList.end();
	ALiter		iter = tmpActionList.begin();
	time_t	currentTime = time(0);
	// check for which actions we are late.
	while (iter != end) {
		LOG_TRACE_COND_STR("_checkActionList checking: " << iter->print());
		bool	actionHandled(false);
		if (iter->hostname == hostname) {
			switch (iter->type) {
			case SB_REGISTER_SERVICE: // pass response to waiting client
				if (currentTime > iter->timestamp+itsMaxConnectTime) {
					_logResult(SB_NO_CONNECTION, iter->servicename, iter->hostname);
					iter->pPort->serviceRegistered(SB_NO_CONNECTION, 0);
					actionHandled = true;
				}
				break;
			case SB_GET_SERVICEINFO: 
				if (currentTime > iter->timestamp+itsMaxResponseTime) {
					_logResult(SB_NO_CONNECTION, iter->servicename, iter->hostname);
					iter->pPort->serviceInfo(SB_NO_CONNECTION, 0, "");
					actionHandled = true;
				}
				break;
			case SB_UNREGISTER_SERVICE:
				if (currentTime > iter->timestamp+itsMaxConnectTime) {
					actionHandled = true;
				}
				break;
			default: 
				ASSERTSTR(false, "Unknown action in actionlist: " << iter->type);
			}
		}
		if (!actionHandled) {
			itsActionList.push_back(*iter);	// keep non-expired action
		}
		iter++;
	}
	tmpActionList.clear();
}

//
// operational(event, port)
//
GCFEvent::TResult ServiceBrokerTask::operational(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("operational:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	switch (event.signal) {
	case F_INIT:
	case F_ENTRY:
		break;

	case F_CONNECTED:
		// we succeeded in making a (re)connection to a serviceBroker.
		// register old services and handle waiting actions if any
		if(static_cast<GTMSBTCPPort*>(&port)->getHostName() == myHostname(false)) {
			_reRegisterServices(&port);
		}
		_doActionList(static_cast<GTMSBTCPPort*>(&port)->getHostName());
		break;

	case F_DISCONNECTED:
		// lost a connection with a service broker
		// close port and remove waiting actions.
		LOG_DEBUG_STR("Connection lost with service broker at " <<
									static_cast<GTMSBTCPPort*>(&port)->getHostName());
		port.close();
		// start reconnect sequence
		itsTimerPort.setTimer(1.0);
		break;

	case F_TIMER:
		// reconnect timer expired, try to reconnect
		_reconnectBrokers();	// reopen not-connected broker ports
		break;

	case SB_SERVICE_REGISTERED: {
		SBServiceRegisteredEvent response(event);
		ALiter		action = _findAction(response.seqnr);
		if (action != itsActionList.end()) {
			_logResult(response.result, action->servicename, action->hostname);
			// remember we registered this service (for reregistering)
			if (response.result == SB_NO_ERROR) {
				itsServiceMap[action->pPort] = KnownService(action->servicename, response.portnumber);
			}
			// pass response to waiting client
			action->pPort->serviceRegistered(response.result, response.portnumber);
			itsActionList.erase(action);
		}
		break;
	}

	case SB_SERVICE_INFO: {
		SBServiceInfoEvent response(event);
		ALiter		action = _findAction(response.seqnr);
		if (action != itsActionList.end()) {
			_logResult(response.result, action->servicename, action->hostname);
			// pass response to waiting client
			action->pPort->serviceInfo(response.result, response.portnumber, 
															response.hostname);
			itsActionList.erase(action);
		}
		break;
	}

	case SB_SERVICE_UNREGISTERED: {
		SBServiceUnregisteredEvent		response(event);
		// remove from 'registered services' map.
		ALiter		action = _findAction(response.seqnr);
		if (action != itsActionList.end()) {
			SMiter		service = itsServiceMap.find(action->pPort);
			if (service != itsServiceMap.end()) {
				itsServiceMap.erase(service);
			}
			itsActionList.erase(action);
		}
		break;
	}

	case SB_SERVICE_REREGISTERED: {
		SBServiceReregisteredEvent		response(event);
		_logResult(response.result, response.servicename, "");
		// Note: action was not in the action list.
		break;
	}

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}


  } // namespace SB
 } // namespace GCF
} // namespace LOFAR
