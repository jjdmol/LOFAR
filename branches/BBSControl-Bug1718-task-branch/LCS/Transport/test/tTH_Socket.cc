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

#include <lofar_config.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_iostream.h>
#include <Common/LofarLogger.h>
#include <Transport/TH_Socket.h>
#include "DH_Socket.h"

using namespace LOFAR;

static bool	serverRole;
static bool	listenerAtDest;
static bool	blocking;
static bool	bidirectional;

#define	BUF_SZ		256

namespace LOFAR
{
int32	sendData(DH_Socket&	outDH, bool		reverseOrder)
{
	uchar		*buffer;
	
	outDH.setBufferSize(BUF_SZ);
	outDH.setCounter(BUF_SZ);
	buffer = outDH.getBuffer();

	// fill buffer
	for (int16 i = 0; i < BUF_SZ; i++) {
		buffer[i] = (reverseOrder) ? BUF_SZ - i : i;
	}

	LOG_INFO(formatString("Sending data %s", reverseOrder ? "in reverse order" :""));
	outDH.write();

	return (0);
}

int32 readAndCheckData(DH_Socket&	inDH, bool		reverseOrder)
{
	// receive data
	LOG_INFO(formatString("Receiving %sdata", reverseOrder ? "reverse ordered " :""));
	inDH.setBufferSize(BUF_SZ);
	while (!inDH.read()) {
		sleep(1);
	}

	const uchar* c = static_cast<uchar*>(inDH.getBuffer());
	for (int16 i = 0; i < BUF_SZ; i++) {
		if (c[i] != static_cast<uchar>((reverseOrder) ? BUF_SZ - i : i)) {
			LOG_FATAL(formatString ("Bytenr %d is 0x%02X and should be 0x%02X",
								  i, c[i], (reverseOrder) ? BUF_SZ - i : i));
			LOG_INFO("Test failed!");
			return (1);
		}
	}

	LOG_INFO("Received data is OK");
	return (0);
}

void showSocketInfo(DH_Socket	&DH)
{
	LOG_INFO_STR("DH name is " << DH.getName());	
	LOG_INFO(formatString("DH state is %s blocking", DH.isBlocking() ? "" : "NON"));	

}

int32 doTest(int32	testnr) 
{
	DH_Socket		DH_Source ("dhSource", 1);
	DH_Socket		DH_Dest   ("dhDest", 1);

	DH_Source.setID(1);
	DH_Dest.setID(2);

	if (!bidirectional) {
		TH_Socket	TCPproto1("localhost", "localhost", 3850+testnr, 
													listenerAtDest, blocking);
		DH_Source.connectTo (DH_Dest, TCPproto1, blocking);
	}
	else {
		TH_Socket	TCPproto1("localhost", "localhost", 3850+testnr, 
													!listenerAtDest, blocking);
		TH_Socket	TCPproto2("localhost", "localhost", 3850+testnr, 
													listenerAtDest, blocking);
		DH_Source.connectBidirectional(DH_Dest, TCPproto1, TCPproto2, blocking);
	}

	LOG_INFO("Setting up connection...");
	if (serverRole) {
		while(!DH_Dest.init()) {
			LOG_INFO("no connection yet, retry in 1 second");
			sleep(1);
		}
	}
	else {
		while(!DH_Source.init()) {
			LOG_INFO("no connection yet, retry in 1 second");
			sleep(1);
		}
	}
	LOG_INFO("Connection made succesfully");

	int32	result = 0;
	if (serverRole) {
		result = readAndCheckData(DH_Dest, false);		// data in normal order
		if (bidirectional) {
			LOG_INFO("Sleeping for 5 seconds to test async comm");
			sleep(5);					// be sure read is active for max errors
			result += sendData(DH_Dest, true);			// data in reverse order
		}
	}
	else {
		result = sendData(DH_Source, false);			// data in normal order
		if (bidirectional) {
			result += readAndCheckData(DH_Source, true);// data in reverse order
		}
	}

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
		serverRole = true;
	}
	else {
		if (!strcmp(argv[1], "-c")) {
			serverRole = false;
		}
		else {
			cout << "ERROR: Parameter must be -s or -c" << endl;
			usageMessage(argv[0]);
			return (1);
		}
	}
	
	INIT_LOGGER(argv[0]);
	int32	result = 0;
	for (int32 testnr = 1; testnr <= 8; testnr++) {
		listenerAtDest = (testnr % 2 == 1);
		blocking	   = (testnr < 5);
		bidirectional  = (testnr % 4 == 3) || (testnr % 4 == 0);

		LOG_INFO(formatString("===== TEST %d =====", testnr));
		LOG_INFO(formatString("%s, %s listener, %sblocking, %sdirectional",
			serverRole ? "server" : "client",
			(serverRole == listenerAtDest) ? "starting" : "no",
			blocking ? "" : "non",
			bidirectional ? "bi" : "uni"));
	
		sleep(2);
		result += doTest(testnr);
	}

	return (result);
}

#endif
