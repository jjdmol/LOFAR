//#
//#  CalServer.cc: implementation of CalServer class
//#
//#  Copyright (C) 2002-2014
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
//#  Note: this file is formatted with tabstop 4
//#
//#  $Id$

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <Common/lofar_bitset.h>
#include <Common/Exception.h>
#include <Common/Version.h>

#include <ApplCommon/LofarDirs.h>

#include <ApplCommon/StationConfig.h>
#include <ApplCommon/StationDatatypes.h>
#include <ApplCommon/AntennaSets.h>

#include <APL/APLCommon/AntennaField.h>
#include <APL/RTCCommon/daemonize.h>
#include <APL/CAL_Protocol/CAL_Protocol.ph>
#include <APL/RSP_Protocol/RSP_Protocol.ph>

#include <MACIO/MACServiceInfo.h>

#include "CalServer.h"
#include "SubArrayMgr.h"
#include "SubArraySubscription.h"
#include <Cal_Server/Package__Version.h>

#ifdef USE_CAL_THREAD
#include "CalibrationThread.h"
#endif

#include "ACMProxy.h"

// from RTCCommon
#include <APL/RTCCommon/Timestamp.h>
#include <APL/RTCCommon/PSAccess.h>

#include <Common/ParameterSet.h>
#include <fstream>
#include <numeric>
#include <signal.h>
#include <getopt.h>

#include <blitz/array.h>

using namespace std;
using namespace blitz;

namespace LOFAR {
  using namespace RTC;
  using namespace RSP_Protocol;
  using namespace CAL_Protocol;
  using namespace GCF::TM;
  namespace CAL {



#define SCHEDULING_DELAY 3

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler t(Exception::terminate);

// static pointer used for signal handler.
static CalServer*	thisCalServer = 0;

//
// CalServer constructor
//
CalServer::CalServer(const string& name, ACCs& accs)
	: GCFTask((State)&CalServer::initial, name),
	m_accs(accs),
	m_converter(0),
	itsClockSetting(0),
	m_n_rspboards(0),
	m_n_rcus(0),
	itsHasSecondRing	(0),
	itsSecondRingActive (true),
	itsFirstRingOn	    (true),
	itsSecondRingOn     (false),
	itsListener			(0),
	itsRSPDriver		(0),
	itsCheckTimer		(0)
#ifdef USE_CAL_THREAD
	, m_calthread(0)
#endif
{
#ifdef USE_CAL_THREAD
	pthread_mutex_init(&m_globallock, 0);
#endif

	LOG_INFO(Version::getInfo<Cal_ServerVersion>("CalServer"));

	if (!GET_CONFIG("CalServer.DisableCalibration", i)) {
		m_converter = new AMC::ConverterClient("localhost");
		ASSERT(m_converter != 0);
	}

	registerProtocol(CAL_PROTOCOL, CAL_PROTOCOL_STRINGS);
	registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_STRINGS);

	itsCheckTimer = new GCFTimerPort(*this, "CheckTimer");
	ASSERTSTR(itsCheckTimer, "Couldn't allocate the timer device");
	itsListener   = new GCFTCPPort(*this, MAC_SVCMASK_CALSERVER, GCFPortInterface::MSPP, CAL_PROTOCOL);
	ASSERTSTR(itsListener, "Couldn't allocate the listener device");
	itsRSPDriver  = new GCFTCPPort(*this, MAC_SVCMASK_RSPDRIVER,  GCFPortInterface::SAP,  RSP_PROTOCOL);
	ASSERTSTR(itsRSPDriver, "Couldn't allocate the port to the RSPDriver");

	StationConfig	SC;
	itsHasSecondRing = SC.hasSplitters;
	if (!itsHasSecondRing) {
		itsSecondRingActive=false;
	}
}

//
// ~CalServer()
//
CalServer::~CalServer()
{
	if (m_converter) delete m_converter;
#ifdef USE_CAL_THREAD
	if (m_calthread) delete m_calthread;
#endif
}

//
// sigintHandler(signum)
//
void CalServer::sigintHandler(int signum)
{
	LOG_WARN (formatString("SIGINT signal detected (%d)",signum));

	// Note we can't call TRAN here because the siginthandler does not know our object.
	if (thisCalServer) {
		thisCalServer->finish();
	}
}

//
// finish
//
void CalServer::finish()
{
	TRAN(CalServer::finishing_state);
}

//
// finishing_state(event, port)
//
// Powerdown all antenna's for safety.
//
GCFEvent::TResult CalServer::finishing_state(GCFEvent& 		event, GCFPortInterface& port)
{
	LOG_INFO_STR ("finishing_state:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		// turn off 'old' timers
		itsCheckTimer->cancelAllTimers();
		RCUmask_t	rcus2switchOff;
		rcus2switchOff.reset();
		for (uint i = 0; i < m_n_rcus; i++) {
			rcus2switchOff.set(i);
		}
		_powerdownRCUs(rcus2switchOff);
		itsCheckTimer->setTimer(0.2);
	} break;

	case F_TIMER: {
		GCFScheduler::instance()->stop();
	} break;

	default:
		LOG_DEBUG("finishing_state, default");
		break;
	}
	return (GCFEvent::HANDLED);
}

//
// undertaker()
//
void CalServer::undertaker()
{
	for (list<GCFPortInterface*>::iterator it = m_dead_clients.begin(); it != m_dead_clients.end(); ++it) {
		delete (*it);
	}
	m_dead_clients.clear();
}

//
// remove_client(port)
//
// A disconnect was received on the given port, stop all related subarrays.
//
void CalServer::remove_client(GCFPortInterface* port)
{
	ASSERTSTR(port != 0, "Trying to remove an already deleted port");

	map<string, GCFPortInterface*>::iterator	iter = m_clients.begin();
	map<string, GCFPortInterface*>::iterator	end  = m_clients.end();
	while (iter != end) {
		if (iter->second == port) {
			// stop subarray if it is still there.
			SubArray* subarray = itsSubArrays.getByName(iter->first);
			if (subarray) {
				_disableRCUs(subarray);
				itsSubArrays.scheduleRemove(subarray);
			}

			// add to dead list.
			m_dead_clients.push_back(iter->second);

			// remove entry from the map we are searching.
			string	subArrayName(iter->first);
			iter++;
			m_clients.erase(subArrayName);
		}
		else {
			iter++;
		} // if port matches
	} // while
}

//
// initial(event,port)
//
GCFEvent::TResult CalServer::initial(GCFEvent& e, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch(e.signal) {
	case F_INIT: {
		try {
#ifdef USE_CAL_THREAD
			pthread_mutex_lock(&m_globallock); // lock for dipolemodels, and sources
#endif
			ConfigLocator cl;

			// load the dipole models
			m_dipolemodels.getAll(cl.locate(GET_CONFIG_STRING("CalServer.DipoleModelFile")));

			// load the source catalog
			m_sources.getAll(cl.locate(GET_CONFIG_STRING("CalServer.SourceCatalogFile")));

            // load the antenna positions
			ASSERTSTR(globalAntennaField(), "Could not load the antennaposition file");
			LOG_DEBUG("Loaded antenna postions file");

			// Load antenna arrays
			//m_arrays.getAll(cl.locate(GET_CONFIG_STRING("CalServer.AntennaArraysFile")));
			//ASSERTSTR(!m_arrays.getNameList().empty(), "No antenna positions found");

			// Setup datapath
			itsDataDir = globalParameterSet()->getString("CalServer.DataDirectory", "/opt/lofar/bin");
			if (itsDataDir.empty()) {
				itsDataDir="/opt/lofar/bin";
			}
			else if ((*itsDataDir.rbegin()) == '/') {		// strip off last /
				itsDataDir.erase(itsDataDir.length()-1);
			}
			LOG_INFO_STR("Interim datafile will be stored in " << itsDataDir);

			// get HBA poweroff delay.
			itsPowerOffDelay = globalParameterSet()->getTime("CalServer.PowerOffDelay", 180);
			LOG_INFO_STR("PowerOff delay = " << itsPowerOffDelay << " seconds.");

#ifdef USE_CAL_THREAD
			// Setup calibration thread
			m_calthread = new CalibrationThread(&itsSubArrays, m_globallock, itsDataDir);

			pthread_mutex_unlock(&m_globallock); // unlock global lock
#endif
		} catch (Exception& e)  {

#ifdef USE_CAL_THREAD
			pthread_mutex_unlock(&m_globallock); // unlock global lock
#endif
			LOG_ERROR_STR("Failed to load configuration files: " << e);
			exit(EXIT_FAILURE);
		}
	}
	break;

	case F_ENTRY: {
		// Open connections with outside world
		if (!itsListener->isConnected()) {
			itsListener->open();
		}
		LOG_DEBUG("opening port: itsRSPDriver");
		itsRSPDriver->autoOpen(30, 0, 2);      // try max 30 times every 2 seconds than report DISCO
	}
	break;

	case F_CONNECTED: {
		// Wait till both connections are on the air
		if ( itsListener->isConnected() && itsRSPDriver->isConnected()) {
			RSPGetconfigEvent getconfig;
			itsRSPDriver->send(getconfig);
		}
	}
	break;

	case RSP_GETCONFIGACK: {
		RSPGetconfigackEvent ack(e);
		// TODO: check status
		m_n_rspboards = ack.n_rspboards;
		m_n_rcus = ack.n_rcus;
		LOG_INFO_STR("nrRSPboards=" << m_n_rspboards << ", nrRCUS=" << m_n_rcus);
		// resize and clear itsRCUcounters.
		itsRCUcounts.assign(m_n_rcus, 0);
		itsPowerOffTime.assign(m_n_rcus, 0);

		// get initial clock setting
		RSPGetclockEvent getclock;
		getclock.timestamp = Timestamp(0,0);
		getclock.cache = true;

		itsRSPDriver->send(getclock);
	}
	break;

	case RSP_GETCLOCKACK: {
		RSPGetclockackEvent getclockack(e);
		if (getclockack.status != RSP_Protocol::RSP_SUCCESS) {
			LOG_FATAL("Failed to get sampling frequency setting");
			exit(EXIT_FAILURE);
		}

		// get clock value in MHz
		itsClockSetting = getclockack.clock;

		LOG_INFO_STR("Initial sampling frequency: " << itsClockSetting << " MHz");

		// subscribe to clock change updates
		RSPSubclockEvent subclock;
		subclock.timestamp = Timestamp(0,0);
		subclock.period = 1;

		itsRSPDriver->send(subclock);
	}
	break;

	case RSP_SUBCLOCKACK: {
		RSPSubclockackEvent ack(e);
		if (ack.status != RSP_Protocol::RSP_SUCCESS) {
			LOG_FATAL("Failed to subscribe to clock status updates.");
			exit(EXIT_FAILURE);
		}

		// subscribe to the splittersettings if this station has splitters.
		if (itsHasSecondRing) {
			RSPSubsplitterEvent		subsplitter;
			subsplitter.timestamp = Timestamp(0,0);
			subsplitter.period    = 1;
			itsRSPDriver->send(subsplitter);
		}
		else {
			TRAN(CalServer::enabled);
		}
	}
	break;

	case RSP_SUBSPLITTERACK: {
		RSPSubsplitterackEvent ack(e);
		if (ack.status != RSP_Protocol::RSP_SUCCESS) {
			LOG_FATAL("Failed to subscribe to splitter status updates.");
			exit(EXIT_FAILURE);
		}
		TRAN(CalServer::enabled);
	}
	break;

	case F_DISCONNECTED: {
		LOG_DEBUG(formatString("port '%s' disconnected, retry in 3 seconds...", port.getName().c_str()));
		port.close();
		port.setTimer(3.0);
	}
	break;

	case F_TIMER: {
		if (!port.isConnected()) {
			LOG_DEBUG(formatString("port '%s' retry of open...", port.getName().c_str()));
			port.open();
		}
	}
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
	break;
	}

	return status;
}

//
// enabled(event,port)
//
GCFEvent::TResult CalServer::enabled(GCFEvent& e, GCFPortInterface& port)
{
	LOG_DEBUG_STR("enabled: " << eventName(e) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY: {
		// switch off receivers by default.
		_disableRCUs(0);
		itsCheckTimer->setTimer(0.0, 1.0);

		// redirect signalhandler to always powerdown the antenna's when quiting...
		thisCalServer = this;
		signal (SIGINT,  CalServer::sigintHandler);	// ctrl-c
		signal (SIGTERM, CalServer::sigintHandler);	// kill
		signal (SIGABRT, CalServer::sigintHandler);	// kill -6

	}
	break;

	case F_ACCEPT_REQ: {
		GCFTCPPort* client = new GCFTCPPort();
		client->init(*this, "client", GCFPortInterface::SPP, CAL_PROTOCOL);
		if (!itsListener->accept(*client)) {
			delete client;
			LOG_ERROR ("Could not setup a connection with a new client");
		}
//		else {
//			m_clients[client] = ""; // empty string to indicate there is a connection, but no subarray yet
//			LOG_INFO(formatString("NEW CLIENT CONNECTED: %d clients connected", m_clients.size()));
//		}
	}
	break;

	case RSP_UPDCLOCK: {
		RSPUpdclockEvent updclock(e);
		// use new sampling frequency
		itsClockSetting = updclock.clock * (uint32)1.0e6;
		LOG_INFO_STR("New sampling frequency: " << itsClockSetting);
	}
	break;

	case RSP_UPDSPLITTER: {
		RSPUpdsplitterEvent updsplitter(e);
		// update admin
		itsSecondRingActive = (updsplitter.splitter.count() == m_n_rspboards);
		LOG_INFO_STR("Second ring is " << (itsSecondRingActive ? "" : "not ") << "active.");
		_updateDataStream(0);
	}
	break;

	case RSP_SETRCUACK: {
		RSPSetrcuackEvent ack(e);
		if (ack.status == RSP_Protocol::RSP_BUSY) {
			// We could be switching off the RCU during a clock switch for the next
			// observation. In that case, we don't need to try again.
			LOG_INFO_STR("Failed to set RCU control register: RSP is busy.");
		} else if (ack.status != RSP_Protocol::RSP_SUCCESS) {
			LOG_FATAL_STR("Failed to set RCU control register.");
			exit (EXIT_FAILURE);
		}
	}
	break;

	case RSP_SETBYPASSACK: {
		RSPSetbypassackEvent ack(e);
		if (ack.status != RSP_Protocol::RSP_SUCCESS) {
			LOG_FATAL_STR("Failed to set Spectral Inversion control register.");
			exit (EXIT_FAILURE);
		}
	}
	break;

	case F_CONNECTED: {
		LOG_INFO(formatString("CONNECTED: port '%s'", port.getName().c_str()));
	}
	break;

	case F_TIMER: {
		// first check Poweroff timer
		RCUmask_t		rcus2switchOff;
		rcus2switchOff.reset();
		time_t		now(time(0L));
		for (uint i = 0; i < m_n_rcus; i++) {				// loop over all rcus
			if (itsPowerOffTime[i] && itsPowerOffTime[i] <= now) {	// elapsed?
				rcus2switchOff.set(i);
				itsPowerOffTime[i] = 0;
			}
		} // for
		if (rcus2switchOff.any()) {
			LOG_INFO ("Delayed power-off of HBA tiles.");
			_powerdownRCUs(rcus2switchOff);		// Finally shut them down.
		}

		// swap buffers
		GCFTimerEvent* timer = static_cast<GCFTimerEvent*>(&e);
		const Timestamp t = Timestamp(timer->sec, timer->usec);
		LOG_DEBUG_STR("updateAll @ " << t);

		//
		// Swap buffers when all calibrations have finished on the front buffer
		// and the back buffer is not locked and is valid (has been filled by ACMProxy).
		//
		if (!m_accs.getFront().isLocked() && !m_accs.getBack().isLocked() && m_accs.getBack().isValid()) {
			LOG_INFO("swapping buffers");

			// start new calibration
			m_accs.swap();
			m_accs.getBack().invalidate(); // invalidate

#ifdef USE_CAL_THREAD
			// join previous calibration thread
			(void)m_calthread->join();
#endif

			itsSubArrays.mutex_lock();
			undertaker(); // destroy dead clients, done here to prevent possible use of closed port
			itsSubArrays.removeDeadArrays();  // remove subarrays scheduled for deletion
			itsSubArrays.activateArrays();     // bring new subarrays to life
			itsSubArrays.mutex_unlock();

			if (GET_CONFIG("CalServer.WriteACCToFile", i)) {
				write_acc();
			}

#ifdef USE_CAL_THREAD
			// start calibration thread
			m_calthread->setACC(&m_accs.getFront());
			m_calthread->run();
#else
			bool	writeGains(GET_CONFIG("CalServer.WriteGainsToFile", i) == 1);
            itsSubArrays.calibrate(m_accs.getFront(), writeGains, itsDataDir);
			itsSubArrays.updateAll();
#endif
		}

#ifdef USE_CAL_THREAD
		// itsSubArrays.updateAll();  // TODO: is update still needed
#endif
	}
	break;

	case F_DISCONNECTED: {
		LOG_INFO(formatString("DISCONNECTED: port %s disconnected", port.getName().c_str()));
		port.close();

		if (itsListener == &port) {
			TRAN(CalServer::initial);
		}
		else {
			// destroy subarray
			remove_client(&port);
		}
	}
	break;

	case CAL_START:
		status = handle_cal_start(e, port);
	break;

	case CAL_STOP:
		status = handle_cal_stop(e, port);
	break;

	case CAL_SUBSCRIBE:
		status = handle_cal_subscribe(e, port);
	break;

	case CAL_UNSUBSCRIBE:
		status = handle_cal_unsubscribe(e, port);
	break;

	case CAL_GETSUBARRAY:
		status = handle_cal_getsubarray(e, port);
	break;

	case F_EXIT:
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
	break;
	}

	return status;
}


//
// handle_cal_start(event, port)
//
GCFEvent::TResult CalServer::handle_cal_start(GCFEvent& e, GCFPortInterface &port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	CALStartEvent 	 start(e);
	CALStartackEvent ack;

	LOG_DEBUG_STR("Received CAL_START:" << start);

	ack.status      = CAL_Protocol::CAL_SUCCESS; // assume succes, until otherwise
	ack.name        = start.name;

	if (itsSubArrays.getByName(start.name)) {
		LOG_ERROR_STR("A subarray with name='" << start.name << "' has already been registered.");
		ack.status = ERR_ALREADY_REGISTERED;

	} else if (string(start.name) == "") {
		LOG_ERROR("Empty subarray name.");
		ack.status = ERR_NO_SUBARRAY_NAME;

	} else if (!globalAntennaSets()->isAntennaSet(start.antennaSet)) {
		// AntenneSet not found, set error status
		LOG_ERROR_STR("AntennaSet '" << start.antennaSet << "' not found.");
		ack.status = ERR_NO_PARENT;

    } else if (globalAntennaSets()->usesLBAfield(start.antennaSet) &&
              ((start.band < BAND_10_70) || (start.band > BAND_30_90))) {
            LOG_ERROR_STR("AntennaSet and band do not match '" << start.antennaSet << ", " <<  start.band << "'.");
            ack.status = ERR_WRONG_BAND;
    } else if (!globalAntennaSets()->usesLBAfield(start.antennaSet) &&
              ((start.band < BAND_110_190) || (start.band > BAND_210_250))) {
            LOG_ERROR_STR("AntennaSet and band do not match '" << start.antennaSet << ", " <<  start.band << "'.");
            ack.status = ERR_WRONG_BAND;
	} else if (start.rcuMask.count() == 0) {
		// empty selection
		LOG_ERROR("Empty antenna selection not allowed.");
		ack.status = ERR_NO_ANTENNAS;

//	TODO: add this code for faster calibration in the future
//	} else if (start.subbandset.count() == 0) {
//		// empty selection
//		LOG_ERROR("Empty subband selection not allowed.");
//		ack.status = ERR_NO_SUBBANDS;

	} else {
		// register because this is a cal_start
		m_clients[start.name] = &port;		// register subarray and port
#if 0
		// Construct a 'select' array (nAntennas, nPol) that knows which antennas the user will use.
		const Array<double, 3>& positions = parent->getAntennaPos();
		Array<bool, 2> select;
		select.resize(positions.extent(firstDim), positions.extent(secondDim));
		select = false;
		int		maxI(positions.extent(firstDim)*positions.extent(secondDim));
		for (int i = 0; i < maxI; i++) {
			// subset is one-dimensional (receiver), select is two-dimensional (antenna, polarization)
			if (start.rcuMask[i]) {
				select(i/2,i%2) = true;
			}
		}

		LOG_DEBUG_STR("m_accs.getBack().getACC().shape()=" << m_accs.getBack().getACC().shape());
		LOG_DEBUG_STR("positions.shape()" << positions.shape());
#endif

		// check start.subset value
		bitset<MAX_RCUS> invalidmask;
		for (uint i = 0; i < m_n_rcus; i++) {
			invalidmask.set(i);
		}
		invalidmask.flip();

		// check dimensions of the various arrays for compatibility
		// m_accs: subbands x pol x pol x antennas x antennas
// DISABLED CHECK AFTER ROLL_OUT OF ITRF BEAMSERVER SINCE AntennaArray.conf CONFLICTS WITH AntennaSets.conf
#if 0
		if (m_accs.getFront().getACC().extent(firstDim) != GET_CONFIG("CalServer.N_SUBBANDS", i)
			|| m_accs.getFront().getACC().extent(secondDim) != positions.extent(secondDim)
			|| m_accs.getFront().getACC().extent(thirdDim)  != positions.extent(secondDim)
			|| m_accs.getFront().getACC().extent(fourthDim) != positions.extent(firstDim)
			|| m_accs.getFront().getACC().extent(fifthDim)  != positions.extent(firstDim))
		{
			LOG_INFO("ACC shape and parent array positions shape don't match.");
			LOG_ERROR_STR("ACC.shape=" << m_accs.getFront().getACC().shape());
			LOG_ERROR_STR("'" << start.antennaSet << "'.shape=" << positions.shape());
			LOG_ERROR_STR("Expecting AntenneArray with " <<
							m_accs.getFront().getACC().extent(fourthDim) << " antennas.");
			ack.status = ERR_RANGE;
		}
		else
#endif
		if ((start.rcuMask & invalidmask).any()) {
			LOG_INFO("CAL_START: Invalid receiver subset.");
			ack.status = ERR_RANGE;
		}
		else {
            RCUmask_t	validRCUs(start.rcuMask & globalAntennaSets()->RCUallocation(start.antennaSet));

            // create subarray to calibrate
            SubArray* subarray = new SubArray(start.name,
                                              start.antennaSet,
                                              validRCUs,
                                              start.band);

            itsSubArrays.scheduleAdd(subarray, &port);

            // calibration will start within one second
            // set the spectral inversion right
            RSPSetbypassEvent	specInvCmd;
            bool				SIon(subarray->SPW().nyquistZone() == 2);// on or off?
            specInvCmd.timestamp = Timestamp(0,0);
            specInvCmd.rcumask   = start.rcuMask;
            specInvCmd.settings().resize(1);
            specInvCmd.settings()(0).setXSI(SIon);
            specInvCmd.settings()(0).setYSI(SIon);
            LOG_DEBUG_STR("NyquistZone = " << subarray->SPW().nyquistZone()
                            << " setting spectral inversion " << ((SIon) ? "ON" : "OFF"));
            itsRSPDriver->send(specInvCmd);

            //RCUSettings rcu_settings;
            //rcu_settings().resize(1);
            //rcu_settings()(0).setMode(start.rcumode);

            _enableRCUs(subarray, SCHEDULING_DELAY + 4);
		}
	}
	port.send(ack); // send ack
	return status;
}

//
// handle_cal_stop(event, stop)
//
GCFEvent::TResult CalServer::handle_cal_stop(GCFEvent& e, GCFPortInterface &port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	// prepare and send a response
	CALStopEvent	stop(e);
	CALStopackEvent	ack;
	ack.name   = stop.name;
	ack.status = CAL_Protocol::CAL_SUCCESS;		// return success: don't bother client with our admin
	port.send(ack);

	_disableRCUs(itsSubArrays.getByName(stop.name));	// switch off unused RCUs

	itsSubArrays.scheduleRemove(stop.name);	// stop calibration

	// remove subarray-port entry from the map.
	map<string, GCFPortInterface*>::iterator	iter = m_clients.begin();
	map<string, GCFPortInterface*>::iterator	end  = m_clients.end();
	while (iter != end) {
		if (iter->second == &port) {
			m_clients.erase(iter);
			break;
		}
		iter++;
	}

	return status;
}

//
// handle_cal_subscribe(event, port)
//
GCFEvent::TResult CalServer::handle_cal_subscribe(GCFEvent& event, GCFPortInterface &port)
{
	CALSubscribeEvent subscribe(event);
	CALSubscribeackEvent ack;
	ack.name = subscribe.name;

	// get subarray by name
	SubArray* subarray = itsSubArrays.getByName(subscribe.name);
	if (subarray) {
		ack.status = CAL_SUCCESS;
		ack.subarray = *subarray; 		// return subarray positions

		// create subscription
		itsSubArrays.addSubscription(subscribe.name, &port);

		LOG_INFO_STR("Subscription succeeded: " << subscribe.name);
	}
	else {
		ack.status = ERR_NO_SUBARRAY;
		LOG_INFO_STR("Subarray not found: " << subscribe.name);
	}

	port.send(ack);

	return (GCFEvent::HANDLED);
}

//
// handle_cal_unsubscribe(event,port)
//
GCFEvent::TResult CalServer::handle_cal_unsubscribe(GCFEvent& event, GCFPortInterface &port)
{
	CALUnsubscribeEvent unsubscribe(event);
	// create ack
	CALUnsubscribeackEvent ack;
	ack.name   = unsubscribe.name;

	if (itsSubArrays.removeSubscription(unsubscribe.name)) {
		ack.status = CAL_SUCCESS;
		LOG_INFO_STR("Subscription deleted: " << unsubscribe.name);
	}
	else {
		ack.status = ERR_NO_SUBARRAY;
		LOG_INFO_STR("Unsubscribe failed. Subbarray not found: " << unsubscribe.name);
	}

	port.send(ack);

	return (GCFEvent::HANDLED);
}

//
// handle_cal_getsubarray(event, port)
//
GCFEvent::TResult CalServer::handle_cal_getsubarray(GCFEvent& e, GCFPortInterface &port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	CALGetsubarrayEvent	 	request(e);
	CALGetsubarrayackEvent	ack;
	ack.status = CAL_Protocol::CAL_SUCCESS;
	ack.subarraymap = itsSubArrays.getSubArrays(request.subarrayname);	// let SubArrays do the job

	// correct status is name was given but nothing was found.
	if (!request.subarrayname.empty() && ack.subarraymap.size() == 0) {
		ack.status = ERR_NO_SUBARRAY;
	}

	port.send(ack);

	return (status);
}

//
//  (subarray*)
//
void CalServer::_enableRCUs(SubArray*	subarray, int delay)
{
	// increment the usecount of the receivers
	RCUmask_t	rcuMask = subarray->RCUMask();
	RCUmask_t	rcus2switchOn;
	rcus2switchOn.reset();
	Timestamp timeStamp;
	for (uint r = 0; r < m_n_rcus; r++) {
		if (rcuMask.test(r)) {
			if (++itsRCUcounts[r] == 1) {
				rcus2switchOn.set(r);
				itsPowerOffTime[r] = 0;	// reset poweroff time
			} // new count is 1
		} // rcu in mask
	} // for all rcus

	// anything to enable? Tell the RSPDriver.
	if (rcus2switchOn.any()) {
		// all RCUs still off? Switch on the datastream also
		_updateDataStream(delay+1);

		RSPSetrcuEvent	enableCmd;
		timeStamp.setNow(delay);
		enableCmd.timestamp = timeStamp;
		enableCmd.rcumask   = rcus2switchOn;
		enableCmd.settings().resize(m_n_rcus);

        blitz::Array<uint,1> rcuModes = subarray->RCUmodes();
        LOG_INFO_STR("rcuModes= " << rcuModes);

        for (uint r = 0; r < m_n_rcus; r++) {
            if (rcus2switchOn.test(r)) {
                enableCmd.settings()(r).setMode((RSP_Protocol::RCUSettings::Control::RCUMode)rcuModes(r));
                enableCmd.settings()(r).setEnable(true);
            }
        }
        LOG_INFO_STR("enableCmd= " << enableCmd);
		sleep (1);
		LOG_INFO("Enabling some rcu's because they are used for the first time");
		itsRSPDriver->send(enableCmd);
	}

	// when the lbl inputs are selected swap the X and the Y.
	//int rcumode(subarray->SPW().rcumode());
	int rcumode(0);  // TODO: add real rcumode or change all to antennaset
	if (rcumode == 1 || rcumode == 2) {		// LBLinput used?
		LOG_INFO("LBL inputs are used, swapping X and Y RCU's");
		RSPSetswapxyEvent	swapCmd;
		swapCmd.timestamp   = timeStamp;
		swapCmd.swapxy	    = true;
		swapCmd.antennamask = RCU2AntennaMask(rcuMask);
		itsRSPDriver->send(swapCmd);
	}
}

//
// _disableRCUs(subarray*)
//
void CalServer::_disableRCUs(SubArray*	subarray)
{
	// decrement the usecount of the receivers and build mask of receiver that may be disabled.
	bool				allSwitchedOff(true);
	RCUmask_t	rcus2switchOff;
	rcus2switchOff.reset();
	Timestamp 			powerOffTime;
	powerOffTime.setNow(itsPowerOffDelay);

	if (subarray) {		// when no subarray is defined skip this loop: switch all rcus off.
		RCUmask_t	rcuMask = subarray->RCUMask();
		for (uint r = 0; r < m_n_rcus; r++) {
			if (rcuMask.test(r)) {
				if (--itsRCUcounts[r] == 0) {
					rcus2switchOff.set(r);
					itsPowerOffTime[r] = powerOffTime;
				} // count reaches 0
			} // rcu in mask

			// independant of the rcuMask check if this receiver is (also) off
			if (itsRCUcounts[r]) { 	// still on?
				allSwitchedOff = false;
			}
		} // for all rcus
	}

	if (allSwitchedOff) { 	// all receivers off? make mask that addresses them all.
		rcus2switchOff.reset();
		for (uint i = 0; i < m_n_rcus; i++) {
			rcus2switchOff.set(i);
		}
//		LOG_INFO("No active rcu's anymore, forcing all units to mode 0 and disable");
	}

    // when the lbl inputs are selected swap the X and the Y.
	LOG_INFO("Resetting swap of X and Y");
	RSPSetswapxyEvent   swapCmd;
	swapCmd.timestamp   = Timestamp(0,0);
	swapCmd.swapxy      = false;
	swapCmd.antennamask = RCU2AntennaMask(rcus2switchOff);
	itsRSPDriver->send(swapCmd);

	_updateDataStream(0); // asap

	if (!subarray) {	// reset at startup of CalServer or stopping LBA's?
		sleep (1);
		_powerdownRCUs(rcus2switchOff);
	}
	// Note: when NOT in startup mode the poweroff is delayed.
}

//
// _powerdownRCUs(rcus2switchOff)
//
void CalServer::_powerdownRCUs(RCUmask_t	rcus2switchOff)
{
	// anything to disable? Tell the RSPDriver.
	if (rcus2switchOff.any()) {
		RSPSetrcuEvent	disableCmd;
		disableCmd.timestamp = Timestamp(0,0);
		disableCmd.rcumask   = rcus2switchOff;
		disableCmd.settings().resize(1);
		disableCmd.settings()(0).setEnable(false);
		disableCmd.settings()(0).setMode(RCUSettings::Control::MODE_OFF);
		LOG_INFO_STR("Disabling " << rcus2switchOff.count() << " rcu's because they are not used anymore");
		itsRSPDriver->send(disableCmd);
	}
}

//
// _dataOnRing(uint	ringNr) : bool
// ringNr: 0|1
//
bool CalServer::_dataOnRing(uint	ringNr)	const
{
	if (ringNr == 1 && !itsSecondRingActive) {
		return (false);
	}

	uint	nrRings(itsHasSecondRing ? 2 : 1);
	ASSERTSTR(ringNr<nrRings, "RingNr "<<ringNr<<" does not exist, station has only ring 0 "<<(itsHasSecondRing?"and 1":""));

	int min(!itsSecondRingActive ?        0 :  ringNr   *(m_n_rcus/2));
	int	max(!itsSecondRingActive ? m_n_rcus : (ringNr+1)*(m_n_rcus/2));
	LOG_DEBUG_STR("_dataOnRing("<<ringNr<<"):"<<min<<"-"<<max<<", splitter="<<(itsSecondRingActive?"ON":"OFF"));
	for (int r = min; r < max; r++) {
		if (itsRCUcounts[r]) {
			return (true);
		}
	}
	return (false);
}

//
// _updateDatastreamSetting(delay)
//
void CalServer::_updateDataStream(uint	delay)
{
	bool	switchFirstOn  = _dataOnRing(0);
	bool	switchSecondOn = itsSecondRingActive && _dataOnRing(1);

	if ((itsFirstRingOn != switchFirstOn) || (itsSecondRingOn != switchSecondOn)) {
		RSPSetdatastreamEvent	dsCmd;
		dsCmd.timestamp.setNow(delay);
		dsCmd.switch_on0 = switchFirstOn;
		dsCmd.switch_on1 = switchSecondOn;
		LOG_INFO_STR("Switching the datastream "<<(switchFirstOn ? "ON":"OFF")<< ", "<<(switchSecondOn ? "ON":"OFF"));
		itsRSPDriver->send(dsCmd);
		itsFirstRingOn  = switchFirstOn;
		itsSecondRingOn = switchSecondOn;
	}
}


#if 0
GCFEvent::TResult CalServer::handle_cal_getsubarray(GCFEvent& e, GCFPortInterface &port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	CALGetsubarrayEvent getsubarray(e);

	// create ack
	CALGetsubarrayackEvent ack;
	ack.status = CAL_Protocol::CAL_SUCCESS;
	ack.subarraymap = itsSubArrays.getSubArrays(getsubarray.subarrayname);
	LOG_INFO("Sending subarray info: " << getsubarray.subarrayname);

	if (ack.subarraymap.empty()) {
        ack.status = ERR_NO_SUBARRAY;
        LOG_INFO("Getsubarray. Subarray not found: " << getsubarray.subarrayname);
	}

	port.send(ack);
	return status;
}
#endif

//
// write_acc()
//
void CalServer::write_acc()
{
	time_t now = time(0);
	struct tm* t = gmtime(&now);
	char filename[PATH_MAX];
	const Array<std::complex<double>, 5>& acc = m_accs.getFront().getACC();
	Array<std::complex<double>, 3> newacc;

	newacc.resize(acc.extent(firstDim),
					acc.extent(secondDim)*acc.extent(fourthDim),
					acc.extent(thirdDim)*acc.extent(fifthDim));

	for (int s = 0; s < newacc.extent(firstDim); s++) {
		for (int i = 0; i < newacc.extent(secondDim); i++) {
			for (int j = 0; j < newacc.extent(thirdDim); j++) {
				newacc(s,i,j) = acc(s,i%2,j%2,i/2,j/2);
			}
		}
	}

	snprintf(filename, PATH_MAX, "%s/%04d%02d%02d_%02d%02d%02d_acc_%dx%dx%d.dat",
		itsDataDir.c_str(),
		t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
		t->tm_hour, t->tm_min, t->tm_sec,
		newacc.extent(firstDim),
		newacc.extent(secondDim),
		newacc.extent(thirdDim));
	FILE* accfile = fopen(filename, "w");

	if (!accfile) {
		LOG_ERROR_STR("failed to open file: " << filename);
		return;
	}

	if ((size_t)newacc.size() != fwrite(newacc.data(), sizeof(complex<double>), newacc.size(), accfile)) {
		LOG_ERROR_STR("failed to write to file: " << filename);
	}

	(void)fclose(accfile);
}

    };  // CAL
};  // LOFAR
