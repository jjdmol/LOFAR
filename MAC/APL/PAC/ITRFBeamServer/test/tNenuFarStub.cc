//
//  tNenuFarStub.cc: Implements an echo server
//
//  Copyright (C) 2014
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
//  $Id: tNenuFarStub.cc 23533 2013-01-22 15:38:33Z overeem $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <Common/hexdump.h>
#include "../src/NenuFarMsg.h"
#include "tNenuFarStub.h"

#include <MACIO/GCF_Event.h>
#include <GCF/TM/GCF_Control.h>

using namespace LOFAR;
using namespace BS;
using namespace LOFAR::GCF;
using namespace LOFAR::GCF::TM;

tNenuFarStub::tNenuFarStub(string name, int	portNr) : 
	GCFTask((State)&tNenuFarStub::initial, name),
	itsListener(0),
	itsTimerPort(0),
	itsPortNumber(portNr)
{
	itsListener = new GCFTCPPort(*this, "NenuFarStub", GCFPortInterface::MSPP, 0);
	ASSERTSTR(itsListener, "failed to alloc listener");

	itsTimerPort = new GCFTimerPort(*this, "timerPort");
	ASSERTSTR(itsTimerPort, "failed to alloc listener");
}

GCFEvent::TResult tNenuFarStub::initial(GCFEvent& event, GCFPortInterface& /*port*/)
{
	switch (event.signal) {
	case F_INIT: {
		LOG_DEBUG("STARTING server");
		itsListener->setHostName("localhost");
		itsListener->setPortNumber(itsPortNumber);
		itsListener->open();
	} break;

    case F_CONNECTED:
		if (itsListener->isConnected()) {
			TRAN(tNenuFarStub::connected);
		}
		break;

    case F_DISCONNECTED:
		ASSERTSTR(false, "Bailing out because server could not be started");
		GCFScheduler::instance()->stop();
		break;

    default:
		LOG_DEBUG_STR ("default:" << eventName(event));
		break;
	}

	return (GCFEvent::HANDLED);
}

GCFEvent::TResult tNenuFarStub::connected(GCFEvent& event, GCFPortInterface& port)
{
	switch (event.signal) {
	case F_ACCEPT_REQ: {
		GCFTCPPort* client = new GCFTCPPort();
		client->init(*this, "client", GCFPortInterface::SPP, 0, true); //raw
		if (!itsListener->accept(*client)) {
			delete client;
		} else {
			LOG_INFO("NEW CLIENT CONNECTED");
		}
	}
	break;

	case F_DATAIN: {
		char	buf[1600];
		ssize_t btsRead = port.recv((void*)&buf[0], 1600);
		buf[btsRead] = '\0';
//		string  s;
//		hexdump(s, buf, btsRead);
//		LOG_INFO_STR("Received command on feedback port: " << s);

		ParameterSet	request = NenuFarMsg::unpack2parset(buf, btsRead+1);
		LOG_INFO_STR("Received:\n" << request);
		string	msgtype;

		// handle special test requests.
		if (request.isDefined("no_answer")) {
			LOG_INFO("REQUESTED FAILURE: no answer");
			return (GCFEvent::HANDLED);
		}
		if (request.isDefined("delay")) {
			int  delay = request.getInt("delay");
			LOG_INFO_STR("REQUESTED DELAY: " << delay << " seconds...");
			sleep (delay);
		}
		if (request.isDefined("presume_msgtype")) {
			msgtype = request.getString("presume_msgtype");
			LOG_INFO_STR("REQUESTED WRONG ANSWERTYPE: " << msgtype);
		}
		if (msgtype.empty()) {
			msgtype = request.getString("msgtype");
		}
		if (msgtype == "ABORT_ALL_BEAMS") {
			return (GCFEvent::HANDLED);
		}

		ParameterSet	answer;
		answer.add("beamName", request.getString("beamName"));
		NenuFarMsg*		msg;
		if (msgtype == "NEW_BEAM") {
			answer.add("accepted", "Yes");
			msg = new NenuFarMsg(0x0100, NEW_BEAM_ACK_MSG, answer);
		}
		else if (msgtype == "STOP_BEAM") {
			answer.add("result", "0");
			msg = new NenuFarMsg(0x0100, STOP_BEAM_ACK_MSG, answer);

		}
		else if (msgtype == "ABORT_BEAM") {
			answer.add("result", "0");
			msg = new NenuFarMsg(0x0100, ABORT_BEAM_ACK_MSG, answer);
		}
		else {
			LOG_ERROR_STR("Received invalid command: " << msgtype);
		}
		LOG_INFO_STR("Sending answer..." << *msg);
		((GCFTCPPort*)&port)->send(msg->data(), msg->size());
		delete msg;
	} break;

    case F_DISCONNECTED:
		LOG_INFO_STR("SERVER received 'disconnect', closing port");
		port.close();
//		TRAN(tNenuFarStub::initial);	// hope this will work...
		break;

	default:
		LOG_DEBUG_STR ("default:" << eventName(event) << "@" << port.getName());
	} 

	return (GCFEvent::HANDLED);
}


int main(int argc,	char*	argv[]) 
{
	GCFScheduler::instance()->init(argc, argv, LOFAR::basename(argv[0]));

	tNenuFarStub	stub("NenuFarStub",1234);
	stub.start();

	GCFScheduler::instance()->run();

	return (0);
}


