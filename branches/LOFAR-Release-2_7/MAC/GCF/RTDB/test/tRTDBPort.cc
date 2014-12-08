//
//  tRTDBPort.cc: Test program to test the majority of the RTDBPort class.
//
//  Copyright (C) 2007
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id: tRTDBPort.cc 13145 2009-04-22 08:47:05Z overeem $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/Exception.h>
#include <MACIO/KVT_Protocol.ph>	// just as a test protocol, could have been any protocol
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/PVSS/PVSSinfo.h>
#include <GCF/PVSS/PVSSservice.h>
#include "tRTDBPort.h"

namespace LOFAR {
  namespace GCF {
  using namespace TM;
  using namespace PVSS;
  namespace RTDB {

bool	readerReady(false);
bool	writerReady(false);

//
// tWriter constructor
//
tWriter::tWriter(const string& name) : 
	GCFTask((State)&tWriter::openPort, name), 
	itsRTDBPort(0),
	itsTimerPort(0)
{
	LOG_DEBUG_STR("tWriter(" << name << ")");

	itsRTDBPort	 = new GCFRTDBPort(*this, "RTDBWriterPort", "DP_from_ruud");
	ASSERTSTR(itsRTDBPort, "Can't allocate RTDBPort");

	itsTimerPort = new GCFTimerPort(*this, "timerPort");
	ASSERTSTR(itsTimerPort, "Can't allocate GCFTimerPort");

	registerProtocol(KVT_PROTOCOL, KVT_PROTOCOL_STRINGS);

}

//
// tWriter destructor
//
tWriter::~tWriter()
{
	LOG_DEBUG("Deleting tWriter");
	if (itsRTDBPort) {
		delete itsRTDBPort;
	}
	if (itsTimerPort) {
		delete itsTimerPort;
	}
}

//
// tWriter openPort (event, port)
//
GCFEvent::TResult tWriter::openPort(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("[W]openPort:" << eventName(e));

	switch (e.signal) {
	case F_INIT: 
		break;

	case F_ENTRY: {
		if (!itsRTDBPort->open()) {
			LOG_FATAL("[W]Calling open failed!");
			TRAN (tWriter::final);
			return (GCFEvent::HANDLED);
		}
		// wait for F_CONNECT or F_DISCONNECT
		itsTimerPort->setTimer(5.0);
	}
	break;

	case F_TIMER:
		LOG_FATAL("[W]'open' of RTDBPort did not result in a F_(DIS)CONNECT");
		TRAN(tWriter::final);
		break;

	case F_CONNECTED:
		LOG_INFO("[W]Calling 'open' was successful, continue with write test");
		TRAN(tWriter::writeTest);
	break;

	case F_DISCONNECTED:
		LOG_FATAL("[W]'open' of RTDBPort resulted in a F_DISCONNECT");
		TRAN(tWriter::final);
		break;

	case F_EXIT:
		itsTimerPort->cancelAllTimers();
		break;

	default:
		return(GCFEvent::NOT_HANDLED);
	}

	return (GCFEvent::HANDLED);
}

//
// tWriter final (event, port)
//
GCFEvent::TResult tWriter::final(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("[W]final:" << eventName(e));

	switch (e.signal) {
	case F_ENTRY:
		if (!writerReady) {
			LOG_FATAL("[W]Writer-task FAILED, ABORTING program");
			itsTimerPort->setTimer(0.0);
		}
		else {
			if (readerReady) {
				LOG_INFO("[W] ### ALL TESTS PASSED SUCCESSFUL ###");
				itsTimerPort->setTimer(1.0);
				break;
			}
			LOG_INFO("[W]WRITE part was successful, waiting for reader to finish");
		}
		break;
	
	case F_TIMER:
		GCFScheduler::instance()->stop();
		break;

	default:
		return(GCFEvent::NOT_HANDLED);
	}

	return (GCFEvent::HANDLED);
}

//
// tWriter writeTest (event, port)
//
GCFEvent::TResult tWriter::writeTest(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("[W]writeTest:" << eventName(e));

	switch (e.signal) {
	case F_ENTRY:
		itsTimerPort->setTimer(1.5);		// wait for the reader to be online
		break;

	case F_EXIT:
		itsTimerPort->cancelAllTimers();
		break;

	case F_TIMER: {
		KVTRegisterEvent	request;
		request.obsID = 25002;
		request.name  = "This is a test string to test the RTDBPort interface";
		ssize_t		btsSent = itsRTDBPort->send(request);
		if (btsSent <= 0) {
			LOG_FATAL_STR("[W]Sending a message resulted in " << btsSent << " bytes being send");
			TRAN(tWriter::final);
			break;
		}
		// assume the event is in the database
		LOG_INFO_STR(btsSent << " bytes were stored in the database");
		TRAN(tWriter::closeTest);
	}
	break;

	default:
		return(GCFEvent::NOT_HANDLED);
	}

	return (GCFEvent::HANDLED);
}

//
// tWriter closeTest (event, port)
//
GCFEvent::TResult tWriter::closeTest(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("[W]closeTest:" << eventName(e));

	switch (e.signal) {
	case F_ENTRY: {
		itsRTDBPort->close();
		itsTimerPort->setTimer(2.0);
	}
	break;

	case F_TIMER:
		LOG_FATAL("[W]closeTest FAILED");
		TRAN(tWriter::final); 
	break;

	case F_DISCONNECTED:
		LOG_INFO("[W]closeTest was also successful.");
		writerReady = true;
		TRAN(tWriter::final);
		break;

	case F_EXIT:
		itsTimerPort->cancelAllTimers();
		break;

	default:
		return(GCFEvent::NOT_HANDLED);
	}

	return (GCFEvent::HANDLED);
}

//
// tReader constructor
//
tReader::tReader(const string& name) : 
	GCFTask((State)&tReader::openPort, name), 
	itsRTDBPort(0),
	itsTimerPort(0)
{
	LOG_DEBUG_STR("tReader(" << name << ")");

	itsRTDBPort	 = new GCFRTDBPort(*this, "RTDBReaderPort", "DP_from_ruud");
	ASSERTSTR(itsRTDBPort, "Can't allocate RTDBPort");

	itsTimerPort = new GCFTimerPort(*this, "timerPort");
	ASSERTSTR(itsTimerPort, "Can't allocate GCFTimerPort");

	registerProtocol(KVT_PROTOCOL, KVT_PROTOCOL_STRINGS);

}

//
// tReader destructor
//
tReader::~tReader()
{
	LOG_DEBUG("Deleting tReader");
	if (itsRTDBPort) {
		delete itsRTDBPort;
	}
	if (itsTimerPort) {
		delete itsTimerPort;
	}
}

//
// tReader openPort (event, port)
//
GCFEvent::TResult tReader::openPort(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("[R]openPort:" << eventName(e));

	switch (e.signal) {
	case F_INIT: 
		break;

	case F_ENTRY: {
		if (!itsRTDBPort->open()) {
			LOG_FATAL("[R]Calling open failed!");
			TRAN (tReader::final);
			return (GCFEvent::HANDLED);
		}
		// wait for F_CONNECT or F_DISCONNECT
		itsTimerPort->setTimer(5.0);
	}
	break;

	case F_TIMER:
		LOG_FATAL("[R]'open' of RTDBPort did not result in a F_(DIS)CONNECT");
		TRAN(tReader::final);
		break;

	case F_CONNECTED:
		LOG_INFO("[R]Calling 'open' was successful, continue with read test");
		TRAN(tReader::readTest);
	break;

	case F_DISCONNECTED:
		LOG_FATAL("[R]'open' of RTDBPort resulted in a F_DISCONNECT");
		TRAN(tReader::final);
		break;

	case F_EXIT:
		itsTimerPort->cancelAllTimers();
		break;

	default:
		return(GCFEvent::NOT_HANDLED);
	}

	return (GCFEvent::HANDLED);
}

//
// tReader final (event, port)
//
GCFEvent::TResult tReader::final(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("[R]final:" << eventName(e));

	switch (e.signal) {
	case F_ENTRY:
		if (!readerReady) {
			LOG_FATAL("[R]Reader-task FAILED, ABORTING program");
			itsTimerPort->setTimer(0.0);
		}
		else {
			if (writerReady) {
				LOG_INFO("[R] ### ALL TESTS PASSED SUCCESSFUL ###");
				itsTimerPort->setTimer(1.0);
				break;
			}
			LOG_INFO("[R]READ part was successful, waiting for writer to finish");
		}
		break;
	
	case F_TIMER:
		GCFScheduler::instance()->stop();
		break;

	default:
		return(GCFEvent::NOT_HANDLED);
	}

	return (GCFEvent::HANDLED);
}

//
// tReader readTest (event, port)
//
GCFEvent::TResult tReader::readTest(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("[R]readTest:" << eventName(e));

	switch (e.signal) {
	case F_ENTRY:
		itsTimerPort->setTimer(5.0);
	break;

	case F_TIMER:
		LOG_FATAL("[R]Readtest FAILED");
		TRAN(tReader::final);
	break;

	case KVT_REGISTER: {
		KVTRegisterEvent	msg(e);
		LOG_INFO_STR("obsID = " << msg.obsID);
		LOG_INFO_STR("name  = " << msg.name);
		ASSERTSTR(msg.obsID == 25002, "ObsID is wrong, expected 25002 iso " << msg.obsID);
		ASSERTSTR(msg.name == "This is a test string to test the RTDBPort interface", 
						"name is wrong, expected 'This is a test string to test the RTDBPort interface' iso " << msg.obsID);
		TRAN(tReader::closeTest);
	}
	break; 

	case F_EXIT:
		itsRTDBPort->cancelAllTimers();
		break;

	default:
		return(GCFEvent::NOT_HANDLED);
		break;
	}

	return (GCFEvent::HANDLED);
}

//
// tReader closeTest (event, port)
//
GCFEvent::TResult tReader::closeTest(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("[R]closeTest:" << eventName(e));

	switch (e.signal) {
	case F_ENTRY: {
		itsRTDBPort->close();
		itsTimerPort->setTimer(2.0);
	}
	break;

	case F_TIMER:
		LOG_FATAL("[R]closeTest FAILED");
		TRAN(tReader::final); 
	break;

	case F_DISCONNECTED:
		LOG_INFO("[R]closeTest was also successful.");
		readerReady = true;
		TRAN(tReader::final);
		break;

	case F_EXIT:
		itsTimerPort->cancelAllTimers();
		break;

	default:
		return(GCFEvent::NOT_HANDLED);
	}

	return (GCFEvent::HANDLED);
}

  } // namespace RTDB
 } // namespace GCF
} // namespace LOFAR


using namespace LOFAR::GCF;

int main(int argc, char* argv[])
{
	TM::GCFScheduler::instance()->init(argc, argv);

	RTDB::tWriter writer_task("writer_task");  
	writer_task.start(); // make initial transition
	RTDB::tReader reader_task("reader_task");  
	reader_task.start(); // make initial transition

	TM::GCFScheduler::instance()->run();

	return 0;
}
