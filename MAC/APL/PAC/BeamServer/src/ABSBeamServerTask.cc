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

BeamServerTask::BeamServerTask(string name)
    : GCFTask((State)&BeamServerTask::initial, name),
      board(*this, "board", GCFPortInterface::SAP, true)
{
  registerProtocol(ABS_PROTOCOL, ABS_PROTOCOL_signalnames);

  client.init(*this, "client", GCFPortInterface::SPP, ABS_PROTOCOL);
  //board.init(*this, "board", GCFPortInterface::SAP, EPA_PROTOCOL, true);

  (void)Beam::setNInstances(ABS_Protocol::N_BEAMLETS);
  (void)Beamlet::setNInstances(ABS_Protocol::N_SUBBANDS);
}

BeamServerTask::~BeamServerTask()
{}

GCFEvent::TResult BeamServerTask::initial(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  //LOG_DEBUG(formatString("initial state received event on port %s", port.getName().c_str()));

  switch(e.signal)
  {
      case F_INIT_SIG:
      {
	  // create a default spectral window from 0MHz to 20MHz
	  // steps of 256kHz
	  SpectralWindow* spw = new SpectralWindow(0e6, 20e6/ABS_Protocol::N_BEAMLETS,
						   ABS_Protocol::N_BEAMLETS);
	  m_spws[0] = spw;

#if 0
	  // create subband set
	  std::set<int> subbands;

	  //
	  // setup beam 0
          //
	  Beam* beam = Beam::getInstance(0);
	  m_beams.insert(0);

	  subbands.insert(0);
	  subbands.insert(10);
	  subbands.insert(20);
	  
	  if (beam->allocate(*m_spws[0], subbands) < 0)
	  {
	      LOG_ERROR("failed to allocate beam 0");
	  }
	  else
	  {
	      beam->getSubbandSelection(m_sbsel);
	  }

	  //
	  // setup beam 1
	  //
	  beam = Beam::getInstance(1);
	  m_beams.insert(1);

	  subbands.clear();
	  subbands.insert(30);
	  subbands.insert(22);
	  subbands.insert(44);
	  subbands.insert(101);
	  subbands.insert(121);
	  subbands.insert(141);

	  if (beam->allocate(*m_spws[0], subbands) < 0)
	  {
	      LOG_ERROR("failed to allocate beam 1");
	  }
	  else
	  {
	      beam->getSubbandSelection(m_sbsel);
	  }

	  beam = Beam::getInstance(2);
	  m_beams.insert(2);
	  if (beam->allocate(*m_spws[0], subbands) < 0)
	  {
	      LOG_ERROR("failed to allocate beam 2");
	  }
	  else
	  {
	      beam->getSubbandSelection(m_sbsel);
	  }

	  //
	  // show total mapping for beam 0 and 1
	  //
	  for (map<int,int>::iterator sel = m_sbsel.begin();
	       sel != m_sbsel.end(); ++sel)
	  {
	      LOG_DEBUG(formatString("(%d,%d)", sel->first, sel->second));
	  }

	  beam->deallocate();
#endif
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
	if (client.isConnected() && board.isConnected())
	{
	  LOG_DEBUG(formatString("port '%s' connected", port.getName().c_str()));
	  TRAN(BeamServerTask::enabled);
	}
      }
      break;

      case F_DISCONNECTED_SIG:
      {
	  LOG_FATAL(formatString("port '%s' disconnected", port.getName().c_str()));
	  exit(EXIT_FAILURE);
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
  static int phase = 0;
  
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
	  board.setTimer(1, (long)1e6-now.tv_usec, 10, 0);
      }
      break;

      case F_TIMER_SIG:
      {
	  struct timeval now;
	  gettimeofday(&now, 0);

	  LOG_DEBUG(formatString("time=(%d,%d)", now.tv_sec, now.tv_usec));

#if 0
	  phase = !phase;
	  
	  if (phase)
	  {
	      wgdisable_action();
	  }
	  else
	  {
	      sbselect();
	      wgenable_action();
	  }
#else
	  compute_timeout_action();

	  wgdisable_action(); // send event on raw ethernet port
#endif
      }
      break;

      case ABS_BEAMALLOC:
      {
	  ABSBeamallocEvent* event = static_cast<ABSBeamallocEvent*>(&e);
	  beamalloc_action(event, port);
      }
      break;

      case ABS_BEAMFREE:
      {
	  ABSBeamfreeEvent* event = static_cast<ABSBeamfreeEvent*>(&e);
	  beamfree_action(event, port);
      }
      break;

      case ABS_BEAMPOINTTO:
      {
	  ABSBeampointtoEvent* event = static_cast<ABSBeampointtoEvent*>(&e);
	  beampointto_action(event, port);
      }
      break;

      case ABS_WGENABLE:
      {
	  ABSWgenableEvent* event = static_cast<ABSWgenableEvent*>(&e);
	  wgenable_action(event);
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

	  // deallocate all beams
	  for (set<int>::iterator bi = m_beams.begin();
	       bi != m_beams.end(); ++bi)
	  {
	    Beam::getInstance(*bi)->deallocate();
	  }

	  TRAN(BeamServerTask::initial);
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
  ABSBeamalloc_AckEvent ack(ba->beam_index, SUCCESS);

  // check parameters
  if (!(beam = Beam::getInstance(ba->beam_index))
      || ((spwindex = ba->spectral_window) != 0)
      || !(ba->n_subbands > 0 && ba->n_subbands < N_BEAMLETS))
  {
      ack.status = ERR_RANGE;
      port.send(ack);
      return;                         // RETURN
  }

  set<int> subbands;
  for (int i = 0; i < ba->n_subbands; i++) subbands.insert(ba->subbands[i]);

  // allocate the beam
  if (beam->allocate(*m_spws[spwindex], subbands) < 0)
  {
      ack.status = ERR_BEAMALLOC;
      port.send(ack);
  }
  else
  {
      m_beams.insert(ba->beam_index);
      update_sbselection();
      port.send(ack);
  }
}

void BeamServerTask::beamfree_action(ABSBeamfreeEvent* bf,
				     GCFPortInterface& port)
{
  ABSBeamfree_AckEvent ack(bf->beam_index, SUCCESS);

  Beam* beam = 0;
  if (!(beam = Beam::getInstance(bf->beam_index)))
  {
      ack.status = ERR_RANGE;
      port.send(ack);
      return;                      // RETURN
  }

  if (beam->deallocate() < 0)
  {
      ack.status = ERR_BEAMFREE;
      port.send(ack);
      return;                     // RETURN
  }

  port.send(ack);
}

void BeamServerTask::beampointto_action(ABSBeampointtoEvent* pt,
					GCFPortInterface& /*port*/)
{
  Beam* beam = Beam::getInstance(pt->beam_index);

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

void BeamServerTask::wgenable_action(ABSWgenableEvent* we)
{
  EPAWgenableEvent ee;
  
  ee.command       = 2; // 2 == waveform enable
  ee.seqnr         = 0;
  ee.pktsize       = 12;
  ee.frequency     = (short)((we->frequency*65535) / SYSTEM_CLOCK_FREQ);
  ee.reserved1     = 0;
  ee.amplitude     = we->amplitude;
  (void)memset(&ee.reserved2, 0, 3);
  ee.sample_period = we->sample_period;

  board.send(GCFEvent(F_RAW_SIG), &ee.command, ee.pktsize);
}

void BeamServerTask::wgenable_action()
{
  EPAWgenableEvent ee;
  
  ee.command       = 2; // 2 == waveform enable
  ee.seqnr         = 0;
  ee.pktsize       = htons(12);
  ee.frequency     = htons((short)((1e6 * 65535) / SYSTEM_CLOCK_FREQ));
  ee.reserved1     = 0;
  ee.amplitude     = 128;
  (void)memset(&ee.reserved2, 0, 3);
  ee.sample_period = 2;

  board.send(GCFEvent(F_RAW_SIG), &ee.command, 12);
  
  cerr << "SENT WGENABLE" << endl;
}

void BeamServerTask::wgdisable_action()
{
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
  lasttime.tv_sec += 20;
  
  // iterate over all beams
  for (set<int>::iterator bi = m_beams.begin();
       bi != m_beams.end(); ++bi)
  {
      Beam::getInstance(*bi)->convertPointings(fromtime, 20);
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
  for (set<int>::iterator bi = m_beams.begin();
       bi != m_beams.end(); ++bi)
  {
      Beam::getInstance(*bi)->getSubbandSelection(m_sbsel);
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
