//#
//#  BeamServer.cc: implementation of BeamServer class
//#
//#  Copyright (C) 2002-2004
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
#include <Common/Version.h>
#include <Common/ParameterSet.h>

#include <MACIO/MACServiceInfo.h>

#include <APL/RTCCommon/daemonize.h>
#include <APL/BS_Protocol/BS_Protocol.ph>
#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/CAL_Protocol/CAL_Protocol.ph>

#include "BeamServer.h"
#include "BeamServerConstants.h"
#include "Beam.h"
#include "Beamlet.h"
#include "Package__Version.h"

#include <getopt.h>
#include <iostream>
#include <sstream>
#include <time.h>
#include <fstream>

#include <netinet/in.h>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>
#include <AMCBase/ConverterClient.h>

using namespace blitz;
namespace LOFAR {
  using namespace BS;
  using namespace RTC;
  using namespace RSP_Protocol;
  using namespace GCF::TM;
  namespace BS {

// global variable for beamformer gain
int g_bf_gain = 0;

//
// BeamServer(name, argc, argv)
//
BeamServer::BeamServer(string name, int argc, char** argv) : 
	GCFTask((State)&BeamServer::initial, name),
	m_beams_modified		(false),
	m_beams		  			(MEPHeader::N_BEAMLETS, MEPHeader::N_SUBBANDS),
	m_converter	  			("localhost"),
	m_instancenr	  		(-1),
	itsUpdateInterval		(UPDATE_INTERVAL),
	itsComputeInterval		(COMPUTE_INTERVAL)
{
	// adopt commandline switches
	parseOptions(argc, argv);

	LOG_INFO(Version::getInfo<BeamServerVersion>("BeamServer"));

	// register protocols for debugging
	registerProtocol(BS_PROTOCOL,  BS_PROTOCOL_STRINGS);
	registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_STRINGS);
	registerProtocol(CAL_PROTOCOL, CAL_PROTOCOL_STRINGS);

	string instanceID;
	if (m_instancenr >= 0) {
		instanceID=formatString("(%d)", m_instancenr);
	}

	// startup TCP connections
	m_acceptor.init (*this, MAC_SVCMASK_BEAMSERVER+ instanceID, GCFPortInterface::MSPP, BS_PROTOCOL);
	m_rspdriver.init(*this, MAC_SVCMASK_RSPDRIVER + instanceID, GCFPortInterface::SAP,  RSP_PROTOCOL);
	m_calserver.init(*this, MAC_SVCMASK_CALSERVER + instanceID, GCFPortInterface::SAP,  CAL_PROTOCOL);

	// alloc a general purpose timer.
	itsUpdateTimer = new GCFTimerPort(*this, "UpdateTimer");

	// read config settings
	itsSetHBAEnabled 	  = (globalParameterSet()->getInt32("BeamServer.DISABLE_SETHBA") == 0);
	itsSetSubbandsEnabled = (globalParameterSet()->getInt32("BeamServer.DISABLE_SETSUBBANDS") == 0);
	itsSetWeightsEnabled  = (globalParameterSet()->getInt32("BeamServer.DISABLE_SETWEIGHTS") == 0);

	// get interval information
	itsHbaInterval 		  = globalParameterSet()->getInt32("BeamServer.HBA_INTERVAL", COMPUTE_INTERVAL);
	if (itsHbaInterval < 2) {
		LOG_FATAL("HBA_INTERVAL to small, must be greater or equal to 2 seconds");
		exit(EXIT_FAILURE);
	}

	// needed for HBA testing
	if (itsHbaInterval < itsComputeInterval) {
		itsComputeInterval = itsHbaInterval;
		itsUpdateInterval = (long)(itsHbaInterval / 2);
	}	
}

//
// ~BeamServer
//
BeamServer::~BeamServer()
{}

// ------------------------------ State machines ------------------------------

//
// initial(event, port)
//
// Get connected to RSPDriver and Calserver and resize arrays with info
// from RSPDriver.
//
GCFEvent::TResult BeamServer::initial(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("initial:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch(event.signal) {
	case F_INIT: {
		ConfigLocator cl;

		// load the HBA Deltas file
		getAllHBADeltas(cl.locate(GET_CONFIG_STRING("BeamServer.HBADeltasFile")));
		LOG_DEBUG_STR("Loading HBADeltas File");

		// load the HBA Delays file
		getAllHBAElementDelays(cl.locate(GET_CONFIG_STRING("BeamServer.HBAElementDelaysFile")));
		LOG_DEBUG_STR("Loading HBADelays File");

	} break;

	case F_ENTRY: {
		// Check explicitly for S_DISCONNECTED state instead of !isConnected()
		// because a transition from ::enabled into ::initial will have state S_CLOSING
		// which is also !isConnected() but in this case we don't want to open the port
		// until after we have handled F_CLOSED for that port.
		if (m_rspdriver.getState() == GCFPortInterface::S_DISCONNECTED) {
			m_rspdriver.open();
		}
		if (m_calserver.getState() == GCFPortInterface::S_DISCONNECTED) {
			m_calserver.open();
		}
	}
	break;

	case F_CONNECTED: {
		if (isEnabled()) {					// all connections made?
			RSPGetconfigEvent getconfig;	// ask RSPDriver current config
			m_rspdriver.send(getconfig);
		}
	}
	break;

	case RSP_GETCONFIGACK: {				// answer from RSPDriver
		RSPGetconfigackEvent ack(event);

		// resize our array to number of current RCUs
		m_nrcus = ack.n_rcus;
		m_weights.resize   (itsComputeInterval, m_nrcus, MEPHeader::N_BEAMLETS);
		m_weights16.resize (itsComputeInterval, m_nrcus, MEPHeader::N_BEAMLETS);

		// initialize matrices
		m_weights   = complex<double>(0,0);
		m_weights16 = complex<int16_t>(0,0);

		// start update timer and start accepting clients
		LOG_DEBUG_STR("Starting Update timer with interval: " << itsUpdateInterval << " secs");
		itsUpdateTimer->setTimer(0, 0, itsUpdateInterval, 0);

		if (m_acceptor.getState() == GCFPortInterface::S_DISCONNECTED) {
			m_acceptor.open();
		}
		TRAN(BeamServer::enabled);
	}
	break;

	case F_DISCONNECTED: {
		port.close();
		// try connecting again in 2 seconds
		port.setTimer((long)2);
		LOG_INFO(formatString("port '%s' disconnected, retry in 2 seconds...", port.getName().c_str()));
	}
	break;

	case F_TIMER: {
		if (port.getState() == GCFPortInterface::S_DISCONNECTED) {
			LOG_INFO(formatString("port '%s' retry of open...", port.getName().c_str()));
			port.open();
		}
	}
	break;

	default:
		LOG_DEBUG("initial:default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

//
// enabled(event, port)
//
GCFEvent::TResult BeamServer::enabled(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("enabled:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult	status = GCFEvent::HANDLED;
	static int 			weightsPeriod = 0;
	static int			hbaPeriod = 0;
	
	undertaker();

	switch (event.signal) {
	case F_ENTRY: {
		m_calserver.setTimer((long)0); // trigger single recall
	}
	break;

	case F_ACCEPT_REQ: {
		GCFTCPPort* client = new GCFTCPPort();
		client->init(*this, "client", GCFPortInterface::SPP, BS_PROTOCOL);
		if (!m_acceptor.accept(*client)) {
			delete client;
		} else {
			m_client_list.push_back(client);
			LOG_INFO(formatString("NEW CLIENT CONNECTED: %d clients connected", 
																m_client_list.size()));
		}
	}
	break;

	case F_CONNECTED:
	break;

	case F_TIMER: {
		// Calserver timer?
		if (&port == &m_calserver) {
			// recall one deferred event, set timer again if an event was handled
			if (recall(port) == GCFEvent::HANDLED) {
				m_calserver.setTimer((long)0);
			}
		} 
		// Update timer?
		else if (&port == itsUpdateTimer) {
			GCFTimerEvent* timer = static_cast<GCFTimerEvent*>(&event);
			LOG_DEBUG_STR("timer=" << Timestamp(timer->sec, timer->usec));
			
			weightsPeriod += itsUpdateInterval;
			if (weightsPeriod >= itsComputeInterval) {
				weightsPeriod = 0;
				// compute new weights and send them weights
				LOG_INFO_STR("computing weights " << Timestamp(timer->sec, timer->usec));
				compute_weights(Timestamp(timer->sec, 0) + LEADIN_TIME);
				send_weights   (Timestamp(timer->sec, 0) + LEADIN_TIME);
			}
			
			hbaPeriod += itsUpdateInterval; 
			if (hbaPeriod >= itsHbaInterval) {
				hbaPeriod = 0;
				if (itsSetHBAEnabled) {
					LOG_INFO_STR("computing HBA delays " << Timestamp(timer->sec, timer->usec));
					compute_HBAdelays(Timestamp(timer->sec,0) + LEADIN_TIME);
					send_HBAdelays   (Timestamp(timer->sec,0) + LEADIN_TIME);
				}
			}
		
			if (m_beams_modified) {
				send_sbselection();
				m_beams_modified = false;
			}
		} // port == itsUpdateTimer

	} // case
	break;

	case BS_BEAMALLOC: {
		BSBeamallocEvent 	allocEvent(event);
		if (beamalloc_start(allocEvent, port)) {
			LOG_INFO_STR("Beam allocation started, going to beamalloc_state");
			TRAN(BeamServer::beamalloc_state);
		}
	}
	break;

	case BS_BEAMFREE: {
		BSBeamfreeEvent 	freeEvent(event);
		if (beamfree_start(freeEvent, port)) {
			LOG_DEBUG_STR("Beam freeing started, going to beamfree_state");
			TRAN(BeamServer::beamfree_state);
		}
	}
	break;

	case BS_BEAMPOINTTO: {
		BSBeampointtoEvent 	pointingEvent(event);
		beampointto_action (pointingEvent, port);
	}
	break;

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

	case CAL_UPDATE: {
		CALUpdateEvent calupd(event);
		if (calupd.status != CAL_Protocol::CAL_SUCCESS) {
			LOG_INFO_STR("Received valid CAL_UPDATE event(" << calupd.status << ").");
			m_beams.updateCalibration(calupd.handle, calupd.gains);
		}
	}
	break;

	case F_DISCONNECTED: {
		if (&m_rspdriver == &port || &m_acceptor == &port || &m_calserver == &port) {
			LOG_WARN("Lost connection with CalServer or RSPDriver, going to 'cleanup'");
			TRAN(BeamServer::cleanup);
		} else {	// some client
			port.close();
			destroyAllBeams(&port);
			m_client_list.remove(&port);
			m_dead_clients.push_back(&port);
		}
	}
	break;

	case F_EXIT:
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
GCFEvent::TResult BeamServer::cleanup(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("cleanup:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: {
		// close connection with all clients.
		if (m_client_list.size() > 0) {
			for (list<GCFPortInterface*>::iterator it = m_client_list.begin();
													it != m_client_list.end(); it++) {
				if ((*it)->isConnected()) {
					(*it)->close();
					destroyAllBeams(*it);
					m_client_list.remove(*it);
					m_dead_clients.push_back(*it);
				}
			}
		}

		m_rspdriver.cancelAllTimers();
		m_rspdriver.close();
		m_acceptor.close();
		m_calserver.close();
		itsUpdateTimer->cancelAllTimers();
		itsUpdateTimer->setTimer(2.0);		// give ports time to close.
	}
	break;

	case F_TIMER: {
		LOG_DEBUG("closed all TCP ports, going to 'initial' state again");
		TRAN(BeamServer::initial);
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
GCFEvent::TResult BeamServer::beamalloc_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("beamalloc_state:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: {
		// subscribe to calibration updates
		CALSubscribeEvent subscribe;
		subscribe.name 		 = m_bt.getBeam()->getSubarrayName();
		subscribe.subbandset = m_bt.getBeam()->getAllocation().getAsBitset();

		LOG_INFO_STR("Subscribing to subarray: " << subscribe.name);
		m_calserver.send(subscribe);
	}
	break;

	case CAL_SUBSCRIBEACK: {
		CALSubscribeackEvent ack(event);
		BSBeamallocackEvent beamallocack;

		if (ack.status == CAL_Protocol::CAL_SUCCESS && 
				(ack.subarray.getSPW().getNumSubbands() >= 
				(int)m_bt.getBeam()->getAllocation()().size()) ) {

			LOG_INFO_STR("Got subscription to subarray " << ack.subarray.getName());
			LOG_DEBUG_STR("ack.subarray.positions=" << ack.subarray.getAntennaPos());

			// set positions on beam
			m_bt.getBeam()->setSubarray(ack.subarray);

			// set calibration handle for this beam
			m_beams.setCalibrationHandle(m_bt.getBeam(), ack.handle);

			// send succesful ack
			beamallocack.status 	  = BS_Protocol::BS_SUCCESS;
			beamallocack.subarrayname = ack.subarray.getName();
			beamallocack.handle 	  = (BS_Protocol::memptr_t)m_bt.getBeam();
			m_bt.getPort()->send(beamallocack);
		} 
		else {
			LOG_ERROR("Failed to subscribe to subarray");
			LOG_DEBUG_STR("getNumSubbands()    = " << ack.subarray.getSPW().getNumSubbands());
			LOG_DEBUG_STR("getAllocation.size()= " << (int)m_bt.getBeam()->getAllocation()().size());

			// failed to subscribe
			beamallocack.status = ERR_BEAMALLOC;
			beamallocack.handle = 0;
			m_bt.getPort()->send(beamallocack);

			// delete the beam (resets m_bt)
			deleteBeam(m_bt);
		}

		LOG_DEBUG("Allocation finished, going back to 'enabled' state");
		TRAN(BeamServer::enabled);
	}
	break;

	case F_DISCONNECTED: {
		LOG_INFO(">>> deferring F_DISCONNECTED event <<<");
		defer(event, port); // process F_DISCONNECTED again in enabled state

		if (&port == m_bt.getPort()) {
			LOG_WARN("Lost connection, going back to 'enabled' state");
			TRAN(BeamServer::enabled);
		}
	}
	break;

	case F_EXIT: {
		// completed current transaction, reset
		m_bt.reset();
	}
	break;

	default:
		LOG_DEBUG("beamalloc_state:default --> defer command");
		// all other events are handled in the enabled state
		defer(event, port);
		break;
	}

	return status;
}

//
// beamfree_state(event, port)
//
GCFEvent::TResult BeamServer::beamfree_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("beamfree_state:" << eventName(event) << "@" << port.getName());
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: {
		// unsubscribe
		CALUnsubscribeEvent unsubscribe;
		unsubscribe.name 	= m_bt.getBeam()->getSubarrayName();
		unsubscribe.handle  = m_beams.findCalibrationHandle(m_bt.getBeam());
		ASSERT(unsubscribe.handle != 0);
		m_calserver.send(unsubscribe);
	}
	break;

	case CAL_UNSUBSCRIBEACK: {
		CALUnsubscribeackEvent	ack(event);
		BSBeamfreeackEvent 		beamfreeack;

		// if the subarray disappeared because
		// it was stopped (CAL_STOP by SRG) then
		// issue a warning but continue
		if (ack.status != CAL_Protocol::CAL_SUCCESS) {
			LOG_WARN("CAL_UNSUBSCRIBE failed");
		}

		// send succesful ack
		beamfreeack.status = BS_Protocol::BS_SUCCESS;
		beamfreeack.handle = (BS_Protocol::memptr_t)m_bt.getBeam();

		m_bt.getPort()->send(beamfreeack);

		// destroy beam, updates m_bt
		deleteBeam(m_bt);

		LOG_DEBUG("Beamfree finished, going back to 'enabled' state");
		TRAN(BeamServer::enabled);
	}
	break;

	case F_DISCONNECTED: {
		LOG_INFO(">>> deferring F_DISCONNECTED event <<<");
		defer(event, port); // process F_DISCONNECTED again in enabled state

		if (&port == m_bt.getPort()) {
			LOG_WARN("Lost connection, going back to 'enabled' state");
			TRAN(BeamServer::enabled);
		}
	}
	break;

	case F_EXIT: {
		// delete beam if not already done so
		if (m_bt.getBeam()) {
			deleteBeam(m_bt);
		}

		// completed current transaction, reset
		m_bt.reset();
	}
	break;

	default:
		LOG_DEBUG("beamfree_state:default --> defer command");
		// all other events are handled in the enabled state
		defer(event, port);
		break;
	}

	return (status);
}

// ------------------------------ internal routines ------------------------------

//
// parseOptions
//
void BeamServer::parseOptions(int	argc, char**	argv)
{
	static struct option long_options[] = {
		{ "instance",   required_argument, 0, 'I' },
		{ "daemonize",  optional_argument, 0, 'd' },
		{ 0, 0, 0, 0 },
	};

	optind = 0; // reset option parsing
	for(;;) {
		int option_index = 0;
		int c = getopt_long(argc, argv, "dI:", long_options, &option_index);

		if (c == -1) {
			break;
		}

		switch (c) {
		case 'I': 	// --instance
			m_instancenr = atoi(optarg);
			break;
		case 'd':
			break;
		default:
			LOG_FATAL (formatString("Unknown option %c", c));
			ASSERT(false);
		} // switch
	} // for loop
}

//
// isEnabled()
//
bool BeamServer::isEnabled()
{
	return (m_rspdriver.isConnected() && m_calserver.isConnected());
}

//
// undertaker()
//
void BeamServer::undertaker()
{
	// cleanup old connections
	for (list<GCFPortInterface*>::iterator it = m_dead_clients.begin();
											it != m_dead_clients.end(); it++) {
		LOG_DEBUG_STR("undertaker: deleting '" << (*it)->getName() << "'");
		delete (*it);
	}
	m_dead_clients.clear();
}

//
// destroyAllBeams(port)
//
void BeamServer::destroyAllBeams(GCFPortInterface* port)
{
	ASSERT(port);

	// deallocate all beams for this client
	for (set<Beam*>::iterator beamit = m_client_beams[port].begin();
								beamit != m_client_beams[port].end(); ++beamit) {
		if (!m_beams.destroy(*beamit)) {
			LOG_WARN("Beam not found...");
		}
	}
	m_client_beams.erase(port);
}

//
// newBeam(beamTransaction, port , name, subarray, beamletAllocation)
//
Beam* BeamServer::newBeam(BeamTransaction& 					bt, 
						  GCFPortInterface* 				port,
						  std::string 						name, 
						  std::string 						subarrayname, 
						  BS_Protocol::Beamlet2SubbandMap	allocation)
{
	LOG_TRACE_FLOW_STR("newBeam(" << port->getName() << "," << name << "," << subarrayname);

	ASSERT(port);

	// check for valid parameters
	// returning 0 will result in a negative ACK
	if (bt.getBeam() != 0 || bt.getPort() != 0) {
		LOG_ERROR("Previous alloc is still in progress");
		 return (0);
	}
	if (name.length() == 0) {
		LOG_ERROR("Name of beam not set, cannot alloc new beam");
		return (0);
	}
	if (subarrayname.length() == 0)  {
		LOG_ERROR("SubArrayName not set, cannot alloc new beam");
		return (0); 
	}

	Beam* beam = m_beams.create(name, subarrayname, allocation);

	if (beam) { // register new beam
		m_client_beams[port].insert(beam);
		m_beams_modified = true;
		bt.set(port, beam);
	}

	return (beam);
}

//
// deleteBeam(beamTransaction)
//
void BeamServer::deleteBeam(BeamTransaction& bt)
{
	ASSERT(bt.getPort() && bt.getBeam());

	// destroy beam
	if (!m_beams.destroy(bt.getBeam())) {
		LOG_WARN("Beam not found...");
	}

	// unregister beam
	m_client_beams[bt.getPort()].erase(bt.getBeam());

	bt.reset();

	// update flag to trigger update of
	// subband selection settings
	m_beams_modified = true;
}

//
// getAllHBADeltas 
//
void BeamServer::getAllHBADeltas(string filename)
{
	ifstream itsFile;
	string itsName;

	// open new file
	if (itsFile.is_open()) {
		itsFile.close();
	}
	itsFile.open(filename.c_str());

	if (!itsFile.good()) {
		itsFile.close();
		return;
	}

	// The file may have comment lines at the top, starting with '#'
	// These must be skipped
	itsName = "#";
	while (itsName[0] == "#") {
	  
	  getline(itsFile, itsName); // read name
	  LOG_DEBUG_STR("HBADeltas Name = " << itsName);
	  if ("" == itsName) {
	    itsFile.close();
	    return;
	  }
	}

	// Now comment lines are skipped, so we can read the full array.
	itsFile >> itsTileRelPos; // read HBA deltas array
	LOG_DEBUG_STR("HBADeltas = " << itsTileRelPos);
}

//
// getAllHBAElementDelays 
//
void BeamServer::getAllHBAElementDelays(string filename)
{
	ifstream itsFile;
	string itsName;

	// open new file
	if (itsFile.is_open()) { 
		itsFile.close();
	}
	itsFile.open(filename.c_str());

	if (!itsFile.good()) {
		itsFile.close();
		return;
	}

	// The file may have comment lines at the top, starting with '#'
	// These must be skipped
	itsName = "#";
	while (itsName[0] == "#") {
	  getline(itsFile, itsName); // read name
	  LOG_DEBUG_STR("HBA ElementDelays Name = " << itsName);
	  if ("" == itsName) {
	    itsFile.close();
	    return;
	  }
	}

	// Now comment lines are skipped, so we can read the full array.
	itsFile >> itsElementDelays; // read HBA element delays array
	//itsElementDelays *= 1E-9; // convert from nSec to Secs
	LOG_DEBUG_STR("HBA ElementDelays = " << itsElementDelays);
}

//
// beamalloc_start(AllocEvent, port)
//
bool BeamServer::beamalloc_start(BSBeamallocEvent& ba,
								 GCFPortInterface& port)
{
	// allocate the beam
	Beam* beam = newBeam(m_bt, &port, ba.name, ba.subarrayname, ba.allocation);

	if (!beam) {
		LOG_FATAL_STR("BEAMALLOC: failed to allocate beam " << ba.name << " on " << ba.subarrayname);

		BSBeamallocackEvent ack;
		ack.handle 		 = 0;
		ack.subarrayname = ba.subarrayname;
		ack.status 		 = BS_Protocol::ERR_RANGE;
		port.send(ack);

		return (false);
	}

	return (true);
}

//
// beamfree_start(freeEvent, port)
//
bool BeamServer::beamfree_start(BSBeamfreeEvent&  bf,
								GCFPortInterface& port)
{
	Beam* beam = (Beam*)bf.handle;

	if (!m_beams.exists(beam)) { 
		LOG_FATAL_STR("BEAMFREE failed: beam " << (Beam*)bf.handle << " does not exist");

		BSBeamfreeackEvent ack;
		ack.handle = bf.handle;
		ack.status = BS_Protocol::ERR_BEAMFREE;
		port.send(ack);

		return (false);
	}

	// remember on which beam we're working
	m_bt.set(&port, beam);

	return (true);
}

//
// beampointto_action(pointEvent, port)
//
bool BeamServer::beampointto_action(BSBeampointtoEvent& pt,
									GCFPortInterface& /*port*/)
{
	Beam*	beam   = (Beam*)pt.handle;

	// check if beam exists.
	if (!m_beams.exists(beam))  {
		LOG_ERROR(formatString("BEAMPOINTTO: invalid beam handle (%d)", pt.handle));
		return (false);
	}

	LOG_INFO_STR("new coordinates for " << beam->getName()
				 << ": " << pt.pointing.angle0() << ", "
				 << pt.pointing.angle1() << ", time=" << pt.pointing.time());

	//
	// If the time is not set, then activate the command
	// 2 * COMPUTE_INTERVAL seconds from now, because that's how
	// long it takes the command to flow through the pipeline.
	//
	Timestamp actualtime;
	actualtime.setNow(2 * COMPUTE_INTERVAL);
	if (pt.pointing.time() == Timestamp(0,0)) {
		pt.pointing.setTime(actualtime);
	}
	beam->addPointing(pt.pointing);

	return (true);
}

BZ_DECLARE_FUNCTION_RET(convert2complex_int16_t, complex<int16_t>)

//
// Convert the weights to 16-bits signed integer.
//
inline complex<int16_t> convert2complex_int16_t(complex<double> cd)
{
	return complex<int16_t>((int16_t)(round(cd.real() * g_bf_gain)),
							(int16_t)(round(cd.imag() * g_bf_gain)));
}

//
// compute_HBAdelays(time)
//
// This method is called once every period of HBA_INTERVAL seconds
// to calculate the weights for all HBA beam(let)s.
//
void BeamServer::compute_HBAdelays(Timestamp time)
{
	// calculate HBA delays for all HBA beams.
	m_beams.calculateHBAdelays(time, itsHbaInterval, &m_converter, itsTileRelPos, itsElementDelays);
}

//
// send_HBAdelays(time)
//
void BeamServer::send_HBAdelays(Timestamp	time)
{
	// We must loop over all beams, unfortunately we don't have such a list
	// because only the beam-factory has access to all beams. So we must
	// implement the sending of the HBA delays in the beamfactory!
//	LOG_INFO_STR("sending hba delays for interval " << time << " : " << time + (long)(itsComputeInterval-1));
	m_beams.sendHBAdelays(time, m_rspdriver);
}

//
// compute_weights(time)
//
// This method is called once every period of COMPUTE_INTERVAL seconds
// to calculate the weights for all beamlets.
//
void BeamServer::compute_weights(Timestamp time)
{
	// calculate weights for all beamlets
	m_beams.calculate_weights(time, itsComputeInterval, &m_converter, m_weights);

	// convert the weights from double to int16
	m_weights16 = convert2complex_int16_t(m_weights);

	LOG_DEBUG(formatString("sizeof(m_weights16) = %d", m_weights16.size()*sizeof(int16_t)));
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
	LOG_DEBUG_STR("sw.time=" << sw.timestamp);
  
	// select all BLPS, no subarraying
	sw.rcumask.reset();
	for (int i = 0; i < m_nrcus; i++) {
		sw.rcumask.set(i);
	}
  
	sw.weights().resize(itsComputeInterval, m_nrcus, MEPHeader::N_BEAMLETS);
	sw.weights() = m_weights16;
  
	LOG_INFO_STR("sending weights for interval " << time << " : " << time + (long)(itsComputeInterval-1));

//  for debugging purposes print 40 subbans of 10 antennas for 1 timestamp.
//	blitz::Array<std::complex<int16>, 2>	sample;
//	sample.resize (10, 40);		// first 10 antennas and first 40 subbands
//	sample = complex<int16_t>(0,0);
//	sample = m_weights16(0, Range(64,74), Range(0,40));
//	LOG_DEBUG_STR("weights sample=" << sample);

	m_rspdriver.send(sw);
}

//
// send_sbselection()
//
void BeamServer::send_sbselection()
{
	if (!itsSetSubbandsEnabled) {
		return;
	}

	RSPSetsubbandsEvent ss;
	ss.timestamp.setNow(0);
	ss.rcumask.reset(); 		// select all BLPS, no subarraying
	for (int i = 0; i < m_nrcus; i++) {
		ss.rcumask.set(i);
	}
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

	Beamlet2SubbandMap sbsel = m_beams.getSubbandSelection();
	LOG_DEBUG(formatString("nrsubbands=%d", sbsel().size()));

	for (map<uint16,uint16>::iterator sel = sbsel().begin();
										sel != sbsel().end(); ++sel) {
		LOG_DEBUG(formatString("(%d,%d)", sel->first, sel->second));

		if (sel->first >= MEPHeader::N_BEAMLETS) {
			LOG_ERROR(formatString("SBSELECTION: invalid src index %d", sel->first));
			continue;
		}

		if (sel->second >= MEPHeader::N_SUBBANDS) {
			LOG_ERROR(formatString("SBSELECTION: invalid tgt index %d", sel->second));
			continue;
		}

		// same selection for x and y polarization
		ss.subbands()(0, (int)sel->first) = sel->second;
	}

	m_rspdriver.send(ss);
}

//
// defer(event, port)
//
void BeamServer::defer(GCFEvent& e, GCFPortInterface& port)
{
	char* 	event = new char[sizeof(e) + e.length];
	memcpy(event, (const char*)&e, sizeof(e) + e.length);
	m_deferred_queue.push_back(pair<char*, GCFPortInterface*>(event, &port));

	LOG_DEBUG_STR(">>> deferring event " << m_deferred_queue.size() << " <<<");
}

//
// recall(port)
//
GCFEvent::TResult BeamServer::recall(GCFPortInterface& /*p*/)
{
	GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

	if (m_deferred_queue.size() > 0) {
		LOG_DEBUG_STR(">>> recalling event " << m_deferred_queue.size() << " <<<");
		pair<char*, GCFPortInterface*> port_event = m_deferred_queue.front();
		m_deferred_queue.pop_front();
		status = doEvent(*(GCFEvent*)(port_event.first), *port_event.second);
		delete [] port_event.first;
	}
	else {
		LOG_DEBUG_STR("Nothing to recall");
	}

	return (status);
}

  } // namespace BS;
} // namespace LOFAR;
