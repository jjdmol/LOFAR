//#  BGPlogProcessor.cc: Captures cout datastreams of CEP programs
//#
//#  Copyright (C) 2009
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <Common/ParameterSet.h>
#include <Common/SystemUtil.h>
#include "BGPlogProcessor.h"

namespace LOFAR {
  using namespace GCF::TM;
  namespace APL {

//
// BGPlogProcessor(name)
//
BGPlogProcessor::BGPlogProcessor(const string&	progName) :
	GCFTask		((State)&BGPlogProcessor::startListener, progName),
	itsListener	(0)
{
	// prepare TCP port to accept connections on
	itsListener = new GCFTCPPort (*this, "BGPlogger:v1_0", GCFPortInterface::MSPP, 0);
	ASSERTSTR(itsListener, "Cannot allocate listener port");
	itsListener->setPortNumber(globalParameterSet()->getInt("BGPlogProcessor.portNr"));

	itsBufferSize = globalParameterSet()->getInt("BGPlogProcessor.bufferSize", 1024);
}

//
// ~BGPlogProcessor()
//
BGPlogProcessor::~BGPlogProcessor()
{
	if (itsListener) { 
		delete itsListener; 
	}
}

//
// startListener(event, port)
//
GCFEvent::TResult BGPlogProcessor::startListener(GCFEvent&	event, GCFPortInterface&	port)
{
	LOG_DEBUG_STR("startListener:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_INIT:
		itsListener->autoOpen(0, 10, 2);	// report within 10 seconds.
		break;

	case F_CONNECTED:
		LOG_DEBUG("Listener is started, going to operational mode");
		TRAN (BGPlogProcessor::operational);
		break;

	case F_DISCONNECTED:
		LOG_FATAL_STR("Cannot open the listener on port " << itsListener->getPortNumber() << ". Quiting!");
		GCFScheduler::instance()->stop();
		break;
	}

	return (GCFEvent::HANDLED);
}

//
// operational(event, port)
//
GCFEvent::TResult BGPlogProcessor::operational(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("operational:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ACCEPT_REQ:
		_handleConnectionRequest();
		break;

	case F_CONNECTED:
		break;

	case F_DISCONNECTED:
		port.close();
	case F_DATAIN:
		_handleDataStream(&port);
		break;
	}

	return (GCFEvent::HANDLED);
}

//
// _handleConnectionRequest()
//
void BGPlogProcessor::_handleConnectionRequest()
{
	GCFTCPPort* 	pNewClient = new GCFTCPPort();
	ASSERT(pNewClient);

	pNewClient->init(*this, "newClient", GCFPortInterface::SPP, 0, true);
	if (!itsListener->accept(*pNewClient)) {
		LOG_WARN("Connection with new client went wrong");
		return;
	}

	// give stream its own buffer.
	streamBuffer_t		stream;
	stream.socket	= pNewClient;
	stream.buffer 	= (char*)malloc(itsBufferSize);
	stream.inPtr  	= 0;
	stream.outPtr 	= 0;
	itsLogStreams[pNewClient] = stream;
	LOG_INFO_STR("Added new client to my admin");
}

//
// _handleDataStream(sid)
//
void BGPlogProcessor::_handleDataStream(GCFPortInterface*	port)
{
	// read in the new bytes
	streamBuffer_t	stream		= itsLogStreams[port];
	LOG_DEBUG_STR("handleDataStream:in=" << stream.inPtr << ", out=" << stream.outPtr);
	int	newBytes = stream.socket->recv(stream.buffer + stream.inPtr, itsBufferSize - stream.inPtr);
	LOG_DEBUG_STR("received " << newBytes << " new bytes");
	if (newBytes < 0) {
//		LOG_ERROR_STR("read on socket " << sid << " returned " << newBytes << ". Closing connection");
		free (stream.buffer);
		stream.socket->close();
		free(stream.socket);
		itsLogStreams.erase(port);
		return;
	}
//	LOG_DEBUG_STR("Received " << newBytes << " bytes at sid " << sid);
	stream.inPtr += newBytes;
	
	// process as much data as possible from the buffer.
	for (int i = stream.outPtr; i <= stream.inPtr; i++) {
		if (stream.buffer[i] != '\n') {
			continue;
		}

		stream.buffer[i] = '\0';
//		LOG_INFO(formatString("SID %d:>%s<", sid, &(stream.buffer[stream.outPtr])));
		LOG_INFO(formatString("(%d,%d)>%s<", stream.outPtr, i, &(stream.buffer[stream.outPtr])));
		_processLogLine(&(stream.buffer[stream.outPtr]));
		stream.outPtr = i+1;
		if (stream.outPtr >= stream.inPtr) {	// All received bytes handled?
			LOG_DEBUG("Reset of read/write pointers");
			stream.inPtr = 0;
			stream.outPtr = 0;
			itsLogStreams[port] = stream;	// copy changes back to admin
			return;
		} 
	}

	if (stream.outPtr > (int)(0.5*itsBufferSize)) {
		// When buffer becomes full shift leftovers to the left.
		LOG_DEBUG_STR("move with: " << stream.inPtr << ", " << stream.outPtr);
		memmove (stream.buffer, stream.buffer + stream.outPtr, (stream.inPtr - stream.outPtr + 1));
		stream.inPtr -= stream.outPtr;
		stream.outPtr = 0;
	}

	itsLogStreams[port] = stream; // copy changes back to admin
}

//
// _getProcessID(char*)
//
string BGPlogProcessor::_getProcessID(char*	cString)
{
	char	delimiter[]="|";
	char*	result = strtok(cString, delimiter);	// get loglevel
	if (!result) {		// unknown line skip it.
		LOG_DEBUG("No loglevel found");
		return ("");
	}

	if (!(result = strtok(NULL, delimiter))) {		// get processID
		LOG_DEBUG("No processID found");
		return ("");
	}
	
	return (result);
}


//
// _processLogLine(char*)
//
void BGPlogProcessor::_processLogLine(char*		cString)
{
	string processName(_getProcessID(cString));
	LOG_DEBUG_STR("Processname=" << processName);
	if (processName.empty()) {
		return;
	}

	// TODO: switch on processName to right analysis routine

	char*	logMsg = strtok(NULL, "|");
	char*	result;

	if ((result = strstr(logMsg, " late: "))) {
		float	late;
		sscanf(result, " late: %f ", &late);	// TODO check result
		LOG_DEBUG_STR("Late: " << late);
	}
	else if ((result = strstr(logMsg, "ION->CN:"))) {
		float	ioTime;
		sscanf(result, "ION->CN:%f", &ioTime);	// TODO check result
		LOG_DEBUG_STR("ioTime: " << ioTime);
	}
	else if ((result = strstr(logMsg, "received ["))) {
		int	blocks0(0), blocks1(0), blocks2(0), blocks3(0);
		sscanf(result, "received [%d,%d,%d,%d]", &blocks0, &blocks1, &blocks2, &blocks3);	// TODO check result
		LOG_DEBUG(formatString("blocks: %d, %d, %d, %d", blocks0, blocks1, blocks2, blocks3));
	}
}

  } // namespace APL
} // namespace LOFAR
