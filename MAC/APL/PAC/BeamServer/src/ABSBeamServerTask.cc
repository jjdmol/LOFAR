//#
//#  ABSBeamServerTask.cc: implementation of ABSBeamServerTask class
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

// this include needs to be first!
#define DECLARE_SIGNAL_NAMES
#include "ABS_Protocol.ph"
#include "RSP_Protocol.ph"

#include "ABSBeamServerTask.h"

#include "ABSBeam.h"
#include "ABSBeamlet.h"

#include <iostream>
#include <sstream>
#include <time.h>
#include <string.h>

#include <netinet/in.h>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <PSAccess.h>

#ifndef ABS_SYSCONF
#define ABS_SYSCONF "."
#endif

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <blitz/array.h>

using namespace LOFAR;
using namespace blitz;
using namespace ABS;
using namespace std;
using namespace boost::posix_time;
using namespace boost::gregorian;

using namespace RSP_Protocol;

#define MAX_N_SPECTRAL_WINDOWS 1
#define COMPUTE_INTERVAL 10
#define UPDATE_INTERVAL  1
#define N_DIM 3 // x, y, z or l, m, n

#define SCALE (1<<(16-2))

#define SYSTEM_CLOCK_FREQ 80e6 // 80 MHz

BeamServerTask::BeamServerTask(string name)
    : GCFTask((State)&BeamServerTask::initial, name),
      m_pos(GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i), N_POL, N_DIM),
      m_weights(COMPUTE_INTERVAL, GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i), N_BEAMLETS, N_POL),
      m_weights16(COMPUTE_INTERVAL, GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i), N_BEAMLETS, N_POL),
      m_subbands_modified(false)
{
  registerProtocol(ABS_PROTOCOL, ABS_PROTOCOL_signalnames);
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);

  m_acceptor.init(*this, "acceptor", GCFPortInterface::MSPP, ABS_PROTOCOL);
  m_rspdriver.init(*this, "rspdriver", GCFPortInterface::SAP, RSP_PROTOCOL);

  (void)Beam::init(N_BEAMLETS, UPDATE_INTERVAL, COMPUTE_INTERVAL);
  (void)Beamlet::init(N_BEAMLETS);

  m_wgsetting.frequency     = 1.5625e6; // 1.5625 MHz
  m_wgsetting.amplitude     = 0x8000 / 256;
  m_wgsetting.enabled       = false;

  m_pos = 0;
#if 0
  // initialize antenna positions
  Range all = Range::all();
  m_pos(all, 0, all) = 0.0;
  m_pos(all, 0, 0)   = 0.0;

  m_pos(all, 1, all) = 0.0;
  m_pos(all, 1, 0)   = 95.9358656;

  LOG_DEBUG_STR(m_pos);
#else
  istringstream config_positions(GET_CONFIG_STRING("RS.ANTENNA_POSITIONS"));
  config_positions >> m_pos;

  LOG_INFO_STR("ANTENNA_POSITIONS = " << m_pos);
#endif

  // initialize weight matrix
  m_weights   = complex<W_TYPE>(0,0);
  m_weights16 = complex<int16_t>(0,0);
}

BeamServerTask::~BeamServerTask()
{}

bool BeamServerTask::isEnabled()
{
  return m_rspdriver.isConnected();
}

GCFEvent::TResult BeamServerTask::initial(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static unsigned long update_timer = (unsigned long)-1;
  
  switch(e.signal)
  {
    case F_INIT:
    {
      // create a default spectral window from 0MHz to 20MHz
      // steps of 156.25 kHz
      SpectralWindow* spw = new SpectralWindow(0.0, 20.0e6/N_SUBBANDS, N_SUBBANDS);
      m_spws[0] = spw;
    }
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
	TRAN(BeamServerTask::enabled);
      }
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

void BeamServerTask::collect_garbage()
{
  for (list<GCFPortInterface*>::iterator it = m_garbage_list.begin();
       it != m_garbage_list.end();
       it++)
  {
    delete (*it);
  }
  m_garbage_list.clear();
}

GCFEvent::TResult BeamServerTask::enabled(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  static int period = 0;

  collect_garbage();
  
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
      client->init(*this, "client", GCFPortInterface::SPP, ABS_PROTOCOL);
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
	compute_weights(timer->sec);

	send_weights();
      }

      if (m_subbands_modified) send_sbselection();
    }
    break;

    case ABS_BEAMPOINTTO:
    case ABS_WGSETTINGS:
    case ABS_BEAMALLOC:
    case ABS_BEAMFREE:
    case ABS_WGENABLE:
    case ABS_WGDISABLE:
      status = handle_abs_request(e, port);
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
      
    case RSP_SETWGACK:
    {
      RSPSetwgackEvent ack(e);
      if (RSP_Protocol::SUCCESS != ack.status)
      {
	LOG_ERROR("RSP_SETWGACK: FAILURE");
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
	TRAN(BeamServerTask::initial);
      }
      else
      {
	// deallocate all beams for this client
	for (set<int>::iterator handle = m_client_beams[&port].begin();
	     handle != m_client_beams[&port].end();
	     ++handle)
	{
	  Beam* b = Beam::getFromHandle(*handle);
	  b->deallocate();
	  m_beams.erase(b);
	}
	m_client_beams.erase(&port);

	// update the subband selection
	update_sbselection();

	m_client_list.remove(&port);
	m_garbage_list.push_back(&port);
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

GCFEvent::TResult BeamServerTask::handle_abs_request(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case ABS_BEAMALLOC:
    {
      ABSBeamallocEvent event(e);
      beamalloc_action(event, port);
    }
    break;

    case ABS_BEAMFREE:
    {
      ABSBeamfreeEvent event(e);
      beamfree_action(event, port);
    }
    break;

    case ABS_BEAMPOINTTO:
    {
      ABSBeampointtoEvent event(e);
      beampointto_action(event, port);
    }
    break;

    case ABS_WGSETTINGS:
    {
      ABSWgsettingsEvent event(e);
      wgsettings_action(event, port);
    }
    break;

    case ABS_WGENABLE:
    {
      wgenable_action();
    }
    break;

    case ABS_WGDISABLE:
    {
      wgdisable_action();
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;  
}

void BeamServerTask::beamalloc_action(ABSBeamallocEvent& ba,
				      GCFPortInterface& port)
{
  int   spwindex = 0;
  Beam* beam = 0;
  ABSBeamallocAckEvent ack;
  ack.handle = -1;
  ack.status = ABS_Protocol::SUCCESS;

  // check parameters
  if (((spwindex = ba.spectral_window) < 0)
      || (spwindex >= MAX_N_SPECTRAL_WINDOWS))
  {
      LOG_ERROR("BEAMALLOC: argument range error");
      ack.status = ERR_RANGE;
      port.send(ack);
      return;                         // RETURN
  }

  if (ba.n_subbands< 0 || ba.n_subbands > N_BEAMLETS)
  {
    LOG_ERROR("BEAMALLOC: n_subbands parameter out of range");

    ack.status = ERR_RANGE;
    port.send(ack);
    return;                          // RETURN
  }

  // create subband selection set
  set<int> subbands;
  for (int i = 0; i < ba.n_subbands; i++) subbands.insert(ba.subbands[i]);

  // check if array of subbands did not contain any duplicate subband
  // selection
  if (ba.n_subbands != (int)subbands.size())
  {
    LOG_ERROR("BEAMALLOC: subband selection contains duplicates");
    ack.status = ERR_RANGE;
    port.send(ack);
    return;                          // RETURN
  }

  // allocate the beam

  if (0 == (beam = Beam::allocate(*m_spws[spwindex], subbands)))
  {
      LOG_ERROR("BEAMALLOC: failed");
      ack.status = ERR_BEAMALLOC;
      port.send(ack);
  }
  else
  {
      ack.handle = beam->handle();
      LOG_DEBUG(formatString("ack.handle=%d", ack.handle));

      m_beams.insert(beam);
      m_client_beams[&port].insert(beam->handle());

      update_sbselection();
      port.send(ack);
  }
}

void BeamServerTask::beamfree_action(ABSBeamfreeEvent& bf,
				     GCFPortInterface& port)
{
  ABSBeamfreeAckEvent ack;
  ack.handle = bf.handle;
  ack.status = ABS_Protocol::SUCCESS;

  Beam* beam = 0;
  if (!(beam = Beam::getFromHandle(bf.handle)))
  {
    LOG_ERROR("BEAMFREE: unknown beam handle");
    
    ack.status = ERR_RANGE;
    port.send(ack);
    return;                      // RETURN
  }

  if (beam->deallocate() < 0)
  {
    LOG_ERROR("BEAMFREE: deallocate failed");
    ack.status = ERR_BEAMFREE;
    port.send(ack);
    return;                     // RETURN
  }

  m_beams.erase(beam);
  m_client_beams[&port].erase(beam->handle());

  update_sbselection();

  port.send(ack);
}

void BeamServerTask::beampointto_action(ABSBeampointtoEvent& pt,
					GCFPortInterface& /*port*/)
{
  Beam* beam = Beam::getFromHandle(pt.handle);

  if (beam)
  {
      time_t pointto_time = pt.time;

      LOG_INFO(formatString("received new coordinates: %f, %f, time=%s",
			    pt.angle[0], pt.angle[1], to_simple_string(from_time_t(pt.time)).c_str()));

      //
      // If the time is not set, then activate the command
      // 2 * COMPUTE_INTERVAL seconds from now, because that's how
      // long it takes the command to flow through the pipeline.
      //
      if (0 == pt.time) pointto_time = time(0) + 2 * COMPUTE_INTERVAL;
      if (beam->addPointing(Pointing(Direction(pt.angle[0],
					       pt.angle[1],
					       (Direction::Types)pt.type),
				     from_time_t(pointto_time))) < 0)
      {
	  LOG_ERROR("BEAMPOINTTO: failed");
      }
  }
  else LOG_ERROR(formatString("BEAMPOINTTO: invalid beam handle (%d)", pt.handle));
}

void BeamServerTask::wgsettings_action(ABSWgsettingsEvent& wgs,
				       GCFPortInterface& port)
{
  ABSWgsettingsAckEvent sa;
  sa.status = ABS_Protocol::SUCCESS;

  // max allowed frequency = 20MHz
  if ((wgs.frequency >= 1e-6)
      && (wgs.frequency <= SYSTEM_CLOCK_FREQ/4.0))
  {
    m_wgsetting.frequency     = wgs.frequency;
    m_wgsetting.amplitude     = wgs.amplitude;
  }
  else
  {
    LOG_ERROR("WGSETTINGS: argument range error");
    sa.status = ABS_Protocol::ERR_RANGE;
  }

  // send ack
  port.send(sa);
}

void BeamServerTask::wgenable_action()
{
  RSPSetwgEvent wg;
  
  wg.timestamp.setNow();
  wg.blpmask.reset();
  for (int i = 0; i < GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i); i++) wg.blpmask.set(i);
  wg.settings().resize(1);
  // scale and convert to uint16
  wg.settings()(0).freq = (uint16)(((m_wgsetting.frequency * (1 << 16)) / SYSTEM_CLOCK_FREQ) + 0.5);
  wg.settings()(0).ampl = m_wgsetting.amplitude;
  wg.settings()(0).nof_usersamples = 0;
  wg.settings()(0).mode = WGSettings::MODE_SINE;
  wg.settings()(0)._pad = 0; /* stop valgrind complaining */

  m_rspdriver.send(wg);
}

void BeamServerTask::wgdisable_action()
{
  RSPSetwgEvent wg;
  
  wg.timestamp.setNow();
  wg.blpmask.reset();
  for (int i = 0; i < GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i); i++) wg.blpmask.set(i);
  wg.settings().resize(1);
  wg.settings()(0).freq = 0;
  wg.settings()(0).ampl = 0;
  wg.settings()(0).nof_usersamples = 0;
  wg.settings()(0).mode = WGSettings::MODE_OFF;
  wg.settings()(0)._pad = 0; /* stop valgrind complaining */

  m_rspdriver.send(wg);
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
void BeamServerTask::compute_weights(long current_seconds)
{
  // convert_pointings for all beams for the next deadline
  time_period compute_period = time_period(from_time_t((time_t)current_seconds)
					   + time_duration(seconds(COMPUTE_INTERVAL)),
					   seconds(COMPUTE_INTERVAL));

  Array<W_TYPE,2> lmns(COMPUTE_INTERVAL, N_DIM);  // l,m,n coordinates
  lmns = 0;

  // iterate over all beams
  for (set<Beam*>::iterator bi = m_beams.begin();
       bi != m_beams.end(); ++bi)
  {
    (*bi)->convertPointings(compute_period);

    lmns = (*bi)->getLMNCoordinates();
    LOG_INFO(formatString("current_pointing=(%f,%f)",
			  (*bi)->pointing().direction().angle1(),
			  (*bi)->pointing().direction().angle2()));
  }

  //cout << "lmns = " << lmns << endl;
  
  Beamlet::calculate_weights(m_pos, m_weights);

  //cout << "m_weights(t=0,element=0,beamlet=0,pol=all) = " << m_weights(0,0,0,Range::all()) << endl;
  //cout << "m_weights(t=0,element=0,beamlet=N_BEAMLETS - 1,pol=all) = " << m_weights(0,0,N_BEAMLETS - 1,Range::all()) << endl;

  // show weights for timestep 0, element 0, all subbands, both polarizations
  //Range all = Range::all();
  //cout << "m_weights=" << m_weights(0, 0, all, all) << endl;

  //
  // need complex conjugate of the weights
  // as 16bit signed integer to send to the board
  //
#if 1
  m_weights16 = convert2complex_int16_t(conj(m_weights));
#else
  for (int i = 0; i < COMPUTE_INTERVAL; i++)
    for (int j = 0; j < GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i); j++)
      for (int k = 0; k < N_BEAMLETS; k++)
	for (int l = 0; l < N_POL; l++)
	  {
	      //
	      // -1 * imaginary part to take complex conjugate of the weight
	      //
	      m_weights16(i,j,k,l) = complex<int16_t>(     (int16_t)round(m_weights(i,j,k,l).real()*SCALE),
						      -1 * (int16_t)round(m_weights(i,j,k,l).imag()*SCALE));
	  }
#endif

  //LOG_DEBUG(formatString("m_weights16 contiguous storage? %s", (m_weights16.isStorageContiguous()?"yes":"no")));
  LOG_DEBUG(formatString("sizeof(m_weights16) = %d", m_weights16.size()*sizeof(int16_t)));
}

void BeamServerTask::send_weights()
{
  RSPSetweightsEvent sw;

  sw.timestamp.setNow(COMPUTE_INTERVAL); // activate after COMPUTE_INTERVAL seconds
  LOG_DEBUG_STR("sw.time=" << sw.timestamp);

  // select all BLPS, no subarraying
  sw.blpmask.reset();
  for (int i = 0; i < GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i); i++) sw.blpmask.set(i);

  sw.weights().resize(COMPUTE_INTERVAL, GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i), N_BEAMLETS, N_POL);
  sw.weights() = m_weights16;

  m_rspdriver.send(sw);
}

void BeamServerTask::update_sbselection()
{
  // update subband selection to take
  // the new beamlets for this beam into account
  m_sbsel.clear();
  for (set<Beam*>::iterator bi = m_beams.begin();
       bi != m_beams.end(); ++bi)
  {
      (*bi)->getSubbandSelection(m_sbsel);
  }

  m_subbands_modified = true;
}

void BeamServerTask::send_sbselection()
{
  RSPSetsubbandsEvent ss;
  
  ss.timestamp.setNow(0);

  // select all BLPS, no subarraying
  ss.blpmask.reset();
  for (int i = 0; i < GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i); i++) ss.blpmask.set(i);

  //
  // Always allocate the array as if all beamlets were
  // used. Because of allocation and deallocation of beams
  // there can be holes in the subband selection.
  //
  // E.g. Beamlets 0-63 are used by beam 0, beamlets 64-127 by
  // beam 1, then beam 0 is deallocated, thus there is a hole
  // of 64 beamlets before the beamlets of beam 1.
  //
  ss.subbands().resize(1, N_BEAMLETS * 2);
  ss.subbands() = 0;

  int nrsubbands = m_sbsel.size() <= 0 ? 0 : m_sbsel.size() * 2;
  LOG_DEBUG(formatString("nrsubbands=%d", nrsubbands));

  int i = 0;
  for (map<int,int>::iterator sel = m_sbsel.begin();
       sel != m_sbsel.end(); ++sel, ++i)
  {
    LOG_DEBUG(formatString("(%d,%d)", sel->first, sel->second));

    if (sel->first >= N_BEAMLETS)
    {
      LOG_ERROR(formatString("SBSELECTION: invalid src index", sel->first));
      continue;
    }
      
    if (sel->second >= N_SUBBANDS)
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

  m_subbands_modified = false;
}

#if 0
void BeamServerTask::saveq_defer(GCFEvent& e)
{
  char* copy = new char [sizeof(GCFEvent) + e.length];
  memcpy(copy, &e, sizeof(GCFEvent) + e.length);
  m_saveq.push_back(copy);
}

GCFEvent* BeamServerTask::saveq_recall()
{
  if (!m_saveq.empty()) return (GCFEvent*)m_saveq.front();
  else                  return 0;
}

void BeamServerTask::saveq_pop()
{
  if (!m_saveq.empty())
  {
    char* e = m_saveq.front();
    m_saveq.pop_front();
    delete [] e;
  }
}

void BeamServerTask::saveq_clear()
{
  while (!m_saveq.empty()) saveq_pop();
}
#endif

int main(int argc, char** argv)
{
#if 0
  char prop_path[PATH_MAX];
  const char* mac_config = getenv("MAC_CONFIG");

  snprintf(prop_path, PATH_MAX-1,
	   "%s/%s", (mac_config?mac_config:"."),
	   "log4cplus.properties");
  INIT_LOGGER(prop_path);
#endif

  LOG_INFO(formatString("Program %s has started", argv[0]));

  try 
  {
    GCF::ParameterSet::instance()->adoptFile("BeamServerPorts.conf");
    GCF::ParameterSet::instance()->adoptFile("RemoteStation.conf");
    GCF::ParameterSet::instance()->adoptFile("BeamServer.conf");
  }
  catch (Exception e)
  {
    cerr << "Failed to load configuration files: " << e.text() << endl;
    exit(EXIT_FAILURE);
  }
  
  GCFTask::init(argc, argv);

  BeamServerTask abs("BeamServer");

  abs.start(); // make initial transition

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
