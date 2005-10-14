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

#include "BS_Protocol.ph"
#include "RSP_Protocol.ph"
#include "CAL_Protocol.ph"

#include "BeamServer.h"

#include "Beam.h"
#include "Beamlet.h"

#include <iostream>
#include <sstream>
#include <time.h>
#include <string.h>

#include <netinet/in.h>

#include <PSAccess.h>

#include <blitz/array.h>

#include <AMCBase/AMCClient/ConverterClient.h>

using namespace LOFAR;
using namespace blitz;
using namespace BS;
using namespace std;
using namespace RTC;

using namespace RSP_Protocol;

#define COMPUTE_INTERVAL ((long)10)
#define UPDATE_INTERVAL  1
#define N_DIM 3 // x, y, z or l, m, n

#define SCALE (1<<(16-2))

#define SYSTEM_CLOCK_FREQ 120e6 // 120 MHz

BeamServer::BeamServer(string name)
    : GCFTask((State)&BeamServer::initial, name),
      m_beams_modified(false),
      m_sampling_frequency(160000000),
      m_nyquist_zone(1),
      m_beams(MEPHeader::N_BEAMLETS, MEPHeader::N_SUBBANDS, AMC::EarthCoord(1.0,1.0,0.0)),
      m_converter("localhost")
{
  registerProtocol(BS_PROTOCOL,  BS_PROTOCOL_signalnames);
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);
  registerProtocol(CAL_PROTOCOL, CAL_PROTOCOL_signalnames);

  m_acceptor.init(*this, "acceptor", GCFPortInterface::MSPP, BS_PROTOCOL);
  m_rspdriver.init(*this, "rspdriver", GCFPortInterface::SAP, RSP_PROTOCOL);
  m_calserver.init(*this, "calserver", GCFPortInterface::SAP, CAL_PROTOCOL);
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
  if (!(0 == bt.getBeam() && 0 == bt.getPort())) return 0; // previous alloc in progress
  if (0 == name.length())                        return 0; // name must be set
  if (0 == subarrayname.length())                return 0; // subarrayname must be set

  Beam* beam = m_beams.get(name, subarrayname, allocation);

  if (beam) {
    // register new beam
    m_client_beams[port].insert(beam);

    m_beams_modified = true;
    bt.set(port, beam);
  }

  return beam;
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
      }
      break;

    case F_ACCEPT_REQ:
      {
	GCFTCPPort* client = new GCFTCPPort();
	client->init(*this, "client", GCFPortInterface::SPP, BS_PROTOCOL);
	m_acceptor.accept(*client);
	m_client_list.push_back(client);

	LOG_INFO(formatString("NEW CLIENT CONNECTED: %d clients connected", m_client_list.size()));
      }
      break;
      
    case F_CONNECTED:
      {
	LOG_INFO(formatString("CONNECTED: port '%s'", port.getName().c_str()));
      }
      break;

    case F_TIMER:
      {
	GCFTimerEvent* timer = static_cast<GCFTimerEvent*>(&e);

	LOG_DEBUG_STR("timer=" << Timestamp(timer->sec, timer->usec));

	period++;
	if (0 == (period % COMPUTE_INTERVAL))
	  {
	    period = 0;

	    // compute new weights after sending weights
	    compute_weights(Timestamp(timer->sec, 0) + COMPUTE_INTERVAL);

	    send_weights(Timestamp(timer->sec,0) + COMPUTE_INTERVAL);
	  }

	if (m_beams_modified)
	  {
	    send_sbselection();

	    m_beams_modified = false;
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
	  LOG_INFO("\n\nReceived valid CAL_UPDATE event.\n\n");

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
	m_calserver.send(subscribe);
      }
      break;

    case CAL_SUBSCRIBEACK:
      {
	CALSubscribeackEvent ack(e);
	BSBeamallocackEvent beamallocack;

	if (CAL_Protocol::SUCCESS == ack.status) {

	  LOG_WARN_STR("ack.subarray.positions=" << ack.subarray.getAntennaPos());

	  // set positions on beam
	  m_bt.getBeam()->setSubarray(ack.subarray);

	  // set calibration handle for this beam
	  m_beams.setCalibrationHandle(m_bt.getBeam(), ack.handle);

	  // send succesful ack
	  beamallocack.status = BS_Protocol::SUCCESS;
	  beamallocack.handle = (uint32)m_bt.getBeam();
	  m_bt.getPort()->send(beamallocack);

	} else {

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
	LOG_INFO("\n>>> deferring F_DISCONNECTED event <<<\n");
	defer(e, port); // process F_DISCONNECTED again in enabled state

	if (&port == m_bt.getPort()) TRAN(BeamServer::enabled);
      }
      break;

    case F_EXIT:
      {
	// completed current transaction, reset
	m_bt.reset();

	// recall any deferred events
	recall(port);
      }
      break;

    default:
      // all other events are handled in the enabled state
      LOG_INFO("\n>>> deferring event <<<\n");
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
	LOG_INFO("\n>>> deferring F_DISCONNECTED event <<<\n");
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

	// recall any deferred events
	recall(port);
      }
      break;

    default:
      // all other events are handled in the enabled state
      LOG_INFO("\n>>> deferring event <<<\n");
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

    LOG_ERROR("BEAMALLOC: failed to allocate beam");

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
    LOG_INFO_STR("received new coordinates: " << pt.pointing.angle0() << ", "
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
  return complex<int16_t>((int16_t)(round(cd.real()*SCALE)),
			  (int16_t)(round(cd.imag()*SCALE)));
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
  if (!GET_CONFIG("BeamServer.DISABLE_SETWEIGHTS", i)) {
    RSPSetweightsEvent sw;

    sw.timestamp = time;
    LOG_DEBUG_STR("sw.time=" << sw.timestamp);

    // select all BLPS, no subarraying
    sw.rcumask.reset();
    for (int i = 0; i < m_nrcus; i++) sw.rcumask.set(i);

    sw.weights().resize(COMPUTE_INTERVAL, m_nrcus, MEPHeader::N_BEAMLETS);
    sw.weights() = m_weights16;

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
  m_deferred_queue[&p].push_back(event);
}

GCFEvent::TResult BeamServer::recall(GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  if (m_deferred_queue[&p].size() > 0) {
    char* event = m_deferred_queue[&p].front();
    m_deferred_queue[&p].pop_front();
    status = dispatch(*(GCFEvent*)event, p);
    delete [] event;
  }
  
  return status;
}

int main(int argc, char** argv)
{
  GCFTask::init(argc, argv);

  LOG_INFO(formatString("Program %s has started", argv[0]));

  try 
  {
    GCF::ParameterSet::instance()->adoptFile("BeamServerPorts.conf");
    GCF::ParameterSet::instance()->adoptFile("RemoteStation.conf");
  }
  catch (Exception e)
  {
    cerr << "Failed to load configuration files: " << e.text() << endl;
    exit(EXIT_FAILURE);
  }

  try
  {
    BeamServer beamserver("BeamServer");

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
