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
#include "ABSConstants.h"

#include <iostream>
#include <time.h>
#include <string.h>

#include <netinet/in.h>

#include <boost/date_time/posix_time/posix_time.hpp>

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

#define MAX_N_SPECTRAL_WINDOWS 1
#define COMPUTE_INTERVAL 10
#define UPDATE_INTERVAL  1

#define SCALE (1<<(16-2))

#define BEAMLETSTATS_INTEGRATION_COUNT 1000
#define EPA_DELAY_TIME 10000

BeamServerTask::BeamServerTask(string name)
    : GCFTask((State)&BeamServerTask::initial, name),
      m_pos(N_ELEMENTS, N_POLARIZATIONS, 3),
      m_weights(COMPUTE_INTERVAL, N_ELEMENTS, N_SUBBANDS, N_POLARIZATIONS),
      m_weights16(COMPUTE_INTERVAL, N_ELEMENTS, N_SUBBANDS, N_POLARIZATIONS),
      m_stats(N_BEAMLETS, BEAMLETSTATS_INTEGRATION_COUNT),
      board(*this, "board", GCFPortInterface::SAP, true)
{
  registerProtocol(ABS_PROTOCOL, ABS_PROTOCOL_signalnames);

  client.init(*this, "client", GCFPortInterface::SPP, ABS_PROTOCOL);
  //board.init(*this, "board", GCFPortInterface::SAP, EPA_PROTOCOL, true);

  (void)Beam::init(ABS::N_BEAMLETS,
		   UPDATE_INTERVAL, COMPUTE_INTERVAL);
  (void)Beamlet::init(ABS::N_SUBBANDS);

  m_wgsetting.frequency     = 1.5e6; // 1MHz
  m_wgsetting.amplitude     = 128;
  m_wgsetting.sample_period = 2; // 80 MHz / 40 MHz == 2
  m_wgsetting.enabled       = false;

  // initialize antenna positions
  Range all = Range::all();
  m_pos(0, 0, all) = 0.0;
  m_pos(0, 0, 0)   = 0.0;

  m_pos(0, 1, all) = 0.0;
  m_pos(0, 1, 0)   = 100.0;

  // initialize weight matrix
  m_weights   = complex<W_TYPE>(0,0);
  m_weights16 = complex<int16_t>(0,0);

#ifdef EPA_Y_POL_IMAG_FIX
  LOG_INFO("This code contains the EPA_Y_POL_IMAG_FIX.");
#endif
#ifdef WEIGHTS_IN_NETWORK_ORDER
  LOG_INFO("Send beamformer weights in big-endian network order.");
#endif
}

BeamServerTask::~BeamServerTask()
{}

bool BeamServerTask::isEnabled()
{
  return client.isConnected() && board.isConnected() && m_stats.isReady();
}

GCFEvent::TResult BeamServerTask::initial(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch(e.signal)
  {
      case F_INIT_SIG:
      {
	  // create a default spectral window from 0MHz to 20MHz
	  // steps of 156.25 kHz
	  SpectralWindow* spw = new SpectralWindow(0.0, 20.0e6/N_SUBBANDS, N_SUBBANDS);
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
	if (isEnabled())
	{
	  TRAN(BeamServerTask::enabled);
	}
	
	// start with WG disabled.
	//if (board.isConnected()) wgdisable_action();
      }
      break;

      case F_DISCONNECTED_SIG:
      {
	  port.setTimer((long)3); // try again in 3 seconds
	  LOG_WARN(formatString("port '%s' disconnected, retry in 3 seconds...", port.getName().c_str()));
	  port.close();
      }
      break;

      case F_TIMER_SIG:
      {
	  LOG_INFO(formatString("port '%s' retry of open...", port.getName().c_str()));
	  port.open();
      }
      break;

      case F_DATAIN_SIG:
      {
	  if (&port == &board)
	  {
	      process_statistics();
	  }
      }
      break;

    case F_EXIT_SIG:
      {
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

GCFEvent::TResult BeamServerTask::enabled(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  static unsigned long update_timer = (unsigned long)-1;
//  static unsigned long compute_timer = (unsigned long)-1;
  static int period = 0;

  switch (e.signal)
    {
#if 0
    case F_ACCEPT_REQ_SIG:
      client.getPortProvider().accept();
      break;
#endif
    case F_ENTRY_SIG:
      {
	ptime nowtime = from_time_t(time(0)); 
	time_duration now = nowtime - ptime(date(1970,1,1));

	// update timer, once every UPDATE_INTERVAL exactly on the second
	update_timer = board.setTimer((2 * UPDATE_INTERVAL) -
				      (now.total_seconds() % UPDATE_INTERVAL), 0,
				      UPDATE_INTERVAL, 0);

#if 0
	// compute timer, once every COMPUTE_INTERVAL exactly on the second
	compute_timer = board.setTimer((2 * COMPUTE_INTERVAL)
				       - (now.total_seconds() % COMPUTE_INTERVAL), 0,
				       COMPUTE_INTERVAL, 0);
#endif
      }
      break;

    case F_TIMER_SIG:
      {
	GCFTimerEvent* timer = static_cast<GCFTimerEvent*>(&e);

	LOG_DEBUG(formatString("timer=(%d,%d)", timer->sec, timer->usec));

	if (0 == (period % COMPUTE_INTERVAL))
	{
	    period = 0;
	    // compute new weights after sending weights
	    compute_timeout_action(timer->sec);
	}

	send_weights(period);

	period++;
#if 0
	if (timer->id == update_timer)
	  {
	    LOG_DEBUG(formatString("update_timer=(%d,%d)", timer->sec, timer->usec));
	    send_weights();
	  }
	else if (timer->id == compute_timer)
	  {
	    LOG_DEBUG(formatString("compute_timer=(%d,%d)", timer->sec, timer->usec));
	    compute_timeout_action(timer->sec);
	  }
	else
	  {
	    LOG_DEBUG(formatString("unknown timer %d", timer->id));
	  }
#endif
      }
      break;

    case ABS_BEAMALLOC:
      {
	ABSBeamallocEvent* event = static_cast<ABSBeamallocEvent*>(&e);
	beamalloc_action(event, port);

#if 0
	if (m_beams.size() == 1)
	  {
	    // enable on the first beam
	    wgenable_action();
	  }
#endif
      }
      break;

    case ABS_BEAMFREE:
      {
	ABSBeamfreeEvent* event = static_cast<ABSBeamfreeEvent*>(&e);
	beamfree_action(event, port);

#if 0
	if (m_beams.size() == 0)
	  {
	    // no more beams, disable WG
	    wgdisable_action();
	  }
#endif
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
	  //wgdisable_action();
      }
      break;

    case F_DATAIN_SIG:
      {
	  if (&port == &board)
	  {
	      process_statistics();
	  }
      }
      break;

    case F_DATAOUT_SIG:
      LOG_DEBUG("dataout");
      break;

    case F_DISCONNECTED_SIG:
      {
	LOG_DEBUG(formatString("port %s disconnected", port.getName().c_str()));
	port.close();

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
	//wgdisable_action();

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

void BeamServerTask::process_statistics()
{
    static char data[ETH_DATA_LEN];

    ssize_t length = board.recv(data, ETH_DATA_LEN);
    unsigned int* seqnr = (unsigned int*)&data[2];
    unsigned int* statsdata = (unsigned int*)&data[6];
    
    if (STATS_PACKET_SIZE == length)
    {
	Array<unsigned int, 3> power_sum(statsdata,
					 shape(N_BEAMLETS / 2, N_POLARIZATIONS, 2),
					 neverDeleteData);
	m_stats.update(power_sum, *seqnr);
    }
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
      LOG_ERROR("argument range error");
      ack.status = ERR_RANGE;
      port.send(ack);
      return;                         // RETURN
  }
  
  // create subband selection set
  set<int> subbands;
  for (int i = 0; i < ba->n_subbands; i++) subbands.insert(ba->subbands[i]);

  // check if array of subbands did not contain any duplicate subband
  // selection
  if (ba->n_subbands != (int)subbands.size())
  {
    LOG_ERROR("subband selection contains duplicates");
    ack.status = ERR_RANGE;
    port.send(ack);
    return;                          // RETURN
  }

  // allocate the beam

  if (0 == (beam = Beam::allocate(*m_spws[spwindex], subbands)))
  {
      LOG_ERROR("Beam::allocate failed");
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
      LOG_ERROR("beam->deallocate failed");
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
      time_t pointto_time = pt->time;

      LOG_DEBUG(formatString("received new coordinates: %f, %f",
			     pt->angle1, pt->angle2));

      //
      // If the time is not set, then activate the command
      // 2 * COMPUTE_INTERVAL seconds from now, because that's how
      // long it takes the command to flow through the pipeline.
      //
      if (0 == pt->time) pointto_time = time(0) + 2 * COMPUTE_INTERVAL;
      if (beam->addPointing(Pointing(Direction(pt->angle1,
					       pt->angle2,
					       (Direction::Types)pt->type),
				     from_time_t(pointto_time))) < 0)
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

  // max allowed frequency = 20MHz
  if ((wgs->frequency >= 1.0e-6)
      && (wgs->frequency <= ABS::SYSTEM_CLOCK_FREQ/4.0))
  {
      m_wgsetting.frequency     = wgs->frequency;
      m_wgsetting.amplitude     = wgs->amplitude;
      m_wgsetting.sample_period = wgs->sample_period;
      
      if (m_wgsetting.enabled) wgenable_action();
  }
  else
  {
      LOG_ERROR("argument range error");
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
  ee.pktsize       = htons(WGENABLE_PACKET_SIZE);
  ee.frequency     = htons((short)((m_wgsetting.frequency * (1 << 16))/ SYSTEM_CLOCK_FREQ));
  ee.reserved1     = 0;
  ee.amplitude     = m_wgsetting.amplitude;
  (void)memset(&ee.reserved2, 0, 3);
  ee.sample_period = m_wgsetting.sample_period;

  board.send(GCFEvent(F_RAW_SIG), &ee.command, WGENABLE_PACKET_SIZE);
  usleep(EPA_DELAY_TIME); // give EPA board time to process buffer
  
  LOG_DEBUG("SENT WGENABLE");
}

void BeamServerTask::wgdisable_action()
{
  // mark disabled
  m_wgsetting.enabled = false;

  EPAWgdisableEvent de;
  
  de.command       = 3; // 3 == waveform disable
  de.seqnr         = 0;
  de.pktsize       = htons(WGDISABLE_PACKET_SIZE);
  (void)memset(&de.reserved1, 0, 8);

  board.send(GCFEvent(F_RAW_SIG), &de.command, WGDISABLE_PACKET_SIZE);
  usleep(EPA_DELAY_TIME); // give EPA board time to process buffer

  LOG_DEBUG("SENT WGDISABLE");
}

BZ_DECLARE_FUNCTION_RET(convert2complex_int16_t, complex<int16_t>)

/**
 * Convert the weights to 16-bits signed integer. Change byte
 * ordering to network order (MSB).
 */
inline complex<int16_t> convert2complex_int16_t(complex<W_TYPE> cd)
{
#ifdef WEIGHTS_IN_NETWORK_ORDER
#ifdef W_TYPE_DOUBLE
  return complex<int16_t>(htons((int16_t)(round(cd.real()*SCALE))),
			  htons((int16_t)(round(cd.imag()*SCALE))));
#else
  return complex<int16_t>(htons((int16_t)(roundf(cd.real()*SCALE))),
			  htons((int16_t)(roundf(cd.imag()*SCALE))));
#endif
#else
#ifdef W_TYPE_DOUBLE
  return complex<int16_t>((int16_t)(round(cd.real()*SCALE)),
			  (int16_t)(round(cd.imag()*SCALE)));
#else
  return complex<int16_t>((int16_t)(roundf(cd.real()*SCALE)),
			  (int16_t)(roundf(cd.imag()*SCALE)));
#endif
#endif
}

/**
 * This method is called once every second
 * to calculate the weights for all beamlets.
 */
void BeamServerTask::compute_timeout_action(long current_seconds)
{
  // convert_pointings for all beams for the next deadline
  time_period compute_period = time_period(from_time_t((time_t)current_seconds)
					   + time_duration(seconds(COMPUTE_INTERVAL)),
					   seconds(COMPUTE_INTERVAL));

  Array<W_TYPE,2> lmns(COMPUTE_INTERVAL, 3);  // l,m,n coordinates

  // iterate over all beams
  for (set<Beam*>::iterator bi = m_beams.begin();
       bi != m_beams.end(); ++bi)
  {
    (*bi)->convertPointings(compute_period);

    lmns = (*bi)->getLMNCoordinates();
    LOG_DEBUG(formatString("current_pointing=(%f,%f)",
			   (*bi)->pointing().direction().angle1(),
			   (*bi)->pointing().direction().angle2()));
  }

  cout << "lmns = " << lmns << endl;
  
  Beamlet::calculate_weights(m_pos, m_weights);

  cout << "m_weights(t=0,element=0,subband=11,pol=all) = " << m_weights(0,0,11,Range::all()) << endl;

  // show weights for timestep 0, element 0, all subbands, both polarizations
  //Range all = Range::all();
  //cout << "m_weights=" << m_weights(0, 0, all, all) << endl;

  //
  // need complex conjugate of the weights
  // as 16bit signed integer to send to the board
  //
#if 0
  m_weights16 = convert2complex_int16_t(conj(m_weights));
#else
  for (int i = 0; i < COMPUTE_INTERVAL; i++)
    for (int j = 0; j < N_ELEMENTS; j++)
      for (int k = 0; k < N_SUBBANDS; k++)
	for (int l = 0; l < N_POLARIZATIONS; l++)
	  {
	      //
	      // -1 * imaginary part to take complex conjugate of the weight
	      //
	      m_weights16(i,j,k,l) = complex<int16_t>(   (int16_t)round(m_weights(i,j,k,l).real()*SCALE),
						      -1*(int16_t)round(m_weights(i,j,k,l).imag()*SCALE));
	  }
#endif

  //LOG_DEBUG(formatString("m_weights16 contiguous storage? %s", (m_weights16.isStorageContiguous()?"yes":"no")));
  LOG_DEBUG(formatString("sizeof(m_weights16) = %d", m_weights16.size()*sizeof(int16_t)));
}

void BeamServerTask::send_weights(int period)
{
  EPABfconfigureEvent bc;
  Range all = Range::all();

  bc.command = 4; // 4 == beamformer configure
  bc.seqnr   = 0;
  
  //
  // -1 to not count padding byte in the EPABfconfigureEvent struct
  //
  bc.pktsize = htons(BFCONFIGURE_PACKET_SIZE);

  //
  // Take address 1 byte before bc.coeff field to account for the
  // padding byte in the EPABfconfigureEvent struct.
  //
  char* coeff = (char*)bc.coeff;
  coeff -= 1;

  Array<complex<int16_t>, 2> weights((complex<int16_t>*)coeff,
				     shape(N_BEAMLETS, N_POLARIZATIONS),
				     neverDeleteData);

  for (int ant = 0; ant < N_ELEMENTS; ant++)
  {
      //bc.antenna = ant;
      
      for (int pol = 0; pol < N_POLARIZATIONS * 2; pol++)
      {
	  weights = m_weights16(period, ant, all, all);

	  if (pol == 0) {
	    // weights for x-real part
	  } else if (pol == 1) {
	    // weights for x-imaginary part
	    weights *= complex<int16_t>(0,1);
	    //imag(weights) *= -1;
	  } else if (pol == 2) {
	    // weights for y-real part
	  } else if (pol == 3) {
	    // weights for y-imaginary part
	    weights *= complex<int16_t>(0,1);
	    //imag(weights) *= -1;
	  }

#ifdef EPA_Y_POL_IMAG_FIX
	  //
	  // There is a bug in the EPA firmware which can be
	  // circumvented by negating the imaginary part
	  // of the weights for the y-polarization.
	  // Ask Wessel Lubberhuizen (lubberhuizen@astron.nl)
	  // for details.
	  //
	  imag(weights(all, 1)) *= -1;
#endif

	  for (int i = 0; i < N_SUBBANDS; i++)
	    for (int j = 0; j < N_POLARIZATIONS; j++)
	      weights(i,j) = complex<int16_t>(htons(weights(i,j).real()),
					      htons(weights(i,j).imag()));

	  bc.phasepol = pol;
	  board.send(GCFEvent(F_RAW_SIG), &bc.command, BFCONFIGURE_PACKET_SIZE);
	  usleep(EPA_DELAY_TIME); // give EPA board time to process buffer
      }
  }

  EPABfenableEvent be;

  be.command = 5; // 5 == beamformer enable
  be.seqnr = 0;
  be.pktsize = htons(BFENABLE_PACKET_SIZE);
  memset(&be.reserved1, 0, 8);

  board.send(GCFEvent(F_RAW_SIG), &be.command, BFENABLE_PACKET_SIZE);
  usleep(EPA_DELAY_TIME); // give EPA board time to process buffer
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
      ss.pktsize = htons(SBSELECT_PACKET_HDR_SIZE + m_sbsel.size()*2);
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
	  if (sel->second > N_BEAMLETS)
	  {
	      LOG_ERROR(formatString("invalid tgt index", sel->first));
	      continue;
	  }

	  // same selection for x and y polarization
	  ss.bands[sel->first*2]   = sel->second * 2;
	  ss.bands[sel->first*2+1] = sel->second * 2 + 1;
      }

      board.send(GCFEvent(F_RAW_SIG), &ss.command, SBSELECT_PACKET_HDR_SIZE + m_sbsel.size()*2);
      usleep(EPA_DELAY_TIME); // give EPA board time to process buffer
  }
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
