//#  LogClient.cc: Filters and stores logmessages
//#
//#  Copyright (C) 2007
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
#include <Common/lofar_fstream.h>
#include <Common/Version.h>
#include <Common/ParameterSet.h>
#include <ApplCommon/StationInfo.h>		// LOFAR_SHARE_LOCATION
//#include <GCF/PVSS/GCF_PVTypes.h>
#include <MACIO/MACServiceInfo.h>
#include <MACIO/LOG_Protocol.ph>
#include <log4cplus/socketappender.h>
#include "LogClient.h"
#include <CUDaemons/Package__Version.h>

using namespace log4cplus;
using namespace log4cplus::helpers;
namespace LOFAR {
	using namespace GCF::TM;
    namespace CUDaemons {

#define		MAX_ADMINLINE_LEN	1024

//
// CodeloggingClient()
//
LogClient::LogClient(const string&	myName) :
	GCFTask((State)&LogClient::initial, myName),
	itsTimerPort		(0),
	itsListener 		(0),
	itsCLmaster			(0),
	itsConnected		(false),
	itsCLMtimerID		(0),
	itsMaxLinesPerFile	(1000),
	itsChunkSize		(10),
	itsSurvivalLinenr	(0),
	itsDrainLinenr		(0),
	itsInSeqnr			(1),
	itsOutSeqnr			(1)
{
	LOG_DEBUG_STR("LogClient(" << myName << ")");
	LOG_INFO(Version::getInfo<CUDaemonsVersion>("LogClient"));

	registerProtocol(F_FSM_PROTOCOL, F_FSM_PROTOCOL_STRINGS);
	registerProtocol(LOG_PROTOCOL,   LOG_PROTOCOL_STRINGS);

	// First init my admin variables
	itsAdminFile 	   = globalParameterSet()->getString("AdminFile", "LogClient.admin");
	itsMaxLinesPerFile = globalParameterSet()->getUint32("MaxLinesPerFile", 1000);
	itsChunkSize       = globalParameterSet()->getUint32("ChunkSize", 10);
	itsMasterHost      = globalParameterSet()->getString("MasterHost", "rs002");
	if (itsAdminFile.find("share") == string::npos) {
		itsAdminFile.insert(0, LOFAR_SHARE_LOCATION);
	}

	// load old settings
	_loadAdmin(itsAdminFile);

	// initialize the ports
	itsListener = new GCFTCPPort(*this, "listener", GCFPortInterface::MSPP, 0, true);
	ASSERTSTR(itsListener, "Can't allocate a listener port");
	itsListener->setPortNumber(MAC_CODELOGGING_PORT);

	itsCLmaster = new GCFTCPPort(*this, MAC_SVCMASK_LOGPROC, GCFPortInterface::SAP, LOG_PROTOCOL);
	itsCLmaster->setHostName(itsMasterHost);
	ASSERTSTR(itsCLmaster, "Can't allocate distribution port");

	itsTimerPort = new GCFTimerPort(*this, "timerPort");
	ASSERTSTR(itsTimerPort, "Can't allocate timer");

}

//
// ~CodeloggingClient()
//
LogClient::~LogClient()
{
	LOG_DEBUG_STR("~LogClient()");
	
	if (itsTimerPort) {	
		delete itsTimerPort;
	}
	if (itsCLmaster) {
		delete itsCLmaster;
	}
	if (itsListener) {
		delete itsListener;
	}
}

//
// initial(event, port)
//
// Try to open our listener socket and startup connection process with master Logger.
//
GCFEvent::TResult LogClient::initial(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY:
	case F_TIMER:
		if (!itsListener->isConnected()) {
			itsListener->open();
		}
	break;

	case F_CONNECTED:
		// Listener is opened. Startup connection with CLmaster and go to the right mode.
		itsCLmaster->open();
		TRAN(LogClient::operational);
	break;

	case F_DISCONNECTED:
		port.setTimer(5.0); // try again after 5 second
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
GCFEvent::TResult LogClient::operational(GCFEvent&			event, 
											 GCFPortInterface&	port)
{
	LOG_DEBUG_STR("operational:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	static unsigned long garbageTimerID = 0;

	switch (event.signal) {
	case F_ENTRY:
		garbageTimerID = itsTimerPort->setTimer(1.0, 5.0); 
	break;

	// Catch incoming connections of new clients
	case F_ACCEPT_REQ: {
		GCFTCPPort*		client(new GCFTCPPort);
		if (!client) {
			LOG_ERROR("Can't allocate new socket for new client");
			return (status);
		}
		client->init(*this, "newClient", GCFPortInterface::SPP, 0, true);
		itsListener->accept(*client);
	}
	break;

	case F_CONNECTED:
		if (&port == itsCLmaster) {
			itsConnected = true;
			LOG_INFO ("Connected to LoggingProcessor");
		}
	break;

	case F_DISCONNECTED: {
		// check for my listener
		ASSERTSTR(itsListener != &port, "Lost listener-port, bailing out"); 

		// check for connection with LoggingProcessor
		if (&port == itsCLmaster) {
			port.close();
			LOG_INFO("Still no connection with LoggingProcessor, retry over 10 seconds");
			itsCLMtimerID = itsTimerPort->setTimer(10.0);
			itsConnected = false;
			break;
		}

		// must be a client.
		LogProcMap::iterator	iter = itsClients.find(&port);
		if (iter == itsClients.end()) {
			LOG_INFO("Connection lost to a not-registered LofarLogger client.");
		}
		else {
			if (iter->second.valid) {
				LOG_INFO_STR("Closing log-stream to " << iter->second.DPname << ", passed " 
							<< iter->second.msgCnt << " messages to the database");
			}
			else if (!iter->second.DPname.empty()) {
				LOG_INFO_STR("Closing log-stream to " << iter->second.DPname);
			}
			else {
				LOG_INFO("Closing unknown log-stream");
			}
		}
		port.close();
		itsClients.erase(&port);
		itsClientsGarbage.push_back(&port);
	}
	break;

	case F_TIMER: {
		GCFTimerEvent&	timerEvent = static_cast<GCFTimerEvent&>(event);
		if (timerEvent.id == itsCLMtimerID) {
			itsCLmaster->open();
			break;
		}
		
		// cleanup the garbage of closed ports to master clients
		GCFPortInterface* pPort;
		for (ClientsList::iterator iter = itsClientsGarbage.begin();
			iter != itsClientsGarbage.end(); ++iter) {
			pPort = *iter;
			delete pPort;
		}
		itsClientsGarbage.clear();
	}  
	break;

	case F_DATAIN: {
		// extract the incoming data to a Logger event object
		// read message size
		SocketBuffer msgSizeBuffer(sizeof(unsigned int));
		if (!_readFromPortData(port, msgSizeBuffer)) {
			break;
		}

		// read message
		unsigned int msgSize = msgSizeBuffer.readInt();
		SocketBuffer buffer(msgSize);
		if (!_readFromPortData(port, buffer)) {
			break;
		}

		// Construct an InternalLoggingEvent from the buffer
		spi::InternalLoggingEvent logEvent = readFromBuffer(buffer);           

		// Has client a known DP?
		string 	PVSSdp(_searchClientDP(logEvent, port));
		if (PVSSdp.empty()) {
			break;
		}

		// construct message: level|loggername|message|file:linenr
		string msg(formatString("%s|%s|%s|%s:%d",
					getLogLevelManager().toString(logEvent.getLogLevel()).c_str(),
					logEvent.getLoggerName().c_str(),
					logEvent.getMessage().c_str(),
					basename(logEvent.getFile().c_str()),
					logEvent.getLine()));

LOG_DEBUG_STR("Storing message " << itsInSeqnr);
		itsMsgBuffer[itsInSeqnr++] = Message(PVSSdp, msg);
		_activateBuffer();
	}
	break;

	case LOG_SEND_MSG_ACK: {
		// remove message from buffer and update admin
		LOGSendMsgAckEvent	ack(event);
		itsMsgBuffer.erase(ack.seqnr);
		itsOutSeqnr = ack.seqnr + 1;

		_activateBuffer();
	}
	break;
	case LOG_SEND_MSG_POOL_ACK: {
		// remove the messages from buffer and update admin
		LOGSendMsgPoolAckEvent	ack(event);
		for (uint32 i = 0; i < itsChunkSize; i++) {
			itsMsgBuffer.erase(ack.seqnr + i);
		}
		itsOutSeqnr = ack.seqnr + itsChunkSize;

		_activateBuffer();
	}
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

//
// _readFromPortData(port, buf)
//
bool LogClient::_readFromPortData(GCFPortInterface& port, SocketBuffer& buf)
{
	size_t res, read = 0;  
	do { 
		res = port.recv(buf.getBuffer() + read, buf.getMaxSize() - read);
		if (res <= 0) {
			break;
		}

		read += res;
	} while (read < buf.getMaxSize());

	return (read == buf.getMaxSize());
}

//
// _activateBuffer
//
// Some messages were removed or added to the Msgbuffer send the next.
//
void LogClient::_activateBuffer()
{
	LOG_TRACE_FLOW("_activateBuffer()");

	// If the buffer is empty load next msg file is there is any
	if (itsMsgBuffer.empty()) {
		if (!_loadNextMessageFile()) {	// every thing done?
			return;						// coffee break;
		}
		itsOutSeqnr = itsMsgBuffer.begin()->first;	// resync seqnr.
	}

	// dependant on the size of the buffer we send one message or 'chunkSize' messages.
	if (itsMsgBuffer.size() > itsChunkSize) {
		// send a pool of messages
		LOGSendMsgPoolEvent		poolEvent;
		poolEvent.seqnr    = itsOutSeqnr;
		poolEvent.msgCount = itsChunkSize;
		poolEvent.DPnames.theVector.resize (itsChunkSize);
		poolEvent.messages.theVector.resize(itsChunkSize);
LOG_DEBUG_STR("outSeq=" << itsOutSeqnr);
		MsgMap::iterator	iter = itsMsgBuffer.begin();
		for (uint32 i = 0; i < itsChunkSize; i++) {
LOG_DEBUG_STR("PoolMsg " << i << ": " << iter->second.message);
			poolEvent.DPnames.theVector[i] = iter->second.DPname;
			poolEvent.messages.theVector[i] = iter->second.message;
			iter++;
		}
		itsCLmaster->send(poolEvent);
	}
	else { // send one message
		LOGSendMsgEvent		logEvent;
		logEvent.seqnr   = itsOutSeqnr;
		logEvent.DPname  = itsMsgBuffer[itsOutSeqnr].DPname;
		logEvent.message = itsMsgBuffer[itsOutSeqnr].message;
		itsCLmaster->send(logEvent);
	}
}

//
// _loadNextMessageFile()
//
// Tries to load the contents of the next message file to the MsgBuffer.
// Returns true if the MsgBuffer was changed.
//
bool LogClient::_loadNextMessageFile()
{
	LOG_TRACE_FLOW("_loadNextMessageFile()");
//	ifstream	admFile(filename.c_str(), ifstream::in);
//	if (!admFile) {

	// TODO: implement this

	return (false);
}

//
// _seachClientDP(logEvent, port)
//
// Returns the name of the DP the message must be logged to.
// When there is no DP connected yet to the port the message is analysed to
// find the name of the DP. In order not the keep searching for DPnames
// the DPname must be in one of the first ten messages otherwise the port
// entry is marked 'invalid' and the messages or not processed anymore.
// The 'valid' flag is also used when the DP does not exist in the database
// or when database report more than 10 errors on that DP.
//
string LogClient::_searchClientDP(spi::InternalLoggingEvent&	logEvent,
									 	 GCFPortInterface&			port)
{
	// Known client ?
	LogProcMap::iterator iter = itsClients.find(&port);
	if (iter == itsClients.end()) {
		itsClients[&port] = LogProc();	// no, new client
		iter = itsClients.find(&port);
	}
	else {	// yes, known client
		if (iter->second.valid) {			// valid DP name
			iter->second.msgCnt++;
			return (iter->second.DPname);	// return DPname [1][3]
		}
	}

	// Filled DPname but invalid name? return failure.
	if (!iter->second.valid && !iter->second.DPname.empty()) {
		return ("");	// [2][4]
	}

	// DPname is not filled in yet, tried to find it if we are still
	// within the first 10 messages.
	if (++(iter->second.msgCnt) > 0) {
		// when msgCnt reached 0 we could report that we tried it 10 times
		// but we don't know the name of the log-stream, so just return
		return ("");
	}

	// Level must be INFO
	if (logEvent.getLogLevel() != INFO_LOG_LEVEL) {
		return ("");
	}

	// Message must start with 'MACProcessScope:"
	string DPname = logEvent.getMessage();
	if (DPname.find("MACProcessScope:") != 0) {
		return ("");
	}

	// A legal DPname must be after it
	DPname.erase(0, sizeof("MACProcessScope:"));
	ltrim(DPname);
	rtrim(DPname);
	// '.' must be converted to the '_' in case of DP names
	uint32	EOS = DPname.size();
	for (uint32 i = 0; i < EOS; i++) {
		if (DPname[i] == '.') {
			DPname[i] = '_';
		}
	} 
	DPname.append(".logMsg");
	itsClients[&port] = LogProc(DPname);
	itsClients[&port].msgCnt = 0;
	itsClients[&port].valid = true;
	LOG_INFO_STR("Starting log-stream for " << DPname);

	return (DPname);
}

//
// _registerFailure(port)
//
void LogClient::_registerFailure(GCFPortInterface&		port)
{
	LogProcMap::iterator iter = itsClients.find(&port);
	if (iter == itsClients.end()) {
		return;
	}

	if (++(iter->second.errCnt) > 10) {
		iter->second.valid = false;
		LOG_INFO_STR("Log-stream to " << iter->second.DPname << " keeps reporting errors, ignoring stream");
	}
}

//
// _processMessage(msg)
//
void LogClient::_processMessage(const string&	message)
{
	if (itsSurvivalLinenr == 0) {	// not writing to file?
		LOGSendMsgEvent		LogEvent;
		LogEvent.seqnr = ++itsOutSeqnr;
		LogEvent.message = message;
		itsCLmaster->send(LogEvent);
		return;
	}

#if 0
	// TODO
	// no connection, writing it to a file
	ifstream	outFile = _openSurvivalFile()
	outfile.write(message, strlen0(message));
	outFile.close();
	itsSurvivalLinenr++;
#endif
}

//
// _loadAdmin(filename)
//
void LogClient::_loadAdmin(const string&	filename)
{

	// Try to open the adminfile.
	ifstream	admFile(filename.c_str(), ifstream::in);
	if (!admFile) {
		LOG_DEBUG_STR("No old administration found(" << filename << ")");
		return;
	}

	// read admin line
	char	line[MAX_ADMINLINE_LEN];
	admFile.getline(line, MAX_ADMINLINE_LEN-1);
	line[MAX_ADMINLINE_LEN-1] = '\0';
	admFile.close();

	// convert line
	char	survivalFile[1024];
	uint32	survivalLinenr;
	char	drainFile[1024];
	uint32	drainLinenr;
	if (sscanf(line, "%s|%d|%s|%d", survivalFile, &survivalLinenr, 
									drainFile, &drainLinenr) != 4) {
		LOG_ERROR_STR("Contents of adminfile " << filename << 
				" has wrong format, ignoring contents with risk on information loss.");
		return;
	}

	// Finally adopt new information
	itsSurvivalFile		= survivalFile;
	itsSurvivalLinenr	= survivalLinenr;
	itsDrainFile		= drainFile;
	itsDrainLinenr		= drainLinenr;
	
}
//
// _saveAdmin(filename)
//
void LogClient::_saveAdmin(const string&	filename)
{
	// Try to create the administration file.
	ofstream	admFile(filename.c_str(), ofstream::out | ofstream::trunc);

	if (!admFile) {
		LOG_ERROR_STR("Unable to open file " << filename << 
						". Can not garantee that no data is lost");
		return;
	}

	LOG_DEBUG ("Saving administration");

	char	line [MAX_ADMINLINE_LEN];
	sprintf(line, "%s|%d|%s|%d", itsSurvivalFile.c_str(), itsSurvivalLinenr, 
								 itsDrainFile.c_str(), itsDrainLinenr);

	admFile.write(line, strlen(line));
	admFile.close();
}

  } // namespace CUDaemons
} // namespace LOFAR
