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

#include "BS_Protocol.ph"
#include "RSP_Protocol.ph"

#include "BeamServer.h"

#include "Beam.h"
#include "Beamlet.h"

#include <iostream>
#include <sstream>
#include <time.h>
#include <string.h>

#include <netinet/in.h>

#include <PSAccess.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <blitz/array.h>

using namespace LOFAR;
using namespace blitz;
using namespace BS;
using namespace std;
using namespace RTC;

using namespace RSP_Protocol;

#define MAX_N_SPECTRAL_WINDOWS 1
#define COMPUTE_INTERVAL 10
#define UPDATE_INTERVAL  1
#define N_DIM 3 // x, y, z or l, m, n

#define SCALE (1<<(16-2))

#define SYSTEM_CLOCK_FREQ 120e6 // 120 MHz

BeamServer::BeamServer(string name)
    : GCFTask((State)&BeamServer::initial, name),
      m_beams_modified(false),
      m_sampling_frequency(160000000),
      m_nyquist_zone(1),
      m_beams(MEPHeader::N_BEAMLETS)
{
  registerProtocol(BS_PROTOCOL,  BS_PROTOCOL_signalnames);
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);

  m_acceptor.init(*this, "acceptor", GCFPortInterface::MSPP, BS_PROTOCOL);
  m_rspdriver.init(*this, "rspdriver", GCFPortInterface::SAP, RSP_PROTOCOL);

}

BeamServer::~BeamServer()
{}

bool BeamServer::isEnabled()
{
  return m_rspdriver.isConnected();
}

GCFEvent::TResult BeamServer::initial(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static unsigned long update_timer = (unsigned long)-1;
  
  switch(e.signal)
    {
    case F_INIT:
      break;

    case F_ENTRY:
      {
	if (!m_rspdriver.isConnected()) m_rspdriver.open();

	// start the update timer if it wasn't already started
	if ((unsigned long)-1 == update_timer) update_timer = m_rspdriver.setTimer(0, 0, UPDATE_INTERVAL, 0);
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
	m_pos.resize(m_nrcus / MEPHeader::N_POL, MEPHeader::N_POL, N_DIM);
	m_weights.resize(COMPUTE_INTERVAL, m_nrcus, MEPHeader::N_BEAMLETS);
	m_weights16.resize(COMPUTE_INTERVAL, m_nrcus, MEPHeader::N_BEAMLETS);

	m_pos = 0;

	LOG_INFO_STR("RS.LBA_POSITIONS = " << m_pos);

	// initialize weight matrix
	m_weights   = complex<W_TYPE>(0,0);
	m_weights16 = complex<int16_t>(0,0);

	TRAN(BeamServer::enabled);
      }
      break;

    case F_DISCONNECTED:
      {
	// no need to set this timer, the update timer will cause re-open anyway
	//port.setTimer((long)3); // try again in 3 seconds
	LOG_DEBUG(formatString("port '%s' disconnected, retry in 3 seconds...", port.getName().c_str()));
	port.close();
      }
      break;

    case F_TIMER:
      {
	if (!port.isConnected())
	  {
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
    delete (*it);
  }
  m_dead_clients.clear();
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
      if (!m_acceptor.isConnected()) m_acceptor.open();
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

      LOG_INFO(formatString("timer=(%d,%d)", timer->sec, timer->usec));

      period++;
      if (0 == (period % COMPUTE_INTERVAL))
      {
	period = 0;

	// compute new weights after sending weights
	compute_weights(Timestamp(timer->sec, 0));

	send_weights();
      }

      if (m_beams_modified)
      {
	send_sbselection();

	m_beams_modified = false;
      }
    }
    break;

    case BS_BEAMPOINTTO:
    case BS_BEAMALLOC:
    case BS_BEAMFREE:
      status = handle_request(e, port);
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

    case F_DISCONNECTED:
    {
      LOG_INFO(formatString("DISCONNECTED: port %s disconnected", port.getName().c_str()));
      port.close();

      if (&m_rspdriver == &port || &m_acceptor == &port)
      {
	m_acceptor.close();
	TRAN(BeamServer::initial);
      }
      else
      {
	// deallocate all beams for this client
	for (set<uint32>::iterator handle = m_client_beams[&port].begin();
	     handle != m_client_beams[&port].end();
	     ++handle)
	{
	  if (!m_beams.destroy((uint32)*handle)) {
	    LOG_WARN("Beam not found...");
	  }
	}
	m_client_beams.erase(&port);

	// update the subband selection
	update_sbselection();

	m_client_list.remove(&port);
	m_dead_clients.push_back(&port);
      }
    }
    break;

    case F_EXIT:
    {
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult BeamServer::handle_request(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case BS_BEAMALLOC:
    {
      BSBeamallocEvent event(e);
      beamalloc_action(event, port);
    }
    break;

    case BS_BEAMFREE:
    {
      BSBeamfreeEvent event(e);
      beamfree_action(event, port);
    }
    break;

    case BS_BEAMPOINTTO:
    {
      BSBeampointtoEvent event(e);
      beampointto_action(event, port);
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;  
}

void BeamServer::beamalloc_action(BSBeamallocEvent& ba,
				  GCFPortInterface& port)
{
  BSBeamallocackEvent ack;
  ack.handle = 0;
  ack.status = BS_Protocol::SUCCESS;

  if (ba.allocation().size() > MEPHeader::N_BEAMLETS)
  {
    LOG_ERROR("BEAMALLOC: allocation larger than N_BEAMLETS");
    
    ack.status = ERR_RANGE;
    port.send(ack);
    return;                          // RETURN
  }

  // allocate the beam
  Beam* beam = m_beams.get(ba.allocation, m_sampling_frequency, m_nyquist_zone);

  if (!beam) {

    LOG_ERROR("BEAMALLOC failed.");
    ack.status = ERR_BEAMALLOC;

  } else {

    ack.handle = (uint32)beam;
    m_client_beams[&port].insert((uint32)beam);
    update_sbselection();
  }

  port.send(ack);
}

void BeamServer::beamfree_action(BSBeamfreeEvent& bf,
				 GCFPortInterface& port)
{
  BSBeamfreeackEvent ack;
  ack.handle = bf.handle;
  ack.status = BS_Protocol::SUCCESS;

  if (!m_beams.destroy(bf.handle)) {

    LOG_ERROR("BEAMFREE failed");

    ack.status = ERR_BEAMFREE;

  } else {

    //m_beams.erase(beam);
    m_client_beams[&port].erase(bf.handle);
    update_sbselection();

  }

  port.send(ack);
}

void BeamServer::beampointto_action(BSBeampointtoEvent& pt,
				    GCFPortInterface& /*port*/)
{
  Beam* beam = m_beams.get(pt.handle);

  if (beam)
    {
      LOG_INFO_STR("received new coordinates: " << pt.angle[0] << ", "
		   << pt.angle[1] << ", time=" << pt.timestamp);

      //
      // If the time is not set, then activate the command
      // 2 * COMPUTE_INTERVAL seconds from now, because that's how
      // long it takes the command to flow through the pipeline.
      //
      if (Timestamp(0,0) == pt.timestamp) pt.timestamp.setNow(2 * COMPUTE_INTERVAL);
      beam->addPointing(Pointing(pt.angle[0],
				 pt.angle[1],
				 pt.timestamp,
				 (Pointing::Type)pt.type));
    }
  else LOG_ERROR(formatString("BEAMPOINTTO: invalid beam handle (%d)", pt.handle));
}

BZ_DECLARE_FUNCTION_RET(convert2complex_int16_t, complex<int16_t>)

/**
 * Convert the weights to 16-bits signed integer.
 */
inline complex<int16_t> convert2complex_int16_t(complex<W_TYPE> cd)
{
#ifdef W_TYPE_DOUBLE
  return complex<int16_t>((int16_t)(round(cd.real()*SCALE)),
			  (int16_t)(round(cd.imag()*SCALE)));
#else
  return complex<int16_t>((int16_t)(roundf(cd.real()*SCALE)),
			  (int16_t)(roundf(cd.imag()*SCALE)));
#endif
}

/**
 * This method is called once every second
 * to calculate the weights for all beamlets.
 */
void BeamServer::compute_weights(Timestamp time)
{
  // calculate weights for all beamlets
  m_beams.calculate_weights(time, COMPUTE_INTERVAL, m_pos, m_weights);

  //
  // need complex conjugate of the weights
  // as 16bit signed integer to send to the board
  //
  m_weights16 = convert2complex_int16_t(conj(m_weights));

  //LOG_DEBUG(formatString("m_weights16 contiguous storage? %s", (m_weights16.isStorageContiguous()?"yes":"no")));
  LOG_DEBUG(formatString("sizeof(m_weights16) = %d", m_weights16.size()*sizeof(int16_t)));
}

void BeamServer::send_weights()
{
  if (!GET_CONFIG("BeamServer.DISABLE_SETWEIGHTS", i)) {
    RSPSetweightsEvent sw;

    sw.timestamp.setNow(COMPUTE_INTERVAL); // activate after COMPUTE_INTERVAL seconds
    LOG_DEBUG_STR("sw.time=" << sw.timestamp);

    // select all BLPS, no subarraying
    sw.rcumask.reset();
    for (int i = 0; i < m_nrcus; i++) sw.rcumask.set(i);

    sw.weights().resize(COMPUTE_INTERVAL, m_nrcus, MEPHeader::N_BEAMLETS);
    sw.weights() = m_weights16;

    m_rspdriver.send(sw);
  }
}

void BeamServer::update_sbselection()
{
  // update subband selection to take
  // the new beamlets for this beam into account
  m_sbsel = m_beams.getSubbandSelection();

  m_beams_modified = true;
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
    ss.subbands().resize(1, MEPHeader::N_BEAMLETS * 2);
    ss.subbands() = 0;

    int nrsubbands = m_sbsel().size() * 2;
    LOG_DEBUG(formatString("nrsubbands=%d", nrsubbands));

    int i = 0;
    for (map<uint16,uint16>::iterator sel = m_sbsel().begin();
	 sel != m_sbsel().end(); ++sel, ++i)
      {
	LOG_DEBUG(formatString("(%d,%d)", sel->first, sel->second));

	if (sel->first >= MEPHeader::N_BEAMLETS)
	  {
	    LOG_ERROR(formatString("SBSELECTION: invalid src index", sel->first));
	    continue;
	  }
      
	if (sel->second >= MEPHeader::N_SUBBANDS)
	  {
	    LOG_ERROR(formatString("SBSELECTION: invalid tgt index", sel->second));
	    continue;
	  }

	// same selection for x and y polarization
	ss.subbands()(0, sel->first*2)   = sel->second * 2;
	ss.subbands()(0, sel->first*2+1) = sel->second * 2 + 1;
      }

    //cout << "ss.subbands() = " << ss.subbands() << endl;

    m_rspdriver.send(ss);
  }
}

int main(int argc, char** argv)
{
  GCFTask::init(argc, argv);

  LOG_INFO(formatString("Program %s has started", argv[0]));

  try 
  {
    GCF::ParameterSet::instance()->adoptFile(BS_SYSCONF "/BeamServerPorts.conf");
    GCF::ParameterSet::instance()->adoptFile(BS_SYSCONF "/RemoteStation.conf");
  }
  catch (Exception e)
  {
    cerr << "Failed to load configuration files: " << e.text() << endl;
    exit(EXIT_FAILURE);
  }

  BeamServer beamserver("BeamServer");

  beamserver.start(); // make initial transition

  try
  {
    GCFTask::run();
  }
  catch (Exception e)
  {
    cerr << "Exception: " << e.text() << endl;
    exit(EXIT_FAILURE);
  }

  LOG_INFO("Normal termination of program");

  return 0;
}
