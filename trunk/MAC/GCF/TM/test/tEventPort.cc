//#  tEventPort.cc: Program to test the EventPort class
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
#include <GCF/TM/EventPort.h>
#include <GCF/GCF_ServiceInfo.h>
#include "Echo_Protocol.ph"

using namespace LOFAR;
using namespace LOFAR::GCF;
using namespace LOFAR::GCF::TM;

int main (int32	argc, char*argv[]) {

	INIT_LOGGER("tEventPort");
	
	EventPort	echoPort("ECHO:EchoServer", false, ECHO_PROTOCOL, "", true);	// syncComm

#if 0
	RSPGetconfigEvent   getConfig;
	rspPort.send(&getConfig);

	RSPGetconfigackEvent ack(rspPort.receive());
	cout << "NrRCUs       = " << ack.n_rcus << endl;
	cout << "NrRSPboards  = " << ack.n_rspboards << endl;
	cout << "MaxRSPboards = " << ack.max_rspboards << endl;
#else
	// note this code will not work but it compiles.
	LOG_DEBUG("going to create an event");
	EchoPingEvent		pingEvent;
	pingEvent.seqnr = 25;
	timeval		pingTime;
	gettimeofday(&pingTime, 0);
	pingEvent.ping_time = pingTime;

	LOG_DEBUG("going to send the event");
	echoPort.send(&pingEvent);

	LOG_DEBUG("going to wait for the answer event");
	EchoEchoEvent ack(*(echoPort.receive()));
	LOG_DEBUG_STR("seqnr: " << ack.seqnr);
	double	someTime;
	someTime = 1.0 * ack.ping_time.tv_sec + (ack.ping_time.tv_usec / 1000000);
	LOG_DEBUG_STR("ping : " << someTime);
	someTime = 1.0 * ack.echo_time.tv_sec + (ack.echo_time.tv_usec / 1000000);
	LOG_DEBUG_STR("pong : " << someTime);

#endif

	return (0);
}

