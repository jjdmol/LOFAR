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

#define SYSTEM_CLOCK_FREQ 120e6 // 120 MHz

BeamServerTask::BeamServerTask(string name, int n_blps)
    : GCFTask((State)&BeamServerTask::initial, name),
      m_pos(n_blps, MEPHeader::N_POL, N_DIM),
      m_weights(COMPUTE_INTERVAL, n_blps, MEPHeader::N_BEAMLETS, MEPHeader::N_POL),
      m_weights16(COMPUTE_INTERVAL, n_blps, MEPHeader::N_BEAMLETS, MEPHeader::N_POL),
      m_beams_modified(false),
      m_n_blps(n_blps)
{
  registerProtocol(ABS_PROTOCOL, ABS_PROTOCOL_signalnames);
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);

  m_acceptor.init(*this, "acceptor", GCFPortInterface::MSPP, ABS_PROTOCOL);
  m_rspdriver.init(*this, "rspdriver", GCFPortInterface::SAP, RSP_PROTOCOL);

  (void)Beam::init(MEPHeader::N_BEAMLETS, UPDATE_INTERVAL, COMPUTE_INTERVAL);
  (void)Beamlet::init(MEPHeader::N_BEAMLETS);

  m_wgsetting.frequency     = 1.5625e6; // 1.5625 MHz
  m_wgsetting.amplitude     = 128;
  m_wgsetting.enabled       = false;

  m_pos = 0;
  m_spw_refcount = 0;

  istringstream config_positions(GET_CONFIG_STRING("RS.ANTENNA_POSITIONS"));
  config_positions >> m_pos;

  LOG_INFO_STR("ANTENNA_POSITIONS = " << m_pos);

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
      if (!SpectralWindowConfig::getInstance().load())
      {
	LOG_ERROR("Failed to load spectral window configurations.");
      }
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

      if (m_beams_modified)
      {
	send_sbselection();
	send_rcusettings();

	m_beams_modified = false;
      }
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

    case RSP_SETRCUACK:
    {
      RSPSetrcuackEvent ack(e);
      if (RSP_Protocol::SUCCESS != ack.status)
      {
	LOG_ERROR("RSP_SETRCUACK: FAILURE");
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
  Beam* beam = 0;
  ABSBeamallocAckEvent ack;
  ack.handle = -1;
  ack.status = ABS_Protocol::SUCCESS;

  if (ba.n_subbands< 0 || ba.n_subbands > MEPHeader::N_BEAMLETS)
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

  if (0 == (beam = Beam::allocate(ba.spectral_window, subbands)))
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
#if 0
  if ((wgs.frequency >= 1e-6)
      && (wgs.frequency <= SYSTEM_CLOCK_FREQ/4.0))
#else
  if (1)
#endif
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
  if (!GET_CONFIG("BeamServer.DISABLE_WG", i))
  {
    RSPSetwgEvent wg;

    wg.timestamp.setNow();
    wg.rcumask.reset();
    for (int i = 0; i < m_n_blps * MEPHeader::N_POL; i++) wg.rcumask.set(i);
    wg.settings().resize(1);
    // scale and convert to uint16
    wg.settings()(0).freq = (uint16)(((m_wgsetting.frequency * (1 << 16)) / SYSTEM_CLOCK_FREQ) + 0.5);
    wg.settings()(0).ampl = m_wgsetting.amplitude;
    wg.settings()(0).phase = 0;
    wg.settings()(0).nof_samples = N_WAVE_SAMPLES;
    wg.settings()(0).mode = WGSettings::MODE_CALC;
    wg.settings()(0).preset = WGSettings::PRESET_SINE;

    m_rspdriver.send(wg);
  }
}

void BeamServerTask::wgdisable_action()
{
  RSPSetwgEvent wg;
  
  wg.timestamp.setNow();
  wg.rcumask.reset();
  for (int i = 0; i < m_n_blps * MEPHeader::N_POL; i++) wg.rcumask.set(i);
  wg.settings().resize(1);
  wg.settings()(0).freq = 0;
  wg.settings()(0).ampl = 0;
  wg.settings()(0).phase = 0;
  wg.settings()(0).nof_samples = N_WAVE_SAMPLES;
  wg.settings()(0).mode = WGSettings::MODE_OFF;
  wg.settings()(0).preset = WGSettings::PRESET_SINE;

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
  m_weights16 = convert2complex_int16_t(conj(m_weights));

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
  for (int i = 0; i < m_n_blps; i++) sw.blpmask.set(i);

  sw.weights().resize(COMPUTE_INTERVAL, m_n_blps, MEPHeader::N_BEAMLETS, MEPHeader::N_POL);
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

  m_beams_modified = true;
}

// void BeamServerTask::update_rcusettings()
// {
// }

void BeamServerTask::send_sbselection()
{
  RSPSetsubbandsEvent ss;
  
  ss.timestamp.setNow(0);

  // select all BLPS, no subarraying
  ss.blpmask.reset();
  for (int i = 0; i < m_n_blps; i++) ss.blpmask.set(i);

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

  int nrsubbands = m_sbsel.size() <= 0 ? 0 : m_sbsel.size() * 2;
  LOG_DEBUG(formatString("nrsubbands=%d", nrsubbands));

  int i = 0;
  for (map<int,int>::iterator sel = m_sbsel.begin();
       sel != m_sbsel.end(); ++sel, ++i)
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

void BeamServerTask::send_rcusettings()
{
  RSPSetrcuEvent rcu;
  
  rcu.timestamp.setNow(0);

  // select all BLPS, no subarraying
  rcu.rcumask.reset();
  for (int i = 0; i < m_n_blps * MEPHeader::N_POL; i++)
  {
    rcu.rcumask.set(i);
  }

  int current_spw = SpectralWindowConfig::getInstance().getCurrent();
  
  if (current_spw >= 0)
  {
    const SpectralWindow* spw = SpectralWindowConfig::getInstance().get(current_spw);
    if (!spw)
    {
      LOG_WARN_STR("No spectral window definition found for spectral window index " << current_spw);
      return;
    }

    rcu.settings().resize(1);
    rcu.settings()(0).value = spw->rcusettings();

    m_rspdriver.send(rcu);
  }
}

int main(int argc, char** argv)
{
  GCFTask::init(argc, argv);

  LOG_INFO(formatString("Program %s has started", argv[0]));

  try 
  {
    GCF::ParameterSet::instance()->adoptFile("BeamServer.conf");
    GCF::ParameterSet::instance()->adoptFile("BeamServerPorts.conf");
    GCF::ParameterSet::instance()->adoptFile("RemoteStation.conf");
  }
  catch (Exception e)
  {
    cerr << "Failed to load configuration files: " << e.text() << endl;
    exit(EXIT_FAILURE);
  }

  int n_blps = GET_CONFIG("BeamServer.N_BLPS", i);
  if (n_blps <= 0 || n_blps > GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i))
  {
    LOG_FATAL(formatString("Error: BeamServer.N_BLPS(%d) less or equal zero or greater than RS.N_RSPBOARDS(%d) * RS.N_BLPS(%d)",
			   n_blps,
			   GET_CONFIG("RS.N_RSPBOARDS", i),
			   GET_CONFIG("RS.N_BLPS", i)));
    exit(EXIT_FAILURE);
  }
  
  BeamServerTask abs("BeamServer", n_blps);

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
