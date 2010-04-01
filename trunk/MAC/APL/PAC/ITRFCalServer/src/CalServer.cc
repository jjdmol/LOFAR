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
#include <Common/Version.h>

#include <APL/APLCommon/AntennaPos.h>
#include <APL/RTCCommon/ResourceCache.h>
#include <APL/ICAL_Protocol/ICAL_Protocol.ph>
#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/LBA_Calibration/lba_calibration.h>		// the matlab stuff

#include <MACIO/MACServiceInfo.h>

#include "CalServer.h"
#include "SubArrays.h"
#include "SubArraySubscription.h"
#include "LBACalibration.h"
//#include "Package__Version.h"

#ifdef USE_CAL_THREAD
#include "CalibrationThread.h"
#endif

#include "ACMProxy.h"

// from RTCCommon
#include <APL/RTCCommon/Timestamp.h>
#include <APL/RTCCommon/PSAccess.h>

#include <APS/ParameterSet.h>
#include <fstream>
#include <signal.h>
#include <getopt.h>

#include <blitz/array.h>

using namespace std;
using namespace blitz;

namespace LOFAR {
  using namespace RTC;
  using namespace RSP_Protocol;
  using namespace ICAL_Protocol;
  namespace CAL {

//
// CalServer constructor
//
CalServer::CalServer(const string&			name, 
					 ResourceCache&			accs, 
					 CalibrationInterface*	theLBAcal) : 
	GCFTask				((State)&CalServer::initial, name),
	itsACCs				(accs), 
	itsACCsSwapped		(false),
	itsLBAcal			(theLBAcal), 
	m_sampling_frequency(0.0),
	m_n_rspboards		(0), 
	m_n_rcus			(0), 
	m_n_subbands		(0),
	m_instancenr		(-1)
#ifdef USE_CAL_THREAD
    , m_calthread		(0)
#endif
{
#ifdef USE_CAL_THREAD
	pthread_mutex_init(&m_globallock, 0);
#endif

//	LOG_INFO(Version::getInfo<CalServerVersion>("CalServer"));

	registerProtocol(ICAL_PROTOCOL, ICAL_PROTOCOL_STRINGS);
	itsListener = new GCFTCPPort(*this, MAC_SVCMASK_CALSERVER, GCFPortInterface::MSPP, ICAL_PROTOCOL);
	ASSERTSTR(itsListener, "Can't create a listener socket");

	registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_STRINGS);
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
	if (m_calthread) {
		delete m_calthread;
	}
#endif
}

//
// undertaker()
//
void CalServer::undertaker()
{
	list<GCFPortInterfac*>::iterator	iter = m_dead_clients.begin();
	list<GCFPortInterfac*>::iterator	end  = m_dead_clients.end();
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
			SubArray* subarray = m_subarrays.getByName(iter->first);
			if (subarray) {
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
	LOG_DEBUG_STR("enabled:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch(e.signal) {
	case F_INIT: {
		try { 

#ifdef USE_CAL_THREAD
			pthread_mutex_lock(&m_globallock); // lock for dipolemodels, and sources
#endif

			ConfigLocator cl;

			// load the dipole models
			LOG_INFO("Reading DipoleModel information");
			m_dipolemodels.getAll(cl.locate(GET_CONFIG_STRING("CalServer.DipoleModelFile")));

			// load the source catalog
			LOG_INFO("Reading SourceCatalog");
			m_sources.getAll(cl.locate(GET_CONFIG_STRING("CalServer.SourceCatalogFile")));

			// load the antenna positions
			ASSERTSTR(globalAntennaPos(), "Could not load the antennaposition file");
			LOG_DEBUG("Loaded antenna postions file");

			m_n_subbands = GET_CONFIG("CalServer.N_SUBBANDS", i);

#ifdef USE_CAL_THREAD
			// Setup calibration thread
			m_calthread = new CalibrationThread(&m_subarrays, itsLBAcal, m_globallock);

			pthread_mutex_unlock(&m_globallock); // unlock global lock
#endif

		} catch (Exception e)  {

#ifdef USE_CAL_THREAD
			pthread_mutex_unlock(&m_globallock); // unlock global lock
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
		LOG_DEBUG("opening port: m_rspdriver");
		itsRSPDriver->open();
	}
	break;

	case F_CONNECTED: {
		if (itsListener->isConnected() && itsRSPDriver->isConnected()) {
			RSPGetconfigEvent getconfig;
			itsRSPDriver->send(getconfig);
		}
	}
	break;

	case RSP_GETCONFIGACK: {
		RSPGetconfigackEvent ack(e);
		m_n_rspboards = ack.n_rspboards;
		m_n_rcus = ack.n_rcus;

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

		// get clock value and convert to MHz
		m_sampling_frequency = getclockack.clock * (uint32)1.0e6;

		LOG_INFO_STR("Initial sampling frequency: " << m_sampling_frequency);

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

		TRAN(CalServer::enabled);
	}
	break;

	case F_DISCONNECTED: {
		LOG_DEBUG(formatString("port '%s' disconnected, retry in 3 seconds...", port.getName().c_str()));
		port.close();
	}
	break;

	case F_CLOSED: {
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
	LOG_DEBUG_STR("enabled:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY: {
		itsHeartBeat->setTimer(0.0, 1.0);
	}
	break;

	case F_ACCEPT_REQ: {
		GCFTCPPort* client = new GCFTCPPort();
		client->init(*this, "client", GCFPortInterface::SPP, ICAL_PROTOCOL);
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
		m_sampling_frequency = updclock.clock * (uint32)1.0e6;

		LOG_INFO_STR("New sampling frequency: " << m_sampling_frequency);
	}
	break;

	case RSP_SETRCUACK: {
		RSPSetrcuackEvent ack(e);
		if (ack.status != RSP_Protocol::SUCCESS) {
			LOG_FATAL("Failed to set RCU control register");
			exit (EXIT_FAILURE);
		}
	}
	break;

	case RSP_SETBYPASSACK: {
		RSPSetbypassackEvent ack(e);
		if (ack.status != RSP_Protocol::SUCCESS) {
			LOG_FATAL("Failed to set Spectral Inversion control register");
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
		GCFTimerEvent* timer = static_cast<GCFTimerEvent*>(&e);

		const Timestamp t = Timestamp(timer->sec, timer->usec);
		LOG_DEBUG_STR("updateAll @ " << t);

		//
		// Swap buffers when all calibrations have finished on the front buffer
		// and the back buffer is not locked and is valid (has been filled by ACMProxy).
		// 
		if (itsACCs.isSwapped() != itsACCsSwapped) {	
			LOG_INFO("ACC buffers where swapped, restarting calibration");
			itsACCsSwapped = itsACCs.isSwapped();

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
			m_calthread->run();
#else
			bool	writeGains(GET_CONFIG("CalServer.WriteGainsToFile", i) == 1);
			if (GET_CONFIG("CalServer.DisableCalibration", i)) {
				m_subarrays.calibrate(0, writeGains);
			} else {
				// Calc position of the calibration sources for this new ACC
				itsLBAcal->repostionSources(time(0L) - (m_n_subbands / 2));
				// Calibrate all beams.
				m_subarrays.calibrate(itsLBAcal, writeGains);
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

	case ICAL_START:
		status = handle_cal_start(e, port);
	break;

	case ICAL_STOP:
		status = handle_cal_stop(e, port);
	break;

	case ICAL_SUBSCRIBE:
		status = handle_cal_subscribe(e, port);
	break;

	case ICAL_UNSUBSCRIBE:
		status = handle_cal_unsubscribe(e, port);
	break;

	case ICAL_GETSUBARRAY:
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

	ICALStartEvent 	 start(e);
	ICALStartackEvent ack;

	ack.status = ICAL_Protocol::ICAL_SUCCESS; // assume succes, until otherwise
	ack.name   = start.name;

	if (m_subarrays.getByName(start.name)) {
		LOG_ERROR_STR("A subarray with name='" << start.name << "' has already been registered.");
		ack.status = ERR_ALREADY_REGISTERED;

	} else if (string(start.name) == "") {
		LOG_ERROR("Empty subarray name.");
		ack.status = ERR_NO_SUBARRAY_NAME;

	} else if (!globalAntennaPos()->isAntennaField(ack.parent)) {
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

// TODO: DO THIS COMPLETELY DIFFERENT
		// Construct a 'select' array (nAntennas, nPol) that knows which antennas the user will use.
		const Array<double, 3>& positions = globalAntennaPos()->AntPos(ack.parent);
//		const Array<double, 3>& positions = parent->getAntennaPos();
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

//		LOG_DEBUG_STR("m_accs.getBack().getACC().shape()=" << m_accs.getBack().getACC().shape());
		LOG_DEBUG_STR("positions.shape()" << positions.shape());

		// check start.subset value
		bitset<MEPHeader::MAX_N_RCUS> invalidmask;
		for (int i = 0; i < m_n_rcus; i++) {
		  invalidmask.set(i);
		}
		invalidmask.flip();

		// check dimensions of the various arrays for compatibility
		if (positions.extent(firstDim) != (m_n_rcus/2) || positions.extent(secondDim) != N_POL) {
			LOG_ERROR_STR("Expecting AntenneArray with " << m_n_rcus/2 << " antennas.");
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
							  m_n_subbands,
							  start.rcumode()(0).getRaw());

			m_subarrays.schedule_add(subarray);

			// calibration will start within one second

			//
			// set the control register of the RCU's 
			//
			RSPSetrcuEvent setrcu;
			setrcu.timestamp = Timestamp(0,0); // immediate

			// TODO: Step20.2: might have to send 2 settings e.g. when using all X-pols
			// mask only available RCUs
			bitset<MEPHeader::MAX_N_RCUS> validmask;
			for (int i = 0; i < m_n_rcus; i++) {
				validmask.set(i);
			}
			setrcu.rcumask = start.subset & validmask;
			setrcu.settings().resize(1);
			setrcu.settings()(0) = start.rcumode()(0);

			// previous LOG statement contained start.rcumask.to_ulong() which
			// throws an exception because the number of bits = 256!
			LOG_DEBUG(formatString("Sending RSP_SETRCU(%08X)", start.rcumode()(0).getRaw()));
			itsRSPDriver->send(setrcu);

			// set the spectral inversion right
			// prepare RSP command
			RSPSetbypassEvent	specInvCmd;
			bool				SIon(start.rcumode()(0).getNyquistZone() == 2);// on or off?
			specInvCmd.timestamp = Timestamp(0,0);
			specInvCmd.rcumask   = setrcu.rcumask;
			specInvCmd.settings().resize(1);
			specInvCmd.settings()(0).setXSI(SIon);
			specInvCmd.settings()(0).setYSI(SIon);
			LOG_DEBUG_STR("NyquistZone = " << start.rcumode()(0).getNyquistZone() 
							<< " setting spectral inversion " << ((SIon) ? "ON" : "OFF"));
			itsRSPDriver->send(specInvCmd);
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
	ICALStopEvent stop(e);
	ICALStopackEvent ack;
	ack.name = stop.name;
	ack.status = ICAL_Protocol::SUCCESS;		// return success: don't bother client with our admin
	port.send(ack);

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

	ICALSubscribeEvent subscribe(e);
	ICALSubscribeackEvent ack;
	ack.status = ICAL_Protocol::SUCCESS;

	// get subarray by name
	SubArray* subarray = m_subarrays.getByName(subscribe.name);

	if (subarray) {
		// create subscription
		SubArraySubscription* subscription = new SubArraySubscription(subarray,
														subscribe.subbandset,
														port);

		ack.handle = (ICAL_Protocol::memptr_t)subscription;
		subarray->attach(subscription); // attach subscription to the subarray
		ack.subarray = *subarray; 		// return subarray positions

		// don't register subarray in cal_subscribe
		// it has already been registerd in cal_start
		LOG_INFO_STR("Subscription succeeded: " << subscribe.name);
	} 
	else {
		ack.status = ERR_NO_SUBARRAY;
		ack.handle = 0;
		//memset(&ack.subarray, 0, sizeof(SubArray));
		// doesn't work with gcc-3.4 ack.subarray = SubArray();
		(void)new((void*)&ack.subarray) SubArray();

		LOG_INFO_STR("Subarray not found: " << subscribe.name);
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

	ICALUnsubscribeEvent unsubscribe(e);

	// create ack
	ICALUnsubscribeackEvent ack;
	ack.name = unsubscribe.name;
	ack.handle = unsubscribe.handle;
	ack.status = ICAL_Protocol::SUCCESS;

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
		LOG_INFO_STR("Subscription failed. Subbarray not found: " << unsubscribe.name);
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

	ICALGetsubarrayEvent	 	request(e);
	ICALGetsubarrayackEvent	ack;
	ack.status = ICAL_Protocol::SUCCESS;
	ack.subarraymap = m_subarrays.getSubArrays(request.subarrayname);	// let SubArrays do the job

	// correct status is name was given but nothing was found.
	if (!request.subarrayname.empty() && ack.subarraymap.size() == 0) {
		ack.status = ERR_NO_SUBARRAY;
	}

	port.send(ack);

	return (status);
}

#if 0
GCFEvent::TResult CalServer::handle_cal_getsubarray(GCFEvent& e, GCFPortInterface &port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  ICALGetsubarrayEvent getsubarray(e);

  // create ack
  ICALGetsubarrayackEvent ack;
  ack.status = ICAL_Protocol::SUCCESS;

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

  } // namespace CAL
} // namespace LOFAR


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

	GCFTask::init(argc, argv, "CalServer");

	LOG_INFO("MACProcessScope: LOFAR_PermSW_CalServer");

	LOG_INFO(formatString("Program %s is starting", argv[0]));

	int nrSubbands = globalParameterSet()->getInt("CalServer.N_SUBBANDS");
	int nrAnts     = globalParameterSet()->getInt("CalServer.N_ANTENNAS");

	// First create the LBACalibration instance because that initializes the matlab env.
	LOG_INFO("Creating the LBA calibration component");
	LBACalibration	calibrationModule;
	
	// Allocate space for the Matlab flavor of the MatLab arrays.
	const mwSize dim[3] = { nrAnts, nrAnts, nrSubbands };
	mwArray*	matlabACC1 = new mwArray(3, dim, mxDOUBLE_CLASS, mxCOMPLEX);
	mwArray*	matlabACC2 = new mwArray(3, dim, mxDOUBLE_CLASS, mxCOMPLEX);
//	mwArray		matlabACC1(3, dim, mxDOUBLE_CLASS, mxCOMPLEX);
//	mwArray		matlabACC2(3, dim, mxDOUBLE_CLASS, mxCOMPLEX);
	ASSERTSTR(matlabACC1 && matlabACC2, "Allocation of Matlab ACC's failed.");
	// Put them in a resource cache.
	ResourceCache	AccCache(&matlabACC1, &matlabACC2);

	// inform the calibr. module about the cache.
	calibrationModule.setACCs(&AccCache);

	// create CalServer and ACMProxy tasks
	// they communicate via the ACCs instance
	try {
		CalServer cal     ("CalServer", AccCache, &calibrationModule, argc, argv);
		ACMProxy  acmproxy("ACMProxy",  AccCache);	// always writes in back buffer.

		cal.start();      // make initial transition
		acmproxy.start(); // make initial transition

		GCFTask::run();
	}
	catch (Exception e) {
		LOG_ERROR_STR("Exception: " << e.text());
		exit(EXIT_FAILURE);
	}

	delete matlabACC1;
	delete matlabACC2;

	LOG_INFO("Normal termination of program");

	return (0);
}
