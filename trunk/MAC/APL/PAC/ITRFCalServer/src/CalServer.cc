//#
//#  CalServer.cc: implementation of CalServer class
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
//#  Note: this file is formatted with tabstop 4
//#
//#  $Id: CalServer.cc 12256 2008-11-26 10:54:14Z overeem $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <Common/LofarConstants.h>
#include <Common/lofar_bitset.h>
#include <Common/ParameterSet.h>
#include <Common/Version.h>

#include <ApplCommon/LofarDirs.h>

#include <APL/APLCommon/AntennaField.h>
#include <APL/APLCommon/AntennaSets.h>
#include <APL/ICAL_Protocol/ICAL_Protocol.ph>
#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/LBA_Calibration/lba_calibration.h>		// the matlab stuff

#include <MACIO/MACServiceInfo.h>

#include "CalServer.h"
#include "SubArrayMgr.h"
#include "LBACalibration.h"
//#include "Package__Version.h"

#ifdef USE_CAL_THREAD
#include "CalibrationThread.h"
#endif

#include "ACMProxy.h"

// from RTCCommon
#include <APL/RTCCommon/Timestamp.h>
#include <APL/RTCCommon/PSAccess.h>

#include <fstream>
#include <signal.h>
#include <getopt.h>

#include <blitz/array.h>

#define SCHEDULING_DELAY 3

using namespace std;
using namespace blitz;

namespace LOFAR {
  using namespace RTC;
  using namespace APLCommon;
  using namespace RSP_Protocol;
  using namespace ICAL_Protocol;
  using namespace GCF::TM;
  namespace ICAL {

//
// CalServer constructor
//
CalServer::CalServer(const string&			name, 
					 ACCcache&				accs, 
					 LBACalibration&		theLBAcal) : 
	GCFTask				((State)&CalServer::initial, name),
	itsACCs				(accs), 
	itsACCsSwapped		(false),
	itsLBAcal			(theLBAcal), 
	itsClockSetting		(0),
	itsLowestSubband	(0),
	itsHighestSubband	(0)
#ifdef USE_CAL_THREAD
    , itsCalibrationThread(0)
#endif
{
#ifdef USE_CAL_THREAD
	pthread_mutex_init(&itsGlobalLock, 0);
#endif

//	LOG_INFO(Version::getInfo<CalServerVersion>("CalServer"));

	// register protocols for debugging
	registerProtocol(ICAL_PROTOCOL, ICAL_PROTOCOL_STRINGS);
	registerProtocol(RSP_PROTOCOL,  RSP_PROTOCOL_STRINGS);

	itsListener  = new GCFTCPPort(*this, MAC_SVCMASK_CALSERVER, GCFPortInterface::MSPP, ICAL_PROTOCOL);
	ASSERTSTR(itsListener, "Can't create a listener socket");

	itsRSPDriver = new GCFTCPPort(*this, MAC_SVCMASK_RSPDRIVER,  GCFPortInterface::SAP,  RSP_PROTOCOL);
	ASSERTSTR(itsRSPDriver, "Can't create a socket to communicate with the RSPDriver");

	itsHeartBeat = new GCFTimerPort(*this, "HeartBeatTimer");
	ASSERTSTR(itsHeartBeat, "Can't allocate my heartbeat timer");
}

//
// ~CalServer()
//
CalServer::~CalServer()
{
#ifdef USE_CAL_THREAD
	if (itsCalibrationThread) {
		delete itsCalibrationThread;
	}
#endif
	if (itsListener)	{ delete itsListener; }
	if (itsRSPDriver)	{ delete itsRSPDriver; }
}


//
// initial(event,port)
//
GCFEvent::TResult CalServer::initial(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("initial:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch(event.signal) {
	case F_INIT: {
		try { 
#ifdef USE_CAL_THREAD
			pthread_mutex_lock(&itsGlobalLock); // lock for dipolemodels, and sources
#endif
			ConfigLocator cl;
			// load the source catalog
//			LOG_INFO("Reading SourceCatalog");
//			m_sources.getAll(cl.locate(GET_CONFIG_STRING("CalServer.SourceCatalogFile")));

			// load the antenna positions
			ASSERTSTR(globalAntennaField(), "Could not load the antennaposition file");
			LOG_DEBUG("Loaded antenna postions file");

			itsLowestSubband  = globalParameterSet()->getInt("CalServer.firstSubband", 0);
			itsHighestSubband = globalParameterSet()->getInt("CalServer.lastSubband", MAX_SUBBANDS-1);
			LOG_INFO_STR("Calibration subbandrange: " << itsLowestSubband << ".." << itsHighestSubband);

			// Setup datapath
			itsDataDir = globalParameterSet()->getString("CalServer.DataDirectory", LOFAR_LOG_LOCATION);
			if (itsDataDir.empty()) {
				itsDataDir = LOFAR_LOG_LOCATION;
			}
			else if ((*itsDataDir.rbegin()) == '/') {       // strip off last /
				itsDataDir.erase(itsDataDir.length()-1);
			}
			LOG_INFO_STR("Interim datafiles will be stored in " << itsDataDir);

			// which tasks should be turned on?
			itsCollectionEnabled  = globalParameterSet()->getBool("CalServer.enableCollection");
			itsCalibrationEnabled = globalParameterSet()->getBool("CalServer.enableCalibration");
			if (itsCalibrationEnabled && !itsCollectionEnabled) {
				LOG_ERROR("Calibration is enabled but Collection NOT, the CalServer will NOT process data!");
			}

#ifdef USE_CAL_THREAD
			// Setup calibration thread
			itsCalibrationThread = new CalibrationThread(itsGlobalLock, antennaFieldName, @@@@@ );

			pthread_mutex_unlock(&itsGlobalLock); // unlock global lock
#endif

		} catch (Exception e)  {

#ifdef USE_CAL_THREAD
			pthread_mutex_unlock(&itsGlobalLock); // unlock global lock
#endif
			LOG_ERROR_STR("Failed to load configuration files: " << e);
			exit(EXIT_FAILURE);
		}
	}
	break;

	case F_ENTRY: {
		if (!itsListener->isConnected()) {
			itsListener->open();
		}
		LOG_INFO("Trying to connect to the RSPDriver");
		itsRSPDriver->autoOpen(30,0,2);	// try max 30 times with 2 seconds interval
	}
	break;

	case F_CONNECTED: {
		if (itsListener->isConnected() && itsRSPDriver->isConnected()) {
			LOG_INFO("Connected to the RSPDriver, asking system configuration and settings");
			RSPGetconfigEvent getconfig;
			itsRSPDriver->send(getconfig);
		}
	}
	break;

	case RSP_GETCONFIGACK: {
		RSPGetconfigackEvent ack(event);
		// resize and clear itsRCUcounters.
		itsRCUcounts.assign(ack.n_rcus, 0);

		// get initial clock setting
		RSPGetclockEvent getclock;
		getclock.timestamp = Timestamp(0,0);
		getclock.cache = true;

		itsRSPDriver->send(getclock);
	}
	break;

	case RSP_GETCLOCKACK: {
		RSPGetclockackEvent getclockack(event);

		if (getclockack.status != RSP_SUCCESS) {
			LOG_FATAL("Failed to get sampling frequency setting");
			exit(EXIT_FAILURE);
		}

		// get clock value and convert to MHz
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
		RSPSubclockackEvent ack(event);
		if (ack.status != RSP_SUCCESS) {
			LOG_FATAL("Failed to subscribe to clock status updates.");
			exit(EXIT_FAILURE);
		}
		TRAN(CalServer::operational);
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
// operational(event,port)
//
GCFEvent::TResult CalServer::operational(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("operational:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: {
		// Note: we switch to operational mode only once, so its allowed to do this here.
		_disableRCUs(0);							// no beams yet, so switch RCU's off.
		itsHeartBeat->setTimer(0.0, 1.0);			// start heartbeat
		if (itsCollectionEnabled) {
			itsACCs.getBack().needsStart();			// let ACMproxy fill the backcache
			if (itsCalibrationEnabled) {
				itsACCs.getFront().needsStart();	// let matlab process the frontCache
			} else {
				itsACCs.getFront().setReady(false);	// assume no matlab processing
			}
		} else {
			itsACCs.getBack().setReady(false);		// assume no collection process.
		}
	}
	break;

	case F_ACCEPT_REQ: {
		GCFTCPPort* client = new GCFTCPPort();
		client->init(*this, "client", GCFPortInterface::SPP, ICAL_PROTOCOL);
		if (!itsListener->accept(*client)) {
			delete client;
			LOG_ERROR ("could not setup a connection with a new client");
		}
//		else {
//			m_clients[client] = ""; // empty string to indicate there is a connection, but no subarray yet
//			LOG_INFO(formatstring("new client connected: %d clients connected", m_clients.size()));
//		}
	}
	break;

	case RSP_UPDCLOCK: {
		RSPUpdclockEvent updclock(event);

		// use new sampling frequency
		itsClockSetting = updclock.clock;
		LOG_INFO_STR("new sampling frequency: " << itsClockSetting << " MHz");

		// REO TODO: stop current calibration, wait 30 seconds for boards to stabelize and restart calibration @@@
	}
	break;

	case RSP_SETRCUACK: {
		RSPSetrcuackEvent ack(event);
		if (ack.status != RSP_SUCCESS) {
			LOG_FATAL_STR("Failed to set RCU control register, status=" << ack.status);
			exit (EXIT_FAILURE);
		}
	}
	break;

	case RSP_SETBYPASSACK: {
		RSPSetbypassackEvent ack(event);
		if (ack.status != RSP_SUCCESS) {
			LOG_FATAL_STR("Failed to set Spectral Inversion control register, status=" << ack.status);
			exit (EXIT_FAILURE);
		}
	}
	break;

	case F_CONNECTED: {
		LOG_INFO(formatString("CONNECTED: port '%s'", port.getName().c_str()));
	}
	break;

	case F_TIMER: {	
		// This is our main loop of the CalServer. We come here every second.
		GCFTimerEvent* timer = static_cast<GCFTimerEvent*>(&event);

		const Timestamp t = Timestamp(timer->sec, timer->usec);
		LOG_DEBUG_STR("heartbeat @ " << t);

		if (!itsCollectionEnabled && !itsCalibrationEnabled) {
			break;
		}

		//
		// Swap buffers when all calibrations have finished on the front buffer
		// and the back buffer is not locked and is valid (has been filled by ACMProxy).
		// 
		LOG_DEBUG_STR("Collect=" << (itsCollectionEnabled ? "Yes":"No") << ":Ready=" << (itsACCs.getBack().isReady() ? "Yes":"No") << ",Cali=" << (itsCalibrationEnabled ? "Yes":"No") << ":Ready=" << (itsACCs.getFront().isReady() ? "Yes":"No"));
		if ((itsCollectionEnabled && !itsACCs.getBack().isReady()) || 
			(itsCalibrationEnabled && !itsACCs.getFront().isReady())) {
			break;
		}

		// both caches are ready, time to join the threads, swap the caches, and restart the cycle again.

		LOG_INFO("New ACC information available, start processing this information");
#ifdef USE_CAL_THREAD
		// join previous calibration thread
		(void)itsCalibrationThread->join();
#endif

		undertaker(); // destroy dead clients, done here to prevent possible use of closed port
		itsSubArrays.removeDeadArrays();  // remove subarrays scheduled for deletion
		itsSubArrays.activateArrays();     // bring new subarrays to life @@@ SHOULD WE DO THIS HERE???

		if (GET_CONFIG("CalServer.WriteACCToFile", i)) {
			write_acc();
		}

		// Swap the caches and restart the collection and calibration
		itsACCs.swap();
		if (itsCollectionEnabled) {
			itsACCs.getBack().needsStart();		// trigger ACMProxy
		}
		if (itsCalibrationEnabled) {
			itsACCs.getFront().needsStart();	// trigger calibrationprocess.
		}

#ifdef USE_CAL_THREAD
		// start calibration thread
		itsCalibrationThread->run(itsSubArrays!!!);
#else
// TODO
		// bool	writeGains(GET_CONFIG("CalServer.WriteGainsToFile", i) == 1);
		for (int rcumode = 1; rcumode <= NR_RCU_MODES; rcumode++) {
			RCUmask_t	theRCUs = itsSubArrays.getRCUs(rcumode);
			if (!theRCUs.none()) {
				// Calibrate all SubArrays (scattered over all rcumodes)
				AntennaGains&	antGains(itsLBAcal.run(rcumode, "LBA", theRCUs));
				itsSubArrays.publishGains(rcumode, antGains);
			}
			else {
				LOG_DEBUG_STR("No active RCUs for mode " << rcumode);
			}
		} // all rcumodes
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

	case ICAL_START:
		status = handle_cal_start(event, port);
	break;

	case ICAL_STOP:
		status = handle_cal_stop(event, port);
	break;

	case ICAL_SUBSCRIBE:
		status = handle_cal_subscribe(event, port);
	break;

	case ICAL_UNSUBSCRIBE:
		status = handle_cal_unsubscribe(event, port);
	break;

	case ICAL_GETSUBARRAY:
		status = handle_cal_getsubarray(event, port);
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
GCFEvent::TResult CalServer::handle_cal_start(GCFEvent& event, GCFPortInterface &port)
{
	GCFEvent::TResult	status = GCFEvent::HANDLED;
	ICALStartEvent		start(event);
	ICALStartackEvent	ack;

	ack.status = ICAL_SUCCESS; // assume succes, until otherwise
	ack.name   = start.name;

	LOG_DEBUG_STR("handling:" << start);

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

	} else if (start.rcuMask.count() == 0) {
		// empty selection
		LOG_ERROR("Empty antenna selection not allowed.");
		ack.status = ERR_NO_ANTENNAS;

	} else if (!(start.rcuMask & globalAntennaSets()->RCUallocation(start.antennaSet)).any()) {
		LOG_ERROR_STR("CAL_START: Invalid receiver subset, rcuMask=" << start.rcuMask << ", allowed=" 
						<< globalAntennaSets()->RCUallocation(start.antennaSet));
		ack.status = ERR_RANGE;
	
	} else {
		// register because this is a cal_start
		m_clients[start.name] = &port;		// register subarray and port

		RCUmask_t	validRCUs(start.rcuMask & globalAntennaSets()->RCUallocation(start.antennaSet));

		// create subarray to calibrate
		RCUSettings::Control	RCUcontrol;		// for conversion from rcumode to LBAfilter setting
		RCUcontrol.setMode((RCUSettings::Control::RCUMode)start.rcumode);
		SubArray* subarray = new SubArray(start.name,
							start.antennaSet,
							validRCUs,
							RCUcontrol.LBAfilter(),
							itsClockSetting * 1.0e6,
							RCUcontrol.getNyquistZone());
		itsSubArrays.scheduleAdd(subarray, &port);

		// calibration will start within one second
		// set the spectral inversion right
		// prepare RSP command
		RSPSetbypassEvent	specInvCmd;
		bool				SIon(RCUcontrol.getNyquistZone() == 2);// on or off?
		specInvCmd.timestamp = Timestamp(0,0);
		specInvCmd.rcumask   = validRCUs;
		specInvCmd.settings().resize(1);
		specInvCmd.settings()(0).setXSI(SIon);
		specInvCmd.settings()(0).setYSI(SIon);
		LOG_INFO_STR("Setting spectral inversion " << ((SIon) ? "ON" : "OFF") << " for the RCUs:" << specInvCmd.rcumask);
		itsRSPDriver->send(specInvCmd);

		//
		// set the control register of the RCU's
		// if in HBA mode turn on HBAs in groups to prevent resetting of boards
		//
		RSPSetrcuEvent	setrcu;
		RCUmask_t		setMask;
		Timestamp		timeStamp;

		#define N_PWR_RCUS_PER_STEP 12

		int nrRCUs   = itsSC.nrRSPs * NR_RCUS_PER_RSPBOARD;		// rcus on this station
		int nPwrRCUs = nrRCUs / 2;  					// only the even rcus deliver power to the HBAs
		int steps    = nPwrRCUs / N_PWR_RCUS_PER_STEP;  // 4 steps for NL stations, 8 steps for IS stations
		int jump     = nrRCUs   / N_PWR_RCUS_PER_STEP;  // jump = 8 for NL stations and 16 for IS stations

		if (steps == 0) { steps = 1; }  // limit for test cabinet
		if (jump < 2) { jump = 2; }     // limit for test cabinet

		// if LBA mode select all rcus in one step
		if (start.rcumode <= 4) {
			steps = 1;
			jump = 2;
		}
		int delay;
		for (int step = 0; step < steps; ++step) {
			setMask.reset();
			// select 12 even(X) rcus and 12 odd(Y) rcus
			for (int rcu = (step * 2); rcu < nrRCUs; rcu += jump) {
				if (validRCUs.test(rcu)) {
					setMask.set(rcu);   // select X (HBA power supply)
					setMask.set(rcu+1); // select Y
				}
			}

			// if any rcus in this masker send command
			if (setMask.any()) {
				delay = SCHEDULING_DELAY + step;
				timeStamp.setNow(delay);
				setrcu.timestamp = timeStamp; // in steps of 1 second

				//TODO: Step20.2: might have to send 2 settings e.g. when using all X-pols

				setrcu.rcumask = setMask;
				setrcu.settings().resize(1);
				setrcu.settings()(0).setMode((RCUSettings::Control::RCUMode)start.rcumode);

				// previous LOG statement contained start.rcumask.to_ulong() which
				// throws an exception because the number of bits = 256!
				LOG_DEBUG(formatString("Sending RSP_SETRCU(%08X)", setrcu.settings()(0).getRaw()));
				itsRSPDriver->send(setrcu);
			}
		}
			_enableRCUs(subarray, delay + 4);
	}
	port.send(ack); // send ack

	return status;
}

//
// handle_cal_stop(event, stop)
//
GCFEvent::TResult CalServer::handle_cal_stop(GCFEvent& event, GCFPortInterface &port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	// prepare and send a response
	ICALStopEvent stop(event);
	ICALStopackEvent ack;
	ack.name = stop.name;
	ack.status = ICAL_SUCCESS;		// return success: don't bother client with our admin
	port.send(ack);

	_disableRCUs(itsSubArrays.getByName(stop.name));//		switch off unused RCUs

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
	ICALSubscribeEvent subscribe(event);
	ICALSubscribeackEvent ack;
	ack.name = subscribe.name;

	// get subarray by name
	SubArray* subarray = itsSubArrays.getByName(subscribe.name);
	if (subarray) {
		ack.status = ICAL_SUCCESS;
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
	ICALUnsubscribeEvent unsubscribe(event);
	// create ack
	ICALUnsubscribeackEvent ack;
	ack.name   = unsubscribe.name;

	if (itsSubArrays.removeSubscription(unsubscribe.name)) {
		ack.status = ICAL_SUCCESS;
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
GCFEvent::TResult CalServer::handle_cal_getsubarray(GCFEvent& event, GCFPortInterface &port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	ICALGetsubarrayEvent	 	request(event);
	ICALGetsubarrayackEvent	ack;
	ack.status = ICAL_SUCCESS;
	ack.subarraymap = itsSubArrays.getSubArrays(request.subarrayname);	// let SubArrays do the job

	// correct status if name was given but nothing was found.
	if (!request.subarrayname.empty() && ack.subarraymap.size() == 0) {
		ack.status = ERR_NO_SUBARRAY;
	}

	port.send(ack);

	return (status);
}

//
// _enableRCUs(subarray*)
//  
void CalServer::_enableRCUs(SubArray*   subarray, int delay)
{   
	// increment the usecount of the receivers
	RCUmask_t rcuMask = subarray->RCUMask();
	RCUmask_t rcus2switchOn;
	rcus2switchOn.reset();
	Timestamp timeStamp;
	for (int r =(itsSC.nrRSPs*NR_RCUS_PER_RSPBOARD)-1; r>= 0; r--) {
		if (rcuMask.test(r)) {
			if(++itsRCUcounts[r] == 1) {
				rcus2switchOn.set(r);
			} // new count is 1
		} // rcu in mask
	} // for all rcus

	// anything to enable? Tell the RSPDriver.
	if (rcus2switchOn.any()) {
		RSPSetrcuEvent  enableCmd;
		timeStamp.setNow(delay);
		enableCmd.timestamp = timeStamp;
		enableCmd.rcumask = rcus2switchOn;
		enableCmd.settings().resize(1);
		enableCmd.settings()(0).setEnable(true);
		sleep (1);
		LOG_INFO_STR("Enabling some rcu's because they are used for the first time");
		itsRSPDriver->send(enableCmd);
	}   
}   

//      
// _disableRCUs(subarray*)
//  
void CalServer::_disableRCUs(SubArray*  subarray)
{   
	// decrement the usecount of the receivers and build mask of receiver that may be disabled.
	bool		allSwitchedOff(true);
	RCUmask_t	rcus2switchOff;
	rcus2switchOff.reset();

	if (subarray) {     // when no subarray is defined skipp this loop: switch all rcus off.
		RCUmask_t rcuMask = subarray->RCUMask();
		for (int r = (itsSC.nrRSPs*NR_RCUS_PER_RSPBOARD)-1; r >=0; r--) {
			if (rcuMask.test(r)) {
				if (--itsRCUcounts[r] == 0) {
					rcus2switchOff.set(r);
				} // count reaches 0
			} // rcu in mask

			// independant of the rcuMask check if this receiver is (also) off
			if (itsRCUcounts[r]) {  // still on?
				allSwitchedOff = false;
			}
		} // for all rcus
	}

	if (allSwitchedOff) {   // all receivers off? force all rcu's to mode 0, disable
		rcus2switchOff.reset();
		for (int r = (itsSC.nrRSPs*NR_RCUS_PER_RSPBOARD)-1; r >=0; r--) {
			rcus2switchOff.set(r);
		}
		LOG_INFO("No active rcu's anymore, forcing all units to mode 0 and disable");
	}
	// anything to disable? Tell the RSPDriver.
	if (rcus2switchOff.any()) {
		RSPSetrcuEvent  disableCmd;
		disableCmd.timestamp = Timestamp(0,0);
		disableCmd.rcumask = rcus2switchOff;
		disableCmd.settings().resize(1);
		disableCmd.settings()(0).setEnable(false);
		disableCmd.settings()(0).setMode(RCUSettings::Control::MODE_OFF);
		sleep (1);
		LOG_INFO_STR("Disabling " << rcus2switchOff.count() << " rcu's because they are not used anymore");
		itsRSPDriver->send(disableCmd);
	}
}


//
// undertaker()
//
void CalServer::undertaker()
{
	list<GCFPortInterface*>::iterator	iter = m_dead_clients.begin();
	list<GCFPortInterface*>::iterator	end  = m_dead_clients.end();
	while (iter != end) {
		delete (*iter);
		++iter;
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
	ASSERT(port != 0);

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
// write_acc()
//
void CalServer::write_acc()
{
// TODO: REIMPLEMENT THIS ROUTINE FOR THE matlabACCs
#if 0
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

	snprintf(filename, PATH_MAX, "%04d%02d%02d_%02d%02d%02d_acc_%dx%dx%d.dat",
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
#endif
}

  } // namespace ICAL
} // namespace LOFAR


