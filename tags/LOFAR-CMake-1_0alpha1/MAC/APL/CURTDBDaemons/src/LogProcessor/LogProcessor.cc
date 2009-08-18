//#  LogProcessor.cc: Filters and stores logmessages in PVSS
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
#include <Common/StringUtil.h>
#include <Common/Version.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <MACIO/GCF_Event.h>
#include <MACIO/MACServiceInfo.h>
#include <MACIO/LOG_Protocol.ph>
#include <GCF/PVSS/PVSSresult.h>
#include <GCF/RTDB/DP_Protocol.ph>
#include <log4cplus/socketappender.h>
#include "LogProcessor.h"
#include "../Package__Version.h"

using namespace log4cplus;
using namespace log4cplus::helpers;
namespace LOFAR {
  using namespace MACIO;
  namespace GCF {
    using namespace TM;
    using namespace PVSS;
    using namespace RTDB;
    namespace RTDBDaemons {

//
// CodeloggingProcessor()
//
LogProcessor::LogProcessor(const string&	myName) :
	GCFTask((State)&LogProcessor::initial, myName),
	itsListener (0),
	itsBackDoor (0),
	itsDPservice(0),
	itsTimerPort(0)
{
	LOG_DEBUG_STR("LogProcessor(" << myName << ")");
	LOG_INFO(Version::getInfo<CURTDBDaemonsVersion>("LogProcessor"));

	registerProtocol(F_FSM_PROTOCOL, F_FSM_PROTOCOL_STRINGS);
	registerProtocol(LOG_PROTOCOL,   LOG_PROTOCOL_STRINGS);
	registerProtocol(DP_PROTOCOL,	 DP_PROTOCOL_STRINGS);

	// initialize the ports
	itsListener = new GCFTCPPort(*this, "listener", GCFPortInterface::MSPP, 0, true);
	ASSERTSTR(itsListener, "Can't allocate a listener port");
	itsListener->setPortNumber(MAC_CODELOGGING_PORT);

	itsBackDoor = new GCFTCPPort(*this, MAC_SVCMASK_LOGPROC, GCFPortInterface::MSPP, 
																		LOG_PROTOCOL);
	ASSERTSTR(itsBackDoor, "Can't allocate a backdoor listener");

	itsDPservice = new DPservice(this);
	ASSERTSTR(itsDPservice, "Can't allocate DataPoint service");

	itsTimerPort = new GCFTimerPort(*this, "timerPort");
	ASSERTSTR(itsTimerPort, "Can't allocate timer");

}

//
// ~CodeloggingProcessor()
//
LogProcessor::~LogProcessor()
{
	LOG_DEBUG_STR("~LogProcessor()");
	
	if (itsTimerPort) {	
		delete itsTimerPort;
	}
	if (itsDPservice) {
		delete itsDPservice;
	}
	if (itsBackDoor) {
		delete itsBackDoor;
	}
	if (itsListener) {
		delete itsListener;
	}
}

//
// initial(event, port)
//
// Try to open our listener socket
//
GCFEvent::TResult LogProcessor::initial(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY:
	case F_TIMER:
		if (!itsListener->isConnected()) {
			LOG_DEBUG("Trying to start the listener for log4cplus");
			itsListener->open();
		}
		else {
			if (!itsBackDoor->isConnected()) {
				LOG_DEBUG("Trying to start the listener for the LogClients");
				itsBackDoor->open();
			}
		}
	break;

	case F_CONNECTED:
		if (&port == itsListener) {		// application listener open?
			itsBackDoor->open();		// also open listener for LogClients
		}
		// Backdoor is also open, go to operational state.
		TRAN(LogProcessor::operational);
	break;

	case F_DISCONNECTED:
		LOG_DEBUG("Starting Listener failed, retry in 5 seconds");
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
GCFEvent::TResult LogProcessor::operational(GCFEvent&			event, 
												GCFPortInterface&	port)
{
	LOG_DEBUG_STR("operational:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	static unsigned long garbageTimerID = 0;

	switch (event.signal) {
	case F_ENTRY:
		garbageTimerID = itsTimerPort->setTimer(1.0, 5.0); 
		// Register my own logging too.
		LOG_INFO("MACProcessScope: LOFAR_PermSW_Daemons_LogProcessor");
	
	break;

	// Catch incoming connections of new clients
	case F_ACCEPT_REQ: {
		GCFTCPPort*		client(new GCFTCPPort);
		if (!client) {
			LOG_ERROR("Can't allocate new socket for new client");
			return (status);
		}
		if (&port == itsListener) {
			client->init(*this, "application", GCFPortInterface::SPP, 0, true);
			itsListener->accept(*client);
			LOG_DEBUG("New connection with an application");
		}
		if (&port == itsBackDoor) {
			client->init(*this, "LogClient", GCFPortInterface::SPP, LOG_PROTOCOL);
			itsBackDoor->accept(*client);
			LOG_DEBUG("New connection with a LogCLient");
		}
	}
	break;

	case F_CONNECTED:
	break;

	case F_DISCONNECTED: {
		ASSERTSTR(itsListener != &port, "Lost listener-port, bailing out"); 
		LogClientMap::iterator	iter = itsClients.find(&port);
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
		// cleanup the garbage of closed ports to master clients
		GCFPortInterface* pPort;
		for (TClients::iterator iter = itsClientsGarbage.begin();
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
		spi::InternalLoggingEvent l4cpLogEvent = readFromBuffer(buffer);           
		LOG_TRACE_RTTI_STR("IN:" << l4cpLogEvent.getMessage());

		// Has client a known DP?
		string 	PVSSdp(_searchClientDP(l4cpLogEvent, port));
		if (PVSSdp.empty()) {
			break;
		}

		// construct message: level|loggername|message|file:linenr
		string msg(formatString("%s|%s|%s|%s:%d",
					getLogLevelManager().toString(l4cpLogEvent.getLogLevel()).c_str(),
					l4cpLogEvent.getLoggerName().c_str(),
					l4cpLogEvent.getMessage().c_str(),
					basename(l4cpLogEvent.getFile().c_str()),
					l4cpLogEvent.getLine()));

#if 0
		// convert the logger event to the PVSSdp value
		GCFPVString value(msg);
		LOG_DEBUG(formatString("Msg: %s", msg.c_str()));

		// timestamp conversion
		timeval		kvlTimestamp;  
		Time		l4pTimestamp(l4cpLogEvent.getTimestamp());
		kvlTimestamp.tv_sec  = l4pTimestamp.sec();
		kvlTimestamp.tv_usec = l4pTimestamp.usec();

		LOG_KEYVALUE_TS(PVSSdp[0], value, KVL_ORIGIN_MAC, kvlTimestamp);
#endif

		// convert logger event to DP log msg
		string plMsg = l4cpLogEvent.getTimestamp().getFormattedTime("%d-%m-%y %H:%M:%S.%q") 
						+ "|" + msg;

		GCFPVString plValue(plMsg);
		PVSSresult	result = itsDPservice->setValue(PVSSdp, plValue);
		if (result != PVSS::SA_NO_ERROR && result != PVSS::SA_SCADA_NOT_AVAILABLE) {
			_registerFailure(port);
		}
	}
	break;

	case LOG_SEND_MSG: {
		LOGSendMsgEvent		logEvent(event);
		LOGSendMsgAckEvent	answer;
		answer.seqnr  = logEvent.seqnr;
		answer.result = PVSS::SA_NO_ERROR;
		PVSSresult	result = itsDPservice->setValue(logEvent.DPname, 
													GCFPVString(logEvent.message));
		switch (result) {
		case PVSS::SA_NO_ERROR:
			break;
		case PVSS::SA_SCADA_NOT_AVAILABLE:
			answer.result = result;
			break;
		default:
			// _registerFailure(port);
			answer.result = result;
		}
		port.send(answer);
	}
	break;

	case LOG_SEND_MSG_POOL: {
		LOGSendMsgPoolEvent		logEvent(event);
		LOGSendMsgPoolAckEvent	answer;
		answer.seqnr  = logEvent.seqnr;
		answer.result = PVSS::SA_NO_ERROR;
		for (uint32 i = 0; i < logEvent.msgCount; i++) {
			PVSSresult	result;
// = itsDPservice->setValue(logEvent.DPnames.theVector[i], 
//											GCFPVString(logEvent.messages.theVector[i]));
			switch (result) {
			case PVSS::SA_NO_ERROR:
				break;
			case PVSS::SA_SCADA_NOT_AVAILABLE:
				answer.result = result;
				i = logEvent.msgCount;		// don't try the others.
				break;
			default:
				// _registerFailure(port);
				answer.result |= result;
			} // switch
		} // for
		port.send(answer);
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
bool LogProcessor::_readFromPortData(GCFPortInterface& port, SocketBuffer& buf)
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
string LogProcessor::_searchClientDP(spi::InternalLoggingEvent&	logEvent,
									 	 GCFPortInterface&			port)
{
	// Known client ?
	LogClientMap::iterator iter = itsClients.find(&port);
	if (iter == itsClients.end()) {
		itsClients[&port] = LogClient();	// no, new client
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
	DPname.append(".process.logMsg");
	itsClients[&port] = LogClient(DPname);
	itsClients[&port].msgCnt = 0;
	itsClients[&port].valid = true;
	LOG_INFO_STR("Starting log-stream for " << DPname);

	return (DPname);
}

//
// _registerFailure(port)
//
void LogProcessor::_registerFailure(GCFPortInterface&		port)
{
	LogClientMap::iterator iter = itsClients.find(&port);
	if (iter == itsClients.end()) {
		return;
	}

	if (++(iter->second.errCnt) > 10) {
		iter->second.valid = false;
		LOG_INFO_STR("Log-stream to " << iter->second.DPname << " keeps reporting errors, ignoring stream");
	}
}


  } // namespace RTDBDaemons
 } // namespace GCF
} // namespace LOFAR
