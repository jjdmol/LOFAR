//#  ServiceBroker.cc: 
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
#include <Common/lofar_fstream.h>
#include <Common/Version.h>

#include <Common/ParameterSet.h>
#include <MACIO/MACServiceInfo.h>
#include <MACIO/SB_Protocol.ph>
#include <MACIO/GCF_Event.h>
//#include <GCF/TM/GCF_Protocols.h>
#include "ServiceBroker.h"
#include <CUDaemons/Package__Version.h>

namespace LOFAR {
    using namespace MACIO;
    using namespace GCF::TM;
	namespace CUDaemons {

static string sSBTaskName("ServiceBroker");

//
// xxx
//
ServiceBroker::ServiceBroker() : 
	GCFTask((State)&ServiceBroker::initial, sSBTaskName),
	itsServiceList(),
	itsListener(),
	itsAdminFile(),
	itsLowerLimit(0),
	itsUpperLimit(0),
	itsNrPorts(0),
	itsNrFreePorts(0)
{
	LOG_INFO(Version::getInfo<CUDaemonsVersion>("ServiceBroker"));

	// register the protocol for debugging purposes
	registerProtocol(SB_PROTOCOL, SB_PROTOCOL_STRINGS);

	// allocate the listener port
	itsListener.init(*this, MAC_SVCMASK_SERVICEBROKER, GCFPortInterface::MSPP, SB_PROTOCOL);
	itsListener.setPortNumber(MAC_SERVICEBROKER_PORT);

	// read the port range I may use
	readRanges();

	// load previous registration if any
	loadAdministration (itsAdminFile);
}

//
// xxx
//
ServiceBroker::~ServiceBroker()
{
  LOG_DEBUG("Deleting ServiceBroker");
}

//
// initial(event, port)
//
GCFEvent::TResult ServiceBroker::initial(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY:
		ASSERTSTR(itsListener.open(), "Cannot open listener. Another ServiceBroker already running?");
		break;

	case F_CONNECTED:
		// Once listener is opened I am operational
		TRAN(ServiceBroker::operational);
		break;

	case F_DISCONNECTED:
		if (&port == &itsListener)
			ASSERTSTR(false, "Cannot open listenerport, another ServiceBroker is running?");
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
GCFEvent::TResult ServiceBroker::operational(GCFEvent& event, GCFPortInterface& port)
{
	LOG_TRACE_FLOW_STR ("operation:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY:
		itsListener.setTimer(300.0);		// to cleanup not-reclaimed portnumbers
		break;

	case F_TIMER:
		cleanupOldRegistrations();
		break;

	case F_CONNECTED:
		break;

	case F_ACCEPT_REQ:
		acceptConnectRequest();
		break;

	case F_DISCONNECTED:      
		if (&port != &itsListener) {
			port.close();
			releasePort(&port);
		}
		// else //TODO: find out this can realy happend
		break;

	case SB_REGISTER_SERVICE: {
		SBRegisterServiceEvent 		request(event);
		SBServiceRegisteredEvent	response;
		response.seqnr = request.seqnr;

		// Is Service already registered?
		if (findService(request.servicename, true)) {
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
		SBServiceUnregisteredEvent	response;
		response.seqnr = request.seqnr;
		
		releaseService(request.servicename);

		response.result = SB_NO_ERROR;
		port.send(response);
		break;
	}

	case SB_GET_SERVICEINFO: {
		SBGetServiceinfoEvent 	request(event);
		SBServiceInfoEvent 		response;
		response.seqnr = request.seqnr;

		uint16	portNr = findService(request.servicename, true);
		if (portNr) {
			LOG_INFO(formatString ("Serviceinfo for %s is %d", 
									request.servicename.c_str(), portNr));
			response.portnumber = portNr;
			response.hostname	= myHostname(false);
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

	case SB_REREGISTER_SERVICE: {
		SBReregisterServiceEvent 	request(event);
		SBServiceReregisteredEvent	response;
		response.seqnr  	 = request.seqnr;
		response.servicename = request.servicename;
		response.result 	 = reRegisterService(request.servicename, request.portnumber, &port);
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
void ServiceBroker::acceptConnectRequest()
{
	LOG_DEBUG ("A new SB client tries to connect. Accept!!!");

	GCFTCPPort* 	pNewSBClientPort = new GCFTCPPort();
	ASSERT(pNewSBClientPort);

	pNewSBClientPort->init(*this, "newClient", GCFPortInterface::SPP, SB_PROTOCOL);
	itsListener.accept(*pNewSBClientPort);
}

//
// readRanges
//
void ServiceBroker::readRanges()
{
	ASSERTSTR (globalParameterSet()->isDefined("firstPortNumber") && 
	    	   globalParameterSet()->isDefined("lastPortNumber"), 
				"Ranges not specified in ParameterSet");

	itsLowerLimit = globalParameterSet()->getUint16("firstPortNumber");
	itsUpperLimit = globalParameterSet()->getUint16("lastPortNumber");

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

	// Finally read or construct name of adminfile.
	if (!globalParameterSet()->isDefined("adminFile")) {
		itsAdminFile = "./SB.admin";
	}
	else {
		itsAdminFile = globalParameterSet()->getString("adminFile");
	}
	LOG_INFO_STR ("Using file " << itsAdminFile << " for administration");
}

//
// claimPortNumber(servicename, port)
//
uint16 ServiceBroker::claimPortNumber(const string& 		aServiceName,
									  GCFPortInterface*		aPort)
{
	int32	idx = 0;

	if (!itsNrFreePorts) {				// must have room.
		return (0);
	}

	// note: an loaded registration may already be present, reuse that address
	uint16 portNr = findService(aServiceName, false);
	if (portNr) {
		idx = portNr2Index(portNr);
	}
	else {
		// no old loaded entry available, search free place
		while (idx < itsNrPorts && itsServiceList[idx].portNumber != 0) {
				idx++;
		}
		itsNrFreePorts--;
		ASSERTSTR (idx < itsNrPorts, "Major programming error in 'claimPortNumber'!");
	}

	// assign port to service
	itsServiceList[idx].portNumber  = index2PortNr(idx);
	itsServiceList[idx].serviceName = aServiceName;
	itsServiceList[idx].ownerPort   = aPort;

	// notify user.
	LOG_INFO(formatString ("Portnumber %d assigned to '%s'.", 
								itsServiceList[idx].portNumber, aServiceName.c_str()));
	LOG_INFO_STR ("Managing " << itsNrPorts - itsNrFreePorts << " ports now");
	saveAdministration	(itsAdminFile);			// to survive crashes

	return (itsServiceList[idx].portNumber);
}

//
// reRegisterService(servicename, portnr)
//
uint16 ServiceBroker::reRegisterService(const string& servicename, uint16 oldPortNr,
											GCFPortInterface*	thePort)
{
	int32	idx = 0;
	int32	nrElems2Check = itsNrPorts - itsNrFreePorts;// prevent checking whole array

	while (idx < itsNrPorts && nrElems2Check > 0) {
		if (itsServiceList[idx].portNumber) {
			nrElems2Check--;
			if (itsServiceList[idx].serviceName == servicename) {
				if (!itsServiceList[idx].ownerPort && itsServiceList[idx].portNumber == oldPortNr) {
					itsServiceList[idx].ownerPort = thePort;
					LOG_INFO_STR("Service " << servicename << " confirmed at " << oldPortNr);
					saveAdministration	(itsAdminFile);			// to survive crashes
					return (SB_NO_ERROR);
				}
				else {
					LOG_ERROR_STR("Recovering of service " << servicename << " at " <<
								oldPortNr << " not possible. Service was at " << 
								itsServiceList[idx].portNumber);
					return (SB_CANT_RECOVER);
				}
			}
		}
		idx++;
	}

	// service not in our admin anymore, try to fullfill the question.
	idx = portNr2Index(oldPortNr);		// convert portnr to array index
	if (itsServiceList[idx].ownerPort == 0) {	// still free?
		// assign port to service
		itsServiceList[idx].portNumber  = oldPortNr;
		itsServiceList[idx].serviceName = servicename;
		itsServiceList[idx].ownerPort   = thePort;
		itsNrFreePorts--;

		LOG_INFO_STR("Service " << servicename << " reregistered at " << oldPortNr);
		saveAdministration	(itsAdminFile);			// to survive crashes
		return (SB_NO_ERROR);
	}

	LOG_ERROR_STR("Recovering of service " << servicename << " at " <<
				oldPortNr << " not possible. Portnr taken by " << 
				itsServiceList[idx].serviceName);
	return (SB_CANT_RECOVER);
}

//
// releaseService(servicename)
//
void ServiceBroker::releaseService(const string& 		aServiceName)
{
	if (itsNrFreePorts == itsNrPorts) {	// ready when nothing was claimed.
		return;
	}

	uint16	portNr = findService(aServiceName, false);
	if (!portNr) {						// unknown service?
		return;
	}

	int32 	idx = portNr2Index(portNr);		// convert portnr to array index

	LOG_INFO(formatString("Service %s(%d) unregistered", aServiceName.c_str(), portNr));
	
	itsServiceList[idx].portNumber  = 0;
	itsServiceList[idx].serviceName = "";
	itsServiceList[idx].ownerPort   = 0;
	itsNrFreePorts++;

	LOG_INFO_STR ("Still managing " << itsNrPorts - itsNrFreePorts << " ports");

	saveAdministration	(itsAdminFile);			// to survive crashes
}

//
// releasePort(port)
//
void ServiceBroker::releasePort(GCFPortInterface*	aPort)
{
	if (!aPort) {						// check args
		return;
	}

	int32	idx = 0;
	int32	nrElems2Check = itsNrPorts - itsNrFreePorts;// prevent checking whole array
	int32	orgCount = nrElems2Check;

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

	if (orgCount != (itsNrPorts - itsNrFreePorts)) {
		saveAdministration (itsAdminFile);
	}
}

//
// findService(servicename)
//
uint16 ServiceBroker::findService(const string& aServiceName, bool usedOnly)
{
	int32	idx = 0;
	int32	nrElems2Check = itsNrPorts - itsNrFreePorts;// prevent checking whole array

	while (idx < itsNrPorts && nrElems2Check > 0) {
		if (itsServiceList[idx].portNumber) {
			nrElems2Check--;
			if (itsServiceList[idx].serviceName == aServiceName) {
				// only return portnumber if port is assigned also to prevent
				// returning numbers of old, not-reregistered, services
				return ((usedOnly && !itsServiceList[idx].ownerPort) ? 0 : 
													itsServiceList[idx].portNumber);
			}
		}
		idx++;
	}
		
	return (0);
}

//
// saveAdministration(fileName)
//
void ServiceBroker::saveAdministration(const string&	aFileName)
{
	// Try to create the administration file.
	ofstream	outFile(aFileName.c_str(), ofstream::out | ofstream::trunc 
														 | ofstream::binary);

	// If the file can not be opened warn to operator.
	if (!outFile) {
		LOG_WARN_STR("Unable to open file " << aFileName 
								<< ". ServiceBroker is not powerfailure save!");
		return;
	}

	LOG_DEBUG ("Saving administration");

	uint16	writeVersion = SB_ADMIN_VERSION;
	uint16	count		 = itsNrPorts - itsNrFreePorts;
	outFile.write((char*)&writeVersion,  sizeof(writeVersion));
	outFile.write((char*)&count, 		 sizeof(count));
	outFile.write((char*)&itsLowerLimit, sizeof(itsLowerLimit));
	outFile.write((char*)&itsUpperLimit, sizeof(itsUpperLimit));
	
	uint16	idx = 0;
	while (idx < itsNrPorts && count > 0) {
		if (itsServiceList[idx].portNumber) {
			// note: the TCPport is not saved because it can not be restored.
			outFile.write((char*)&itsServiceList[idx].portNumber, 
						   sizeof(itsServiceList[idx].portNumber));
			uint16	size = itsServiceList[idx].serviceName.length() + 1;	// save 0 also
			char	srvName[256];
			strcpy (srvName, itsServiceList[idx].serviceName.c_str());
			outFile.write((char*)&size, sizeof(size));
			outFile.write(srvName, size);
			count--;
		}
		idx++;
	}
	outFile.close();
}

//
// loadAdministration(filename)
//
void ServiceBroker::loadAdministration(const string&	aFileName)
{
	// Try to open the adminfile.
	ifstream		inFile(aFileName.c_str(), ifstream::in | ifstream::binary);
	if (!inFile) {
		LOG_DEBUG_STR ("No old administration found(" << aFileName << ")");
		return;
	}

	// read header info
	uint16	readVersion;		// for future version management
	uint16	count;				// nr of entries in the file
	uint16	lowerLimit;			// range settings
	uint16	upperLimit;
	inFile.read((char*)&readVersion, sizeof(readVersion));
	inFile.read((char*)&count,		 sizeof(count));
	inFile.read((char*)&lowerLimit,  sizeof(lowerLimit));
	inFile.read((char*)&upperLimit,  sizeof(upperLimit));

	LOG_DEBUG_STR ("Loading " << count << " old registrations from file " << aFileName);

	while (count) {
		uint16	portNumber;
		inFile.read((char*)&portNumber, sizeof(portNumber));

		uint16	size;
		char	servicename[256];
		inFile.read((char*)&size, sizeof(size));
		inFile.read(servicename, size);

		if (portNumber < itsLowerLimit || portNumber >= itsUpperLimit) {
			LOG_DEBUG_STR ("Portnumber " << portNumber 
										 << " not in current range, ignoring");
		}
		else {
			itsServiceList[portNr2Index(portNumber)].portNumber  = portNumber;
			itsServiceList[portNr2Index(portNumber)].serviceName = servicename;
			itsServiceList[portNr2Index(portNumber)].ownerPort   = 0;
			itsNrFreePorts--;
			LOG_DEBUG_STR ("Loading " << servicename << "@" << portNumber);	
		}
		count--;
	}

	inFile.close();
}

//
// cleanupOldRegistrations()
//
void ServiceBroker::cleanupOldRegistrations()
{
	LOG_DEBUG ("Cleanup of old not reclaimed registrations");

	bool	removedSome = false;
	uint16	idx = 0;
	while (idx < itsNrPorts) {
		if (itsServiceList[idx].portNumber && !itsServiceList[idx].ownerPort) {
			LOG_DEBUG_STR ("Cleanup old " << itsServiceList[idx].serviceName
										  << "@" << itsServiceList[idx].portNumber);
			itsServiceList[idx].portNumber  = 0;
			itsServiceList[idx].serviceName = "";
			itsServiceList[idx].ownerPort	= 0;
			itsNrFreePorts++;
			removedSome = true;
		}
		idx++;
	}

	LOG_INFO_STR ("Now managing " << itsNrPorts - itsNrFreePorts << " ports");

	if (removedSome) {
		saveAdministration(itsAdminFile);
	}
}


  } // namespace SB
} // namespace LOFAR
