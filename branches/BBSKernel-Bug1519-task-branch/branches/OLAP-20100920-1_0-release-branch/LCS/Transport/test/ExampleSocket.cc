//# tTH_Socket.cc: Tests the functionality of the TH_Socket class
//#
//# Copyright (C) 2002-2004
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifdef USE_NOSOCKETS
int main()
{
  return 3;
}

#else

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_iostream.h>
#include <Common/StringUtil.h>
#include <Common/LofarLogger.h>
#include <Transport/TH_Socket.h>
#include <Transport/Connection.h>
#include <DH_Socket.h>

using namespace LOFAR;

static bool			readerRole;
static bool			listenerAtDest;
static bool			blocking;
static bool			bidirectional;
static DH_Socket	*theDH = 0;

#define	BUF_SZ		256000

namespace LOFAR
{

//
// sendData(outConn, reverseOrder)
//
int32	sendData(Connection&	outConn, bool		reverseOrder)
{
	uchar		*buffer;
	
	LOG_TRACE_COND("ResizingBuffer");
	theDH->setBufferSize(BUF_SZ);
	theDH->setCounter(BUF_SZ);
	buffer = theDH->getBuffer();

	// fill buffer
	LOG_TRACE_COND("Filling buffer");
	for (int32 i = 0; i < BUF_SZ; i++) {
		buffer[i] = (((reverseOrder) ? BUF_SZ - i : i) & 0xFF);
	}

	LOG_INFO(formatString("Sending data %s", 
						  reverseOrder ? "in reverse order" :""));
	outConn.write();
	outConn.waitForWrite();
	return (0);
}

//
// readAndCheckData(outConn, reverseOrder)
//
int32 readAndCheckData(Connection&	inConn, bool reverseOrder)
{
	// receive data
	LOG_INFO(formatString("Receiving %sdata", 
									reverseOrder ? "reverse ordered " :""));
	theDH->setBufferSize(BUF_SZ);

	// Read the data and be sure everthing is received
#if YOU_WANT_TO_WAIT_BLOCKING_IN_WAIT_FOR_READ
	inConn.read();
	inConn.waitForRead();
#else
	int32 count = 1;
	while(!inConn.read()) {
		count++;
	}
	LOG_INFO_STR("Data read in " << count << " tries");
#endif

	// Check the contents of the buffer.
	const uchar* c = static_cast<uchar*>(theDH->getBuffer());
	for (int32 i = 0; i < BUF_SZ; i++) {
		if (c[i] != static_cast<uchar>
							((((reverseOrder) ? BUF_SZ - i : i) & 0xFF))) {
			LOG_FATAL(formatString ("Bytenr %d is 0x%02X and should be 0x%02X",
					  i, c[i], (((reverseOrder) ? BUF_SZ - i : i) & 0xFF)));
			LOG_INFO("Test failed!");
			return (1);
		}
	}

	LOG_INFO("Received data is OK");
	return (0);
}

//
// doTest(testnr)
//
// Four important (global) flags determine the behaviour of the test:
//
// readerRole		true = start with reading;   false = start with writing
// listenerAtDest	true = don't start listener; false = start listener
// bidirectional	true = do read+write test;   false = one way test
// blocking			true = sync. communication;  false = async. communication
int32 doTest(int32	testnr) 
{
	// Init dataholder
	theDH = new DH_Socket("theDH", 1);	
	theDH->init();

	// Create TCP socket.
	string		service(formatString("%d", testnr+3850));
	TH_Socket	*testSocket;
	int32		result;

	if (readerRole == listenerAtDest) {
		// open server socket, but force that client is first
		sleep(3);
		testSocket = new TH_Socket(service, blocking);
	}
	else {
		// open client socket
		testSocket = new TH_Socket("localhost", service, blocking);
	}

	// make the connection
	LOG_INFO("Setting up connection...");
	while (!testSocket->init()) {
		LOG_INFO("no connection yet, retry in 1 second");
		sleep (1);
	}
	LOG_INFO("Connection made succesfully");

	// Do the test
	if (readerRole) {
		Connection	readConn ("read" , 0, theDH, testSocket);
		LOG_TRACE_COND ("About to read data");
		result = readAndCheckData(readConn, false);// data in normal order

		if (bidirectional) {
			Connection	writeConn("write", theDH, 0, testSocket);
			LOG_INFO("Sleeping for 5 seconds to test async comm");
			sleep(5);					// be sure read is active for max errors
			LOG_TRACE_COND ("About to write data");
			result += sendData(writeConn, true);	    // data in reverse order
		}
	}
	else {	// writerRole
		Connection	writeConn("write", theDH, 0, testSocket);
		LOG_TRACE_COND ("About to write data");
		result = sendData(writeConn, false);		// data in normal order

		if (bidirectional) {
			Connection	readConn ("read", 0, theDH, testSocket);
			LOG_TRACE_COND ("About to read data");
			result += readAndCheckData(readConn, true);// data in reverse order
		}
	}

	delete theDH;
	theDH = 0;

	delete testSocket;
	testSocket = 0;

	if (result == 0) {
		LOG_INFO ("Test passed succesful!");
	}
	return (result);
}

void usageMessage(char	*progName)
{
	cout << "Usage: " << progName << " -s|-c" << endl;
}


} // namespace LOFAR

int main (int32 argc, char*	argv[]) {

	// check invocation
	if (argc != 2) {
		usageMessage(argv[0]);
		return (1);
	}

	// check first parameter
	if (!strcmp(argv[1], "-s")) {
		readerRole = true;
	}
	else {
		if (!strcmp(argv[1], "-c")) {
			readerRole = false;
		}
		else {
			cout << "ERROR: Parameter must be -s or -c" << endl;
			usageMessage(argv[0]);
			return (1);
		}
	}
	
	INIT_LOGGER(argv[0]);

#ifdef HAVE_BGL
	LOG_WARN("These tests cannot be run unmodified on a Blue Gene/L");
	return 3;
#endif

	LOG_INFO("Executing 8 tests with changing blockingmode, listenerside and uni-/bidirectional mode");
	LOG_INFO_STR("Buffer exchanged is " << BUF_SZ << " bytes tall");

	int32	result = 0;
	for (int32 testnr = 1; testnr <= 8; testnr++) {
		listenerAtDest = (testnr % 2 == 1);
		blocking	   = (testnr < 5);
		bidirectional  = (testnr % 4 == 3) || (testnr % 4 == 0);

		LOG_INFO(formatString("===== TEST %d =====", testnr));
		LOG_INFO(formatString("%s, %s listener, %sblocking, %sdirectional",
			readerRole ? "server" : "client",
			(readerRole == listenerAtDest) ? "starting" : "no",
			blocking ? "" : "non",
			bidirectional ? "bi" : "uni"));
	
		sleep(2);
		result += doTest(testnr);
	}

	return (result);
}

#endif
