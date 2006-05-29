//#  GSB_Controller.cc: 
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

#include <APS/ParameterSet.h>
#include <GCF/Utils.h>
#include <GCF/GCF_ServiceInfo.h>
#include <../SB_Protocol.ph>
#include <GSB_Defines.h>
#include "GSB_Controller.h"

namespace LOFAR {
 namespace GCF {
  using namespace TM;
  namespace SB {

static string sSBTaskName("ServiceBrokerTask");

//
// xxx
//
GSBController::GSBController() : 
	GCFTask((State)&GSBController::initial, sSBTaskName)
{
	// register the protocol for debugging purposes
	registerProtocol(SB_PROTOCOL, SB_PROTOCOL_signalnames);

	// allocate the listener port
	itsListener.init(*this, MAC_SVCMASK_SERVICEBROKER, 
								GCFPortInterface::MSPP, SB_PROTOCOL);

	// read the port range I may use
	readRanges();
}

//
// xxx
//
GSBController::~GSBController()
{
  LOG_DEBUG("Deleting ServiceBroker");
}

//
// initial(event, port)
//
GCFEvent::TResult GSBController::initial(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY:
	case F_TIMER:
		itsListener.open();		// Open Listener port
		break;

	case F_CONNECTED:
		// Once listener is opened I am operational
		TRAN(GSBController::operational);
		break;

	case F_DISCONNECTED:
		if (&port == &itsListener)
			itsListener.setTimer(1.0); // try again after 1 second
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

//
// operational(event, port)
//
GCFEvent::TResult GSBController::operational(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_CONNECTED:   
		break;

	case F_ACCEPT_REQ:
		acceptConnectRequest();
		break;

	case F_DISCONNECTED:      
		if (&port != &itsListener) {
			port.close();
		}
		// else //TODO: find out this can realy happend
		break;

	case F_CLOSED:
		releasePort(&port);
		break;

	case SB_REGISTER_SERVICE: {
		SBRegisterServiceEvent 		request(event);
		SBServiceRegisteredEvent	response;
		response.seqnr = request.seqnr;

		// Is Service already registered?
		if (findService(request.servicename)) {
			LOG_ERROR(formatString("Service %s already exist", 
									request.servicename.c_str()));
			response.result = SB_SERVICE_ALREADY_EXIST;
		}
		else {
			uint16	portNr = claimPortNumber(request.servicename, &port);
			if (portNr > 0) {
				response.result 	= SB_NO_ERROR;
				response.portnumber = portNr;
			}
			else {
				LOG_ERROR(formatString ("All available port numbers are claimed (%s)", 
										request.servicename.c_str()));
				response.result = SB_NO_FREE_PORTNR;
			}
		}
		port.send(response);
		break;
	}

	case SB_UNREGISTER_SERVICE: {
		SBUnregisterServiceEvent 	request(event);
		releaseService(request.servicename);
		break;
	}

	case SB_GET_SERVICEINFO: {
		SBGetServiceinfoEvent 	request(event);
		SBServiceInfoEvent 		response;
		response.seqnr = request.seqnr;

		uint16	portNr = findService(request.servicename);
		if (portNr) {
			LOG_INFO(formatString ("Serviceinfo for %s is %d", 
									request.servicename.c_str(), portNr));
			response.portnumber = portNr;
			response.hostname	= Common::myHostname();
			response.result 	= SB_NO_ERROR;
		}
		else {
			LOG_ERROR(formatString ("Unknown service: %s", 
									request.servicename.c_str()));
			response.result = SB_UNKNOWN_SERVICE;        
		}
		port.send(response);
		break;
	}

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// acceptConnectRequest
//
void GSBController::acceptConnectRequest()
{
	LOG_DEBUG ("A new SB client tries to connect. Accept!!!");

	GTMSBTCPPort* 	pNewSBClientPort = new GTMSBTCPPort();
	ASSERT(pNewSBClientPort);

	pNewSBClientPort->init(*this, "newClient", GCFPortInterface::SPP, SB_PROTOCOL);
	itsListener.accept(*pNewSBClientPort);
}

//
// readRanges
//
void GSBController::readRanges()
{
	ASSERTSTR (ACC::APS::globalParameterSet()->isDefined("firstPortNumber") && 
	    	   ACC::APS::globalParameterSet()->isDefined("lastPortNumber"), 
				"Ranges not specified in ParameterSet");

	itsLowerLimit = ACC::APS::globalParameterSet()->getUint16("firstPortNumber");
	itsUpperLimit = ACC::APS::globalParameterSet()->getUint16("lastPortNumber");

	ASSERTSTR(itsLowerLimit < itsUpperLimit, "Invalid portnumber range specified");
	ASSERTSTR(itsLowerLimit > 1023, "Portnumbers below 1024 may not be used");

	itsNrPorts = itsUpperLimit - itsLowerLimit;
	ASSERTSTR(itsNrPorts < 1000, 
							"Range too large, broker can manage only 1000 ports");

	itsServiceList = vector<TServiceInfo> (itsNrPorts);
	for (uint32 idx = 0; idx < itsNrPorts; idx++) {
		itsServiceList[idx].portNumber  = 0;
		itsServiceList[idx].serviceName = "";
		itsServiceList[idx].ownerPort   = 0;
	}
	
	itsNrFreePorts = itsNrPorts;

	LOG_INFO (formatString("Managing portnumbers %d till %d (%d)", 
							itsLowerLimit, itsUpperLimit, itsNrPorts));
}

//
// claimPortNumber(serviceName, port)
//
uint16 GSBController::claimPortNumber(const string& 		aServiceName,
									  GCFPortInterface*		aPort)
{
	int32	idx = 0;

	if (!itsNrFreePorts) {				// must have room.
		return (0);
	}

	while (idx < itsNrPorts) {			// scan whole array
		if (itsServiceList[idx].portNumber == 0) {
			itsServiceList[idx].portNumber  = itsLowerLimit + idx;
			itsServiceList[idx].serviceName = aServiceName;
			itsServiceList[idx].ownerPort   = aPort;
			itsNrFreePorts--;
			LOG_INFO(formatString ("Portnumber %d assigned to '%s'.", 
								itsServiceList[idx].portNumber, aServiceName.c_str()));
			LOG_INFO_STR ("Managing " << itsNrPorts - itsNrFreePorts << " ports now");
			return (itsServiceList[idx].portNumber);
		}
		idx++;
	}

	ASSERTSTR (false, "Major programming error in 'claimPortNumber'!");
}

//
// releaseService(servicename)
//
void GSBController::releaseService(const string& 		aServiceName)
{
	if (itsNrFreePorts == itsNrPorts) {	// ready when nothing was claimed.
		return;
	}

	uint16	portNr = findService(aServiceName);
	if (!portNr) {						// unknown service?
		return;
	}

	int32 	idx = portNr - itsLowerLimit;		// convert portnr to array index

	LOG_INFO(formatString("Service %s(%d) unregistered", aServiceName.c_str(), portNr));
	
	itsServiceList[idx].portNumber  = 0;
	itsServiceList[idx].serviceName = "";
	itsServiceList[idx].ownerPort   = 0;
	itsNrFreePorts++;

	LOG_INFO_STR ("Still managing " << itsNrPorts - itsNrFreePorts << " ports");
}

//
// releasePort(port)
//
void GSBController::releasePort(GCFPortInterface*	aPort)
{
	if (!aPort) {						// check args
		return;
	}

	int32	idx = 0;
	int32	nrElems2Check = itsNrPorts - itsNrFreePorts;// prevent checking whole array

	while (idx < itsNrPorts && nrElems2Check > 0) {
		if (itsServiceList[idx].portNumber) {			// used entry?
			nrElems2Check--;
			if (itsServiceList[idx].ownerPort == aPort) {
				LOG_INFO(formatString("Service %s (%d) unregistered", 
							itsServiceList[idx].serviceName.c_str(), 
							itsServiceList[idx].portNumber));
				itsServiceList[idx].portNumber  = 0;
				itsServiceList[idx].serviceName = "";
				itsServiceList[idx].ownerPort   = 0;
				itsNrFreePorts++;
			}
		}
		idx++;
	}
	LOG_INFO_STR ("Still managing " << itsNrPorts - itsNrFreePorts << " ports");
}

//
// findService(serviceName)
//
uint16 GSBController::findService(const string& aServiceName)
{
	int32	idx = 0;
	int32	nrElems2Check = itsNrPorts - itsNrFreePorts;// prevent checking whole array

	while (idx < itsNrPorts && nrElems2Check > 0) {
		if (itsServiceList[idx].portNumber) {
			nrElems2Check--;
			if (itsServiceList[idx].serviceName == aServiceName) {
				return (itsServiceList[idx].portNumber);
			}
		}
		idx++;
	}
		
	return (0);
}

  } // namespace SB
 } // namespace GCF
} // namespace LOFAR
