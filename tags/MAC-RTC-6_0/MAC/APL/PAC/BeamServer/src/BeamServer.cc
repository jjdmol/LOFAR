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

#include <GCF/GCF_ServiceInfo.h>

#include <APL/RTCCommon/daemonize.h>
#include <APL/BS_Protocol/BS_Protocol.ph>
#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/CAL_Protocol/CAL_Protocol.ph>

#include "BeamServer.h"

#include "Beam.h"
#include "Beamlet.h"

#include <iostream>
#include <sstream>
#include <time.h>
#include <string.h>
#include <getopt.h>

#include <netinet/in.h>

#include <APL/RTCCommon/PSAccess.h>

#include <blitz/array.h>

#include <AMCBase/ConverterClient.h>

using namespace LOFAR;
using namespace blitz;
using namespace BS;
using namespace std;
using namespace RTC;

using namespace RSP_Protocol;

#define LEADIN_TIME ((long)10)
#define COMPUTE_INTERVAL ((long)10)
#define UPDATE_INTERVAL  5
#define N_DIM 3 // x, y, z or l, m, n

#define SCALE (1<<(16-2))

#define SYSTEM_CLOCK_FREQ 120e6 // 120 MHz

//
// global variable for beamformer gain
//
static int g_bf_gain = 0;

//
// parseOptions
//
void BeamServer::parseOptions(int	argc,
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
	case 'd':
	  break;
    default:
      LOG_FATAL (formatString("Unknown option %c", c));
      ASSERT(false);
    } // switch
  } // for loop
}

BeamServer::BeamServer(string name, int argc, char** argv)
    : GCFTask((State)&BeamServer::initial, name),
      m_beams_modified(false),
      m_sampling_frequency(160000000),
      m_nyquist_zone(1),
      m_beams(MEPHeader::N_BEAMLETS, MEPHeader::N_SUBBANDS),
      m_converter("localhost"),
	  m_instancenr(-1)
{
  // adopt commandline switches
  parseOptions(argc, argv);

  registerProtocol(BS_PROTOCOL,  BS_PROTOCOL_signalnames);
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);
  registerProtocol(CAL_PROTOCOL, CAL_PROTOCOL_signalnames);

  string instanceID;
  if (m_instancenr >= 0) {
    instanceID=formatString("(%d)", m_instancenr);
  }
  m_acceptor.init(*this, MAC_SVCMASK_BEAMSERVER + instanceID, GCFPortInterface::MSPP, BS_PROTOCOL);
  m_rspdriver.init(*this, MAC_SVCMASK_RSPDRIVER + instanceID, GCFPortInterface::SAP, RSP_PROTOCOL);
  m_calserver.init(*this, MAC_SVCMASK_CALSERVER + instanceID, GCFPortInterface::SAP, CAL_PROTOCOL);
}

BeamServer::~BeamServer()
{}

bool BeamServer::isEnabled()
{
  return m_rspdriver.isConnected() && m_calserver.isConnected();
}

GCFEvent::TResult BeamServer::initial(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch(e.signal)
    {
    case F_INIT:
      break;

    case F_ENTRY:
      {
	/**
	 * Check explicitly for S_DISCONNECTED state instead of !isConnected()
	 * because a transition from ::enabled into ::initial will have state S_CLOSING
	 * which is also !isConnected() but in this case we don't want to open the port
	 * until after we have handled F_CLOSED for that port.
	 */
	if (GCFPortInterface::S_DISCONNECTED == m_rspdriver.getState()) m_rspdriver.open();
	if (GCFPortInterface::S_DISCONNECTED == m_calserver.getState()) m_calserver.open();
      }
      break;

    case F_CONNECTED:
      {
	LOG_INFO(formatString("CONNECTED: port '%s' connected", port.getName().c_str()));
	if (isEnabled())
	  {
	    RSPGetconfigEvent getconfig;
	    m_rspdriver.send(getconfig);
	  }
      }
      break;

    case RSP_GETCONFIGACK:
      {
	RSPGetconfigackEvent ack(e);
      
	m_nrcus = ack.n_rcus;
	m_weights.resize(COMPUTE_INTERVAL, m_nrcus, MEPHeader::N_BEAMLETS);
	m_weights16.resize(COMPUTE_INTERVAL, m_nrcus, MEPHeader::N_BEAMLETS);

	// initialize weight matrix
	m_weights   = complex<double>(0,0);
	m_weights16 = complex<int16_t>(0,0);

	// start update timer and start accepting clients
	m_rspdriver.setTimer(0, 0, UPDATE_INTERVAL, 0);
	if (GCFPortInterface::S_DISCONNECTED == m_acceptor.getState()) m_acceptor.open();

	TRAN(BeamServer::enabled);
      }
      break;

    case F_DISCONNECTED:
      {
	port.close();
      }
      break;

    case F_CLOSED:
      {
	// try connecting again in 2 seconds
	port.setTimer((long)2);
	LOG_DEBUG(formatString("port '%s' disconnected, retry in 2 seconds...", port.getName().c_str()));
      }
      break;

    case F_TIMER:
      {
	if (GCFPortInterface::S_DISCONNECTED == port.getState()) {
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

void BeamServer::undertaker()
{
  for (list<GCFPortInterface*>::iterator it = m_dead_clients.begin();
       it != m_dead_clients.end();
       it++)
  {
    LOG_DEBUG_STR("undertaker: deleting '" << (*it)->getName() << "'");
    delete (*it);
  }
  m_dead_clients.clear();
}

void BeamServer::destroyAllBeams(GCFPortInterface* port)
{
  ASSERT(port);

  // deallocate all beams for this client
  for (set<Beam*>::iterator beamit = m_client_beams[port].begin();
       beamit != m_client_beams[port].end(); ++beamit)
    {
      if (!m_beams.destroy(*beamit)) {
	LOG_WARN("Beam not found...");
      }
    }
  m_client_beams.erase(port);
}

Beam* BeamServer::newBeam(BeamTransaction& bt, GCFPortInterface* port,
			  std::string name, std::string subarrayname, BS_Protocol::Beamlet2SubbandMap allocation)
{
	ASSERT(port);

	// check for valid parameters
	// returning 0 will result in a negative ACK
	if (bt.getBeam() != 0 || bt.getPort() != 0) {
		LOG_DEBUG("Previous alloc is still in progress");
		 return (0);
	}
	if (name.length() == 0) {
		LOG_DEBUG("Name of beam not set, cannot alloc new beam");
		return (0);
	}
	if (subarrayname.length() == 0)  {
		LOG_DEBUG("SubArrayName not set, cannot alloc new beam");
		return (0); 
	}

	Beam* beam = m_beams.get(name, subarrayname, allocation);

	if (beam) { // register new beam
		m_client_beams[port].insert(beam);
		m_beams_modified = true;
		bt.set(port, beam);
	}

	return (beam);
}

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

GCFEvent::TResult BeamServer::enabled(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  static int period = 0;

  undertaker();

  switch (e.signal)
    {
    case F_ENTRY:
      {
	m_calserver.setTimer((long)0); // trigger single recall
      }
      break;

    case F_ACCEPT_REQ:
      {
	GCFTCPPort* client = new GCFTCPPort();
	client->init(*this, "client", GCFPortInterface::SPP, BS_PROTOCOL);
	if (!m_acceptor.accept(*client)) delete client;
	else {
	  m_client_list.push_back(client);
	  LOG_INFO(formatString("NEW CLIENT CONNECTED: %d clients connected", m_client_list.size()));
	}
      }
      break;
      
    case F_CONNECTED:
      {
	LOG_INFO(formatString("CONNECTED: port '%s'", port.getName().c_str()));
      }
      break;

    case F_TIMER:
      {
	if (&port == &m_calserver) {

	  // recall one deferred event, set timer again if an event was handled
	  if (recall(port) == GCFEvent::HANDLED) m_calserver.setTimer((long)0);

	} else {
	  // &port == &m_rspdriver

	  GCFTimerEvent* timer = static_cast<GCFTimerEvent*>(&e);

	  LOG_DEBUG_STR("timer=" << Timestamp(timer->sec, timer->usec));

	  period += UPDATE_INTERVAL;
	  if (period >= COMPUTE_INTERVAL)
	    {
	      period = 0;

	      // compute new weights and send them weights
	      LOG_INFO_STR("computing weights " << Timestamp(timer->sec, timer->usec));
	      compute_weights(Timestamp(timer->sec, 0) + LEADIN_TIME);

	      send_weights(Timestamp(timer->sec, 0) + LEADIN_TIME);
	    }

	  if (m_beams_modified)
	    {
	      send_sbselection();

	      m_beams_modified = false;
	    }
	}
      }
      break;

    case BS_BEAMALLOC:
      {
	BSBeamallocEvent event(e);
	if (beamalloc_start(event, port)) {
	  TRAN(BeamServer::beamalloc_state);
	}
      }
      break;

    case BS_BEAMFREE:
      {
	BSBeamfreeEvent event(e);
	if (beamfree_start(event, port)) {
	  TRAN(BeamServer::beamfree_state);
	}
      }
      break;

    case BS_BEAMPOINTTO:
      {
	BSBeampointtoEvent event(e);
	beampointto_action(event, port);
      }
      break;
      
    case RSP_SETWEIGHTSACK:
      {
	RSPSetweightsackEvent ack(e);
	if (RSP_Protocol::SUCCESS != ack.status)
	  {
	    LOG_ERROR("RSP_SETWEIGHTSACK: FAILURE");
	  }
      }
      break;
      
    case RSP_SETSUBBANDSACK:
      {
	RSPSetsubbandsackEvent ack(e);
	if (RSP_Protocol::SUCCESS != ack.status)
	  {
	    LOG_ERROR("RSP_SETSUBBANDSACK: FAILURE");
	  }
      }
      break;

    case CAL_UPDATE:
      {
	CALUpdateEvent calupd(e);

	if (CAL_Protocol::SUCCESS == calupd.status) {
	  LOG_INFO("Received valid CAL_UPDATE event.");

	  m_beams.updateCalibration(calupd.handle, calupd.gains);
	}
      }
      break;

    case F_DISCONNECTED:
      {
	LOG_INFO(formatString("DISCONNECTED: port %s disconnected", port.getName().c_str()));

	if (&m_rspdriver == &port || &m_acceptor == &port || &m_calserver == &port) {
	  TRAN(BeamServer::cleanup);
	} else {
	  port.close();
	}
      }
      break;
	  
    case F_CLOSED:
      {
	LOG_INFO(formatString("CLOSED: port %s closed", port.getName().c_str()));

	if (!(&m_rspdriver == &port || &m_acceptor == &port || &m_calserver == &port)) {
	  destroyAllBeams(&port);

	  m_client_list.remove(&port);
	  m_dead_clients.push_back(&port);
	}
      }
      break;

    case F_EXIT:
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

GCFEvent::TResult BeamServer::cleanup(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
    {

    case F_ENTRY:
      {
	/**
	 * close connection with all clients.
	 */
	if (m_client_list.size() > 0) {
	  for (list<GCFPortInterface*>::iterator it = m_client_list.begin();
	       it != m_client_list.end();
	       it++)
	    {
	      if ((*it)->isConnected()) (*it)->close();
	    }
	} else {
	  /**
	   * cancelAllTimers is called here BEFORE m_rspdriver.close()
	   * because cancelAllTimers really cancels all timers, also any
	   * internal timers such as the zero timers used to send internal
	   * signals such as F_CLOSED!!! If you call cancelAllTimers after
	   * m_rspdriver.close() you will never receive an F_CLOSED on m_rspdriver.
	   */
	  m_rspdriver.cancelAllTimers();
	  m_rspdriver.close();
	  m_acceptor.close();
	  m_calserver.close();
	  TRAN(BeamServer::initial);
	}
      }
      break;

    case F_CLOSED:
      {
	LOG_INFO(formatString("CLOSED: port %s closed", port.getName().c_str()));

	if (!(&m_rspdriver == &port || &m_acceptor == &port || &m_calserver == &port)) {
	  destroyAllBeams(&port);

	  m_client_list.remove(&port);
	  m_dead_clients.push_back(&port);
	}

	if (0 == m_client_list.size()) {
	  /**
	   * cancelAllTimers is called here BEFORE m_rspdriver.close()
	   * because cancelAllTimers really cancels all timers, also any
	   * internal timers such as the zero timers used to send internal
	   * signals such as F_CLOSED!!! If you call cancelAllTimers after
	   * m_rspdriver.close() you will never receive an F_CLOSED on m_rspdriver.
	   */
	  m_rspdriver.cancelAllTimers();
	  m_rspdriver.close();
	  m_acceptor.close();
	  m_calserver.close();
	  TRAN(BeamServer::initial);
	}
      }
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

GCFEvent::TResult BeamServer::beamalloc_state(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
    {
    case F_ENTRY:
      {
	// subscribe to calibration updates
	CALSubscribeEvent subscribe;
	subscribe.name = m_bt.getBeam()->getSubarrayName();
	subscribe.subbandset = m_bt.getBeam()->getAllocation().getAsBitset();

	LOG_INFO_STR("Subscribing to subarray: " << subscribe.name);

	m_calserver.send(subscribe);
      }
      break;

    case CAL_SUBSCRIBEACK:
      {
	CALSubscribeackEvent ack(e);
	BSBeamallocackEvent beamallocack;

	if (CAL_Protocol::SUCCESS == ack.status
	    && (ack.subarray.getSPW().getNumSubbands() >=
		(int)m_bt.getBeam()->getAllocation()().size()) ) {

	  LOG_INFO_STR("Got subscription to subarray " << ack.subarray.getName());
	  LOG_DEBUG_STR("ack.subarray.positions=" << ack.subarray.getAntennaPos());

	  // set positions on beam
	  m_bt.getBeam()->setSubarray(ack.subarray);

	  // set calibration handle for this beam
	  m_beams.setCalibrationHandle(m_bt.getBeam(), ack.handle);

	  // send succesful ack
	  beamallocack.status = BS_Protocol::SUCCESS;
	  beamallocack.handle = (uint32)m_bt.getBeam();
	  m_bt.getPort()->send(beamallocack);

	} else {

	  LOG_INFO("Failed to subscribe to subarray");

	  // failed to subscribe
	  beamallocack.status = ERR_BEAMALLOC;
	  beamallocack.handle = 0;
	  m_bt.getPort()->send(beamallocack);

	  // delete the beam (resets m_bt)
	  deleteBeam(m_bt);
	}

	TRAN(BeamServer::enabled);
      }
      break;

    case F_DISCONNECTED:
      {
	LOG_INFO(">>> deferring F_DISCONNECTED event <<<");
	defer(e, port); // process F_DISCONNECTED again in enabled state

	if (&port == m_bt.getPort()) TRAN(BeamServer::enabled);
      }
      break;

    case F_EXIT:
      {
	// completed current transaction, reset
	m_bt.reset();
      }
      break;

    default:
      // all other events are handled in the enabled state
      defer(e, port);
      break;
    }

  return status;
}

GCFEvent::TResult BeamServer::beamfree_state(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
    {
    case F_ENTRY:
      {
	// unsubscribe
	CALUnsubscribeEvent unsubscribe;
	unsubscribe.name = m_bt.getBeam()->getSubarrayName();
	unsubscribe.handle = m_beams.findCalibrationHandle(m_bt.getBeam());
	ASSERT(0 != unsubscribe.handle);
	m_calserver.send(unsubscribe);
      }
      break;

    case CAL_UNSUBSCRIBEACK:
      {
	CALUnsubscribeackEvent ack(e);
	BSBeamfreeackEvent beamfreeack;

	// if the subarray disappeared because
	// it was stopped (CAL_STOP by SRG) then
	// issue a warning but continue
	if (CAL_Protocol::SUCCESS != ack.status) {
	  LOG_WARN("CAL_UNSUBSCRIBE failed");
	}

	// send succesful ack
	beamfreeack.status = BS_Protocol::SUCCESS;
	beamfreeack.handle = (uint32)m_bt.getBeam();

	m_bt.getPort()->send(beamfreeack);

	// destroy beam, updates m_bt
	deleteBeam(m_bt);

	TRAN(BeamServer::enabled);
      }
      break;

    case F_DISCONNECTED:
      {
	LOG_INFO(">>> deferring F_DISCONNECTED event <<<");
	defer(e, port); // process F_DISCONNECTED again in enabled state

	if (&port == m_bt.getPort()) TRAN(BeamServer::enabled);
      }
      break;

    case F_EXIT:
      {
	// delete beam if not already done so
	if (m_bt.getBeam()) {
	  deleteBeam(m_bt);
	}

	// completed current transaction, reset
	m_bt.reset();
      }
      break;

    default:
      // all other events are handled in the enabled state
      defer(e, port);
      break;
    }

  return status;
}

bool BeamServer::beamalloc_start(BSBeamallocEvent& ba,
				 GCFPortInterface& port)
{
  // allocate the beam
  Beam* beam = newBeam(m_bt, &port, ba.name, ba.subarrayname, ba.allocation);

  if (!beam) {

    LOG_INFO("BEAMALLOC: failed to allocate beam");

    BSBeamallocackEvent ack;
    ack.handle = 0;
    ack.status = BS_Protocol::ERR_RANGE;
    port.send(ack);

    return false;
  }

  return true;
}

bool BeamServer::beamfree_start(BSBeamfreeEvent&  bf,
				GCFPortInterface& port)
{
  Beam* beam = (Beam*)bf.handle;

  if (!m_beams.exists(beam)) {

    LOG_ERROR("BEAMFREE failed: beam does not exist");

    BSBeamfreeackEvent ack;
    ack.handle = bf.handle;
    ack.status = BS_Protocol::ERR_BEAMFREE;
    port.send(ack);

    return false;

  }

  // remember on which beam we're working
  m_bt.set(&port, beam);

  return true;
}

bool BeamServer::beampointto_action(BSBeampointtoEvent& pt,
				    GCFPortInterface& /*port*/)
{
  bool status = true;

  Beam* beam = (Beam*)pt.handle;

  if (m_beams.exists(beam))  {
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
    if (Timestamp(0,0) == pt.pointing.time()) pt.pointing.setTime(actualtime);
    beam->addPointing(pt.pointing);
  } else {
    LOG_ERROR(formatString("BEAMPOINTTO: invalid beam handle (%d)", pt.handle));
    status = false;
  }

  return status;
}

BZ_DECLARE_FUNCTION_RET(convert2complex_int16_t, complex<int16_t>)

/**
 * Convert the weights to 16-bits signed integer.
 */
inline complex<int16_t> convert2complex_int16_t(complex<double> cd)
{
  return complex<int16_t>((int16_t)(round(cd.real() * g_bf_gain)),
			  (int16_t)(round(cd.imag() * g_bf_gain)));
}

/**
 * This method is called once every period
 * of COMPUTE_INTERVAL seconds
 * to calculate the weights for all beamlets.
 */
void BeamServer::compute_weights(Timestamp time)
{
  // calculate weights for all beamlets
  m_beams.calculate_weights(time, COMPUTE_INTERVAL, m_weights, &m_converter);

  // convert the weights from double to int16
  m_weights16 = convert2complex_int16_t(m_weights);

  LOG_DEBUG(formatString("sizeof(m_weights16) = %d", m_weights16.size()*sizeof(int16_t)));
}

void BeamServer::send_weights(Timestamp time)
{

  LOG_DEBUG_STR("weights_uint16=" << m_weights16);

  if (!GET_CONFIG("BeamServer.DISABLE_SETWEIGHTS", i)) {
    RSPSetweightsEvent sw;

    sw.timestamp = time;
    LOG_DEBUG_STR("sw.time=" << sw.timestamp);

    // select all BLPS, no subarraying
    sw.rcumask.reset();
    for (int i = 0; i < m_nrcus; i++) sw.rcumask.set(i);

    sw.weights().resize(COMPUTE_INTERVAL, m_nrcus, MEPHeader::N_BEAMLETS);
    sw.weights() = m_weights16;

    LOG_INFO_STR("sending weights for interval " << time << " : " << time + (long)(COMPUTE_INTERVAL-1));
    m_rspdriver.send(sw);
  }
}

void BeamServer::send_sbselection()
{
  if (!GET_CONFIG("BeamServer.DISABLE_SETSUBBANDS", i)) {
    RSPSetsubbandsEvent ss;
  
    ss.timestamp.setNow(0);

    // select all BLPS, no subarraying
    ss.rcumask.reset();
    for (int i = 0; i < m_nrcus; i++) ss.rcumask.set(i);

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
	 sel != sbsel().end(); ++sel)
    {
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
}

void BeamServer::defer(GCFEvent& e, GCFPortInterface& p)
{
  char* event = new char[sizeof(e) + e.length];
  memcpy(event, (const char*)&e, sizeof(e) + e.length);
  m_deferred_queue.push_back(pair<char*, GCFPortInterface*>(event, &p));
  LOG_DEBUG_STR(">>> deferring event " << m_deferred_queue.size() << " <<<");
}

GCFEvent::TResult BeamServer::recall(GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  if (m_deferred_queue.size() > 0) {
    LOG_DEBUG_STR(">>> recalling event " << m_deferred_queue.size() << " <<<");
    pair<char*, GCFPortInterface*> port_event = m_deferred_queue.front();
    m_deferred_queue.pop_front();
    status = dispatch(*(GCFEvent*)(port_event.first), *port_event.second);
    delete [] port_event.first;
  }
  
  return status;
}

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

  GCFTask::init(argc, argv);

  LOG_INFO(formatString("Program %s has started", argv[0]));

  try 
  {
    ConfigLocator cl;
    globalParameterSet()->adoptFile(cl.locate("RemoteStation.conf"));

    // set global bf_gain
    g_bf_gain = GET_CONFIG("BeamServer.BF_GAIN", i);
  }
  catch (Exception e)
  {
    cerr << "Failed to load configuration files: " << e.text() << endl;
    exit(EXIT_FAILURE);
  }

  try
  {
    BeamServer beamserver("BeamServer", argc, argv);

    beamserver.start(); // make initial transition

    GCFTask::run();
  }
  catch (Exception e)
  {
    cerr << "Exception: " << e.text() << endl;
    exit(EXIT_FAILURE);
  }

  LOG_INFO(formatString("Normal termination of program %s", argv[0]));

  return 0;
}
