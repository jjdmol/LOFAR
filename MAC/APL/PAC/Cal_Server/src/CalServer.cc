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
//#  $Id$

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <Common/lofar_bitset.h>
#include <Common/Version.h>
#include <ApplCommon/StationConfig.h>

#include <APL/RTCCommon/daemonize.h>
#include <APL/CAL_Protocol/CAL_Protocol.ph>
#include <APL/RSP_Protocol/RSP_Protocol.ph>

#include <MACIO/MACServiceInfo.h>

#include "CalServer.h"
#include "SubArrays.h"
#include "SubArraySubscription.h"
#include "RemoteStationCalibration.h"
#include "CalibrationAlgorithm.h"
#include "Package__Version.h"

#ifdef USE_CAL_THREAD
#include "CalibrationThread.h"
#endif

#include "ACMProxy.h"

// from RTCCommon
#include <APL/RTCCommon/Timestamp.h>
#include <APL/RTCCommon/PSAccess.h>

#include <Common/ParameterSet.h>
#include <fstream>
#include <signal.h>
#include <getopt.h>

#include <blitz/array.h>

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace CAL;
using namespace RTC;
using namespace RSP_Protocol;
using namespace CAL_Protocol;
using namespace GCF::TM;

#define NPOL 2

#define SCHEDULING_DELAY 3

//
// parseOptions
//
void CalServer::parseOptions(int	argc,
			     char**	argv)
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

	case 'd':	// --daemonize
	  break;

    default:
      LOG_FATAL (formatString("Unknown option %c", c));
      ASSERT(false);
    } // switch
  } // for loop
}

//
// CalServer constructor
//
CalServer::CalServer(const string& name, ACCs& accs, int argc, char** argv)
  : GCFTask((State)&CalServer::initial, name),
    m_accs(accs), 
	m_cal(0), 
	m_converter(0),
    m_sampling_frequency(0.0),
    m_n_rspboards(0), 
	m_n_rcus(0), 
	m_instancenr(-1)
#ifdef USE_CAL_THREAD
    , m_calthread(0)
#endif
{
#ifdef USE_CAL_THREAD
	pthread_mutex_init(&m_globallock, 0);
#endif

	LOG_INFO(Version::getInfo<Cal_ServerVersion>("CalServer"));

	// adopt commandline switches
	parseOptions (argc, argv);

	if (!GET_CONFIG("CalServer.DisableCalibration", i)) {
		m_converter = new AMC::ConverterClient("localhost");
		ASSERT(m_converter != 0);
	}

	string	instanceID;
	if (m_instancenr >= 0) {
		instanceID = formatString("(%d)", m_instancenr);
	}

	registerProtocol(CAL_PROTOCOL, CAL_PROTOCOL_STRINGS);
	m_acceptor.init(*this, MAC_SVCMASK_CALSERVER, GCFPortInterface::MSPP, CAL_PROTOCOL);

	registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_STRINGS);
	m_rspdriver.init(*this, MAC_SVCMASK_RSPDRIVER,  GCFPortInterface::SAP,  RSP_PROTOCOL);
}

//
// ~CalServer()
//
CalServer::~CalServer()
{
	if (m_cal)       delete m_cal;
	if (m_converter) delete m_converter;
#ifdef USE_CAL_THREAD
	if (m_calthread) delete m_calthread;
#endif
}

//
// undertaker()
//
void CalServer::undertaker()
{
	for (list<GCFPortInterface*>::iterator it = m_dead_clients.begin();
											it != m_dead_clients.end(); ++it) {
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
	ASSERT(port != 0);

	map<string, GCFPortInterface*>::iterator	iter = m_clients.begin();
	map<string, GCFPortInterface*>::iterator	end  = m_clients.end();
	while (iter != end) {
		if (iter->second == port) {
			// stop subarray if it is still there.
			SubArray* subarray = m_subarrays.getByName(iter->first);
			if (subarray) {
				_disableRCUs(subarray);
				m_subarrays.schedule_remove(subarray);
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

			// Load antenna arrays
			m_arrays.getAll(cl.locate(GET_CONFIG_STRING("CalServer.AntennaArraysFile")));

			// Setup datapath
			itsDataDir = globalParameterSet()->getString("CalServer.DataDirectory", "/opt/lofar/bin");
			if (itsDataDir.empty()) {
				itsDataDir="/opt/lofar/bin";
			}
			else if ((*itsDataDir.rbegin()) == '/') {		// strip off last /
				itsDataDir.erase(itsDataDir.length()-1);
			}
			LOG_INFO_STR("Interim datafile will be stored in " << itsDataDir);

			// Setup calibration algorithm
			m_cal = new RemoteStationCalibration(m_sources, m_dipolemodels, *m_converter);

#ifdef USE_CAL_THREAD
			// Setup calibration thread
			m_calthread = new CalibrationThread(&m_subarrays, m_cal, m_globallock, itsDataDir);

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
		if (!m_acceptor.isConnected()) {
			m_acceptor.open();
		}
		LOG_DEBUG("opening port: m_rspdriver");
		m_rspdriver.open();
	}
	break;

	case F_CONNECTED: {
		if ( m_acceptor.isConnected() && m_rspdriver.isConnected()) {
			RSPGetconfigEvent getconfig;
			m_rspdriver.send(getconfig);
		}
	}
	break;

	case RSP_GETCONFIGACK: {
		RSPGetconfigackEvent ack(e);
		m_n_rspboards = ack.n_rspboards;
		m_n_rcus = ack.n_rcus;
		LOG_INFO_STR("nrRSPboards=" << m_n_rspboards << ", nrRCUS=" << m_n_rcus);
		// resize and clear itsRCUcounters.
		itsRCUcounts.assign(m_n_rcus, 0);

		// get initial clock setting
		RSPGetclockEvent getclock;
		getclock.timestamp = Timestamp(0,0);
		getclock.cache = true;

		m_rspdriver.send(getclock);
	}
	break;

	case RSP_GETCLOCKACK: {
		RSPGetclockackEvent getclockack(e);

		if (getclockack.status != RSP_Protocol::RSP_SUCCESS) {
			LOG_FATAL("Failed to get sampling frequency setting");
			exit(EXIT_FAILURE);
		}

		// get clock value and convert to MHz
		m_sampling_frequency = getclockack.clock * (uint32)1.0e6;

		LOG_INFO_STR("Initial sampling frequency: " << m_sampling_frequency);

		// subscribe to clock change updates
		RSPSubclockEvent subclock;
		subclock.timestamp = Timestamp(0,0);
		subclock.period = 1;

		m_rspdriver.send(subclock);
	}
	break;

	case RSP_SUBCLOCKACK: {
		RSPSubclockackEvent ack(e);

		if (ack.status != RSP_Protocol::RSP_SUCCESS) {
			LOG_FATAL("Failed to subscribe to clock status updates.");
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
		_disableRCUs(0);
		m_acceptor.setTimer(0.0, 1.0);
	}
	break;

	case F_ACCEPT_REQ: {
		GCFTCPPort* client = new GCFTCPPort();
		client->init(*this, "client", GCFPortInterface::SPP, CAL_PROTOCOL);
		if (!m_acceptor.accept(*client)) {
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
		m_sampling_frequency = updclock.clock * (uint32)1.0e6;

		LOG_INFO_STR("New sampling frequency: " << m_sampling_frequency);
	}
	break;

	case RSP_SETRCUACK: {
		RSPSetrcuackEvent ack(e);
		if (ack.status != RSP_Protocol::RSP_SUCCESS) {
			LOG_FATAL_STR("Failed to set RCU control register, error = " << ack.status);
			exit (EXIT_FAILURE);
		}
	}
	break;

	case RSP_SETBYPASSACK: {
		RSPSetbypassackEvent ack(e);
		if (ack.status != RSP_Protocol::RSP_SUCCESS) {
			LOG_FATAL_STR("Failed to set Spectral Inversion control register, error = " << ack.status);
			exit (EXIT_FAILURE);
		}
	}
	break;

	case F_CONNECTED: {
		LOG_INFO(formatString("CONNECTED: port '%s'", port.getName().c_str()));
	}
	break;

	case F_TIMER: {
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

			m_subarrays.mutex_lock();
			undertaker(); // destroy dead clients, done here to prevent possible use of closed port
			m_subarrays.undertaker();  // remove subarrays scheduled for deletion
			m_subarrays.creator();     // bring new subarrays to life
			m_subarrays.mutex_unlock();

			if (GET_CONFIG("CalServer.WriteACCToFile", i)) {
				write_acc();
			}

#ifdef USE_CAL_THREAD
			// start calibration thread
			m_calthread->setACC(&m_accs.getFront());
			m_calthread->run();
#else
			bool	writeGains(GET_CONFIG("CalServer.WriteGainsToFile", i) == 1);
			if (GET_CONFIG("CalServer.DisableCalibration", i)) {
				m_subarrays.calibrate(0, m_accs.getFront(), writeGains, itsDataDir);
			} else {
				m_subarrays.calibrate(m_cal, m_accs.getFront(), writeGains, itsDataDir);
			}
			m_subarrays.updateAll();
#endif
		}

#ifdef USE_CAL_THREAD
		m_subarrays.updateAll();
#endif
	}
	break;

	case F_DISCONNECTED: {
		LOG_INFO(formatString("DISCONNECTED: port %s disconnected", port.getName().c_str()));
		port.close();

		if (&m_acceptor == &port) {
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

	ack.status = CAL_Protocol::CAL_SUCCESS; // assume succes, until otherwise
	ack.name   = start.name;

	// find parent AntennaArray
	const AntennaArray* parent = m_arrays.getByName(start.parent);

	if (m_subarrays.getByName(start.name)) {
		LOG_ERROR_STR("A subarray with name='" << start.name << "' has already been registered.");
		ack.status = ERR_ALREADY_REGISTERED;

	} else if (string(start.name) == "") {
		LOG_ERROR("Empty subarray name.");
		ack.status = ERR_NO_SUBARRAY_NAME;

	} else if (!parent) {
		// parent not found, set error status
		LOG_ERROR_STR("Parent array '" << start.parent << "' not found.");
		ack.status = ERR_NO_PARENT;

	} else if (start.subset.count() == 0) {
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

		// Construct a 'select' array (nAntennas, nPol) that knows which antennas the user will use.
		const Array<double, 3>& positions = parent->getAntennaPos();
		Array<bool, 2> select;
		select.resize(positions.extent(firstDim), positions.extent(secondDim));
		select = false;
		int		maxI(positions.extent(firstDim)*positions.extent(secondDim));
		for (int i = 0; i < maxI; i++) {
			// subset is one-dimensional (receiver), select is two-dimensional (antenna, polarization)
			if (start.subset[i]) {
				select(i/2,i%2) = true;
			}
		}

		LOG_DEBUG_STR("m_accs.getBack().getACC().shape()=" << m_accs.getBack().getACC().shape());
		LOG_DEBUG_STR("positions.shape()" << positions.shape());

		// check start.subset value
		bitset<MEPHeader::MAX_N_RCUS> invalidmask;
		for (int i = 0; i < m_n_rcus; i++) {
		  invalidmask.set(i);
		}
		invalidmask.flip();

		// check dimensions of the various arrays for compatibility
		// m_accs: subbands x pol x pol x antennas x antennas
		if (m_accs.getFront().getACC().extent(firstDim) != GET_CONFIG("CalServer.N_SUBBANDS", i)
			|| m_accs.getFront().getACC().extent(secondDim) != positions.extent(secondDim)
			|| m_accs.getFront().getACC().extent(thirdDim)  != positions.extent(secondDim)
			|| m_accs.getFront().getACC().extent(fourthDim) != positions.extent(firstDim)
			|| m_accs.getFront().getACC().extent(fifthDim)  != positions.extent(firstDim))
		{
			LOG_INFO("ACC shape and parent array positions shape don't match.");
			LOG_ERROR_STR("ACC.shape=" << m_accs.getFront().getACC().shape());
			LOG_ERROR_STR("'" << start.parent << "'.shape=" << positions.shape());
			LOG_ERROR_STR("Expecting AntenneArray with " << 
							m_accs.getFront().getACC().extent(fourthDim) << " antennas.");
			ack.status = ERR_RANGE;
		}
		else if ((start.subset & invalidmask).any()) {
			LOG_INFO("CAL_START: Invalid receiver subset.");
			ack.status = ERR_RANGE;
		}
		else {
			// create subarray to calibrate
			SubArray* subarray = new SubArray(start.name,
							  parent->getGeoLoc(),
							  positions.copy(),
							  select,
							  m_sampling_frequency,
							  start.rcumode()(0).getNyquistZone(),
							  GET_CONFIG("CalServer.N_SUBBANDS", i),
							  start.rcumode()(0).getRaw());

			m_subarrays.schedule_add(subarray);


			_enableRCUs(subarray);


			bitset<MEPHeader::MAX_N_RCUS> validmask;
			
			for (int rcu = 0; rcu < m_n_rcus; ++rcu) {
				validmask.set(rcu);   // select rcu
			}
			
			// calibration will start within one second
			// set the spectral inversion right
			// prepare RSP command
			RSPSetbypassEvent	specInvCmd;
			bool				SIon(start.rcumode()(0).getNyquistZone() == 2);// on or off?
			specInvCmd.timestamp = Timestamp(0,0);
			specInvCmd.rcumask   = start.subset & validmask;
			specInvCmd.settings().resize(1);
			specInvCmd.settings()(0).setXSI(SIon);
			specInvCmd.settings()(0).setYSI(SIon);
			LOG_DEBUG_STR("NyquistZone = " << start.rcumode()(0).getNyquistZone() 
							<< " setting spectral inversion " << ((SIon) ? "ON" : "OFF"));
			m_rspdriver.send(specInvCmd);


			//
			// set the control register of the RCU's 
			// if in HBA mode turn on HBAs in groups to prevent resetting of boards
			// 
			
			RSPSetrcuEvent setrcu;
			bitset<MEPHeader::MAX_N_RCUS> testmask;
			Timestamp timeStamp;
			
			#define N_PWR_RCUS_PER_STEP 12
			
			int nPwrRCUs = m_n_rcus / 2;  // only the even rcus deliver power to the HBAs
			int steps    = nPwrRCUs / N_PWR_RCUS_PER_STEP;  // 4 steps for NL stations, 8 steps for IS stations
			int jump     = m_n_rcus / N_PWR_RCUS_PER_STEP;  // jump = 8 for NL stations and 16 for IS stations
			
			if (steps == 0) { steps = 1; }  // limit for test cabinet
			if (jump < 2) { jump = 2; }     // limit for test cabinet
					
			// if LBA mode select all rcus in one step
			if (start.rcumode()(0).getMode() <= 4) { 
				steps = 1;
				jump = 2;
			}
			
			for (int step = 0; step < steps; ++step) {
				validmask.reset();
				// select 12 even(X) rcus and 12 odd(Y) rcus
				for (int rcu = (step * 2); rcu < m_n_rcus; rcu += jump) {
					validmask.set(rcu);   // select X (HBA power supply)
					validmask.set(rcu+1); // select Y
				}
				
				// if any rcus in this masker send command
				testmask = start.subset & validmask;
				if (testmask.any()) {
					int delay = SCHEDULING_DELAY + step;
					timeStamp.setNow(delay);
					setrcu.timestamp = timeStamp; // in steps of 1 second

					//TODO: Step20.2: might have to send 2 settings e.g. when using all X-pols
			
					setrcu.rcumask = start.subset & validmask;
					setrcu.settings().resize(1);
					setrcu.settings()(0) = start.rcumode()(0);
	
					// previous LOG statement contained start.rcumask.to_ulong() which
					// throws an exception because the number of bits = 256!
					LOG_DEBUG(formatString("Sending RSP_SETRCU(%08X)", start.rcumode()(0).getRaw()));
					m_rspdriver.send(setrcu);
				}
			}

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

	_disableRCUs(m_subarrays.getByName(stop.name));	// switch off unused RCUs

	m_subarrays.schedule_remove(stop.name);	// stop calibration

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
GCFEvent::TResult CalServer::handle_cal_subscribe(GCFEvent& e, GCFPortInterface &port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	CALSubscribeEvent subscribe(e);
	CALSubscribeackEvent ack;
	ack.status = CAL_Protocol::CAL_SUCCESS;

	// get subarray by name
	SubArray* subarray = m_subarrays.getByName(subscribe.name);

	if (subarray) {
		// create subscription
		SubArraySubscription* subscription = new SubArraySubscription(subarray,
														subscribe.subbandset,
														port);

		ack.handle = (CAL_Protocol::memptr_t)subscription;
		subarray->attach(subscription); // attach subscription to the subarray
		ack.subarray = *subarray; 		// return subarray positions

		// don't register subarray in cal_subscribe
		// it has already been registerd in cal_start
		LOG_DEBUG_STR("Subscription succeeded: " << subscribe.name);
	} 
	else {
		ack.status = ERR_NO_SUBARRAY;
		ack.handle = 0;
		//memset(&ack.subarray, 0, sizeof(SubArray));
		// doesn't work with gcc-3.4 ack.subarray = SubArray();
		(void)new((void*)&ack.subarray) SubArray();

		LOG_WARN_STR("Subarray not found: " << subscribe.name);
	}

	port.send(ack);

	return status;
}

//
// handle_cal_unsubscribe(event,port)
//
GCFEvent::TResult CalServer::handle_cal_unsubscribe(GCFEvent& e, GCFPortInterface &port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	CALUnsubscribeEvent unsubscribe(e);

	// create ack
	CALUnsubscribeackEvent ack;
	ack.name = unsubscribe.name;
	ack.handle = unsubscribe.handle;
	ack.status = CAL_Protocol::CAL_SUCCESS;

	// find associated subarray
	SubArray* subarray = m_subarrays.getByName(unsubscribe.name);
	if (subarray) {
		// detach subscription, this destroys the subscription
		subarray->detach((SubArraySubscription*)unsubscribe.handle);

		// handle is no longer valid
		LOG_INFO_STR("Subscription deleted: " << unsubscribe.name);
	} 
	else {
		ack.status = ERR_NO_SUBARRAY;
		LOG_WARN_STR("Subscription failed. Subbarray not found: " << unsubscribe.name);
	}

	port.send(ack);

	return status;
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
	ack.subarraymap = m_subarrays.getSubArrays(request.subarrayname);	// let SubArrays do the job

	// correct status is name was given but nothing was found.
	if (!request.subarrayname.empty() && ack.subarraymap.size() == 0) {
		ack.status = ERR_NO_SUBARRAY;
	}

	port.send(ack);

	return (status);
}

//
// _enableRCUs(subarray*)
//
void CalServer::_enableRCUs(SubArray*	subarray)
{
	// increment the usecount of the receivers
	SubArray::RCUmask_t	rcuMask = subarray->getRCUMask();
	SubArray::RCUmask_t	rcus2switchOn;
	rcus2switchOn.reset();
	for (int r = 0; r < m_n_rcus; r++) {
		if (rcuMask.test(r)) {	
			if(++itsRCUcounts[r] == 1) {
				rcus2switchOn.set(r);
			} // new count is 1
		} // rcu in mask
	} // for all rcus

	// anything to enable? Tell the RSPDriver.
	if (rcus2switchOn.any()) {
		RSPSetrcuEvent	enableCmd;
		enableCmd.timestamp = Timestamp(0,0);
		enableCmd.rcumask = rcus2switchOn;
		enableCmd.settings().resize(1);
		enableCmd.settings()(0).setEnable(true);
		sleep (1);
		LOG_INFO_STR("Enabling some rcu's because they are used for the first time");
		m_rspdriver.send(enableCmd);
	}
}

//
// _disableRCUs(subarray*)
//
void CalServer::_disableRCUs(SubArray*	subarray)
{
	// decrement the usecount of the receivers and build mask of receiver that may be disabled.
	bool				allSwitchedOff(true);
	SubArray::RCUmask_t	rcus2switchOff;
	rcus2switchOff.reset();

	if (subarray) {		// when no subarray is defined skipp this loop: switch all rcus off.
		SubArray::RCUmask_t	rcuMask = subarray->getRCUMask();
		for (int r = 0; r < m_n_rcus; r++) {
			if (rcuMask.test(r)) {	
				if (--itsRCUcounts[r] == 0) {
					rcus2switchOff.set(r);
				} // count reaches 0
			} // rcu in mask

			// independant of the rcuMask check if this receiver is (also) off
			if (itsRCUcounts[r]) { 	// still on?
				allSwitchedOff = false;
			}
		} // for all rcus
	}

	if (allSwitchedOff) { 	// all receivers off? force all rcu's to mode 0, disable
		rcus2switchOff.reset();
		for (int i = 0; i < m_n_rcus; i++) {
			rcus2switchOff.set(i);
		}
		LOG_INFO("No active rcu's anymore, forcing all units to mode 0 and disable");
	}

	// anything to disable? Tell the RSPDriver.
	if (rcus2switchOff.any()) {
		RSPSetrcuEvent	disableCmd;
		disableCmd.timestamp = Timestamp(0,0);
		disableCmd.rcumask = rcus2switchOff;
		disableCmd.settings().resize(1);
		disableCmd.settings()(0).setEnable(false);
		disableCmd.settings()(0).setMode(RCUSettings::Control::MODE_OFF);
		sleep (1);
		LOG_INFO_STR("Disabling " << rcus2switchOff.count() << " rcu's because they are not used anymore");
		m_rspdriver.send(disableCmd);
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

  // find associated subarray
  SubArray* subarray = m_subarrays.getByName(getsubarray.name);
  if (subarray) {

    // return antenna positions
    ack.positions = *(static_cast<AntennaArray*>(subarray));

    LOG_INFO("Sending subarray info: " << getsubarray.name);

  } else {

    // TODO: need sensible value for ack.positions, it is not initialized at this point
    ack.status = ERR_NO_SUBARRAY;

    LOG_INFO("Getsubarray. Subarray not found: " << getsubarray.name);

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

//
// MAIN
//
int main(int argc, char** argv)
{
	/* daemonize if required */
	if (argc >= 2) {
		if (!strcmp(argv[1], "-d")) {
			if (0 != daemonize(false)) {
				cerr << "Failed to background this process: " << strerror(errno) << endl;
				exit(EXIT_FAILURE);
			}
		}
	}

	GCFScheduler::instance()->init(argc, argv, "CalServer");

	LOG_INFO("MACProcessScope: LOFAR_PermSW_CalServer");

	LOG_INFO(formatString("Program %s has started", argv[0]));

	ACCs* 			accs; // the ACC buffers
	StationConfig	SC;
	accs = new ACCs(GET_CONFIG("CalServer.N_SUBBANDS", i), SC.nrRSPs * NR_ANTENNAS_PER_RSPBOARD, NPOL);

	if (!accs) {
		LOG_FATAL("Failed to allocate memory for the ACC arrays.");
		exit(EXIT_FAILURE);
	}

	//
	// create CalServer and ACMProxy tasks
	// they communicate via the ACCs instance
	//
	try {
		CalServer cal     ("CalServer", *accs, argc, argv);
		ACMProxy  acmproxy("ACMProxy",  *accs);

		cal.start();      // make initial transition
		acmproxy.start(); // make initial transition

		GCFScheduler::instance()->run();
	}
	catch (Exception& e) {
		LOG_ERROR_STR("Exception: " << e.text());
		exit(EXIT_FAILURE);
	}

	delete accs;

	LOG_INFO("Normal termination of program");

	return 0;
}
