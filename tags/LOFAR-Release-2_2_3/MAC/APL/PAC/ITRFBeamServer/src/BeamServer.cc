//#
//#  BeamServer.cc: implementation of BeamServer class
//#
//#  Copyright (C) 2002-2009
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
#include <Common/LofarLocators.h>
#include <Common/LofarBitModeInfo.h>
#include <Common/lofar_complex.h>
#include <Common/Version.h>
#include <Common/ParameterSet.h>
#include <Common/StreamUtil.h>
#include <ApplCommon/StationConfig.h>
#include <ApplCommon/AntennaSets.h>

#include <MACIO/MACServiceInfo.h>

#include <APL/APLCommon/AntennaField.h>
#include <APL/IBS_Protocol/IBS_Protocol.ph>
#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/CAL_Protocol/CAL_Protocol.ph>

#include "BeamServer.h"
#include "BeamServerConstants.h"
#include <ITRFBeamServer/Package__Version.h>

#include <getopt.h>
#include <iostream>
#include <sstream>
#include <time.h>
#include <fstream>
#include <bitset>

#include <netinet/in.h>
#include <blitz/array.h>

using namespace blitz;
using namespace std;
namespace LOFAR {
  using namespace RTC;
  using namespace IBS_Protocol;
  using namespace RSP_Protocol;
  using namespace GCF::TM;
  namespace BS {

int	gBeamformerGain = 0;

//
// BeamServer(name)
//
BeamServer::BeamServer(const string& name, long	timestamp) : 
	GCFTask((State)&BeamServer::con2rspdriver, name),
	itsCurrentBitsPerSample (MAX_BITS_PER_SAMPLE),
	itsCurrentMaxBeamlets   (maxBeamlets(itsCurrentBitsPerSample)),
	itsNrLBAbeams			(0),
	itsNrHBAbeams			(0),
	itsListener				(0),
	itsRSPDriver			(0),
	itsCalServer			(0),
	itsBeamsModified		(false),
	itsAnaBeamMgr			(0),
	itsCalTableMode1		(0),
	itsCalTableMode3		(0),
	itsCalTableMode5		(0),
	itsCalTableMode6		(0),
	itsCalTableMode7		(0),
	itsUpdateInterval		(0),
	itsComputeInterval		(0),
	itsHBAUpdateInterval	(0),
	itsTestSingleShotTimestamp(timestamp)
{
	LOG_INFO(Version::getInfo<ITRFBeamServerVersion>("ITRFBeamServer"));

	// register protocols for debugging
	registerProtocol(IBS_PROTOCOL, IBS_PROTOCOL_STRINGS);
	registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_STRINGS);
	registerProtocol(CAL_PROTOCOL, CAL_PROTOCOL_STRINGS);

	// startup TCP connections
	itsListener  = new GCFTCPPort(*this, MAC_SVCMASK_BEAMSERVER, GCFPortInterface::MSPP, IBS_PROTOCOL);
	itsRSPDriver = new GCFTCPPort(*this, MAC_SVCMASK_RSPDRIVER,  GCFPortInterface::SAP,  RSP_PROTOCOL);
	itsCalServer = new GCFTCPPort(*this, MAC_SVCMASK_CALSERVER,  GCFPortInterface::SAP,  CAL_PROTOCOL);
	ASSERTSTR(itsListener,  "Cannot allocate port to start listener");
	ASSERTSTR(itsRSPDriver, "Cannot allocate port for RSPDriver");
	ASSERTSTR(itsCalServer, "Cannot allocate port for CalServer");

	// alloc a general purpose timer.
	itsDigHeartbeat = new GCFTimerPort(*this, "DigitalHeartbeatTimer");
	itsAnaHeartbeat = new GCFTimerPort(*this, "AnalogueHeartbeatTimer");
	itsConnectTimer = new GCFTimerPort(*this, "ConnectTimer");
	ASSERTSTR(itsDigHeartbeat, "Cannot allocate the digital heartbeat timer");
	ASSERTSTR(itsAnaHeartbeat, "Cannot allocate the analogue heartbeat timer");
	ASSERTSTR(itsConnectTimer, "Cannot allocate a general purpose timer");

	// Create casacore based converter to J2000
	itsJ2000Converter = new CasaConverter("J2000");
	ASSERTSTR(itsJ2000Converter, "Can't create a casacore converter to J2000");

	// read config settings
	itsSetHBAEnabled 	  = globalParameterSet()->getBool("BeamServer.EnableSetHBA", true);
	itsSetSubbandsEnabled = globalParameterSet()->getBool("BeamServer.EnableSetSubbands", true);
	itsSetWeightsEnabled  = globalParameterSet()->getBool("BeamServer.EnableSetWeights", true);
	itsStaticCalEnabled   = globalParameterSet()->getBool("BeamServer.EnableStaticCalibration", true);
	gBeamformerGain	      = globalParameterSet()->getInt32("BeamServer.BeamformerGain", 8000);
	if (!itsSetSubbandsEnabled) {
		LOG_WARN("Setting of subbands IS DISABLED!!!");
	}
	if (!itsSetWeightsEnabled) {
		LOG_WARN("Setting of weights IS DISABLED!!!");
	}

	// get interval information
	itsUpdateInterval    = globalParameterSet()->getInt32("BeamServer.UpdateInterval",  DIG_INTERVAL);
	itsComputeInterval   = globalParameterSet()->getInt32("BeamServer.ComputeInterval", DIG_COMPUTE_INTERVAL);
	itsHBAUpdateInterval = globalParameterSet()->getInt32("BeamServer.HBAUpdateInterval",  HBA_INTERVAL);
	if (itsHBAUpdateInterval < HBA_MIN_INTERVAL) {
		LOG_FATAL_STR("HBAUpdateInterval is too small, must be greater or equal to " << 
						HBA_MIN_INTERVAL << " seconds");
		exit(EXIT_FAILURE);
	}

	itsLBArcus.resize(MAX_RCUS, 0);
	itsHBArcus.resize(MAX_RCUS, 0);

	// read static calibrationtables if available
	StationConfig	SC;
	if (itsStaticCalEnabled) {
		_loadCalTable(1, SC.nrRSPs);
		_loadCalTable(3, SC.nrRSPs);
		_loadCalTable(5, SC.nrRSPs);
		_loadCalTable(6, SC.nrRSPs);
		_loadCalTable(7, SC.nrRSPs);
	}
	else {
		LOG_WARN("Static calibration is disabled!");
	}

}

//
// ~BeamServer
//
BeamServer::~BeamServer()
{
	if (itsDigHeartbeat)	{ delete itsDigHeartbeat; }
	if (itsAnaHeartbeat)	{ delete itsAnaHeartbeat; }
	if (itsCalServer)	 	{ delete itsCalServer; }
	if (itsRSPDriver)	 	{ delete itsRSPDriver; }
	if (itsListener)	 	{ delete itsListener; }
	if (itsAnaBeamMgr)		{ delete itsAnaBeamMgr; }
}

// ------------------------------ State machines ------------------------------

//
// con2rspdriver(event, port)
//
// Get connected to RSPDriver.
//
GCFEvent::TResult BeamServer::con2rspdriver(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("con2rspdriver:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch(event.signal) {
	case F_INIT: {
		ConfigLocator cl;
		// load the antennasets
		ASSERTSTR(globalAntennaSets(), "Could not load the antennasets file");
		LOG_DEBUG("Loaded AntennaSets file");

		// load the antenna positions
		ASSERTSTR(globalAntennaField(), "Could not load the antennaposition file");
		LOG_DEBUG("Loaded antenna postions file");
		LOG_DEBUG_STR("LBA rcu=" << globalAntennaField()->RCUPos("LBA"));
		LOG_DEBUG_STR("HBA rcu=" << globalAntennaField()->RCUPos("HBA"));
		LOG_DEBUG_STR("HBA0 rcu=" << globalAntennaField()->RCUPos("HBA0"));
		LOG_DEBUG_STR("HBA1 rcu=" << globalAntennaField()->RCUPos("HBA1"));
	} break;

	case F_ENTRY: {
		LOG_INFO("Waiting for connection with RSPDriver");
		itsRSPDriver->autoOpen(30, 0, 2);	// try max 30 times every 2 seconds than report DISCO
	}
	break;

	case F_CONNECTED: {
		LOG_INFO("Connected to RSPDriver, asking configuation");
		TRAN(BeamServer::askConfiguration);
	}
	break;

	case F_DISCONNECTED: {
		port.close();
		port.setTimer(10.0);
		LOG_INFO(formatString("port '%s' disconnected, retry in 10 seconds...", port.getName().c_str()));
	}
	break;

	case F_TIMER: {
		LOG_INFO_STR("port.getState()=" << port.getState());
		LOG_INFO(formatString("port '%s' retry to open...", port.getName().c_str()));
		itsRSPDriver->autoOpen(30, 0, 2);	// try max 30 times every 2 seconds than report DISCO
	}
	break;

	default:
		LOG_DEBUG("con2rspdriver:default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}
//
// askConfiguration(event, port)
//
// Ask the RSPdriver what hardware is available
//
GCFEvent::TResult BeamServer::askConfiguration(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("askConfiguration:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch(event.signal) {
	case F_ENTRY: {
		LOG_INFO("Asking RSPDriver the station configuation");
		RSPGetconfigEvent getconfig;	// ask RSPDriver current config
		itsRSPDriver->send(getconfig);
	}
	break;

	case RSP_GETCONFIGACK: {				// answer from RSPDriver
		RSPGetconfigackEvent ack(event);

		// resize our array to the amount of current RCUs
		itsMaxRCUs      = ack.n_rcus;
		itsMaxRSPboards = ack.max_rspboards;
		LOG_INFO_STR("Station has " << itsMaxRCUs << " RCU's and " << itsMaxRSPboards << " RSPBoards");

		itsConnectTimer->cancelAllTimers();
		TRAN(BeamServer::subscribeSplitter);
	}
	break;

	case F_DISCONNECTED: {
		port.close();
		if (&port == itsRSPDriver) {
			LOG_WARN("Lost connection with the RSPDriver, going back to the reconnect state");
			itsConnectTimer->cancelAllTimers();
			TRAN(BeamServer::con2rspdriver);
		}
	}
	break;

	default:
		LOG_DEBUG("askConfiguration:default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

//
// subscribeSplitter(event, port)
//
// Get connected to RSPDriver.
//
GCFEvent::TResult BeamServer::subscribeSplitter(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("subscribeSplitter:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch(event.signal) {
	case F_ENTRY: {
		// send request for splitter info
		LOG_INFO("Requesting a subscription on the splitter state");
		RSPSubsplitterEvent		subSplitter;
		subSplitter.period = 1;
		itsRSPDriver->send(subSplitter);
		itsConnectTimer->setTimer(5.0);
		// wait for ack message
	}
	break;

	case RSP_SUBSPLITTERACK: {
		itsConnectTimer->cancelAllTimers();
		RSPSubsplitterackEvent	ack(event);
		if (ack.status != RSP_SUCCESS) {
			LOG_INFO("Could not get a subscription on the splitter, retry in 5 seconds");
		}
		itsConnectTimer->setTimer(5.0);
		// wait for update message.
	}
	break;

	case RSP_UPDSPLITTER: {
		itsConnectTimer->cancelAllTimers();
		RSPUpdsplitterEvent		answer(event);
		if (answer.status != RSP_SUCCESS) {
			LOG_INFO("Could not get a subscription on the splitter, retry in 5 seconds");
			itsConnectTimer->setTimer(5.0);
			break;
		}

		itsSplitterOn = answer.splitter[0];
		LOG_INFO_STR("The ringsplitter is " << (itsSplitterOn ? "ON" : "OFF"));
		_createBeamPool();		// (re)allocate memory for the beamlet mapping

		TRAN(BeamServer::subscribeBitmode);
//		TRAN(BeamServer::con2calserver);
	}
	break;

	case F_TIMER: {
		LOG_INFO("Requesting a subscription on the splitters again.");
		RSPSubsplitterEvent		subSplitter;
		subSplitter.period = 1;
		itsRSPDriver->send(subSplitter);
    }
    break;

	case F_DISCONNECTED: {
		port.close();
		if (&port == itsRSPDriver) {
			LOG_WARN("Lost connection with the RSPDriver, going back to the reconnect state");
			itsConnectTimer->cancelAllTimers();
			TRAN(BeamServer::con2rspdriver);
		}
	}
	break;

	default:
		LOG_DEBUG("subscribeSplitter:default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

#if 1
//
// subscribeBitmode(event, port)
//
// Take subscription on changes in the bitmode
//
GCFEvent::TResult BeamServer::subscribeBitmode(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("subscribeBitmode:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch(event.signal) {
	case F_ENTRY: {
		// send request for splitter info
		LOG_INFO("Requesting a subscription on the bitmode");
		RSPSubbitmodeEvent		subBitmode;
		subBitmode.period = 1;
		itsRSPDriver->send(subBitmode);
		itsConnectTimer->setTimer(5.0);
		// wait for update event.
	}
	break;

	case RSP_SUBBITMODEACK: {
		itsConnectTimer->cancelAllTimers();
		RSPSubbitmodeackEvent		ack(event);
		if (ack.status != RSP_SUCCESS) {
			LOG_INFO("Could not get a subscription on the bitmode, retry in 5 seconds");
			break;
		}
		itsConnectTimer->setTimer(5.0);
		// wait for update event.
	}
	break;

	case RSP_UPDBITMODE: {
		itsConnectTimer->cancelAllTimers();
		RSPUpdbitmodeEvent		answer(event);
		if (answer.status != RSP_SUCCESS) {
			LOG_INFO("Could not get a subscription on the bitmode, retry in 5 seconds");
			itsConnectTimer->setTimer(5.0);
			break;
		}

		itsCurrentBitsPerSample = MIN_BITS_PER_SAMPLE;
		for (uint i = 0; i < itsMaxRSPboards; i++) {
			itsCurrentBitsPerSample = (answer.bits_per_sample[i] > itsCurrentBitsPerSample) ? answer.bits_per_sample[i] : itsCurrentBitsPerSample;
		}
		itsCurrentMaxBeamlets = maxBeamlets(itsCurrentBitsPerSample);
		LOG_INFO_STR("The bitmode is " << itsCurrentBitsPerSample << " bits");

		_createBeamPool();		// (re)allocate memory for the beamlet mapping

		TRAN(BeamServer::con2calserver);
	}
	break;

	case F_TIMER: {
		LOG_INFO("Requesting a subscription on the bitmode again.");
		RSPSubbitmodeEvent		subBitmode;
		subBitmode.period = 1;
		itsRSPDriver->send(subBitmode);
    }
    break;

	case F_DISCONNECTED: {
		port.close();
		if (&port == itsRSPDriver) {
			LOG_WARN("Lost connection with the RSPDriver, going back to the reconnect state");
			itsConnectTimer->cancelAllTimers();
			TRAN(BeamServer::con2rspdriver);
		}
	}
	break;

	default:
		LOG_DEBUG("subscribeBitmode:default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}
#endif

//
// con2calserver(event, port)
//
// Get connected to CalServer.
//
GCFEvent::TResult BeamServer::con2calserver(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("con2calserver:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch(event.signal) {
	case F_ENTRY: {
		itsCalServer->open();
	}
	break;

	case F_CONNECTED: {
		// start update timer and start accepting clients
		LOG_INFO_STR("Starting digital pointing timer with interval: " << itsUpdateInterval << " secs");
		itsDigHeartbeat->setTimer(0, 0, itsUpdateInterval, 0);
		if (itsSetHBAEnabled) {
			LOG_INFO_STR("Starting analogue pointing timer with interval: " << itsHBAUpdateInterval << " secs");
			itsAnaHeartbeat->setTimer(2, 0, itsHBAUpdateInterval, 0);	// start 2 seconds later
		}
		else {
			LOG_WARN("Analogue beamforming of HBA tiles IS SWITCHED OFF!");
		}

		if ((itsListener->getState() != GCFPortInterface::S_CONNECTING) &&
			(itsListener->getState() != GCFPortInterface::S_CONNECTED)) {
			itsListener->open();
		}
		TRAN(BeamServer::enabled);
	}
	break;

	case F_DISCONNECTED: {
		port.close();
		port.setTimer(2.0);
		LOG_INFO(formatString("port '%s' disconnected, retry in 2 seconds...", port.getName().c_str()));
	}
	break;

	case F_TIMER: {
		LOG_INFO_STR("port.getState()=" << port.getState());
		LOG_INFO(formatString("port '%s' retry to open...", port.getName().c_str()));
		itsCalServer->open();
	}
	break;

	default:
		LOG_DEBUG("con2calserver:default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

//
// enabled(event, port)
//
// This is our main statemachine
//
GCFEvent::TResult BeamServer::enabled(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("enabled:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult	status = GCFEvent::HANDLED;
	static int 			weightsPeriod = 0;
	
	undertaker();	// remove dead clients from admin

	switch (event.signal) {
	case F_ENTRY:
		LOG_INFO("BeamServer is ready to accept beams.");
	break;

	case F_ACCEPT_REQ: {
		GCFTCPPort* client = new GCFTCPPort();
		client->init(*this, "client", GCFPortInterface::SPP, IBS_PROTOCOL);
		if (!itsListener->accept(*client)) {
			delete client;
		} else {
			itsClientList.push_back(client);
			LOG_INFO(formatString("NEW CLIENT CONNECTED: %d clients connected", 
																itsClientList.size()));
		}
	}
	break;

	case F_CONNECTED:
	break;

	case F_TIMER: {
		// Check if it is time to do some calculations
		if (&port == itsDigHeartbeat) {
			GCFTimerEvent* timer = static_cast<GCFTimerEvent*>(&event);
			LOG_DEBUG_STR("timer=" << Timestamp(timer->sec, timer->usec));
			
			weightsPeriod += itsUpdateInterval;
			if (weightsPeriod >= itsComputeInterval) {
				weightsPeriod = 0;
				// compute new weights and send the weights
				LOG_INFO_STR("Computing weights " << Timestamp(timer->sec, timer->usec));
				compute_weights(Timestamp(timer->sec + LEADIN_TIME, 0L));
				send_weights   (Timestamp(timer->sec + LEADIN_TIME, 0L));
			}
			
			if (itsBeamsModified) {
				send_sbselection();
				itsBeamsModified = false;
			}
			return (GCFEvent::HANDLED);
		} // port == itsDigHeartbeat

		if (&port == itsAnaHeartbeat) {
			GCFTimerEvent* timer = static_cast<GCFTimerEvent*>(&event);
			if (timer->sec - itsLastHBACalculationTime < HBA_MIN_INTERVAL) {
				LOG_INFO_STR("Skipping HBA delay calculations: too soon after previous run");
			}
			else {
				LOG_INFO_STR("Computing HBA delays " << Timestamp(timer->sec, timer->usec));
				itsAnaBeamMgr->calculateHBAdelays(Timestamp((long)timer->sec + LEADIN_TIME, 0L), *itsJ2000Converter);
				itsAnaBeamMgr->sendHBAdelays     (*itsRSPDriver);
				itsLastHBACalculationTime = timer->sec;
			}
			return (GCFEvent::HANDLED);
		}
		
		LOG_ERROR_STR("Unknown F_TIMER event occurred at port " << port.getName() << "! Ignoring it.");
	} // case
	break;

	// ---------- requests from the clients ----------
	case IBS_GETCALINFO: {
		IBSGetcalinfoackEvent	answer;
		ostringstream	oss;
		if (itsCalTableMode1)	oss << *itsCalTableMode1 << endl;
		if (itsCalTableMode3)	oss << *itsCalTableMode3 << endl;
		if (itsCalTableMode5)	oss << *itsCalTableMode5 << endl;
		if (itsCalTableMode6)	oss << *itsCalTableMode6 << endl;
		if (itsCalTableMode7)	oss << *itsCalTableMode7 << endl;
		answer.info = oss.str();
		port.send(answer);
	}
	break;

	case IBS_BEAMALLOC: {
		IBSBeamallocEvent 	allocEvent(event);
		LOG_DEBUG_STR("ALLOC event=" << allocEvent);
		if (beamalloc_start(allocEvent, port)) {
			LOG_INFO_STR("Beam allocation started, going to beamalloc_state");
			TRAN(BeamServer::beamalloc_state);
		}
	}
	break;

	case IBS_BEAMFREE: {
		IBSBeamfreeEvent 	freeEvent(event);
		if (beamfree_start(freeEvent, port)) {
			LOG_DEBUG_STR("Beam freeing started, going to beamfree_state");
			TRAN(BeamServer::beamfree_state);
		}
	}
	break;

	case IBS_POINTTO: {
		IBSPointtoEvent 	pointingEvent(event);
		IBSPointtoackEvent	ack;
		ack.beamName = pointingEvent.beamName;
		ack.pointing = pointingEvent.pointing;
		ack.analogue = pointingEvent.analogue;
		ack.status = beampointto_action (pointingEvent, port);
		port.send(ack);
	}
	break;

	// ---------- responses on the messages I sent ----------
	case RSP_SETHBAACK: {
		RSPSethbaackEvent ack(event);
		if (ack.status != RSP_Protocol::RSP_SUCCESS) {
			LOG_ERROR_STR("RSP_SETHBAACK: FAILURE(" << ack.status << ")");
		}
	}
	break;

	case RSP_SETWEIGHTSACK: {
		RSPSetweightsackEvent ack(event);
		if (ack.status != RSP_Protocol::RSP_SUCCESS) {
			LOG_ERROR_STR("RSP_SETWEIGHTSACK: FAILURE(" << ack.status << ")");
		}
	}
	break;

	case RSP_SETSUBBANDSACK: {
		RSPSetsubbandsackEvent ack(event);
		if (ack.status != RSP_Protocol::RSP_SUCCESS) {
			LOG_ERROR_STR("RSP_SETSUBBANDSACK: FAILURE(" << ack.status << ")");
		}
	}
	break;

	// ---------- update events from my subscriptions ----------
	case CAL_UPDATE: {
		CALUpdateEvent calupd(event);
		if (calupd.status != CAL_Protocol::CAL_SUCCESS) {
			LOG_INFO_STR("Received valid CAL_UPDATE event(" << calupd.status << ").");
// TODO			updateCalibration(calupd.handle, calupd.gains);
		}
	}
	break;

	case RSP_UPDSPLITTER: {
		RSPUpdsplitterEvent		answer(event);
		// TODO: don't ignore status field!
		itsSplitterOn = answer.splitter[0];
		LOG_INFO_STR("The ringsplitter is switched " << (itsSplitterOn ? "ON" : "OFF"));
		_createBeamPool();
	}
	break;

	case RSP_UPDBITMODE: {
		RSPUpdbitmodeEvent		answer(event);
		// TODO: don't ignore status field!
		itsCurrentBitsPerSample = MIN_BITS_PER_SAMPLE;
		for (uint i = 0; i < itsMaxRSPboards; i++) {
			itsCurrentBitsPerSample = (answer.bits_per_sample[i] > itsCurrentBitsPerSample) ? answer.bits_per_sample[i] : itsCurrentBitsPerSample;
		}
		itsCurrentMaxBeamlets = maxBeamlets(itsCurrentBitsPerSample);
		LOG_INFO_STR("The bitmode changed to " << itsCurrentBitsPerSample << " bits");
		_createBeamPool();
	}
	break;

	// ---------- connection administration ----------
	case F_DISCONNECTED: {
		if (&port == itsListener || &port == itsRSPDriver || &port == itsCalServer) {
			LOG_WARN("Lost connection with CalServer or RSPDriver, going to 'cleanup'");
			TRAN(BeamServer::cleanup);
		} else {	// some client
			port.close();
			destroyAllBeams(&port);
			itsClientList.remove(&port);
			itsDeadClients.push_back(&port);
		}
	}
	break;

	default:
		LOG_DEBUG("enabled:default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

//
// cleanup(event, port)
//
// Some fatal error occured, closed down everything and try to startup every thing again
//
GCFEvent::TResult BeamServer::cleanup(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("cleanup:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: {
		LOG_FATAL_STR("Serious trouble! Closing all connections and trying to restart...");
		// close connection with all clients.
		if (itsClientList.size() > 0) {
			for (list<GCFPortInterface*>::iterator it = itsClientList.begin();
													it != itsClientList.end(); it++) {
				if ((*it)->isConnected()) {
					(*it)->close();
					destroyAllBeams(*it);
					itsClientList.remove(*it);
					itsDeadClients.push_back(*it);
				}
			}
		}

		itsRSPDriver->cancelAllTimers();
		itsRSPDriver->close();
		itsListener->close();
		itsCalServer->close();
		itsDigHeartbeat->cancelAllTimers();
		itsAnaHeartbeat->cancelAllTimers();
		itsConnectTimer->cancelAllTimers();
		itsConnectTimer->setTimer(2.0);		// give ports time to close.
	}
	break;

	case F_TIMER:
		LOG_DEBUG("closed all TCP ports, going to 'con2rspdriver' state for restart");
		TRAN(BeamServer::con2rspdriver);
	break;

	case F_EXIT:
		itsConnectTimer->cancelAllTimers();
		break;

	default:
		LOG_DEBUG("cleanup:default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

//
// beamalloc_state(event, port)
//
// Allocation of the new beam is technically possible, get permission of the calserver
//
GCFEvent::TResult BeamServer::beamalloc_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("beamalloc_state:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		// subscribe to calibration updates
		CALSubscribeEvent subscribe;
		subscribe.name 		 = itsBeamTransaction.getBeam()->name();
		subscribe.subbandset = itsBeamTransaction.getBeam()->allocation().getSubbandBitset();

		LOG_INFO_STR("Subscribing to subarray: " << subscribe.name);
		LOG_DEBUG_STR("subbands= " << subscribe.subbandset);
		itsCalServer->send(subscribe);
		itsConnectTimer->setTimer(5.0);
	}
	break;

	case CAL_SUBSCRIBEACK: {
		itsConnectTimer->cancelAllTimers();
		CALSubscribeackEvent ack(event);
		IBSBeamallocackEvent beamallocack;

		if (ack.status == CAL_Protocol::CAL_SUCCESS && 
				(ack.subarray.getSPW().getNumSubbands() >= 
				(int)itsBeamTransaction.getBeam()->allocation()().size()) ) {

			LOG_INFO_STR("Got subscription to subarray " << ack.subarray.getName());
			LOG_DEBUG_STR("ack.subarray.positions=" << ack.subarray.getAntennaPos());

			// set positions on beam
			DigitalBeam*	theBeam = itsBeamTransaction.getBeam();
//TODO			theBeam->setSubarray(ack.subarray);

			// set the scale of the beamlets: (-2PI * freq * i)/lightspeed
			_scaleBeamlets(theBeam->allocation(), theBeam->ringNr(), ack.subarray.getSPW());

			// set calibration handle for this beam
//TODO		// future ITRF CalServer will not return a handle, it will use the beamName we passed to it.
			itsBeamTransaction.getBeam()->calibrationHandle(ack.handle);

			// send succesful ack
			beamallocack.status 	  = IBS_Protocol::IBS_NO_ERR;
			beamallocack.antennaGroup = ack.subarray.getName();
			beamallocack.beamName 	  = itsBeamTransaction.getBeam()->name();
			itsBeamTransaction.getPort()->send(beamallocack);

			// add beam to our pool
			LOG_DEBUG_STR("Adding beam " << itsBeamTransaction.getBeam()->name() << " to the pool");
			itsBeamPool[itsBeamTransaction.getBeam()->name()] = itsBeamTransaction.getBeam();
		} 
		else {
			LOG_ERROR("Failed to subscribe to subarray");
			LOG_DEBUG_STR("getNumSubbands()    = " << ack.subarray.getSPW().getNumSubbands());
			LOG_DEBUG_STR("allocation.size()= " << (int)itsBeamTransaction.getBeam()->allocation()().size());

			// failed to subscribe
			beamallocack.status 	  = IBS_BEAMALLOC_ERR;
			beamallocack.antennaGroup = "";
			beamallocack.beamName 	  = itsBeamTransaction.getBeam()->name();
			itsBeamTransaction.getPort()->send(beamallocack);

			// delete the beam (resets itsBeamTransaction)
			deleteTransactionBeam();
		}

		LOG_DEBUG("Allocation finished, going back to 'enabled' state");
		TRAN(BeamServer::enabled);
	}
	break;

	case F_TIMER: {
		if (&port != itsConnectTimer) {
			// Timer event may be from one of the pointing timers, ignore them
			LOG_INFO_STR(">>> TimerEvent on port " << port.getName() << " while in alloc state, ignoring it");
			return (GCFEvent::HANDLED);
//			return ((&port==itsAnaHeartbeat) ? GCFEvent::NEXT_STATE : GCFEvent::HANDLED); TODO: FIX THIS
		}
		IBSBeamallocackEvent beamallocack;
		LOG_ERROR_STR("Timeout on starting the calibration of beam " << itsBeamTransaction.getBeam()->name());
		beamallocack.status       = IBS_BEAMALLOC_ERR;
		beamallocack.antennaGroup = "";
		beamallocack.beamName     = itsBeamTransaction.getBeam()->name();
		itsBeamTransaction.getPort()->send(beamallocack);

		// delete the beam (resets itsBeamTransaction)
		deleteTransactionBeam();
		TRAN(BeamServer::enabled);
	}
	break;

	case F_DISCONNECTED: {
		LOG_INFO(">>> deferring F_DISCONNECTED event <<<");
		if (&port == itsBeamTransaction.getPort()) {
			LOG_WARN("Lost connection, going back to 'enabled' state");
			TRAN(BeamServer::enabled);
		}
		queueTaskEvent(event, port);
		return (GCFEvent::HANDLED);
//		return (GCFEvent::NEXT_STATE);
	}
	break;

	case F_EXIT: {
		// completed current transaction, reset
		itsBeamTransaction.reset();
		itsConnectTimer->cancelAllTimers();
	}
	break;

	default:
		LOG_DEBUG("beamalloc_state:default --> defer command");
		// all other events are handled in the enabled state
		return (GCFEvent::NEXT_STATE);
	}

	return (GCFEvent::HANDLED);
}

//
// beamfree_state(event, port)
//
// Client wants to stop the beam, tell the CalServer to do so.
//
GCFEvent::TResult BeamServer::beamfree_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("beamfree_state:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		// unsubscribe
		CALUnsubscribeEvent unsubscribe;
		unsubscribe.name 	= itsBeamTransaction.getBeam()->name();
		unsubscribe.handle  = itsBeamTransaction.getBeam()->calibrationHandle();
//TODO	Handle will be replaced by beamName for ITRF CalServer
		ASSERT(unsubscribe.handle != 0);
		itsCalServer->send(unsubscribe);
		itsConnectTimer->setTimer(5.0);
	}
	break;

	case CAL_UNSUBSCRIBEACK: {
		CALUnsubscribeackEvent	ack(event);
		IBSBeamfreeackEvent 		beamfreeack;

		// if the subarray disappeared because
		// it was stopped (CAL_STOP by SRG) then
		// issue a warning but continue
		if (ack.status != CAL_Protocol::CAL_SUCCESS) {
			LOG_WARN_STR("CAL_UNSUBSCRIBE failed, status = " << ack.status);
		}

		// send succesful ack
		beamfreeack.status   = IBS_Protocol::IBS_NO_ERR;
		beamfreeack.beamName = itsBeamTransaction.getBeam()->name();
		itsBeamTransaction.getPort()->send(beamfreeack);

		// destroy beam, updates itsBeamTransaction
		deleteTransactionBeam();

		LOG_DEBUG("Beamfree finished, going back to 'enabled' state");
		TRAN(BeamServer::enabled);
	}
	break;

	case F_TIMER: {
		if (&port != itsConnectTimer) {
			// Timer event may be from one of the pointing timers, ignore them
			LOG_INFO_STR(">>> TimerEvent on port " << port.getName() << " while in free state, ignoring it");
			return (GCFEvent::HANDLED);
//			return ((&port==itsAnaHeartbeat) ? GCFEvent::NEXT_STATE : GCFEvent::HANDLED); TODO: FIX THIS
		}
		LOG_DEBUG_STR("Timeout on ending subscription of beam " << itsBeamTransaction.getBeam()->name());
		TRAN(BeamServer::enabled);
	}
	break;

	case F_DISCONNECTED: {
		LOG_INFO(">>> deferring F_DISCONNECTED event <<<");
		if (&port == itsBeamTransaction.getPort()) {
			LOG_WARN("Lost connection, going back to 'enabled' state");
			TRAN(BeamServer::enabled);
		}
		queueTaskEvent(event, port);
		return (GCFEvent::HANDLED);
//		return (GCFEvent::NEXT_STATE);
	}
	break;

	case F_EXIT: {
		// delete beam if not already done so
		if (itsBeamTransaction.getBeam()) {
			deleteTransactionBeam();
		}

		// completed current transaction, reset
		itsBeamTransaction.reset();
		itsConnectTimer->cancelAllTimers();
	}
	break;

	default:
		LOG_DEBUG("beamfree_state:default --> defer command");
		// all other events are handled in the enabled state
		return (GCFEvent::NEXT_STATE);
	}

	return (GCFEvent::HANDLED);
}

// ------------------------------ internal routines ------------------------------

//
// beamalloc_start(AllocEvent, port)
//
bool BeamServer::beamalloc_start(IBSBeamallocEvent& ba,
								 GCFPortInterface& port)
{
	// allocate the beam
	int		beamError(IBS_NO_ERR);
	DigitalBeam* beam = checkBeam(&port, ba.beamName, ba.antennaSet, ba.allocation, ba.rcumask, ba.ringNr, ba.rcuMode, &beamError);

	if (!beam) {
		LOG_FATAL_STR("BEAMALLOC: failed to allocate beam " << ba.beamName << " on " << ba.antennaSet);

		IBSBeamallocackEvent ack;
		ack.beamName 	 = ba.beamName;
		ack.antennaGroup = ba.antennaSet;
		ack.status 		 = beamError;
		port.send(ack);

		return (false);
	}

	return (true);
}

//
// beamfree_start(freeEvent, port)
//
bool BeamServer::beamfree_start(IBSBeamfreeEvent&  bf,
								GCFPortInterface& port)
{
	map<string, DigitalBeam*>::iterator	beamIter = itsBeamPool.find(bf.beamName);
	if (beamIter == itsBeamPool.end()) { 
		LOG_FATAL_STR("BEAMFREE failed: beam '" << bf.beamName << "' does not exist");

		IBSBeamfreeackEvent ack;
		ack.beamName = bf.beamName;
		ack.status = IBS_Protocol::IBS_BEAMFREE_ERR;
		port.send(ack);

		return (false);
	}

	// remember on which beam we're working
	itsBeamTransaction.set(&port, beamIter->second);
	itsBeamTransaction.allocationDone();

	return (true);
}

//
// beampointto_action(pointEvent, port)
//
int BeamServer::beampointto_action(IBSPointtoEvent&		ptEvent,
								   GCFPortInterface&	/*port*/)
{
// TODO what about analogue(-only) beams???
	map<string, DigitalBeam*>::iterator	beamIter = itsBeamPool.find(ptEvent.beamName);
	if (beamIter == itsBeamPool.end()) {
		LOG_ERROR(formatString("BEAMPOINTTO: invalid beam '%s'", ptEvent.beamName.c_str()));
		return (IBS_UNKNOWN_BEAM_ERR);
	}

	// sanity check for reference system
	if (!itsJ2000Converter->isValidType(ptEvent.pointing.getType())) {
		LOG_ERROR_STR(ptEvent.pointing.getType() << " is not a valid reference system, pointing rejected");
		return (IBS_INVALID_TYPE_ERR);
	}

	LOG_INFO_STR("new pointing for " << (ptEvent.analogue ? "analogue" : "digital") << " beam " << beamIter->second->name() << ": " << ptEvent.pointing);

	//
	// If the time is not set, then activate the command
	// 2 * COMPUTE_INTERVAL seconds from now, because that's how
	// long it takes the command to flow through the pipeline.
	//
	Timestamp actualtime;
	actualtime.setNow(2 * DIG_COMPUTE_INTERVAL);
	if (ptEvent.pointing.time() == Timestamp(0,0)) {
		ptEvent.pointing.setTime(actualtime);
	}

	// TEMP CODE FOR EASY TESTING THE BEAMSERVER VALUES
	if (itsTestSingleShotTimestamp) {
		ptEvent.pointing.setTime(Timestamp(itsTestSingleShotTimestamp,0));
	}
	// END OF TEMP CODE

	LOG_DEBUG_STR("Starttime of pointing is " << ptEvent.pointing.time());
	if (!ptEvent.analogue) {
		return (beamIter->second->addPointing(ptEvent.pointing));
	}

	// The analogue must be started several seconds before the observationstart time because the I2C bus
	// needs several seconds to pass the information
	int		activationTime = _idealStartTime(Timestamp::now().sec(), 
				ptEvent.pointing.time(), HBA_MIN_INTERVAL, 
				itsLastHBACalculationTime+itsHBAUpdateInterval, HBA_MIN_INTERVAL, itsHBAUpdateInterval);
	int		timeShift(ptEvent.pointing.time().sec() - activationTime);
	// update pointing
	ptEvent.pointing.setTime(Timestamp(activationTime,0));
	if (ptEvent.pointing.duration() && timeShift) {
		ptEvent.pointing.setDuration(ptEvent.pointing.duration()+timeShift);
		LOG_INFO_STR("Extended duration of " << beamIter->second->name() << " with " << timeShift << " seconds because starttime was shifted");
	}

	// note we don't know if we added the beam before, just do it again and ignore returnvalue.
	itsAnaBeamMgr->addBeam(AnalogueBeam(ptEvent.beamName, beamIter->second->antennaSetName(), 
									beamIter->second->rcuMask(), ptEvent.rank));
	if (!itsAnaBeamMgr->addPointing(ptEvent.beamName, ptEvent.pointing)) {
		return (IBS_UNKNOWN_BEAM_ERR);
	}

	itsAnaHeartbeat->setTimer(activationTime - Timestamp::now().sec());

	LOG_INFO_STR("Analogue beam for beam " << beamIter->second->name() << " will be active at " << Timestamp(activationTime+HBA_MIN_INTERVAL, 0));
	LOG_INFO_STR("Analogue pointing for beam " << beamIter->second->name() << " will be send at " << Timestamp(activationTime, 0));
	LOG_DEBUG_STR("ptEvent.pointing.time()  =" << ptEvent.pointing.time());
	LOG_DEBUG_STR("itsLastHBACalculationTime=" << itsLastHBACalculationTime);
	LOG_DEBUG_STR("itsHBAUpdateInterval     =" << itsHBAUpdateInterval);
	LOG_DEBUG_STR("Timestamp::now().sec()   =" << Timestamp::now().sec());
	LOG_DEBUG_STR("activationTime           =" << activationTime);

	return (IBS_NO_ERR);
}

//
// _idealStartTime(now,t1,d1,t2,d2,p2)
//
// t1: start time of beam
// d1: period it takes to get the beam active
// t2: nexttime heartbeat will happen
// d2: period it takes to complete the heartbeat
// p2: interval of the heartbeat.
// Returns the time the activation of t1 should be started.
//
int	BeamServer::_idealStartTime (int now, int t1, int d1, int t2, int d2, int p2) const
{
	int	t1start = t1-d1;				// ideal starttime
	if (t1start < now) 					// not before now ofcourse
		t1start = now;
	int nearestt2 = (t1start<=t2 ? t2 : t2+((t1-t2)/p2)*p2);
	if (t1start > nearestt2 && t1start < nearestt2+d2)	// not during heartbeat period
		t1start = nearestt2;

	return (t1start);
}

//
// undertaker()
//
void BeamServer::undertaker()
{
	// cleanup old connections
	for (list<GCFPortInterface*>::iterator it = itsDeadClients.begin();
											it != itsDeadClients.end(); it++) {
		LOG_DEBUG_STR("undertaker: deleting '" << (*it)->getName() << "'");
		delete (*it);
	}
	itsDeadClients.clear();
}

// -------------------- beam administration --------------------
//
// _createBeamPool()
//
void BeamServer::_createBeamPool()
{
	LOG_INFO_STR("Creating new beampool, shutting down " << itsBeamPool.size() << " existing beams");
	// close connection with all clients.
	while (itsClientList.size() > 0) {
		list<GCFPortInterface*>::iterator it = itsClientList.begin();
		if ((*it)->isConnected()) {
			(*it)->close();
			destroyAllBeams(*it);
			itsClientList.remove(*it);
			itsDeadClients.push_back(*it);
		}
	}

	// remove old pool if any
	itsBeamPool.clear();

	// make a new one based on the current value of the splitter.
	int		nrBeamlets = (itsSplitterOn ? 2 : 1 ) * itsCurrentMaxBeamlets;
	LOG_INFO_STR("Initializing space for " << nrBeamlets << " beamlets");
	itsBeamletAllocation.clear();
	itsBeamletAllocation.resize(nrBeamlets, BeamletAlloc_t(0,0.0));

	delete itsAnaBeamMgr;
	itsAnaBeamMgr = new AnaBeamMgr(itsMaxRCUs, (itsSplitterOn ? 2 : 1 ));
	ASSERTSTR(itsAnaBeamMgr, "Failed to create an Manager for the analogue beams.");

	// initialize matrices
	int	nPlanes          = MAX_BITS_PER_SAMPLE / itsCurrentBitsPerSample;
	int	beamletsPerPlane = maxBeamletsPerPlane(itsCurrentBitsPerSample);
	LOG_DEBUG(formatString("Size weights arrays set to %d x %d x %d", itsMaxRCUs, nPlanes, beamletsPerPlane));
	itsWeights.resize   (itsMaxRCUs, nPlanes, beamletsPerPlane);
	itsWeights16.resize (itsMaxRCUs, nPlanes, beamletsPerPlane);
	itsWeights   = complex<double>(0,0);
	itsWeights16 = complex<int16_t>(0,0);
}

//
// destroyAllBeams(port)
//
void BeamServer::destroyAllBeams(GCFPortInterface* port)
{
	ASSERT(port);		// sanity check

	// must still be in our admin
	if (itsClientBeams.find(port) == itsClientBeams.end()) {
		LOG_INFO_STR("No beams related (anymore) to port " << port->getName());
		return;
	}

	// deallocate all beams for this client
	set<DigitalBeam*>::iterator beamIter = itsClientBeams[port].begin();
	set<DigitalBeam*>::iterator end      = itsClientBeams[port].end();
	while (beamIter != end) {
		LOG_INFO_STR("Stopping beam " << (*beamIter)->name());
		_releaseBeamlets((*beamIter)->allocation(), (*beamIter)->ringNr());
		_unregisterBeamRCUs(**beamIter);
		DigitalBeam*	beam = *beamIter;
		++beamIter;
		map<string, DigitalBeam*>::iterator	beamNameIter = itsBeamPool.find(beam->name());
		if (beamNameIter != itsBeamPool.end()) {
			itsBeamPool.erase(beamNameIter);
		}
		itsAnaBeamMgr->deleteBeam(beam->name());
		delete beam;
	}
	itsClientBeams.erase(port);
}

//
// checkBeam(beamTransaction, port , name, subarray, beamletAllocation)
//
DigitalBeam* BeamServer::checkBeam(GCFPortInterface* 				port,
						  std::string 						name, 
						  std::string 						antennaSetName, 
						  IBS_Protocol::Beamlet2SubbandMap	allocation,
						  bitset<LOFAR::MAX_RCUS>		    rcumask,
						  uint								ringNr,
						  uint								rcuMode,
						  int*								beamError)
{
	LOG_TRACE_FLOW_STR("checkBeam(port=" << port->getName() << ", name=" << name << ", subarray=" << antennaSetName 
										<< ", ring=" << ringNr);

	ASSERT(port);

	// check for valid parameters
	// returning 0 will result in a negative ACK
	if (itsBeamTransaction.getBeam() != 0 || itsBeamTransaction.getPort() != 0) {
		LOG_ERROR("Previous alloc is still in progress");
		*beamError = IBS_BEAMALLOC_ERR;
		return (0);
	}
	if (name.length() == 0) {
		LOG_ERROR("Name of beam not set, cannot alloc new beam");
		*beamError = IBS_NO_NAME_ERR;
		return (0);
	}
	if (antennaSetName.length() == 0)  {
		LOG_ERROR_STR("AntennaSet name not set, cannot alloc beam " << name);
		*beamError = IBS_NO_ANTENNASET_ERR;
		return (0); 
	}
	if (!globalAntennaSets()->isAntennaSet(antennaSetName))  {
		LOG_ERROR_STR("Unknown antennaSet name, cannot alloc beam " << name);
		*beamError = IBS_NO_ANTENNASET_ERR;
		return (0); 
	}
	if (itsSplitterOn) {		// check allocation
		uint	ringLimit = itsMaxRCUs / 2;
		for (uint r = 0; r < itsMaxRCUs; r++) {
			if (rcumask.test(r) && (ringNr != (uint)(r < ringLimit ? 0 : 1))) {
				LOG_ERROR_STR("RCU's specified in the wrong ring for beam " << name << ", (rcu=" << r << ")");
				*beamError = IBS_WRONG_RING_ERR;
				return (0);
			}
		}
	}
	else if (ringNr != 0) {		// splitter is off so ringNr must be 0
		LOG_ERROR_STR("Splitter is off, ring segment 1 does not exist at this moment.");
		*beamError = IBS_SPLITTER_OFF_ERR;
		return (0);
	}

	// nr of subbands should fit in the beamlet space.
	if (static_cast<int>(allocation.getSubbandBitset().count()) > itsCurrentMaxBeamlets) {
		LOG_ERROR_STR("Too many subbands specified (" << allocation.getSubbandBitset().count() << ") only " 
					<< itsCurrentMaxBeamlets << " allowed");
		return (0);
	}

	if (!_checkBeamlets(allocation, ringNr)) {
		LOG_ERROR_STR("Beamlets of beam " << name << " overlap with beamlets of other beams");
		*beamError = IBS_BEAMALLOC_ERR;
		return (0);
	}

	DigitalBeam* beam = new DigitalBeam(name, antennaSetName, allocation, rcumask, ringNr, rcuMode);

	if (beam) { // register new beam
		itsClientBeams[port].insert(beam);
		itsBeamsModified = true;
		itsBeamTransaction.set(port, beam);
		_allocBeamlets(allocation, ringNr);
		itsBeamTransaction.allocationDone();
		_registerBeamRCUs(*beam);
		_logBeamAdministration();
	}
	LOG_INFO_STR("Beam " << name << " succesfully created.");

	return (beam);
}

//
// deleteTransactionBeam()
//
void BeamServer::deleteTransactionBeam()
{
	ASSERT(itsBeamTransaction.getPort() && itsBeamTransaction.getBeam());
	LOG_INFO_STR("Deleting beam " << itsBeamTransaction.getBeam()->name());

	// release beamlets
	if (itsBeamTransaction.isAllocationDone()) {
		_releaseBeamlets(itsBeamTransaction.getBeam()->allocation(), 
						 itsBeamTransaction.getBeam()->ringNr());
	}
	
	// unregister beam
	_unregisterBeamRCUs(*(itsBeamTransaction.getBeam()));
	itsClientBeams[itsBeamTransaction.getPort()].erase(itsBeamTransaction.getBeam());

	map<string, DigitalBeam*>::iterator	beamIter = itsBeamPool.find(itsBeamTransaction.getBeam()->name());
	if (beamIter != itsBeamPool.end()) {
		itsBeamPool.erase(beamIter);
	}
	itsAnaBeamMgr->deleteBeam(itsBeamTransaction.getBeam()->name());
	delete itsBeamTransaction.getBeam();
	itsBeamTransaction.reset();

	// update flag to trigger update of
	// subband selection settings
	itsBeamsModified = true;
	_logBeamAdministration();
}

// -------------------- itsBeamletAllocation administration --------------------
//
// _checkBeamlets
//
bool BeamServer::_checkBeamlets(IBS_Protocol::Beamlet2SubbandMap&	allocation,
							    uint								ringNr)
{
	LOG_DEBUG_STR("Checking allocation of beamlets for ring " << ringNr);

	// create a Beamlet object for every Beamletnr in the Beamlets array
	// map<beamletnr, subbandnr>
	map<uint16,uint16>::const_iterator iter = allocation().begin();
	map<uint16,uint16>::const_iterator end  = allocation().end();
	// first check if the allocation is valid
	for ( ; iter != end; iter++) {
		//											v--- beamletnumber
		int	index(ringNr * itsCurrentMaxBeamlets + iter->first);
		if (itsBeamletAllocation[index].subbandNr) {
			LOG_ERROR_STR("Beamlet " << iter->first << "(" << index << 
							") is already assigned to subband " << iter->second);
			return (false);
		}
	} 
	return (true);
}

//
// _allocBeamlets
//
void BeamServer::_allocBeamlets(IBS_Protocol::Beamlet2SubbandMap&	allocation,
							    uint								ringNr)
{
	LOG_DEBUG_STR("Allocating " << allocation().size() << " beamlets for ring " << ringNr);

	// map<beamletnr, subbandnr>
	map<uint16,uint16>::const_iterator iter = allocation().begin();
	map<uint16,uint16>::const_iterator end  = allocation().end();
	for ( ; iter != end; iter++) {
		itsBeamletAllocation[ringNr * itsCurrentMaxBeamlets + iter->first].subbandNr = iter->second;
		// NOTE: we like to set the scaling for each beamlets here also but we need
		// the spectral window of the antenneSet for that. We will receive that info
		// from the CalServer in a later state and calc the scalings than.
		itsBeamletAllocation[ringNr * itsCurrentMaxBeamlets + iter->first].scaling = complex<double>(0.0, 0.0);
	} 

	LOG_INFO_STR("Assignment of subbands to beamlets succesfull.");
}

//
// _scaleBeamlets(allocation, ringNr, spectralWindow)
//
void BeamServer::_scaleBeamlets(IBS_Protocol::Beamlet2SubbandMap&	allocation,
							    uint								ringNr,
								const CAL::SpectralWindow&			spw)
{
	const double speedOfLight = SPEED_OF_LIGHT_MS;

	LOG_DEBUG_STR("Scaling beamlets in ring " << ringNr << " for " << spw);

	// map<beamletnr, subbandnr>
	map<uint16,uint16>::iterator iter = allocation().begin();
	map<uint16,uint16>::iterator end  = allocation().end();
	for ( ; iter != end; iter++) {
		// first: beamletIndex, second: subbandnr
		double	freq  = spw.getSubbandFreq(iter->second);
		int		index = ringNr * itsCurrentMaxBeamlets + iter->first;
		itsBeamletAllocation[index].scaling = -2.0 * M_PI * freq * complex<double>(0.0,1.0) / speedOfLight;
		LOG_TRACE_OBJ_STR("scaling subband[" << itsBeamletAllocation[index].subbandNr << 
						  "]@beamlet[" << index << "] = " << itsBeamletAllocation[index].scaling <<
						  ", freq = " << freq);
	}
}

//
// _releaseBeamlets(alloction, ringNr)
//
void BeamServer::_releaseBeamlets(IBS_Protocol::Beamlet2SubbandMap&	allocation,
							      uint								ringNr)
{
	LOG_DEBUG_STR("Releasing beamlets in ring " << ringNr);

	// map<beamletnr, subbandnr>
	map<uint16,uint16>::iterator iter = allocation().begin();
	map<uint16,uint16>::iterator end  = allocation().end();
	for ( ; iter != end; iter++) {
		itsBeamletAllocation[ringNr * itsCurrentMaxBeamlets + iter->first].subbandNr = 0;
		itsBeamletAllocation[ringNr * itsCurrentMaxBeamlets + iter->first].scaling   = complex<double>(0.0, 0.0);
	} 

	LOG_INFO_STR("Assigned beamlets released succesfully.");
}

// -------------------- RCU allocation administration --------------------
//
// _registerBeamRCUs(beam)
//
void BeamServer::_registerBeamRCUs(const DigitalBeam&	beam)
{
	LOG_DEBUG_STR("_registerBeamRCUS(" << beam.name() << ")");

	bool	isLBAbeam = globalAntennaSets()->usesLBAfield(beam.antennaSetName());
	vector<uint>&		RCUcounts = isLBAbeam ? itsLBArcus : itsHBArcus;
	bitset<MAX_RCUS>&	rcuBitset = isLBAbeam ? itsLBAallocation : itsHBAallocation;

	for (int r = MAX_RCUS - 1; r >= 0; r--) {
		if (beam.rcuMask().test(r)) {
			RCUcounts[r]++;
			rcuBitset.set(r, true);
		}	
	}
	if (isLBAbeam) {
		itsNrLBAbeams++;
	}
	else {
		itsNrHBAbeams++;
	}
}

//
// _unregisterBeamRCUs(beam)
//
void BeamServer::_unregisterBeamRCUs(const DigitalBeam&	beam)
{
	LOG_DEBUG_STR("_unregisterBeamRCUS(" << beam.name() << ")");

	bool	isLBAbeam = globalAntennaSets()->usesLBAfield(beam.antennaSetName());
	vector<uint>&		RCUcounts = isLBAbeam ? itsLBArcus : itsHBArcus;
	bitset<MAX_RCUS>&	rcuBitset = isLBAbeam ? itsLBAallocation : itsHBAallocation;

	for (int r = MAX_RCUS - 1; r >= 0; r--) {
		if (beam.rcuMask().test(r)) {
			if (--RCUcounts[r] == 0) {
				rcuBitset.set(r, false);
			}
		}	
	}
	if (isLBAbeam) {
		itsNrLBAbeams--;
	}
	else {
		itsNrHBAbeams--;
	}
}

//
// _logBeamAdministration()
//
void BeamServer::_logBeamAdministration()
{
	LOG_DEBUG_STR("Nr LBA beams: " << itsNrLBAbeams);
	if (itsNrLBAbeams) {
		stringstream		os;
		writeVector(os, itsLBArcus);
		LOG_DEBUG_STR("LBA RCUs : " << os.str());
		LOG_DEBUG_STR("as bitset: " << itsLBAallocation);
	}

	LOG_DEBUG_STR("Nr HBA beams: " << itsNrHBAbeams);
	if (itsNrHBAbeams) {
		stringstream		os;
		writeVector(os, itsHBArcus);
		LOG_DEBUG_STR("HBA RCUs : " << os.str());
		LOG_DEBUG_STR("as bitset: " << itsHBAallocation);
	}
}

// -------------------- Reconstruction of calibrationfactor --------------------

//
// _getCalFactor(rcumode, rcu, subbandNr)
//
complex<double>	BeamServer::_getCalFactor(uint rcuMode, uint rcu, uint subbandNr)
{
	complex<double>	result(1.0, 0.0);

	switch (rcuMode) {
	case 1:
	case 2:
		if (itsCalTableMode1) { result = itsCalTableMode1->calFactor(rcu, subbandNr); } break;
	case 3:
	case 4:
		if (itsCalTableMode3) { result = itsCalTableMode3->calFactor(rcu, subbandNr); } break;
	case 5:
		if (itsCalTableMode5) { result = itsCalTableMode5->calFactor(rcu, subbandNr); } break;
	case 6:
		if (itsCalTableMode6) { result = itsCalTableMode6->calFactor(rcu, subbandNr); } break;
	case 7:
		if (itsCalTableMode7) { result = itsCalTableMode7->calFactor(rcu, subbandNr); } break;
	default:
		break;
	}

//	LOG_DEBUG_STR("calFactor(" << rcuMode << "," << rcu << "," << subbandNr << ")=" << result);
	return (result);
}

void BeamServer::_loadCalTable(uint rcuMode, uint nrRSPBoards)
{
	StatCal**	tableHandle(0);
	switch (rcuMode) {
		case 1:
		case 2: tableHandle = &itsCalTableMode1; break;
		case 3:
		case 4: tableHandle = &itsCalTableMode3; break;
		case 5: tableHandle = &itsCalTableMode5; break;
		case 6: tableHandle = &itsCalTableMode6; break;
		case 7: tableHandle = &itsCalTableMode7; break;
		default: return;
	}
	
	(*tableHandle) = new StatCal(rcuMode, nrRSPBoards);
	if ((*tableHandle) && !(*tableHandle)->isValid()) {
		delete (*tableHandle);
		(*tableHandle) = 0;
		switch (rcuMode) {
			case 1:
			case 2: LOG_WARN ("NO CALIBRATION TABLE FOUND FOR MODE 1 AND 2"); break;
			case 3:
			case 4: LOG_WARN ("NO CALIBRATION TABLE FOUND FOR MODE 3 AND 4"); break;
			case 5:
			case 6:
			case 7: LOG_WARN_STR ("NO CALIBRATION TABLE FOUND FOR MODE " << rcuMode); break;
		}
		return;
	}

	LOG_INFO_STR(**tableHandle);
}


// -------------------- Tracking calculations --------------------

//
// blitz2vector
//
vector<double>	BeamServer::blitz2vector(const blitz::Array<double,1>&	anBA) const
{
	vector<double>  resultVect(3);
	resultVect[0] = anBA(0);
	resultVect[1] = anBA(1);
	resultVect[2] = anBA(2);
     
	return (resultVect);
}

//
// Convert the weights to 16-bits signed integer.
//
inline complex<int16_t> convert2complex_int16_t(complex<double> cd)
{
	return complex<int16_t>((int16_t)(round(cd.real() * gBeamformerGain)),
							(int16_t)(round(cd.imag() * gBeamformerGain)));
}
BZ_DECLARE_FUNCTION_RET(convert2complex_int16_t, complex<int16_t>)

//
// compute_weights(time)
//
// This method is called once every period of COMPUTE_INTERVAL seconds
// to calculate the weights for all beamlets.
//
void BeamServer::compute_weights(Timestamp weightTime)
{
	// TEMP CODE FOR EASY TESTING THE BEAMSERVER VALUES
	if (itsTestSingleShotTimestamp) {
		weightTime = Timestamp(itsTestSingleShotTimestamp,0);
	}
	// END OF TEMP CODE

	LOG_INFO_STR("Calculating weights for time " << weightTime);

	// reset all weights
	LOG_DEBUG_STR("Weights array has size: " << itsWeights.extent(firstDim) << "x" << 
								itsWeights.extent(secondDim) << "x" << itsWeights.extent(thirdDim));
	itsWeights = 0.0;

	// get ptr to antennafield information
	AntennaField *gAntField = globalAntennaField();

	int beamletsPerPlane = maxBeamletsPerPlane(itsCurrentBitsPerSample);
	// Check both LBA and HBA antennas
	for (uint	fieldNr = 0; fieldNr < 4; fieldNr++) {
		string	fieldName;
		switch(fieldNr) {			// TODO: needs improvement
			case 0: fieldName = "LBA";	break;
			case 1: fieldName = "HBA";	break;
			case 2: fieldName = "HBA0";	break;
			case 3: fieldName = "HBA1";	break;
		}
		bool LBAfield = (fieldNr == 0);

		LOG_DEBUG_STR("Checking " << fieldName << " antennas");
		// Any beams on this antenna field?
		if ((LBAfield && !itsNrLBAbeams) || (!LBAfield && !itsNrHBAbeams)) {
			LOG_DEBUG_STR("No beams defined for these antennas");
			continue;
		}

		// Get ITRF position of the RCU's [rcu, xyz]
		blitz::Array<double, 2> rcuPosITRF = gAntField->RCUPos(fieldName);
		if (rcuPosITRF.size() == 0) {
			LOG_DEBUG_STR("No antennas defined in this field");
			continue;
		}
//		LOG_DEBUG_STR("ITRFRCUPos = " << rcuPosITRF);

		// Get geographical location of subarray in ITRF
		blitz::Array<double, 1> fieldCentreITRF = gAntField->Centre(fieldName);
//		LOG_DEBUG_STR("ITRF position antennaField: " << fieldCentreITRF);

		// convert ITRF position of all antennas to J2000 for timestamp t
		blitz::Array<double,2>	rcuJ2000Pos; // [rcu, xyz]
		if (!itsJ2000Converter->doConversion("ITRF", rcuPosITRF, fieldCentreITRF, weightTime, rcuJ2000Pos)) {
			LOG_FATAL_STR("Conversion of antennas to J2000 failed");
			continue;
		}

		// Lengths of the vector of the antennaPosition i.r.t. the fieldCentre,
		blitz::Array<double,1>	rcuPosLengths = gAntField->RCULengths(fieldName);
//		LOG_DEBUG_STR("rcuPosLengths = " << rcuPosLengths);

		// denormalize length of vector
		rcuJ2000Pos = rcuJ2000Pos(tensor::i, tensor::j) * rcuPosLengths(tensor::i);
//		LOG_DEBUG_STR("J2000RCUPos@fullLength=" << rcuJ2000Pos);

		// for all beams using this field
		map<string, DigitalBeam*>::iterator	beamIter = itsBeamPool.begin();
		map<string, DigitalBeam*>::iterator	end		 = itsBeamPool.end();
		for ( ; beamIter != end; ++beamIter) {
			// must be of the same antenna field.
			if (globalAntennaSets()->antennaField(beamIter->second->antennaSetName()) != fieldName) {
				continue;
			}
			// Get the RCU index-schema for this antennaSet.
			vector<int16>	posIndex = globalAntennaSets()->positionIndex(beamIter->second->antennaSetName());

			// Get the right pointing
			Pointing	currentPointing = beamIter->second->pointingAtTime(weightTime);
			blitz::Array<double,2>	sourceJ2000xyz;		// [1, xyz]
			blitz::Array<double,2>	curPoint(1,2);		// [1, angles]
			curPoint(0,0) = currentPointing.angle0();
			curPoint(0,1) = currentPointing.angle1();
			LOG_INFO_STR("current pointing for beam " << beamIter->second->name() << ":" << currentPointing);
			if (!itsJ2000Converter->doConversion(currentPointing.getType(), curPoint, fieldCentreITRF, weightTime, sourceJ2000xyz)) {
				LOG_FATAL_STR("Conversion of source to J2000 failed");
				continue;
			}
			LOG_INFO(formatString("sourceJ2000xyz: [ %9.6f, %9.6f, %9.6f ]", 
							sourceJ2000xyz(0,0), sourceJ2000xyz(0,1), sourceJ2000xyz(0,2)));

			// Note: Beamlet numbers depend on the ring.
			int	firstBeamlet(gAntField->ringNr(fieldName) * itsCurrentMaxBeamlets);
			LOG_DEBUG_STR("first beamlet of field " << fieldName << "=" << firstBeamlet);
			// Note: RCUallocation is stationbased, rest info is fieldbased, 
			bitset<MAX_RCUS>	RCUallocation(beamIter->second->rcuMask());
			for (int rcu = 0; rcu < MAX_RCUS; rcu++) {
				if (!RCUallocation.test(rcu)) {			// all RCUS switched on in LBA/HBA mode
					continue;
				}
	
				//
				// For all beamlets that belong to this beam calculate the weight
				// Note: weight is in-procduct for RCUpos and source Pos and depends on 
				// the frequency of the subband.
				//
				boost::dynamic_bitset<>	beamletAllocation;
				beamletAllocation.resize(itsCurrentMaxBeamlets);
				beamletAllocation = beamIter->second->allocation().getBeamletBitset(itsCurrentMaxBeamlets);
				int		nrBeamlets = beamletAllocation.size();
				for (int	beamlet = 0; beamlet < nrBeamlets; beamlet++) {
					if (!beamletAllocation.test(beamlet)) {
						continue;
					}

					complex<double>	CalFactor = _getCalFactor(beamIter->second->rcuMode(), rcu, 
																itsBeamletAllocation[beamlet+firstBeamlet].subbandNr);
					int	bitPlane = beamlet / beamletsPerPlane;
					itsWeights(rcu, bitPlane, beamlet % beamletsPerPlane) = 
						CalFactor * exp(itsBeamletAllocation[beamlet+firstBeamlet].scaling * 
								(rcuJ2000Pos((int)posIndex[rcu], 0) * sourceJ2000xyz(0,0) +
								 rcuJ2000Pos((int)posIndex[rcu], 1) * sourceJ2000xyz(0,1) +
								 rcuJ2000Pos((int)posIndex[rcu], 2) * sourceJ2000xyz(0,2)));

					// some debugging
					if (beamlet%100==0) {
						LOG_DEBUG_STR("itsWeights(" << rcu << "," << bitPlane << "," << beamlet << ")="
										<< itsWeights(rcu, bitPlane, beamlet)
										<< " : rcuPos[" << posIndex[rcu] << "]=" << rcuJ2000Pos((int)posIndex[rcu],0)
										<< " : CalFactor=" << CalFactor);
					}
				} // beamlets
			} // rcus
		} // beams
	} // antennafield

	// convert the weights from double to int16
	itsWeights16 = convert2complex_int16_t(itsWeights);

	LOG_DEBUG(formatString("sizeof(itsWeights16) = %d", itsWeights16.size()*sizeof(int16_t)));
}

//
// send_weights(time)
//
void BeamServer::send_weights(Timestamp time)
{
	if (!itsSetWeightsEnabled) {
		return;
	}
  
	RSPSetweightsEvent sw;
	sw.timestamp = time;
	LOG_DEBUG_STR("weights.time=" << sw.timestamp);
  
	// select all BLPS, no subarraying
	sw.rcumask.reset();
	for (uint i = 0; i < itsMaxRCUs; i++) {
		sw.rcumask.set(i);
	}
  
	int	nPlanes = MAX_BITS_PER_SAMPLE / itsCurrentBitsPerSample;
	sw.weights().resize(1, itsMaxRCUs, nPlanes, itsCurrentMaxBeamlets / nPlanes);
	sw.weights()(0, Range::all(), Range::all(), Range::all()) = itsWeights16;
  
	LOG_INFO_STR("sending weights for interval " << time << " : " << time + (long)(itsComputeInterval-1));

//  for debugging purposes print 40 subbands of 10 antennas for 1 timestamp.
//	blitz::Array<std::complex<int16>, 2>	sample;
//	sample.resize (10, 40);		// first 10 antennas and first 40 subbands
//	sample = complex<int16_t>(0,0);
//	sample = itsWeights16(0, Range(64,74), Range(0,40));
//	LOG_DEBUG_STR("weights sample=" << sample);

	itsRSPDriver->send(sw);
}

//
// send_sbselection()
//
void BeamServer::send_sbselection()
{
	if (!itsSetSubbandsEnabled) {
		return;
	}

	for (uint ringNr = 0; ringNr <= (itsSplitterOn ? 1 : 0); ringNr++) {
		RSPSetsubbandsEvent ss;
		ss.timestamp.setNow(0);
		ss.rcumask.reset();
		// splitter=OFF: all RCUs ; splitter=ON: 2 runs with half the RCUs
		int	maxRCUs = (itsSplitterOn ? itsMaxRCUs / 2 : itsMaxRCUs);
		for (int i = 0; i < maxRCUs; i++) {
			ss.rcumask.set((ringNr * maxRCUs) + i);
		}
		LOG_DEBUG_STR("Collecting subbandselection for ring " << ringNr << " (" << maxRCUs << " RCUs)");
		//
		// Always allocate the array as if all beamlets were
		// used. Because of allocation and deallocation of beams
		// there can be holes in the subband selection.
		//
		// E.g. Beamlets 0-63 are used by beam 0, beamlets 64-127 by
		// beam 1, then beam 0 is deallocated, thus there is a hole
		// of 64 beamlets before the beamlets of beam 1.
		//
		int	beamletsPerPlane = maxBeamletsPerPlane(itsCurrentBitsPerSample);
		ss.subbands.setType(SubbandSelection::BEAMLET);
		ss.subbands.beamlets().resize(1, MAX_BITS_PER_SAMPLE/itsCurrentBitsPerSample, beamletsPerPlane);
		ss.subbands.beamlets() = 0;

		// reconstruct the selection
		Beamlet2SubbandMap selection;
		map<string, DigitalBeam*>::iterator	beamIter = itsBeamPool.begin();
		map<string, DigitalBeam*>::iterator	beamEnd  = itsBeamPool.end();
		while (beamIter != beamEnd) {
			DigitalBeam*	beam = beamIter->second;
			LOG_DEBUG_STR("Beam " << beam->name() << " is in ring " << beam->ringNr());
			if (beam->ringNr() == ringNr) {
				selection().insert(beam->allocation()().begin(), beam->allocation()().end());
			}
			beamIter++;
		}

		LOG_DEBUG(formatString("nrsubbands=%d", selection().size()));
		map<uint16,uint16>::iterator	iter = selection().begin();
		map<uint16,uint16>::iterator	end  = selection().end();
		for ( ; iter != end; ++iter) {
			LOG_DEBUG(formatString("(%d,%d)", iter->first, iter->second));

			if (iter->first >= itsCurrentMaxBeamlets) {
				LOG_ERROR(formatString("SBSELECTION: invalid src index %d (max=%d)", iter->first, itsCurrentMaxBeamlets));
				continue;
			}

			if (iter->second >= MAX_SUBBANDS) {
				LOG_ERROR(formatString("SBSELECTION: invalid tgt index %d (max=%d)", iter->second, MAX_SUBBANDS));
				continue;
			}

			// same selection for x and y polarization
			ss.subbands.beamlets()(0, iter->first/beamletsPerPlane, iter->first%beamletsPerPlane) = iter->second;
		}

		if (selection().size()) {
			LOG_DEBUG_STR("Sending subbandselection for ring segment " << ringNr);
			LOG_DEBUG_STR(ss.subbands.beamlets());
			itsRSPDriver->send(ss);
		} 
		else {
			LOG_DEBUG_STR("No subbandselection for ring segment " << ringNr);
			// TODO [200710] Shouldn't we send empty selections also (after cleanup of beams).
			//      Can the RSPDriver handle that???
		}
	}
}

  } // namespace BS;
} // namespace LOFAR;
