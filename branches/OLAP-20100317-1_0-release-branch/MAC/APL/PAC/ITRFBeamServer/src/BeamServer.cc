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
#include <Common/lofar_complex.h>
#include <Common/Version.h>
#include <Common/ParameterSet.h>
#include <Common/StreamUtil.h>

#include <MACIO/MACServiceInfo.h>

#include <APL/RTCCommon/daemonize.h>
#include <APL/IBS_Protocol/IBS_Protocol.ph>
#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/CAL_Protocol/CAL_Protocol.ph>

#include "BeamServer.h"
#include "BeamServerConstants.h"
#include "Package__Version.h"

#include <getopt.h>
#include <iostream>
#include <sstream>
#include <time.h>
#include <fstream>

#include <netinet/in.h>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

using namespace blitz;
namespace LOFAR {
  using namespace APLCommon;
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
	itsNrLBAbeams			(0),
	itsNrHBAbeams			(0),
	itsListener				(0),
	itsRSPDriver			(0),
	itsCalServer			(0),
	itsBeamsModified		(false),
	itsAnaBeamMgr			(0),
	itsUpdateInterval		(UPDATE_INTERVAL),
	itsComputeInterval		(COMPUTE_INTERVAL),
	itsTestSingleShotTimestamp(timestamp)
{
//	LOG_INFO(Version::getInfo<Beam_ServerVersion>("BeamServer"));

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

	// read config settings
	itsSetHBAEnabled 	  = (globalParameterSet()->getInt32("BeamServer.DISABLE_SETHBA") == 0);
	itsSetSubbandsEnabled = (globalParameterSet()->getInt32("BeamServer.DISABLE_SETSUBBANDS") == 0);
	itsSetWeightsEnabled  = (globalParameterSet()->getInt32("BeamServer.DISABLE_SETWEIGHTS") == 0);
	gBeamformerGain	      =  globalParameterSet()->getInt32("BeamServer.BF_GAIN");

	// get interval information
	itsHbaInterval 		  = globalParameterSet()->getInt32("BeamServer.HBA_INTERVAL", COMPUTE_INTERVAL);
	if (itsHbaInterval < 10) {
		LOG_FATAL("HBA_INTERVAL is too small, must be greater or equal to 10 seconds");
		exit(EXIT_FAILURE);
	}

	itsLBArcus.resize(MAX_RCUS, 0);
	itsHBArcus.resize(MAX_RCUS, 0);

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
		ASSERTSTR(globalAntennaPos(), "Could not load the antennaposition file");
		LOG_DEBUG("Loaded antenna postions file");
		LOG_DEBUG_STR("LBA rcu=" << globalAntennaPos()->RCUPos("LBA"));
		LOG_DEBUG_STR("HBA rcu=" << globalAntennaPos()->RCUPos("HBA"));
		LOG_DEBUG_STR("HBA0 rcu=" << globalAntennaPos()->RCUPos("HBA0"));
		LOG_DEBUG_STR("HBA1 rcu=" << globalAntennaPos()->RCUPos("HBA1"));
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
// Ask the RSPdriver what ahrdware is available
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
		itsMaxRCUs = ack.n_rcus;
		LOG_INFO_STR("Station has " << itsMaxRCUs << " RCU's");

		// initialize matrices
		itsWeights.resize   (itsMaxRCUs, MAX_BEAMLETS);
		itsWeights16.resize (itsMaxRCUs, MAX_BEAMLETS);
		itsWeights   = complex<double>(0,0);
		itsWeights16 = complex<int16_t>(0,0);

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
	}
	break;

	case RSP_UPDSPLITTER: {
		RSPUpdsplitterEvent		answer(event);
		if (answer.status != RSP_SUCCESS) {
			LOG_INFO("Could not get a subscription on the splitter, retry in 5 seconds");
			itsConnectTimer->setTimer(5.0);
			break;
		}

		itsSplitterOn = answer.splitter[0];
		LOG_INFO_STR("The ringsplitter is " << (itsSplitterOn ? "ON" : "OFF"));
		_createBeamPool();		// (re)allocate memory for the beamlet mapping

		itsConnectTimer->cancelAllTimers();
		TRAN(BeamServer::con2calserver);
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
		itsCalServer->autoOpen(5, 0, 2);	// try max 5 times every 2 seconds than report DISCO
	}
	break;

	case F_CONNECTED: {
		// start update timer and start accepting clients
//		LOG_DEBUG_STR("Starting Heartbeat timer with interval: " << itsUpdateInterval << " secs");
//		itsDigHeartbeat->setTimer(0, 0, itsUpdateInterval, 0);
		itsDigHeartbeat->setTimer(10.0);	// FOR TEST SET TIMER ONCE !!!
		itsAnaHeartbeat->setTimer(15.0);	// FOR TEST SET TIMER ONCE !!!

		if ((itsListener->getState() != GCFPortInterface::S_CONNECTING) &&
			(itsListener->getState() != GCFPortInterface::S_CONNECTED)) {
			itsListener->open();
		}
		TRAN(BeamServer::enabled);
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
		itsCalServer->autoOpen(5, 0, 2);	// try max 5 times every 2 seconds than report DISCO
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
	static int			hbaPeriod = 0;
	
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
				LOG_INFO_STR("computing weights " << Timestamp(timer->sec, timer->usec));
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
			hbaPeriod += itsHbaInterval; 
			if (hbaPeriod >= itsHbaInterval) {
				hbaPeriod = 0;
				if (itsSetHBAEnabled) {
					GCFTimerEvent* timer = static_cast<GCFTimerEvent*>(&event);
					LOG_INFO_STR("computing HBA delays " << Timestamp(timer->sec, timer->usec));
					itsAnaBeamMgr->calculateHBAdelays(Timestamp((long)timer->sec + LEADIN_TIME, 0L), itsJ2000Converter);
					itsAnaBeamMgr->sendHBAdelays     (*itsRSPDriver);
				}
			}
			return (GCFEvent::HANDLED);
		}
		
		LOG_ERROR_STR("Unknown F_TIMER event occurred at port " << port.getName() << "! Ignoring it.");
	} // case
	break;

	// ---------- requests from the clients ----------
	case IBS_BEAMALLOC: {
		IBSBeamallocEvent 	allocEvent(event);
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

	case F_TIMER: {
		LOG_DEBUG("closed all TCP ports, going to 'con2rspdriver' state for restart");
		TRAN(BeamServer::con2rspdriver);
	}
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
		return (GCFEvent::NEXT_STATE);
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
			LOG_WARN("CAL_UNSUBSCRIBE failed");
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
		return (GCFEvent::NEXT_STATE);
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
	DigitalBeam* beam = checkBeam(&port, ba.beamName, ba.antennaSet, ba.allocation, ba.rcumask, ba.ringNr, &beamError);

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
	if (!itsJ2000Converter.isValidType(ptEvent.pointing.getType())) {
		LOG_ERROR_STR(ptEvent.pointing.getType() << " is not a valid reference system, pointing rejected");
		return (IBS_INVALID_TYPE_ERR);
	}

	LOG_INFO_STR("new pointing for " << beamIter->second->name() << ": " << ptEvent.pointing);

	//
	// If the time is not set, then activate the command
	// 2 * COMPUTE_INTERVAL seconds from now, because that's how
	// long it takes the command to flow through the pipeline.
	//
	Timestamp actualtime;
	actualtime.setNow(2 * COMPUTE_INTERVAL);
	if (ptEvent.pointing.time() == Timestamp(0,0)) {
		ptEvent.pointing.setTime(actualtime);
	}

	// TEMP CODE FOR EASY TESTING THE BEAMSERVER VALUES
	if (itsTestSingleShotTimestamp) {
		ptEvent.pointing.setTime(Timestamp(itsTestSingleShotTimestamp,0));
	}
	// END OF TEMP CODE

	if (ptEvent.analogue) {
		// note we don't know if we added the beam before, just do it again and ignore returnvalue.
		itsAnaBeamMgr->addBeam(AnalogueBeam(ptEvent.beamName, beamIter->second->antennaSetName(), 
										beamIter->second->rcuMask(), ptEvent.rank));
		return (itsAnaBeamMgr->addPointing(ptEvent.beamName, ptEvent.pointing) ? 
					IBS_NO_ERR : IBS_UNKNOWN_BEAM_ERR);
	}
	else {
		return (beamIter->second->addPointing(ptEvent.pointing));
	}
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
	int		nrBeamlets = (itsSplitterOn ? 2 : 1 ) * MEPHeader::N_BEAMLETS;
	LOG_INFO_STR("Initializing space for " << nrBeamlets << " beamlets");
	itsBeamletAllocation.clear();
	itsBeamletAllocation.resize(nrBeamlets, BeamletAlloc_t(0,0.0));

	delete itsAnaBeamMgr;
	itsAnaBeamMgr = new AnaBeamMgr(itsMaxRCUs, (itsSplitterOn ? 2 : 1 ));
	ASSERTSTR(itsAnaBeamMgr, "Failed to create an Manager for the analogue beams.");
}

//
// destroyAllBeams(port)
//
void BeamServer::destroyAllBeams(GCFPortInterface* port)
{
	ASSERT(port);

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
						  LOFAR::bitset<LOFAR::MAX_RCUS>	rcumask,
						  int								ringNr,
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
		int		ringLimit = itsMaxRCUs / 2;
		for (int r = 0; r < itsMaxRCUs; r++) {
			if (rcumask.test(r) && (ringNr != (r < ringLimit ? 0 : 1))) {
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

	if (!_checkBeamlets(allocation, ringNr)) {
		LOG_ERROR_STR("Beamlets of beam " << name << " overlap with beamlets of other beams");
		*beamError = IBS_BEAMALLOC_ERR;
		return (0);
	}

	DigitalBeam* beam = new DigitalBeam(name, antennaSetName, allocation, rcumask, ringNr);

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
		int	index(ringNr * LOFAR::MAX_BEAMLETS + iter->first);
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
	LOG_DEBUG_STR("Allocating beamlets for ring " << ringNr);

	// map<beamletnr, subbandnr>
	map<uint16,uint16>::const_iterator iter = allocation().begin();
	map<uint16,uint16>::const_iterator end  = allocation().end();
	for ( ; iter != end; iter++) {
		itsBeamletAllocation[ringNr * LOFAR::MAX_BEAMLETS + iter->first].subbandNr = iter->second;
		// NOTE: we like to set the scaling for each beamlets here also but we need
		// the spectral window of the antenneSet for that. We will receive that info
		// from the CalServer in a later state and calc the scalings than.
		itsBeamletAllocation[ringNr * LOFAR::MAX_BEAMLETS + iter->first].scaling = complex<double>(0.0, 0.0);
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
		int		index = ringNr * LOFAR::MAX_BEAMLETS + iter->first;
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
		itsBeamletAllocation[ringNr * LOFAR::MAX_BEAMLETS + iter->first].subbandNr = 0;
		itsBeamletAllocation[ringNr * LOFAR::MAX_BEAMLETS + iter->first].scaling   = complex<double>(0.0, 0.0);
	} 

	LOG_INFO_STR("Assigned beamlets released succesfully.");
}

// -------------------- RCU allocation administration --------------------
//
// _registerBeamRCUs(beam)
//
void BeamServer::_registerBeamRCUs(const DigitalBeam&	beam)
{
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

BZ_DECLARE_FUNCTION_RET(convert2complex_int16_t, complex<int16_t>)

//
// Convert the weights to 16-bits signed integer.
//
inline complex<int16_t> convert2complex_int16_t(complex<double> cd)
{
	return complex<int16_t>((int16_t)(round(cd.real() * gBeamformerGain)),
							(int16_t)(round(cd.imag() * gBeamformerGain)));
}

//
// compute_weights(time)
//
// This method is called once every period of COMPUTE_INTERVAL seconds
// to calculate the weights for all beamlets.
//
bool BeamServer::compute_weights(Timestamp weightTime)
{
	// TEMP CODE FOR EASY TESTING THE BEAMSERVER VALUES
	if (itsTestSingleShotTimestamp) {
		weightTime = Timestamp(itsTestSingleShotTimestamp,0);
	}
	// END OF TEMP CODE

	LOG_INFO_STR("Calculating weights for time " << weightTime);

	// reset all weights
	LOG_DEBUG_STR("Weights array has size: " << itsWeights.extent(firstDim) << "x" << itsWeights.extent(secondDim));
	itsWeights(Range::all(), Range::all()) = 0.0;

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
		blitz::Array<double, 2> rcuPosITRF = globalAntennaPos()->RCUPos(fieldName);
		LOG_DEBUG_STR("ITRFRCUPos = " << rcuPosITRF);

		// Get geographical location of subarray in ITRF
		blitz::Array<double, 1> fieldCentreITRF = globalAntennaPos()->Centre(fieldName);
		LOG_DEBUG_STR("ITRF position antennaField: " << fieldCentreITRF);

		// convert ITRF position of all antennas to J2000 for timestamp t
		blitz::Array<double,2>	rcuJ2000Pos; // [rcu, xyz]
		if (!itsJ2000Converter.doConversion("ITRF", rcuPosITRF, fieldCentreITRF, weightTime, rcuJ2000Pos)) {
			LOG_FATAL_STR("Conversion of antennas to J2000 failed");
			return(false);
		}

		// Lengths of the vector of the antennaPosition i.r.t. the fieldCentre,
		blitz::Array<double,1>	rcuPosLengths = globalAntennaPos()->RCULengths(fieldName);
		LOG_DEBUG_STR("rcuPosLengths = " << rcuPosLengths);

		// denormalize length of vector
		rcuJ2000Pos = rcuJ2000Pos(tensor::i, tensor::j) * rcuPosLengths(tensor::i);
		LOG_DEBUG_STR("J2000RCUPos@fullLength=" << rcuJ2000Pos);

		// for all beams using this field
		map<string, DigitalBeam*>::iterator	beamIter = itsBeamPool.begin();
		map<string, DigitalBeam*>::iterator	end		 = itsBeamPool.end();
		for ( ; beamIter != end; ++beamIter) {
			// must be of the same antenna field.
			if (globalAntennaSets()->antennaField(beamIter->second->antennaSetName()) != fieldName) {
				continue;
			}
			// Get the right pointing
			Pointing	currentPointing = beamIter->second->pointingAtTime(weightTime);
			blitz::Array<double,2>	sourceJ2000xyz;		// [1, xyz]
			blitz::Array<double,2>	curPoint(1,2);		// [1, angles]
			curPoint(0,0) = currentPointing.angle0();
			curPoint(0,1) = currentPointing.angle1();
			if (!itsJ2000Converter.doConversion(currentPointing.getType(), curPoint, fieldCentreITRF, weightTime, sourceJ2000xyz)) {
				LOG_FATAL_STR("Conversion of source to J2000 failed");
				return(false);
			}
			LOG_DEBUG_STR("sourceJ2000xyz:" << sourceJ2000xyz);

			bitset<MAX_RCUS>	RCUallocation(beamIter->second->rcuMask());
			for (int rcu = 0; rcu < MAX_RCUS; rcu++) {
				if (!RCUallocation.test(rcu)) {			// all RCUS switched on in LBA/HBA mode
					continue;
				}
	
				//
				// For all beamlets that belong to this beam calculate the weight
				// Note: weight is in-procduct for RCUpos and source Pos and depends on frequency of the subband.
				//
				bitset<MAX_BEAMLETS>	beamletAllocation = beamIter->second->allocation().getBeamletBitset();
				int		nrBeamlets = beamletAllocation.size();
				for (int	beamlet = 0; beamlet < nrBeamlets; beamlet++) {
					if (!beamletAllocation.test(beamlet)) {
						continue;
					}

					itsWeights(rcu, beamlet) = exp(itsBeamletAllocation[beamlet].scaling * 
							(rcuJ2000Pos(rcu, 0) * sourceJ2000xyz(0,0) +
							 rcuJ2000Pos(rcu, 1) * sourceJ2000xyz(0,1) +
							 rcuJ2000Pos(rcu, 2) * sourceJ2000xyz(0,2)));

					// some debugging
					if (itsWeights(rcu, beamlet) != complex<double>(1,0)) {
						stringstream	str;
						str.precision(20);
						str << "itsWeights(" << rcu << "," << beamlet << ")=" << itsWeights(rcu, beamlet);
						LOG_DEBUG_STR(str.str());
//						LOG_DEBUG_STR("itsWeights(" << rcu << "," << beamlet << ")=" << itsWeights(rcu, beamlet));
					}
					// some more debugging
					if (rcu==3 && beamlet==6) {
						LOG_DEBUG_STR("###scaling     = " << itsBeamletAllocation[beamlet].scaling);
						LOG_DEBUG_STR("###J2000RCUpos = " << rcuJ2000Pos(rcu, Range::all()));
						LOG_DEBUG_STR("###J2000xyz    = " << sourceJ2000xyz(0,Range::all()));
					}
				} // beamlets
			} // rcus
		} // beams
	} // antennafield

	// convert the weights from double to int16
	itsWeights16 = convert2complex_int16_t(itsWeights);

	LOG_DEBUG(formatString("sizeof(itsWeights16) = %d", itsWeights16.size()*sizeof(int16_t)));

	return (true);
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
	for (int i = 0; i < itsMaxRCUs; i++) {
		sw.rcumask.set(i);
	}
  
	sw.weights().resize(1, itsMaxRCUs, MEPHeader::N_BEAMLETS);
	sw.weights()(0, Range::all(), Range::all()) = itsWeights16;
  
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

	for (int ringNr = 0; ringNr <= (itsSplitterOn ? 1 : 0); ringNr++) {
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
		ss.subbands.setType(SubbandSelection::BEAMLET);
		ss.subbands().resize(1, MEPHeader::N_BEAMLETS);
		ss.subbands() = 0;

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

			if (iter->first >= MEPHeader::N_BEAMLETS) {
				LOG_ERROR(formatString("SBSELECTION: invalid src index %d", iter->first));
				continue;
			}

			if (iter->second >= MEPHeader::N_SUBBANDS) {
				LOG_ERROR(formatString("SBSELECTION: invalid tgt index %d", iter->second));
				continue;
			}

			// same selection for x and y polarization
			ss.subbands()(0, (int)iter->first) = iter->second;
		}

		if (selection().size()) {
			LOG_DEBUG_STR("Sending subbandselection for ring segment " << ringNr);
			LOG_DEBUG_STR(ss.subbands());
			itsRSPDriver->send(ss);
		} 
		else {
			LOG_DEBUG_STR("No subbandselection for ring segment " << ringNr);
		}
	}
}

  } // namespace BS;
} // namespace LOFAR;
