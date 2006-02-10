//#
//#  rspctl.cc: command line interface to the RSPDriver
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
//#

//#
//# Usage:
//#
//# rspctl --weights         [--select=<set>]  # get weights
//# rspctl --weights=value.re[,value.im] [--select=<set>]  # set weights as complex value
//# rspctl --aweights        [--select=<set> ] # get weights as power and angle
//# rspctl --aweights=amplitude[,angle] [--select=<set> ]  # set weights as amplitude and angle
//# rspctl --subbands        [--select=<set>]  # get subband selection
//# rspctl --subbands=<list> [--select=<set>]  # set subband selection
//# rspctl --rcu             [--select=<set>]  # get RCU control
//# rspctl --rcu=0x00000000  [--select=<set>]  # set RCU control
//# rspctl --wg              [--select=<set>]  # get waveform generator settings
//# rspctl --wg=freq         [--select=<set>]  # set wg freq is in Hz (float)
//# rspctl --status          [--select=<set>]  # get status
//# rspctl --statistics[=(subband|beamlet)]
//#                          [--select=<set>]
//#                          [--duration=<seconds>]
//#                          [--integration=<seconds>]
//#                          [--directory=<directory>]
//#                          [--feport=<hostname>:<port>] # get subband (default) or beamlet statistics
//# rspctl --xcstatistics    [--select=<set>]
//#                          [--duration=<seconds>]
//#                          [--integration=<seconds>]
//#                          [--directory=<directory>]
//#                          [--feport=<hostname>:<port>]  # get cross correlation statistics
//# rspctl --xcsubband                         # get the subband selection for cross correlation
//# rspctl --xcsubband=<int>                   # set the subband to cross correlate
//# rspctl --clocks          [--select=<set>]  # get or set the clock frequency of clocks
//# rspctl --version         [--select=<set>]  # get version information
//# rspctl --rspclear		 [--select=<set>]  # clear FPGA registers on rspboards
//#

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RSP_Protocol/EPA_Protocol.ph>

#include <APL/RTCCommon/PSAccess.h>

#include <iostream>
#include <Common/lofar_sstream.h>
#include <sys/time.h>
#include <string.h>
#include <blitz/array.h>
#include <getopt.h>
#include <complex>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <set>

#include <APL/RTCCommon/gnuplot_i.h>

#include "rspctl.h"

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace EPA_Protocol;
using namespace RSP_Protocol;
using namespace rspctl;
using namespace RTC;

// local funtions
static void usage();

//
// Sample frequency, should be queried from the RSPDriver
//
#define SAMPLE_FREQUENCY 160.0e6

/**
 * Function to convert the complex semi-floating point representation used by the
 * EPA firmware to a complex<double>.
 */
BZ_DECLARE_FUNCTION_RET(convert_to_powerangle, complex<double>)
inline complex<double> convert_to_powerangle(complex<double> val)
{
  double angle = 0.0;
  double power = real(val)*real(val) + imag(val)*imag(val);

  if (power > 0.0)
  {
    power = 12 + 5*log10(power); // adjust scaling to allow comparison to subband statistics
  }

  if (0.0 == real(val))
  {

    if (imag(val) > 0)
      angle = 90.0;
    else if (imag(val) < 0)
      angle = 270;

  }
  else
  {

    angle = 45.0 * atan(imag(val)/real(val)) / atan(1.0);

    if (real(val) > 0.0)
    {
      if (imag(val) < 0)
        angle += 360.0;
    }
    else
      angle += 180.0;

  }

  return complex<double>(power, angle);
}

BZ_DECLARE_FUNCTION_RET(convert_to_powerangle_from_int16, complex<double>)
inline complex<double> convert_to_powerangle_from_int16(complex<int16> int16val)
{
  // scale and convert from int16 to double in range (-1,1]
  complex<double> cdval = complex<double>((double)real(int16val)/(1<<14),
					  (double)imag(int16val)/(1<<14));

  //
  // r   = amplitude
  // phi = angle
  // a   = real part of complex weight
  // b   = imaginary part of complex weight
  //
  // a + ib = r * e^(i * phi)
  // r   = sqrt(a^2 + b^2)
  // phi = atan(b/a)
  //

  return complex<double>(::sqrt(real(cdval)*real(cdval) + imag(cdval)*imag(cdval)),
			 ((::atan(imag(cdval) / real(cdval))) / M_PI) * 180.0); 
}

WeightsCommand::WeightsCommand(GCFPortInterface& port) : Command(port), m_type(WeightsCommand::COMPLEX)
{
}

void WeightsCommand::send()
{
  if (getMode())
  {
    // GET
    RSPGetweightsEvent getweights;

    getweights.timestamp = Timestamp(0,0);
    getweights.rcumask = getRCUMask();
    getweights.cache = true;

    m_rspport.send(getweights);
  }
  else
  {
    // SET
    RSPSetweightsEvent setweights;
    setweights.timestamp = Timestamp(0,0);
    setweights.rcumask = getRCUMask();

    logMessage(cerr,formatString("rcumask.count()=%d",setweights.rcumask.count()));

    setweights.weights().resize(1, setweights.rcumask.count(), MEPHeader::N_BEAMLETS);

    // -1 < m_value <= 1
    complex<double> value = m_value;
    value *= (1<<14); // -.99999 should become -16383 and 1 should become 16384
    setweights.weights() = complex<int16>((int16)value.real(), (int16)value.imag()); // complex<int16>((int16)value,0);

    m_rspport.send(setweights);
  }
}

GCFEvent::TResult WeightsCommand::ack(GCFEvent& e)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case RSP_GETWEIGHTSACK:
    {
      RSPGetweightsackEvent ack(e);
      bitset<MAX_N_RCUS> mask = getRCUMask();

      if (SUCCESS != ack.status) {
	logMessage(cerr,"Error: RSP_GETWEIGHTS command failed.");
	GCFTask::stop();
	return status;
      }

      if (WeightsCommand::COMPLEX == m_type) {
	int rcuin = 0;
	for (int rcuout = 0; rcuout < get_ndevices(); rcuout++)
	{
	  if (mask[rcuout])
	  {
	    std::ostringstream logStream;
	    logStream << ack.weights()(0, rcuin++, Range::all());
	    logMessage(cout,formatString("RCU[%02d].weights=%s", rcuout,logStream.str().c_str()));
	  }
	}
      } else {
	blitz::Array<complex<double>, 3> ackweights;
	ackweights.resize(ack.weights().shape());

	// convert to amplitude and angle
	ackweights = convert_to_powerangle_from_int16(ack.weights());

	int rcuin = 0;
	for (int rcuout = 0; rcuout < get_ndevices(); rcuout++)
	{
	  if (mask[rcuout])
	  {
	    std::ostringstream logStream;
	    logStream << ackweights(0, rcuin++, Range::all());
	    logMessage(cout,formatString("RCU[%02d].weights=%s", rcuout,logStream.str().c_str()));
	  }
	}
      }
    }
    break;

    case RSP_SETWEIGHTSACK:
    {
      RSPSetweightsackEvent ack(e);

      if (SUCCESS != ack.status)
      {
        logMessage(cerr,"Error: RSP_SETWEIGHTS command failed.");
      }
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  GCFTask::stop();

  return status;
}

SubbandsCommand::SubbandsCommand(GCFPortInterface& port) : Command(port), m_type(0)
{
}

void SubbandsCommand::send()
{
  if (getMode())
  {
    // GET
    RSPGetsubbandsEvent getsubbands;

    getsubbands.timestamp = Timestamp(0,0);
    getsubbands.rcumask = getRCUMask();
    getsubbands.cache = true;
    getsubbands.type = m_type;

    m_rspport.send(getsubbands);
  }
  else
  {
    // SET
    RSPSetsubbandsEvent setsubbands;
    setsubbands.timestamp = Timestamp(0,0);
    setsubbands.rcumask = getRCUMask();
    setsubbands.subbands.setType(m_type);

    logMessage(cerr,formatString("rcumask.count()=%d",setsubbands.rcumask.count()));

    // if only 1 subband selected, apply selection to all
    switch (m_type) {

    case SubbandSelection::BEAMLET:
      {
	if (1 == m_subbandlist.size()) {
	  setsubbands.subbands().resize(1, MEPHeader::N_BEAMLETS);
	  std::list<int>::iterator it = m_subbandlist.begin();
	  setsubbands.subbands() = (*it);
	} else {
	  setsubbands.subbands().resize(1, m_subbandlist.size());
	  
	  int i = 0;
	  std::list<int>::iterator it;
	  for (it = m_subbandlist.begin(); it != m_subbandlist.end(); it++, i++)
	    {
	      if (i >= MEPHeader::N_BEAMLETS) break;
	      setsubbands.subbands()(0, i) = (*it);
	    }
	}
      }
      break;

    case SubbandSelection::XLET:
      {
	setsubbands.subbands().resize(1,1);
	std::list<int>::iterator it = m_subbandlist.begin();
	setsubbands.subbands() = (*it);
      }
      break;

    default:
      LOG_FATAL("invalid subbanselection type");
      exit(EXIT_FAILURE);
      break;
    }

    m_rspport.send(setsubbands);
  }
}

GCFEvent::TResult SubbandsCommand::ack(GCFEvent& e)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case RSP_GETSUBBANDSACK:
    {
      RSPGetsubbandsackEvent ack(e);
      bitset<MAX_N_RCUS> mask = getRCUMask();

      std::ostringstream msg;
      msg << "getsubbandsack.timestamp=" << ack.timestamp;
      logMessage(cout, msg.str());

      if (SUCCESS == ack.status)
      {
        int rcuin = 0;
        for (int rcuout = 0; rcuout < get_ndevices(); rcuout++)
        {

          if (mask[rcuout])
          {
            std::ostringstream logStream;
            logStream << ack.subbands()(rcuin++, Range::all());
	    if (SubbandSelection::BEAMLET == m_type) {
	      logMessage(cout,formatString("RCU[%02d].subbands=%s", rcuout,logStream.str().c_str()));
	    } else {
	      logMessage(cout,formatString("RCU[%02d].xcsubbands=%s", rcuout,logStream.str().c_str()));
	    }
          }
        }
      }
      else
      {
        logMessage(cerr,"Error: RSP_GETSUBBANDS command failed.");
      }
    }
    break;

    case RSP_SETSUBBANDSACK:
    {
      RSPSetsubbandsackEvent ack(e);

      std::ostringstream msg;
      msg << "setsubbandsack.timestamp=" << ack.timestamp;
      logMessage(cout, msg.str());

      if (SUCCESS != ack.status)
      {
        logMessage(cerr,"Error: RSP_SETSUBBANDS command failed.");
      }
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  GCFTask::stop();

  return status;
}

RCUCommand::RCUCommand(GCFPortInterface& port) : Command(port)
{
}

void RCUCommand::send()
{
  if (getMode())
  {
    // GET
    RSPGetrcuEvent getrcu;

    getrcu.timestamp = Timestamp(0,0);
    getrcu.rcumask = getRCUMask();
    getrcu.cache = true;

    m_rspport.send(getrcu);
  }
  else
  {
    // SET
    RSPSetrcuEvent setrcu;
    setrcu.timestamp = Timestamp(0,0);
    setrcu.rcumask = getRCUMask();

    setrcu.settings().resize(1);
    setrcu.settings()(0) = m_control;

    for (int i = 0; i < setrcu.settings().extent(firstDim); i++) {
      printf("control(%d)=0x%08x\n", i, setrcu.settings()(i).getRaw());
    }

    m_rspport.send(setrcu);
  }
}

GCFEvent::TResult RCUCommand::ack(GCFEvent& e)
{
  switch (e.signal)
  {
    case RSP_GETRCUACK:
    {
      RSPGetrcuackEvent ack(e);
      bitset<MAX_N_RCUS> mask = getRCUMask();

      if (SUCCESS == ack.status)
      {
        int rcuin = 0;
        for (int rcuout = 0; rcuout < get_ndevices(); rcuout++)
        {

          if (mask[rcuout])
          {
            logMessage(cout,formatString("RCU[%02d].control=0x%08x",rcuout, ack.settings()(rcuin++).getRaw()));
          }
        }
      }
      else
      {
        logMessage(cerr,"Error: RSP_GETRCU command failed.");
      }
    }
    break;

    case RSP_SETRCUACK:
    {
      RSPSetrcuackEvent ack(e);

      if (SUCCESS != ack.status)
      {
        logMessage(cerr,"Error: RSP_SETRCU command failed.");
      }
    }
  }

  GCFTask::stop();

  return GCFEvent::HANDLED;
}

//
// RSUCommand
//
RSUCommand::RSUCommand(GCFPortInterface& port) : Command(port)
{
}

void RSUCommand::send()
{
  if (getMode()) {
    // GET
	// does not exist yet
  }
  else {
    // SET
    RSPSetrsuEvent setrsu;
    setrsu.timestamp = Timestamp(0,0);
    setrsu.rcumask = getRCUMask();

    setrsu.settings().resize(1);
    setrsu.settings()(0) = m_control;

    for (int i = 0; i < setrsu.settings().extent(firstDim); i++) {
      printf("control(%d)=0x%08x\n", i, setrsu.settings()(i).getRaw());
    }

    m_rspport.send(setrsu);
  }
}

GCFEvent::TResult RSUCommand::ack(GCFEvent& e)
{
  switch (e.signal) {
#if 0
    case RSP_GETRSUACK: {
      RSPGetrsuackEvent ack(e);
      bitset<MAX_N_RCUS> mask = getRCUMask();

      if (SUCCESS == ack.status) {
        int boardin = 0;
        for (int boardout = 0; boardout < get_ndevices(); boardout++) {

          if (mask[boardout]) {
            logMessage(cout,formatString("RSU[%02d].control=0x%08x",boardout, ack.settings()(boardin++).getRaw()));
          }
        }
      }
      else {
        logMessage(cerr,"Error: RSP_GETRSU command failed.");
      }
    }
    break;
#endif
    case RSP_SETRSUACK: {
      RSPSetrsuackEvent ack(e);

      if (SUCCESS != ack.status) {
        logMessage(cerr,"Error: RSP_SETRSU command failed.");
      }
    }
  }

  GCFTask::stop();

  return GCFEvent::HANDLED;
}

//
// ClocksCommand
//
ClocksCommand::ClocksCommand(GCFPortInterface& port) : Command(port)
{
}

void ClocksCommand::send()
{
  if (getMode())
  {
    // GET
    RSPGetclocksEvent getclocks;

    getclocks.timestamp = Timestamp(0,0);
    getclocks.rspmask = getRSPMask();
    getclocks.cache = true;

    m_rspport.send(getclocks);
  }
  else
  {
    // SET
    RSPSetclocksEvent setclocks;
    setclocks.timestamp = Timestamp(0,0);

    setclocks.rspmask = getRSPMask();
    setclocks.clocks().resize(1);
    setclocks.clocks()(0) = m_clock;

    m_rspport.send(setclocks);
  }
}

GCFEvent::TResult ClocksCommand::ack(GCFEvent& e)
{
  switch (e.signal)
  {
    case RSP_GETCLOCKSACK:
    {
      RSPGetclocksackEvent ack(e);
      bitset<MAX_N_RSPBOARDS> mask = getRSPMask();

      if (SUCCESS == ack.status)
      {
        int rspin = 0;
        for (int rspout = 0; rspout < get_ndevices(); rspout++)
        {

          if (mask[rspout])
          {
            logMessage(cout,formatString("RSP[%02d] clock=%d",rspout, ack.clocks()(rspin++)));
          }
        }
      }
      else
      {
        logMessage(cerr,"Error: RSP_GETCLOCKS command failed.");
      }
    }
    break;

    case RSP_SETCLOCKSACK:
    {
      RSPSetclocksackEvent ack(e);

      if (SUCCESS != ack.status)
      {
        logMessage(cerr,"Error: RSP_SETCLOCKS command failed.");
      }
    }
  }

  GCFTask::stop();

  return GCFEvent::HANDLED;
}

WGCommand::WGCommand(GCFPortInterface& port) : Command(port),
    m_frequency(0.0),
    m_phase(0),
    m_amplitude(1<<23),
	m_mode(0)
{
}

void WGCommand::send()
{
  if (getMode())
  {
    // GET
    RSPGetwgEvent wgget;
    wgget.timestamp = Timestamp(0,0);
    wgget.rcumask = getRCUMask();
    wgget.cache = true;
    m_rspport.send(wgget);
  }
  else
  {
    // SET
    RSPSetwgEvent wgset;

    wgset.timestamp = Timestamp(0,0);
    wgset.rcumask = getRCUMask();
    wgset.settings().resize(1);
    wgset.settings()(0).freq = (uint32)((m_frequency * ((uint32)-1) / SAMPLE_FREQUENCY) + 0.5);
    wgset.settings()(0).phase = m_phase;
    wgset.settings()(0).ampl = m_amplitude;
    wgset.settings()(0).nof_samples = N_WAVE_SAMPLES;

    if (m_frequency < 1e-6)
    {
      wgset.settings()(0).mode = WGSettings::MODE_OFF;
    }
    else /* frequency ok */
    {
      if (m_mode == 0) { 	/* forget to set mode? assume calc mode */
        wgset.settings()(0).mode = WGSettings::MODE_CALC;
    	wgset.settings()(0).nof_samples = 1024;
      }
	  else {
        wgset.settings()(0).mode = m_mode;
    	wgset.settings()(0).nof_samples = 1024;
      }
    }
    wgset.settings()(0).preset = 0; // or one of PRESET_[SINE|SQUARE|TRIANGLE|RAMP]

    m_rspport.send(wgset);
  }
}

GCFEvent::TResult WGCommand::ack(GCFEvent& e)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case RSP_GETWGACK:
    {
      RSPGetwgackEvent ack(e);

      if (SUCCESS == ack.status)
      {

        // print settings
        bitset<MAX_N_RCUS> mask = getRCUMask();
        int rcuin = 0;
        for (int rcuout = 0; rcuout < get_ndevices(); rcuout++)
        {

          if (mask[rcuout])
          {
            logMessage(cout,formatString("RCU[%02d].wg=[freq=%6d, phase=%3d(%5.3f), ampl=%3d, nof_samples=%6d, mode=%3d]",
                   rcuout,
                   ack.settings()(rcuin).freq,
                   ack.settings()(rcuin).phase,
                   (double)ack.settings()(rcuin).phase / 256 * 2 * M_PI,
                   ack.settings()(rcuin).ampl,
                   ack.settings()(rcuin).nof_samples,
                   ack.settings()(rcuin).mode));
            rcuin++;
          }
        }
      }
      else
      {
        logMessage(cerr,"Error: RSP_GETWG command failed.");
      }
    }
    break;

    case RSP_SETWGACK:
    {
      RSPSetwgackEvent ack(e);

      if (SUCCESS != ack.status)
      {
        logMessage(cerr,"Error: RSP_SETWG command failed.");
      }
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  GCFTask::stop();

  return status;
}

StatusCommand::StatusCommand(GCFPortInterface& port) : 
	Command(port),
	m_board()
{
}

void StatusCommand::send()
{
	if (getMode()) { // GET
		RSPGetstatusEvent getstatus;

		getstatus.timestamp = Timestamp(0,0);
		getstatus.rcumask = getRCUMask();
		getstatus.cache = true;

		m_rspport.send(getstatus);
	}
	else { // SET
		logMessage(cerr,"Setting status not yet allowed");
	}
}

GCFEvent::TResult StatusCommand::ack(GCFEvent& event)
{
	switch (event.signal) {
	case RSP_GETSTATUSACK: {
		RSPGetstatusackEvent ack(event);
		bitset<MAX_N_RCUS> mask = getRCUMask();

		if (ack.status != SUCCESS) {
			logMessage(cerr,"Error: RSP_GETSTATUS command failed.");
			break;
		}

		BoardStatus&	board = ack.sysstatus.board()(0);
		logMessage(cout,formatString("1.2 V: %3.2f , 2.5 V: %3.2f",
						0.013  * board.rsp.voltage_1_5, 
						0.0172 * board.rsp.voltage_3_3));
		logMessage(cout,formatString("BP_temp: %2d , BP_clock: %3d",
						board.rsp.bp_temp, board.rsp.bp_clock));
		logMessage(cout,formatString("Temp AP0: %3d , AP1: %3d , AP2: %3d , AP3: %3d", 
						board.rsp.ap0_temp, board.rsp.ap1_temp, 
						board.rsp.ap2_temp, board.rsp.ap3_temp));
		logMessage(cout,formatString("Ethernet nr frames: %ld , nr errors: %ld , last error: %d",
						board.eth.nof_frames, board.eth.nof_errors,
						board.eth.last_error));
		logMessage(cout,formatString("MEP sequencenr: %ld , error: %d",
						board.mep.seqnr, board.mep.error));
		logMessage(cout,formatString("Errors ri: %5d ,  rcuX: %5d ,  rcuY: %5d,   lcu: %5d,    cep: %5d",
						board.diag.ri_errors, board.diag.rcux_errors,
						board.diag.rcuy_errors, board.diag.lcu_errors,
					 	board.diag.cep_errors));
		logMessage(cout,formatString("   serdes: %5d , ap0ri: %5d , ap1ri: %5d, ap2ri: %5d , ap3ri: %5d",
						board.diag.serdes_errors, board.diag.ap0_ri_errors,
						board.diag.ap1_ri_errors, board.diag.ap2_ri_errors,
						board.diag.ap3_ri_errors));
		logMessage(cout,formatString("Sync     diff     count   samples    slices"));
		for (int blp = 0; blp < 4; blp++) {
//			BSStatus*	bs= &(board.ap0_sync)+(blp*sizeof(BSStatus));
			BSStatus*	bs= &(board.ap0_sync)+blp;
			logMessage(cout,formatString("%d:  %9ld %9ld %9ld %9ld", 
						blp, bs->ext_count, bs->sync_count,
						bs->sample_offset, bs->slice_count));
		}
		logMessage(cout,formatString("Status   pllX      pllY overflowX overflowY"));
		for (int ap = 0; ap < 4; ap++) {
//			APStatus*	as= &(board.ap0_rcu)+(ap*sizeof(APStatus));
			APStatus*	as= &(board.ap0_rcu)+ap;
			logMessage(cout,formatString("%d:  %9ld %9ld %9ld %9ld", 
								ap, as->pllx, as->plly, 
								as->nof_overflowx, as->nof_overflowy));
		}
	}
	break;
	}

	GCFTask::stop();
	return GCFEvent::HANDLED;
}

StatisticsBaseCommand::StatisticsBaseCommand(GCFPortInterface& port) : FECommand(port),
  m_subscriptionHandle(0),
  m_duration(0),
  m_endTime(),
  m_integration(1),
  m_nseconds(0),
  m_directory(""),
  m_file(0)
{
}

StatisticsCommand::StatisticsCommand(GCFPortInterface& port) : StatisticsBaseCommand(port),
  m_type(Statistics::SUBBAND_POWER),
  m_stats()
{
}

void StatisticsCommand::send()
{
  if (getMode())
  {
    if(m_directory.length()>0)
    {
      logMessage(cout,formatString("Dumping statistics in %s",m_directory.c_str()));
    }
    else
    {
      char cwd[PATH_MAX];
      logMessage(cout,formatString("Dumping statistics in %s",getcwd(cwd,PATH_MAX)));
    }
  
    // SUBSCRIBE
    RSPSubstatsEvent substats;

    substats.timestamp = Timestamp(0,0);
    substats.rcumask = getRCUMask();
    substats.period = 1;
    substats.type = m_type;
    substats.reduction = SUM;

    m_rspport.send(substats);
  }
  else
  {
    // SET
    logMessage(cerr,"Error: set mode not support for option '--statistics'");
    GCFTask::stop();
  }
}

void StatisticsCommand::stop()
{
  if (getMode())
  {
    // UNSUBSCRIBE
    RSPUnsubstatsEvent unsubstats;
    unsubstats.handle = m_subscriptionHandle;
    m_rspport.send(unsubstats);
  }
}

void StatisticsCommand::capture_statistics(Array<double, 2>& stats, const Timestamp& timestamp)
{
  if (0 == m_nseconds)
  {
    // initialize values array
    m_stats.resize(stats.shape());
    m_stats = 0.0;
  }
  else
  {
    if ( sum(stats.shape()) != sum(m_stats.shape()) )
    {
      logMessage(cerr, "Error: statistics shape mismatch");
      exit(EXIT_FAILURE);
    }
  }

  m_stats += stats;
  m_nseconds++; // advance to next second

  if (0 == m_nseconds % m_integration)
  {
    if (m_integration > 0) 
    {
      m_stats /= m_integration;
    }

    std::ostringstream logStream;
    logStream << "statistics update at " << timestamp;
    logMessage(cout,logStream.str());
    
    if(m_duration == 0)
    {
      plot_statistics(m_stats, timestamp);
    }
    else
    {
      dump_statistics(m_stats, timestamp);
      
      Timestamp timeNow;
      timeNow.setNow();
      if(timeNow >= m_endTime)
      {
        logMessage(cout,"Statistics capturing successfully ended.");
        stop();
        GCFTask::stop();
      }
    }
    
    m_stats = 0.0; //reset statistics
  }
}
  
void StatisticsCommand::plot_statistics(Array<double, 2>& stats, const Timestamp& timestamp)
{
  static gnuplot_ctrl* handle = 0;
  int n_freqbands = stats.extent(secondDim);
  bitset<MAX_N_RCUS> mask = getRCUMask();

  // initialize the freq array
  firstIndex i;

  if (!handle)
  {
    handle = gnuplot_init();
    if (!handle)
      return;

    //gnuplot_setstyle(handle, "steps");
    gnuplot_cmd(handle, "set grid x y");
  }

  gnuplot_cmd(handle, "set ylabel \"dB\"");
  gnuplot_cmd(handle, "set yrange [0:140]");

  switch (m_type)
  {
    case Statistics::SUBBAND_POWER:
      gnuplot_cmd(handle, "set xrange [0:%f]", SAMPLE_FREQUENCY / 2.0);
      break;
    case Statistics::BEAMLET_POWER:
      gnuplot_cmd(handle, "set xrange [0:%d]", MEPHeader::N_BEAMLETS);
      break;
  }

  char plotcmd[2048];
  time_t seconds = timestamp.sec();
  strftime(plotcmd, 255, "set title \"%s - %a, %d %b %Y %H:%M:%S  %z\"", gmtime(&seconds));

  gnuplot_cmd(handle, plotcmd);

  strcpy(plotcmd, "plot ");

  int count = 0;
  for (int rcuout = 0; rcuout < get_ndevices(); rcuout++)
  {
    if (mask[rcuout])
    {
      if (count > 0)
        strcat(plotcmd, ",");
      count++;

      switch (m_type)
      {
        case Statistics::SUBBAND_POWER:
	  gnuplot_cmd(handle, "set xlabel \"Frequency (MHz)\"");
          snprintf(plotcmd + strlen(plotcmd), 128, "\"-\" using (%.1f/%.1f*$1):(10*log10($2)) title \"(RCU=%d)\" with steps ",
                   SAMPLE_FREQUENCY, n_freqbands*2.0, rcuout);
          break;
        case Statistics::BEAMLET_POWER:
	  gnuplot_cmd(handle, "set xlabel \"Beamlet index\"");
          snprintf(plotcmd + strlen(plotcmd), 128, "\"-\" using (1.0*$1):(10*log10($2)) title \"Beamlet Power (RSP board %d, %c)\" with steps ",
		   (rcuout/2), (rcuout%2?'Y':'X'));
          break;
        default:
          logMessage(cerr,"Error: invalid m_type");
          exit(EXIT_FAILURE);
          break;
      }
    }
  }

  gnuplot_cmd(handle, plotcmd);

  gnuplot_write_matrix(handle, stats);

  //gnuplot_cmd(handle, "set nomultiplot");
}

void StatisticsCommand::dump_statistics(Array<double, 2>& stats, const Timestamp& timestamp)
{
  bitset<MAX_N_RCUS> mask = getRCUMask();

  int result_rcu=0;
  for (int rcuout = 0; rcuout < get_ndevices(); rcuout++)
  {
    if (mask[rcuout])
    {
      char timestring[256];
      time_t seconds = timestamp.sec();
      strftime(timestring, 255, "%Y%m%d_%H%M%S", gmtime(&seconds));
      char fileName[PATH_MAX];
      switch (m_type)
      {
        case Statistics::SUBBAND_POWER:
          snprintf(fileName, PATH_MAX, "%s%s_sst_rcu%03d.dat", m_directory.c_str(), timestring, rcuout);
          break;
        case Statistics::BEAMLET_POWER:
          snprintf(fileName, PATH_MAX, "%s%s_bst_rcu%03d.dat", m_directory.c_str(), timestring, rcuout);
          break;
      
        default:
          logMessage(cerr,"Error: invalid m_type");
          exit(EXIT_FAILURE);
          break;
      }
      FILE* file = getFile(rcuout,fileName);
      if (stats.extent(secondDim)
          != (int)fwrite(stats(result_rcu, Range::all()).data(), sizeof(double),
             stats.extent(secondDim), file))
      {
        logMessage(cerr,formatString("Error: unable to write to file %s",fileName));
        exit(EXIT_FAILURE);
      }
    }
    result_rcu++;
  }
}

GCFEvent::TResult StatisticsCommand::ack(GCFEvent& e)
{
  if (e.signal == RSP_SUBSTATSACK)
  {
    RSPSubstatsackEvent ack(e);

    if (SUCCESS != ack.status)
    {
      logMessage(cerr,"failed to subscribe to statistics");
      exit(EXIT_FAILURE);
    }

    return GCFEvent::HANDLED;
  }

  if (e.signal != RSP_UPDSTATS)
    return GCFEvent::NOT_HANDLED;

  RSPUpdstatsEvent upd(e);

  if (SUCCESS == upd.status)
  {
    capture_statistics(upd.stats(),upd.timestamp);
  }
  else
  {
    logMessage(cerr,"Error: statistics update failed.");
  }

  return GCFEvent::HANDLED;
}

XCStatisticsCommand::XCStatisticsCommand(GCFPortInterface& port) : StatisticsBaseCommand(port),
  m_stats()
{
}

void XCStatisticsCommand::send()
{
  if (getMode())
  {
    if(m_directory.length()>0)
    {
      logMessage(cout,formatString("Dumping statistics in %s",m_directory.c_str()));
    }
    else
    {
      char cwd[PATH_MAX];
      logMessage(cout,formatString("Dumping statistics in %s",getcwd(cwd,PATH_MAX)));
    }
  
    // SUBSCRIBE
    RSPSubxcstatsEvent subxcstats;

    subxcstats.timestamp = Timestamp(0,0);
    subxcstats.rcumask = getRCUMask();
    subxcstats.period = 1;

    m_rspport.send(subxcstats);
  }
  else
  {
    // SET
    logMessage(cerr,"Error: set mode not support for option '--xcstatistics'");
    GCFTask::stop();
  }
}

void XCStatisticsCommand::stop()
{
  if (getMode())
  {
    // UNSUBSCRIBE
    RSPUnsubxcstatsEvent unsubxcstats;
    unsubxcstats.handle = m_subscriptionHandle;
    m_rspport.send(unsubxcstats);
  }
}

void XCStatisticsCommand::capture_xcstatistics(Array<complex<double>, 4>& stats, const Timestamp& timestamp)
{
  if (0 == m_nseconds)
  {
    // initialize values array
    m_stats.resize(stats.shape());
    m_stats = 0.0;
  }
  else
  {
    if ( sum(stats.shape()) != sum(m_stats.shape()) )
    {
      logMessage(cerr, "Error: xcstatistics shape mismatch");
      exit(EXIT_FAILURE);
    }
  }

  m_stats += stats;
  m_nseconds++; // advance to next second

  if (0 == m_nseconds % m_integration)
  {
    if (m_integration > 0) 
    {
      m_stats /= m_integration;
    }

    std::ostringstream logStream;
    logStream << "xcstatistics update at " << timestamp;
    logMessage(cout,logStream.str());
    
    if(m_duration == 0)
    {
      blitz::Array<complex<double>, 4> pastats;
      pastats.resize(m_stats.shape());
      pastats = convert_to_powerangle(m_stats);

      for (int i = 0; i < pastats.extent(firstDim) * pastats.extent(thirdDim); i++) {

	string logString;
	for (int j = 0; j < pastats.extent(secondDim) * pastats.extent(fourthDim); j++) {
	  logString += string(formatString("%3.0f:%03.0f ", real(pastats(i%2,j%2,i/2,j/2)), imag(pastats(i%2,j%2,i/2,j/2))));
	}
	logMessage(cout,logString);
      }

      plot_xcstatistics(m_stats, timestamp);
    }
    else
    {
      dump_xcstatistics(m_stats, timestamp);
      
      Timestamp timeNow;
      timeNow.setNow();
      if(timeNow >= m_endTime)
      {
        logMessage(cout,"XCStatistics capturing successfully ended.");
        stop();
        GCFTask::stop();
      }
    }
    
    m_stats = 0.0; //reset statistics
  }
}
  
void XCStatisticsCommand::plot_xcstatistics(Array<complex<double>, 4>& xcstats, const Timestamp& timestamp)
{
  static gnuplot_ctrl* handle = 0;
  int n_ant = xcstats.extent(thirdDim);

  // initialize the freq array
  firstIndex i;

  if (!handle)
  {
    handle = gnuplot_init();
    if (!handle)
      return;

    //gnuplot_setstyle(handle, "steps");
    gnuplot_cmd(handle, "set grid x y");
  }

  //gnuplot_cmd(handle, "set xlabel \"Frequency (MHz)\" 0, 1.5");
  gnuplot_cmd(handle, "set xlabel \"RCU\"");
  gnuplot_cmd(handle, "set ylabel \"RCU\"");
  gnuplot_cmd(handle, "set xrange [0:%d]", n_ant-1);
  gnuplot_cmd(handle, "set yrange [0:%d]", n_ant-1);

  char plotcmd[2048];
  time_t seconds = timestamp.sec();
  strftime(plotcmd, 255, "set title \"%s - %a, %d %b %Y %H:%M:%S  %z\"", gmtime(&seconds));

  gnuplot_cmd(handle, plotcmd);

  gnuplot_cmd(handle,
              "set view 0,0\n"
              "set ticslevel 0\n"
              "unset xtics\n"
              "unset ytics\n"
              "unset colorbox\n"
              "set key off\n"
              "set border 0\n");

  gnuplot_cmd(handle, "splot \"-\" matrix with points ps 12 pt 5 palette");

  //xcstats = 10.0*log(real(xcstats)*real(xcstats))/log(10.0);

  gnuplot_write_matrix(handle, real(xcstats(0,0,Range::all(),Range::all())), true);
}

void XCStatisticsCommand::dump_xcstatistics(Array<complex<double>, 4>& stats, const Timestamp& timestamp)
{
  Array<complex<double>, 2> thestats;

  thestats.resize(stats.extent(firstDim) * stats.extent(thirdDim),
      stats.extent(secondDim) * stats.extent(fourthDim));

  for (int i = 0; i < thestats.extent(firstDim); i++)
    for (int j = 0; j < thestats.extent(secondDim); j++)
      thestats(i,j) = stats(i % 2, j % 2, i/2, j/2);

  char timestring[256];
  time_t seconds = timestamp.sec();
  strftime(timestring, 255, "%Y%m%d_%H%M%S", gmtime(&seconds));
  char fileName[PATH_MAX];
  snprintf(fileName, PATH_MAX, "%s%s_xst.dat", m_directory.c_str(), timestring);
  FILE* file = getFile(0,fileName);

  if (thestats.size()
      != (int)fwrite(thestats.data(), sizeof(complex<double>),
         thestats.size(), file))
  {
    logMessage(cerr,formatString("Error: unable to write to file %s",fileName));
    exit(EXIT_FAILURE);
  }
}

GCFEvent::TResult XCStatisticsCommand::ack(GCFEvent& e)
{
  if (e.signal == RSP_SUBXCSTATSACK)
  {
    RSPSubxcstatsackEvent ack(e);

    if (SUCCESS != ack.status)
    {
      logMessage(cerr,"failed to subscribe to xcstatistics");
      exit(EXIT_FAILURE);
    }
    else
    {
      m_subscriptionHandle = ack.handle;
    }

    return GCFEvent::HANDLED;
  }

  if (e.signal != RSP_UPDXCSTATS)
    return GCFEvent::NOT_HANDLED;

  RSPUpdxcstatsEvent upd(e);

  if (SUCCESS == upd.status)
  {
    capture_xcstatistics(upd.stats(),upd.timestamp);
  }

  return GCFEvent::HANDLED;
}

VersionCommand::VersionCommand(GCFPortInterface& port) : Command(port)
{
}

void VersionCommand::send()
{
  RSPGetversionEvent getversion;

  getversion.timestamp = Timestamp(0,0);
  getversion.cache = true;

  m_rspport.send(getversion);
}

GCFEvent::TResult VersionCommand::ack(GCFEvent& e)
{
  RSPGetversionackEvent ack(e);

  if (SUCCESS == ack.status)
  {
    for (int rsp=0; rsp < get_ndevices(); rsp++)
    {
      logMessage(cout,formatString("RSP[%02d] BP version = %d.%d, AP version = %d.%d",
                                   rsp,
                                   ack.versions.bp()(rsp).fpga_maj,
				   ack.versions.bp()(rsp).fpga_min,
                                   ack.versions.ap()(rsp).fpga_maj,
				   ack.versions.ap()(rsp).fpga_min));
    }
  }
  else
  {
    logMessage(cerr,"Error: RSP_GETVERSION command failed.");
  }
  GCFTask::stop();

  return GCFEvent::HANDLED;
}

RSPCtl::RSPCtl(string name, int argc, char** argv)
    : GCFTask((State)&RSPCtl::initial, name), m_command(0),
    m_nrcus(0), m_nrspboards(0), m_argc(argc), m_argv(argv)
{
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);
#ifdef ENABLE_RSPFE
  registerProtocol(RSPFE_PROTOCOL, RSPFE_PROTOCOL_signalnames);
#endif

  m_server.init(*this, "server", GCFPortInterface::SAP, RSP_PROTOCOL);
}

RSPCtl::~RSPCtl()
{
  if (m_command)
    delete m_command;
}

GCFEvent::TResult RSPCtl::initial(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch(e.signal)
  {
    case F_INIT:
    break;

    case F_ENTRY:
    {
      if (!m_server.isConnected())
        m_server.open();
    }
    break;

    case F_CONNECTED:
    {
      if (m_server.isConnected())
      {
        RSPGetconfigEvent getconfig;
        m_server.send(getconfig);
      }
    }
    break;

    case RSP_GETCONFIGACK:
    {
      RSPGetconfigackEvent ack(e);
      m_nrcus = ack.n_rcus;
      m_nrspboards = ack.n_rspboards;
      logMessage(cout,formatString("n_rcus     =%d",m_nrcus));
      logMessage(cout,formatString("n_rspboards=%d",m_nrspboards));
      TRAN(RSPCtl::docommand);
    }
    break;

    case F_DISCONNECTED:
    {
      port.setTimer((long)1);
      port.close();
    }
    break;

    case F_TIMER:
    {
      // try again
      m_server.open();
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult RSPCtl::docommand(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
    {
      if (0 == (m_command = parse_options(m_argc, m_argv)))
      {
        logMessage(cerr,"Warning: no command specified.");
        exit(EXIT_FAILURE);
      }
      // check if a connection must be made with a frontend. If so, connect first
      // and send the command to the rspdriver when connected with the frontend
      FECommand* feCommand = dynamic_cast<FECommand*>(m_command);
      if(feCommand != 0)
      {
        if(feCommand->isFrontEndSet())
        {
          feCommand->connect(*this);
        }
        else
        {
          m_command->send();
        }
      }
      else
      {
        m_command->send();
      }
    }
    break;

    case F_CONNECTED:
    {
      // connection with te frontend! send the command to the rsp driver
      FECommand* feCommand = dynamic_cast<FECommand*>(m_command);
      if(feCommand != 0)
      {
        if(feCommand->isConnected(port))
        {
          m_command->send();
        }
      }
    }
    break;
    
    case F_DISCONNECTED:
    {
      port.close();
      logMessage(cerr,formatString("Error: port '%s' disconnected.",port.getName().c_str()));
      exit(EXIT_FAILURE);
    }
    break;

    case RSP_GETRCUACK:
    case RSP_SETRCUACK:
    case RSP_SETRSUACK:
    case RSP_GETSTATSACK:
    case RSP_SUBSTATSACK:
    case RSP_UPDSTATS:
    case RSP_SUBXCSTATSACK:
    case RSP_UPDXCSTATS:
    case RSP_GETVERSIONACK:
    case RSP_GETSUBBANDSACK:
    case RSP_SETSUBBANDSACK:
    case RSP_SETWEIGHTSACK:
    case RSP_GETWEIGHTSACK:
    case RSP_GETWGACK:
    case RSP_SETWGACK:
    case RSP_SETCLOCKSACK:
    case RSP_GETCLOCKSACK:
    case RSP_GETSTATUSACK:
      status = m_command->ack(e); // handle the acknowledgement
      break;
      
#ifdef ENABLE_RSPFE
    case RSPFE_STOP_RSPCTL:
      logMessage(cout,"Rspctl stopped by frontend.");
      m_command->stop();
      GCFTask::stop();
      break;
#endif

    default:
      logMessage(cerr,"Error: unhandled event.");
//      GCFTask::stop(); ignore
      break;
  }

  return status;
}

void RSPCtl::mainloop()
{
  start(); // make initial transition
  GCFTask::run();
}

static void usage()
{
  cout << "rspctl usage:" << endl;
  cout << endl;
  cout << "rspctl --weights        [--select=<set>]  # get weights as complex values" << endl;
  cout << "  Example --select sets: --select=1,2,4:7 or --select=1:3,5:7" << endl;
  cout << "rspctl --weights=value.re[,value.im] [--select=<set>]  # set weights as complex value" << endl;
  cout << "rspctl --aweights       [--select=<set>]  # get weights as power and angle (in degrees)" << endl;
  cout << "rspctl --aweights=amplitude[,angle] [--select=<set>]  # set weights as amplitude and angle (in degrees)" << endl;
  cout << "rspctl --subbands       [--select=<set>]  # get subband selection" << endl;
  cout << "rspctl --subbands=<set> [--select=<set>]  # set subband selection" << endl;
  cout << "  Example --subbands sets: --subbands=0:39 or --select=0:19,40:59" << endl;
  cout << "rspctl --rcu            [--select=<set>]  # show current rcu control setting" << endl;
  cout << "rspctl --rcu=0x00000000 [--select=<set>]  # set the rcu control registers" << endl;
  cout << "     mask      value    " << endl;
  cout << "  0x0000007F INPUT_DELAY Sample delay for the data from the RCU." << endl;
  cout << "  0x00000080 SPEC_INV    Enable spectral inversion (1) if needed." << endl;
  cout << endl;
  cout << "  0x00000100 LBL-EN      supply LBL antenna on (1) or off (0)" << endl;
  cout << "  0x00000200 LBH-EN      sypply LBH antenna on (1) or off (0)" << endl;
  cout << "  0x00000400 HB-EN       supply HB on (1) or off (0)" << endl;
  cout << "  0x00000800 BANDSEL     low band (1) or high band (0)" << endl;
  cout << "  0x00001000 HB-SEL-0    HBA filter selection" << endl;
  cout << "  0x00002000 HB-SEL-1    HBA filter selection" << endl;
  cout << "               Options : HBA-SEL-0 HBA-SEL-1 Function" << endl;
  cout << "                             0          0      210-270 MHz" << endl;
  cout << "                             0          1      170-230 MHz" << endl;
  cout << "                             1          0      110-190 MHz" << endl;
  cout << "                             1          1      all off" << endl;
  cout << "  0x00004000 VL-EN       low band supply on (1) or off (0)" << endl;
  cout << "  0x00008000 VH-EN       high band supply on (1) or off (0)" << endl;
  cout << endl;
  cout << "  0x00010000 VDIG-EN     ADC supply on (1) or off (0)" << endl;
  cout << "  0x00020000 LB-SEL-0    LBA input selection" << endl;
  cout << "  0x00040000 LB-SEL-1    HP filter selection" << endl;
  cout << "               Options : LB-SEL-0 LB-SEL-1 Function" << endl;
  cout << "                             0        0    10-90 MHz + 10 MHz HPF" << endl;
  cout << "                             0        1    30-80 MHz + 10 MHz HPF" << endl;
  cout << "                             1        0    10-90 MHz + 30 MHz HPF" << endl;
  cout << "                             1        1    30-80 MHz + 30 MHz HPF" << endl;
  cout << "  0x00080000 ATT-CNT-4   on (1) is  1dB attenuation" << endl;
  cout << "  0x00100000 ATT-CNT-3   on (1) is  2dB attenuation" << endl;
  cout << "  0x00200000 ATT-CNT-2   on (1) is  4dB attenuation" << endl;
  cout << "  0x00300000 ATT-CNT-1   on (1) is  8dB attenuation" << endl;
  cout << "  0x00800000 ATT-CNT-0   on (1) is 16dB attenuation" << endl;
  cout << endl;
  cout << "  0x01000000 PRSG        pseudo random sequence generator on (1), off (0)" << endl;
  cout << "  0x02000000 RESET       on (1) hold board in reset" << endl;
  cout << "  0xFC000000 TBD         reserved" << endl;
  cout << endl;
  cout << "rspctl [ --rcumode        |" << endl;
  cout << "         --rcuprsg        |" << endl;
  cout << "         --rcureset       |" << endl;
  cout << "         --rcuattenuation |" << endl;
  cout << "         --rcuspecinv     |" << endl;
  cout << "         --rcudelay" << endl;
  cout << "       ]+ [--select=<set>] # control RCU by combining any of these commands with selection" << endl;
  cout << endl;
  cout << "       --rcumode=[0..7] # set the RCU in a specific mode" << endl;
  cout << "         Possible values: 0 = OFF" << endl;
  cout << "                          1 = LBL 10MHz HPF 0x00017900" << endl;
  cout << "                          2 = LBL 30MHz HPF 0x00057900" << endl;
  cout << "                          3 = LBH 10MHz HPF 0x00037A00" << endl;
  cout << "                          4 = LBH 30MHz HPF 0x00077A00" << endl;
  cout << "                          5 = HB 110-190MHz 0x0007A400" << endl;
  cout << "                          6 = HB 170-230MHz 0x00079400" << endl;
  cout << "                          7 = HB 210-270MHz 0x00078400" << endl;
  cout << "       --rcuprsg                 # turn psrg on" << endl;
  cout << "       --rcureset                # hold rcu in reset" << endl;
  cout << "       --rcuattenuation=[0..127] # set the RCU attenuation" << endl;
  cout << "       --rcuspecinv              # enable spectral inversion" << endl;
  cout << "       --rcudelay=[0..31]        # set the delay for rcu's" << endl;
  cout << endl;
  cout << "rspctl --wg                  [--select=<set>]  # get waveform generator settings" << endl;
  cout << "rspctl --wg=freq [--phase=..][--select=<set>]  # set waveform generator settings" << endl;
  cout << "rspctl --status         [--select=<set>]  # get status" << endl;
  cout << "rspctl --statistics[=(subband|beamlet)]   # get subband (default) or beamlet statistics" << endl;
  cout << "             [--select=<set>]             #" << endl;
  cout << "             [--duration=<seconds>]       #" << endl;
  cout << "             [--integration=<seconds>]    #" << endl;
  cout << "             [--directory=<directory>]    #" << endl;
#ifdef ENABLE_RSPFE
  cout << "             [--feport=<hostname>:<port>] #" << endl;
#endif
  cout << "rspctl --xcstatistics   [--select=<set>]  # get crosscorrelation statistics" << endl;
  cout << "             [--duration=<seconds>]       #" << endl;
  cout << "             [--integration=<seconds>]    #" << endl;
  cout << "             [--directory=<directory>]    #" << endl;
#ifdef ENABLE_RSPFE
  cout << "             [--feport=<hostname>:<port>] #" << endl;
#endif
  cout << "rspctl --xcsubband                        # get the subband selection for cross correlation" << endl;
  cout << "rspctl --xcsubband=<int>                  # set the subband to cross correlate" << endl;
  cout << "rspctl --clocks=<int>    [--select=<set>] # get or set the clock frequency of clocks in Hz" << endl;
  cout << "rspctl --version         [--select=<set>] # get version information" << endl;
  cout << "rspctl --rspclear        [--select=<set>] # clear FPGA registers on RSPboard" << endl;
}

Command* RSPCtl::parse_options(int argc, char** argv)
{
  Command*    command        = 0;
  RCUCommand* rcumodecommand = 0;
  list<int> select;

  // select all by default
  select.clear();
  for (int i = 0; i < MAX_N_RCUS; ++i)
    {
      select.push_back(i);
    }

  optind = 0; // reset option parsing
  //opterr = 0; // no error reporting to stderr
  while (1)
    {
      static struct option long_options[] =
	{
	  { "select",         required_argument, 0, 'l' },
	  { "weights",        optional_argument, 0, 'w' },
	  { "aweights",       optional_argument, 0, 'a' },
	  { "subbands",       optional_argument, 0, 's' },
	  { "rcu",            optional_argument, 0, 'r' },
	  { "rcumode",        required_argument, 0, 'm' },
	  { "rcuprsg",        no_argument,       0, 'p' },
	  { "rcureset",       no_argument,       0, 'e' },
	  { "rcuattenuation", required_argument, 0, 'n' },
	  { "rcuspecinv",     no_argument,       0, 'u' },
	  { "rcudelay",       required_argument, 0, 'y' },
	  { "wg",             optional_argument, 0, 'g' },
	  { "wgmode",         required_argument, 0, 'G' },
	  { "phase",          required_argument, 0, 'P' },
	  { "status",         no_argument,       0, 'q' },
	  { "statistics",     optional_argument, 0, 't' },
	  { "xcstatistics",   no_argument,       0, 'x' },
	  { "xcsubband",      optional_argument, 0, 'z' },
	  { "clocks",         optional_argument, 0, 'c' },
	  { "version",        no_argument,       0, 'v' },
//	  { "rspreset",       optional_argument, 0, 'R' },
	  { "rspclear",       optional_argument, 0, 'C' },
	  { "help",           no_argument,       0, 'h' },
#ifdef ENABLE_RSPFE
	  { "feport",         required_argument, 0, 'f' },
#endif
	  { "duration",       required_argument, 0, 'd' },
	  { "integration",    required_argument, 0, 'i' },
	  { "directory"  ,    required_argument, 0, 'D' },

	  { 0, 0, 0, 0 },
	};

      int option_index = 0;
      int c = getopt_long(argc, argv,
			  "l:w::a::s::r::g::qt::xz::vc::hf:d:i:", long_options, &option_index);

      if (c == -1)
	break;

      switch (c)
	{
	case 'l': 	// --select
	  if (optarg)
	    {
	      if (!command || 0 == command->get_ndevices())
		{
		  logMessage(cerr,"Error: 'command' argument should come before --select argument");
		  exit(EXIT_FAILURE);
		}
	      select = strtolist(optarg, command->get_ndevices());
	      if (select.empty())
		{
		  logMessage(cerr,"Error: invalid or missing '--select' option");
		  exit(EXIT_FAILURE);
		}
	    }
	  else
	    {
	      logMessage(cerr,"Error: option '--select' requires an argument");
	    }
	  break;

	case 'w':	// --weights
	  {
	    if (command)
	      delete command;
	    WeightsCommand* weightscommand = new WeightsCommand(m_server);
	    weightscommand->setType(WeightsCommand::COMPLEX);
	    command = weightscommand;

	    command->set_ndevices(m_nrcus);

	    if (optarg)
	      {
		weightscommand->setMode(false);
		double re = 0.0, im = 0.0;
		int numitems = sscanf(optarg, "%lf,%lf", &re, &im);
		if (numitems == 0 || numitems == EOF) {
		  logMessage(cerr,"Error: invalid weights value. Should be of the format "
			     "'--weights=value.re[,value.im]' where value is a floating point value in the range (-1,1].");
		  exit(EXIT_FAILURE);
		}
		weightscommand->setValue(complex<double>(re,im));
	      }
	  }
	  break;

	case 'a':	// --aweights
	  {
	    if (command)
	      delete command;
	    WeightsCommand* weightscommand = new WeightsCommand(m_server);
	    weightscommand->setType(WeightsCommand::ANGLE);
	    command = weightscommand;

	    command->set_ndevices(m_nrcus);

	    if (optarg)
	      {
		weightscommand->setMode(false);
		double amplitude = 0.0, angle = 0.0;
		int numitems = sscanf(optarg, "%lf,%lf", &amplitude, &angle);
		if (numitems == 0 || numitems == EOF) {
		  logMessage(cerr,"Error: invalid aweights value. Should be of the format "
			     "'--weights=amplitude[,angle]' where angle is in degrees.");
		  exit(EXIT_FAILURE);
		}
	  
		if (angle < -180.0 || angle > 180.0) {
		  logMessage(cerr, "Error: invalid angle, should be between -180 < angle < 180.0.");
		  exit(EXIT_FAILURE);
		}
	  
		//weightscommand->setValue(complex<double>(amplitude * ::cos(angle), amplitude * ::sin(angle)));
		weightscommand->setValue(amplitude * exp(complex<double>(0,angle / 180.0 * M_PI)));
	      }
	  }
	  break;

	case 's':	// --subbands
	  {
	    if (command)
	      delete command;
	    SubbandsCommand* subbandscommand = new SubbandsCommand(m_server);
	    subbandscommand->setType(SubbandSelection::BEAMLET);

	    command = subbandscommand;
	    command->set_ndevices(m_nrcus);

	    if (optarg)
	      {
		subbandscommand->setMode(false);
		list<int> subbandlist = strtolist(optarg, MEPHeader::N_SUBBANDS);
		if (subbandlist.empty())
		  {
		    logMessage(cerr,"Error: invalid or empty '--subbands' option");
		    exit(EXIT_FAILURE);
		  }
		subbandscommand->setSubbandList(subbandlist);
	      }
	  }
	  break;

	case 'r': // --rcu
	  {
	    if (command)
	      delete command;
	    RCUCommand* rcucommand = new RCUCommand(m_server);
	    command = rcucommand;

	    command->set_ndevices(m_nrcus);

	    if (optarg)
	      {
		rcucommand->setMode(false);
		unsigned long controlopt = strtoul(optarg, 0, 0);
		if ( controlopt > 0xFFFFFFFF )
		  {
		    logMessage(cerr,"Error: option '--rcu' parameter must be < 0xFFFFFFFF");
		    delete command;
		    return 0;
		  }

		rcucommand->control().setRaw((uint32)controlopt);
	      }
	  }
	  break;

	case 'm': // --rcumode
	case 'p': // --rcuprsg
	case 'e': // --rcureset
	case 'n': // --rcuattenuation
	case 'u': // --rcuspecinv
	case 'y': // --recudelay
	  {
	    // instantiate once, then reuse to add control bits
	    if (!rcumodecommand) {
	      if (command) delete command;
	      rcumodecommand = new RCUCommand(m_server);
	    }

	    command = rcumodecommand;
	    command->set_ndevices(m_nrcus);

	    if ('m' == c || 'n' == c || 'y' == c) {
	      if (!optarg) {
		logMessage(cerr,"Error: option requires an argument");
		delete command;
		return 0;
	      }
	    }

	    rcumodecommand->setMode(false);
	    unsigned long controlopt = 0;

	    switch (c) {

	    case 'm': // --rcumode
	      controlopt = strtoul(optarg, 0, 0);
	      if (controlopt >= 8) {
		logMessage(cerr,"Error: --rcumode value should be < 8");
		delete command;
		return 0;
	      }
	      rcumodecommand->control().setMode((RCUSettings::Control::RCUMode)controlopt);
	      break;

	    case 'p': // --rcuprsg
	      rcumodecommand->control().setPRSG(true);
	      break;

	    case 'e': // --rcureset
	      rcumodecommand->control().setReset(true);
	      break;

	    case 'n': // --rcuattenuation
	      controlopt = strtoul(optarg, 0, 0);
	      if (controlopt >= 32) {
		logMessage(cerr,"Error: --rcuattenuation value should be < 32");
		delete command;
		return 0;
	      }
	      rcumodecommand->control().setAttenuation((uint8)controlopt);
	      break;

	    case 'u': // --rcuspecinv
	      rcumodecommand->control().setSpecinv(true);
	      break;

	    case 'y': // --recudelay
	      controlopt = strtoul(optarg, 0, 0);
	      if (controlopt >= 128) {
		logMessage(cerr,"Error: --rcudelay value should be < 128");
		delete command;
		return 0;
	      }
	      rcumodecommand->control().setDelay((uint8)controlopt);
	      break;
	    }
	  }
	  break;

	case 'g':	// --wg
	  {
	    if (command)
	      delete command;
	    WGCommand* wgcommand = new WGCommand(m_server);
	    command = wgcommand;

	    command->set_ndevices(m_nrcus);

	    if (optarg)
	      {
		wgcommand->setMode(false);
		double frequency = atof(optarg);
		if ( frequency < 0 )
		  {
		    logMessage(cerr,"Error: option '--wg' parameter must be > 0");
		    delete command;
		    return 0;
		  }
		wgcommand->setFrequency(frequency);
	      }
	  }
	  break;

	case 'G':	// --wgmode
	  {
	    if (optarg) {
		int mode = atoi(optarg);
		if (mode != 0 && mode != 1 && mode != 3 && mode != 5) {
		    logMessage(cerr,"Error: option '--wgmode' parameter must be 0,1,3 or 5");
		    delete command;
		    return 0;
		}
		WGCommand*	wgcommand = dynamic_cast<WGCommand*>(command);
		wgcommand->setWaveMode(mode);
	      }
	  }
	  break;

	case 'P':	// --phase
	  {
	    if (optarg) {
		double phase = atof(optarg);
		if (phase < 0 || phase > (M_PI * 2.0)) {
		    logMessage(cerr,"Error: option '--phase' parameter must be between 0 and 2 pi");
		    delete command;
		    return 0;
		}
		WGCommand*	wgcommand = dynamic_cast<WGCommand*>(command);
		wgcommand->setPhase((uint8)(phase / (2 * M_PI) * 256));
	      }
	  }
	  break;

	case 'q' :	// --status
	  {
	    if (command)
	      delete command;
	    StatusCommand* statuscommand = new StatusCommand(m_server);
	    command = statuscommand;

	    command->set_ndevices(m_nrcus);
	  }
	  break;

	case 't':	// --statistics
	  {
	    if (command)
	      delete command;
	    StatisticsCommand* statscommand = new StatisticsCommand(m_server);
	    command = statscommand;

	    command->set_ndevices(m_nrcus);

	    if (optarg)
	      {
		if (!strcmp(optarg, "subband")) {
		  statscommand->setType(Statistics::SUBBAND_POWER);
		} else if (!strcmp(optarg, "beamlet")) {
		  command->set_ndevices(m_nrspboards * MEPHeader::N_POL);
		  statscommand->setType(Statistics::BEAMLET_POWER);
		} else {
		  LOG_FATAL_STR("invalid statistics type '" << optarg << "'");
		  exit(EXIT_FAILURE);
		}
	      }
	  }
	  break;

	case 'x':	// -- xcstatistics
	  {
	    if (command)
	      delete command;
	    XCStatisticsCommand* xcstatscommand = new XCStatisticsCommand(m_server);
	    command = xcstatscommand;
	    command->set_ndevices(m_nrcus);
	    command->setSelectable(false); // no selection allowed
	  }
	  break;

	case 'z':	// -- xcsubbands
	  {
	    if (command)
	      delete command;
	    SubbandsCommand* subbandscommand = new SubbandsCommand(m_server);
	    subbandscommand->setType(SubbandSelection::XLET);
	    command = subbandscommand;

	    command->set_ndevices(m_nrcus);

	    if (optarg)
	      {
		subbandscommand->setMode(false);

		int subband = atoi(optarg);

		if (subband < 0 || subband >= MEPHeader::N_SUBBANDS)
		  {
		    logMessage(cerr,formatString("Error: argument to --xcsubband out of range, value must be >= 0 and < %d",MEPHeader::N_SUBBANDS));
		    exit(EXIT_FAILURE);
		  }

		list<int> subbandlist;
		for (int rcu = 0; rcu < m_nrcus / MEPHeader::N_POL; rcu++)
		  {
		    subbandlist.push_back(subband);
		  }
		subbandscommand->setSubbandList(subbandlist);
	      }
	  }
	  break;

	case 'c':	// --clocks
	  {
	    if (command)
	      delete command;
	    ClocksCommand* clockcommand = new ClocksCommand(m_server);
	    command = clockcommand;

	    command->set_ndevices(m_nrspboards);

	    if (optarg)
	      {
		clockcommand->setMode(false);
		double clock = atof(optarg);
		if ( 160000000 != (uint32)clock && 200000000 != (uint32)clock)
		  {
		    logMessage(cerr,"Error: option '--clocks' parameter must be 160000000 or 200000000");
		    delete command;
		    return 0;
		  }
		clockcommand->setClock((uint32)clock);

	      }
	  }
	  break;

	case 'C': // --rspclear
	  {
	    if (command)
	      delete command;
	    RSUCommand* rsucommand = new RSUCommand(m_server);
	    command = rsucommand;
	    command->set_ndevices(m_nrspboards);

		rsucommand->setMode(false);	// is a SET command
	    rsucommand->control().setClear(true);
	  }
	  break;

	case 'v':	// --version
	  {
	    if (command)
	      delete command;
	    VersionCommand* versioncommand = new VersionCommand(m_server);
	    command = versioncommand;
	    command->set_ndevices(m_nrspboards);
	  }
	  break;

	case 'h':	// --help
	  usage();
	  break;

#ifdef ENABLE_RSPFE
	case 'f':	// --feport
	  if (optarg)
	    {
	      if (!command || 0 == command->get_ndevices())
		{
		  logMessage(cerr,"Error: 'command' argument should come before --feport argument");
		  exit(EXIT_FAILURE);
		}
	      FECommand* feCommand = dynamic_cast<FECommand*>(command);
	      if (feCommand == 0)
		{
		  logMessage(cerr,"Error: 'feport' argument can not be used in conjunction with the specified command");
		  exit(EXIT_FAILURE);
		}
	      feCommand->setFrontEnd(optarg);
	    }
	  else
	    {
	      logMessage(cerr,"Error: option '--feport' requires an argument");
	    }
	  break;
#endif

	case 'd':	// --duration
	  if (optarg)
	    {
	      if (!command || 0 == command->get_ndevices())
		{
		  logMessage(cerr,"Error: 'command' argument should come before --duration argument");
		  exit(EXIT_FAILURE);
		}
	      StatisticsBaseCommand* statisticsBaseCommand = dynamic_cast<StatisticsBaseCommand*>(command);
	      if (statisticsBaseCommand == 0)
		{
		  logMessage(cerr,"Error: 'duration' argument can not be used in conjunction with the specified command");
		  exit(EXIT_FAILURE);
		}
	      statisticsBaseCommand->setDuration(atoi(optarg));
	    }
	  else
	    {
	      logMessage(cerr,"Error: option '--duration' requires an argument");
	    }
	  break;

	case 'i':	// -- integration
	  if (optarg)
	    {
	      if (!command || 0 == command->get_ndevices())
		{
		  logMessage(cerr,"Error: 'command' argument should come before --integration argument");
		  exit(EXIT_FAILURE);
		}
	      StatisticsBaseCommand* statisticsBaseCommand = dynamic_cast<StatisticsBaseCommand*>(command);
	      if (statisticsBaseCommand == 0)
		{
		  logMessage(cerr,"Error: 'integration' argument can not be used in conjunction with the specified command");
		  exit(EXIT_FAILURE);
		}
	      statisticsBaseCommand->setIntegration(atoi(optarg));
	    }
	  else
	    {
	      logMessage(cerr,"Error: option '--integration' requires an argument");
	    }
	  break;

	case 'D':	// -- directory
	  if (optarg)
	    {
	      if (!command || 0 == command->get_ndevices())
		{
		  logMessage(cerr,"Error: 'command' argument should come before --directory argument");
		  exit(EXIT_FAILURE);
		}
	      StatisticsBaseCommand* statisticsBaseCommand = dynamic_cast<StatisticsBaseCommand*>(command);
	      if (statisticsBaseCommand == 0)
		{
		  logMessage(cerr,"Error: 'directory' argument can not be used in conjunction with the specified command");
		  exit(EXIT_FAILURE);
		}
	      statisticsBaseCommand->setDirectory(optarg);
	    }
	  else
	    {
	      logMessage(cerr,"Error: option '--directory' requires an argument");
	    }
	  break;

	case '?':
	default:
	  exit(EXIT_FAILURE);
	  break;
	}
    }

  if (command)
    {
      if (!command->getSelectable())
	{
	  // select all
	  select.clear();
	  for (int i = 0; i < MAX_N_RCUS; ++i)
	    {
	      select.push_back(i);
	    }
	}
      command->setSelect(select);
    }

  return command;
}

std::list<int> RSPCtl::strtolist(const char* str, int max)
{
  string inputstring(str);
  char* start = (char*)inputstring.c_str();
  char* end   = 0;
  bool  range = false;
  long prevval = 0;
  list<int> resultset;

  resultset.clear();

  while (start)
  {
    long val = strtol(start, &end, 10); // read decimal numbers
    start = (end ? (*end ? end + 1 : 0) : 0); // advance
    if (val >= max || val < 0)
    {
      logMessage(cerr,formatString("Error: value %ld out of range",val));
      resultset.clear();
      return resultset;
    }

    if (end)
    {
      switch (*end)
      {
        case ',':
        case 0:
        {
          if (range)
          {
            if (0 == prevval && 0 == val)
              val = max - 1;
            if (val < prevval)
            {
              logMessage(cerr,"Error: invalid range specification");
              resultset.clear();
              return resultset;
            }
            for (long i = prevval; i <= val; i++)
              resultset.push_back(i);
          }

          else
          {
            resultset.push_back(val);
          }
          range=false;
        }
        break;

        case ':':
          range=true;
          break;

        default:
          logMessage(cerr,formatString("Error: invalid character %c",*end));
          resultset.clear();
          return resultset;
          break;
      }
    }
    prevval = val;
  }

  return resultset;
}

void RSPCtl::logMessage(ostream& stream, const string& message)
{
  if(m_command != 0)
  {
    m_command->logMessage(stream,message);
  }
  else
  {
    stream << message << endl;
  }
}

int main(int argc, char** argv)
{
  GCFTask::init(argc, argv);

  LOG_INFO(formatString("Program %s has started", argv[0]));

  RSPCtl c("RSPCtl", argc, argv);

  try
  {
    c.mainloop();
  }
  catch (Exception e)
  {
    cerr << "Exception: " << e.text() << endl;
    exit(EXIT_FAILURE);
  }

  LOG_INFO("Normal termination of program");

  return 0;
}
