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
#include "EPA_Protocol.ph"

#include "ABSBeamServerTask.h"

#include "ABSBeam.h"
#include "ABSBeamlet.h"

#include <iostream>
#include <sys/time.h>
#include <string.h>

#include <netinet/in.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
using namespace LOFAR;

using namespace ABS;
using namespace std;

#define MAX_N_SPECTRAL_WINDOWS 1
#define COMPUTE_INTERVAL 10
#define UPDATE_INTERVAL  1

BeamServerTask::BeamServerTask(string name)
    : GCFTask((State)&BeamServerTask::initial, name),
      board(*this, "board", GCFPortInterface::SAP, true)
{
  registerProtocol(ABS_PROTOCOL, ABS_PROTOCOL_signalnames);

  client.init(*this, "client", GCFPortInterface::SPP, ABS_PROTOCOL);
  //board.init(*this, "board", GCFPortInterface::SAP, EPA_PROTOCOL, true);

  // COMPUTE_INTERVAL seconds compute_interval
  (void)Beam::init(ABS_Protocol::N_BEAMLETS,
		   UPDATE_INTERVAL, COMPUTE_INTERVAL);
  (void)Beamlet::init(ABS_Protocol::N_SUBBANDS);

  m_wgsetting.frequency     = 1e6; // 1MHz
  m_wgsetting.amplitude     = 128;
  m_wgsetting.sample_period = 2;
  m_wgsetting.enabled       = false;
}

BeamServerTask::~BeamServerTask()
{}

GCFEvent::TResult BeamServerTask::initial(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch(e.signal)
  {
      case F_INIT_SIG:
      {
	  // create a default spectral window from 0MHz to 20MHz
	  // steps of 256kHz
	  SpectralWindow* spw = new SpectralWindow(0e6, 20e6/ABS_Protocol::N_BEAMLETS,
						   ABS_Protocol::N_BEAMLETS);
	  m_spws[0] = spw;
      }
      break;

      case F_ENTRY_SIG:
      {
	  if (!client.isConnected()) client.open(); // need this otherwise GTM_Sockethandler is not called
	  board.setAddr("eth0", "aa:bb:cc:dd:ee:ff");
	  if (!board.isConnected()) board.open();
      }
      break;

      case F_CONNECTED_SIG:
      {
	LOG_DEBUG(formatString("port '%s' connected", port.getName().c_str()));
	if (client.isConnected() && board.isConnected())
	{
	  TRAN(BeamServerTask::enabled);
	}
	
	// start with WG disabled.
	if (board.isConnected()) wgdisable_action();
      }
      break;

      case F_DISCONNECTED_SIG:
      {
	  LOG_FATAL(formatString("port '%s' disconnected", port.getName().c_str()));
	  exit(EXIT_FAILURE);
      }
      break;

      case F_DATAIN_SIG:
      {
	  if (&port == &board)
	  {
	      // read data to clear F_DATAIN_SIG
	      char data[ETH_DATA_LEN];
	      ssize_t length = board.recv(data, ETH_DATA_LEN);
	      
	      length=length; // keep compiler happy

	      LOG_DEBUG(".");
	  }
      }
      break;

      default:
	  status = GCFEvent::NOT_HANDLED;
	  break;
  }

  return status;
}

GCFEvent::TResult BeamServerTask::enabled(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  static unsigned long update_timer = (unsigned long)-1;
  static unsigned long compute_timer = (unsigned long)-1;
  
  switch (e.signal)
  {
#if 0
      case F_ACCEPT_REQ_SIG:
	  client.getPortProvider().accept();
	  break;
#endif
      case F_ENTRY_SIG:
      {
	  struct timeval now;

	  // start second timer, exactly on the next second
	  gettimeofday(&now, 0);

	  // update timer, once every UPDATE_INTERVAL exactly on the second
	  update_timer = board.setTimer((2 * UPDATE_INTERVAL) -
					(now.tv_sec % UPDATE_INTERVAL),
					(long)1e6-now.tv_usec,
					UPDATE_INTERVAL, 0);

	  // compute timer, once every COMPUTE_INTERVAL exactly on the second
	  compute_timer = board.setTimer((2 * COMPUTE_INTERVAL)
					 - (now.tv_sec % COMPUTE_INTERVAL),
					 (long)1e6-now.tv_usec,
					 COMPUTE_INTERVAL, 0);
      }
      break;

      case F_TIMER_SIG:
      {
	  GCFTimerEvent* timer = static_cast<GCFTimerEvent*>(&e);
	  struct timeval now;
	  gettimeofday(&now, 0);

	  LOG_INFO(formatString("timer=(%d,%d)", timer->sec, timer->usec));
	  if (timer->id == update_timer)
	  {
	      LOG_INFO(formatString("update_timer=(%d,%d)", now.tv_sec, now.tv_usec));
	      compute_timeout_action();
	  }
	  else if (timer->id == compute_timer)
	  {
	      LOG_INFO(formatString("compute_timer=(%d,%d)", now.tv_sec, now.tv_usec));
	  }
	  else
	  {
	      LOG_INFO(formatString("unknown timer %d", timer->id));
	  }
      }
      break;

      case ABS_BEAMALLOC:
      {
	  ABSBeamallocEvent* event = static_cast<ABSBeamallocEvent*>(&e);
	  beamalloc_action(event, port);

	  if (m_beams.size() == 1)
	  {
	      // enable on the first beam
	      wgenable_action();
	  }
      }
      break;

      case ABS_BEAMFREE:
      {
	  ABSBeamfreeEvent* event = static_cast<ABSBeamfreeEvent*>(&e);
	  beamfree_action(event, port);

	  if (m_beams.size() == 0)
	  {
	      // no more beams, disable WG
	      wgdisable_action();
	  }
      }
      break;

      case ABS_BEAMPOINTTO:
      {
	  ABSBeampointtoEvent* event = static_cast<ABSBeampointtoEvent*>(&e);
	  beampointto_action(event, port);
      }
      break;

      case ABS_WGSETTINGS:
      {
	  ABSWgsettingsEvent* event = static_cast<ABSWgsettingsEvent*>(&e);
	  wgsettings_action(event, port);
	  if (m_wgsetting.enabled) wgenable_action();
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

      case F_DATAIN_SIG:
      {
	  char data[ETH_DATA_LEN];
	  // ignore DATAIN
	  ssize_t length = board.recv(data, ETH_DATA_LEN);
	  //cout << "received " << length << endl;

	  length=length; // keep compiler happy

#if 0
	  EPADataEvent de;
	  
	  memcpy((void*)&de.fill, (void*)data, sizeof(de)-sizeof(GCFEvent));

	  cout << "sizeof(GCFEvent) = " << sizeof(GCFEvent) << endl;
	  cout << "sizeof(de) = " << sizeof(de) << endl;
#endif

	  unsigned int* seqnr = (unsigned int*)&data[2];
	  cerr << "seqnr=" << *seqnr << endl;
      }
      break;

      case F_DATAOUT_SIG:
	  LOG_DEBUG("dataout");
	  break;

      case F_DISCONNECTED_SIG:
     {
	 LOG_DEBUG(formatString("port %s disconnected", port.getName().c_str()));

	 TRAN(BeamServerTask::initial);
     }
     break;

      case F_EXIT_SIG:
      {
	  // deallocate all beams
	  for (set<Beam*>::iterator bi = m_beams.begin();
	       bi != m_beams.end(); ++bi)
	  {
	      (*bi)->deallocate();
	  }

	  // disable the waveform generator
	  wgdisable_action();

	  // cancel timers
	  board.cancelAllTimers();
      }
      break;

      default:
	  status = GCFEvent::NOT_HANDLED;
	  break;
  }

  return status;
}

void BeamServerTask::beamalloc_action(ABSBeamallocEvent* ba,
				      GCFPortInterface& port)
{
  int   spwindex = 0;
  Beam* beam = 0;
  ABSBeamalloc_AckEvent ack(-1, SUCCESS);

  // check parameters
  if (((spwindex = ba->spectral_window) < 0)
      || (spwindex >= MAX_N_SPECTRAL_WINDOWS)
      || !(ba->n_subbands > 0 && ba->n_subbands <= N_BEAMLETS))
  {
      ack.status = ERR_RANGE;
      port.send(ack);
      return;                         // RETURN
  }
  
  set<int> subbands;
  for (int i = 0; i < ba->n_subbands; i++) subbands.insert(ba->subbands[i]);

  // allocate the beam

  if (0 == (beam = Beam::allocate(*m_spws[spwindex], subbands)))
  {
      ack.status = ERR_BEAMALLOC;
      port.send(ack);
  }
  else
  {
      ack.handle = beam->handle();
      LOG_DEBUG(formatString("ack.handle=%d", ack.handle));
      m_beams.insert(beam);
      update_sbselection();
      port.send(ack);
  }
}

void BeamServerTask::beamfree_action(ABSBeamfreeEvent* bf,
				     GCFPortInterface& port)
{
  ABSBeamfree_AckEvent ack(bf->handle, SUCCESS);

  Beam* beam = 0;
  if (!(beam = Beam::getFromHandle(bf->handle)))
  {
      LOG_ERROR("Beam::getFromHandle failed");
      ack.status = ERR_RANGE;
      port.send(ack);
      return;                      // RETURN
  }

  if (beam->deallocate() < 0)
  {
      LOG_ERROR("beam->deallocate() failed");
      ack.status = ERR_BEAMFREE;
      port.send(ack);
      return;                     // RETURN
  }

  port.send(ack);
}

void BeamServerTask::beampointto_action(ABSBeampointtoEvent* pt,
					GCFPortInterface& /*port*/)
{
  Beam* beam = Beam::getFromHandle(pt->handle);

  if (beam)
  {
      if (beam->addPointing(Pointing(Direction(pt->angle1,
					       pt->angle2,
					       (Direction::Types)pt->type),
				     pt->time)) < 0)
      {
	  LOG_ERROR("beam not allocated");
      }
  }
  else LOG_ERROR("invalid beam_index in BEAMPOINTTO");
}

void BeamServerTask::wgsettings_action(ABSWgsettingsEvent* wgs,
				       GCFPortInterface& port)
{
  ABSWgsettings_AckEvent sa(SUCCESS);

  if ((wgs->frequency >= 1.0e-6)
      && (wgs->frequency <= 80.0e6))
  {
      m_wgsetting.frequency     = wgs->frequency;
      m_wgsetting.amplitude     = wgs->amplitude;
      m_wgsetting.sample_period = wgs->sample_period;
      
      if (m_wgsetting.enabled) wgenable_action();
  }
  else
  {
      sa.status = ERR_RANGE;
  }

  // send ack
  port.send(sa);
}

void BeamServerTask::wgenable_action()
{
  // mark enabled
  m_wgsetting.enabled = true;

  // send WG enable using settings
  // from m_wgsetting field
  EPAWgenableEvent ee;

  ee.command       = 2; // 2 == waveform enable
  ee.seqnr         = 0;
  ee.pktsize       = htons(12);
  ee.frequency     = htons((short)((m_wgsetting.frequency * 65535) / SYSTEM_CLOCK_FREQ));
  ee.reserved1     = 0;
  ee.amplitude     = m_wgsetting.amplitude;
  (void)memset(&ee.reserved2, 0, 3);
  ee.sample_period = m_wgsetting.sample_period;

  board.send(GCFEvent(F_RAW_SIG), &ee.command, 12);
  
  cerr << "SENT WGENABLE" << endl;
}

void BeamServerTask::wgdisable_action()
{
  // mark disabled
  m_wgsetting.enabled = false;

  EPAWgdisableEvent de;
  
  de.command       = 3; // 3 == waveform disable
  de.seqnr         = 0;
  de.pktsize       = htons(12);
  (void)memset(&de.reserved1, 0, 8);

  board.send(GCFEvent(F_RAW_SIG), &de.command, 12);

  cerr << "SENT WGDISABLE" << endl;
}

/**
 * This method is called once every second
 * to calculate the weights for all beamlets.
 */
void BeamServerTask::compute_timeout_action()
{
  // convert_pointings for all beams for the next deadline

  static struct timeval lasttime = { 0, 0 };
  struct timeval fromtime = lasttime;
  gettimeofday(&lasttime, 0);
  lasttime.tv_sec += COMPUTE_INTERVAL;
  
  // iterate over all beams
  for (set<Beam*>::iterator bi = m_beams.begin();
       bi != m_beams.end(); ++bi)
  {
      (*bi)->convertPointings(fromtime);
  }

  calculate_weights();
  send_weights();
}

void BeamServerTask::calculate_weights()
{
  // iterate over all beamlets
  Beamlet::calculate_weights();
}

void BeamServerTask::send_weights()
{
  EPABfconfigureEvent bc;

  bc.command = 4; // 4 == beamformer configure
  bc.seqnr = 0;
  bc.pktsize = 1030;

  // set all coefficients to 1
  memset(bc.coeff, 0, 512*sizeof(short));
  for (int i = 0; i < 512; i+=2) bc.coeff[i] = 1;

  for (int ant = 0; ant < N_ANTENNAS; ant++)
  {
      bc.antenna = ant;
      
      for (int php = 0; php < N_PHASEPOL; php++)
      {
	  bc.phasepol = php;
      }

      board.send(GCFEvent(F_RAW_SIG), &bc.command, bc.pktsize);
  }

  EPABfenableEvent be;

  be.command = 5; // 5 == beamformer enable
  be.seqnr = 0;
  be.pktsize = 12;
  memset(&be.reserved1, 0, 8);

  board.send(GCFEvent(F_RAW_SIG), &be.command, be.pktsize);
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

  send_sbselection();
}

void BeamServerTask::send_sbselection()
{
  if (m_sbsel.size() > 0)
  {
      EPASubbandselectEvent ss;

      ss.command = 1; // 1 == subband selection
      ss.seqnr = 0;
      ss.pktsize = htons(5 + m_sbsel.size()*2);
      ss.nofbands = (m_sbsel.size() <= 0 ? 0 : m_sbsel.size()*2 - 1);
      
      memset(&ss.bands, 0, sizeof(ss.bands));
      int i = 0;
      for (map<int,int>::iterator sel = m_sbsel.begin();
	   sel != m_sbsel.end(); ++sel, ++i)
      {
	  LOG_DEBUG(formatString("(%d,%d)", sel->first, sel->second));

	  if (i != sel->first)
	  {
	      LOG_ERROR(formatString("invalid src index %d", sel->first));
	      continue;
	  }
	  if (sel->second > 254)
	  {
	      LOG_ERROR(formatString("invalid tgt index", sel->first));
	      continue;
	  }

	  // same selection for x and y polarization
	  ss.bands[sel->first*2]   = sel->second*2;
	  ss.bands[sel->first*2+1] = sel->second*2;
      }

      board.send(GCFEvent(F_RAW_SIG), &ss.command, 5 + m_sbsel.size()*2);
  }
}

void BeamServerTask::sbselect()
{
    EPASubbandselectEvent ss;

    ss.command = 1; // 1 == subband selection
    ss.seqnr = 0;
    ss.pktsize = htons(5+1);
    ss.nofbands = 0;
    ss.bands[0] = 0;
    board.send(GCFEvent(F_RAW_SIG), &ss.command, 5+1);
}

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

  GCFTask::init(argc, argv);

  BeamServerTask abs("ABS");

  abs.start(); // make initial transition

  GCFTask::run();

  LOG_INFO("Normal termination of program");

  return 0;
}
