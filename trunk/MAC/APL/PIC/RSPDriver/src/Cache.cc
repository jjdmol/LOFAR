//#  Cache.cc: implementation of the Cache class
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

#include "StationSettings.h"
#include "Cache.h"
#include "Sequencer.h"

#include <APL/RTCCommon/PSAccess.h>

#include <blitz/array.h>

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

/**
 * Instance pointer for the Cache singleton class.
 */
Cache* Cache::m_instance = 0;

/**
 * CacheBuffer implementation
 */

CacheBuffer::CacheBuffer(Cache* cache) : m_cache(cache)
{
  reset(); // reset by allocating memory and settings default values

  // When the sample frequency (m_clock) is modified the RSP
  // board goes through a reset. For this reason we reset all
  // values in the cacht to default (using Cache::reset), but
  // the value of m_clock should not be reset. For that reason
  // it is initialized here separately outside reset().
  m_clock = GET_CONFIG("RSPDriver.DEFAULT_SAMPLING_FREQUENCY", i);

  // print sizes of the cache
  LOG_DEBUG_STR("m_beamletweights().size()     =" << m_beamletweights().size()     * sizeof(complex<int16>));
  LOG_DEBUG_STR("m_subbandselection().size()   =" << m_subbandselection().size()   * sizeof(uint16));
  LOG_DEBUG_STR("m_rcusettings().size()        =" << m_rcusettings().size()        * sizeof(uint8));
  LOG_DEBUG_STR("m_rsusettings().size()        =" << m_rsusettings().size()        * sizeof(uint8));
  LOG_DEBUG_STR("m_wgsettings().size()         =" << m_wgsettings().size()         * sizeof(WGSettings::WGRegisterType));
  LOG_DEBUG_STR("m_subbandstats().size()       =" << m_subbandstats().size()       * sizeof(uint16));
  LOG_DEBUG_STR("m_beamletstats().size()       =" << m_beamletstats().size()       * sizeof(double));
  LOG_DEBUG_STR("m_xcstats().size()            =" << m_xcstats().size()            * sizeof(complex<double>));
  LOG_DEBUG_STR("m_systemstatus.board().size() =" << m_systemstatus.board().size() * sizeof(EPA_Protocol::BoardStatus));
  LOG_DEBUG_STR("m_versions.bp().size()        =" << m_versions.bp().size()        * sizeof(EPA_Protocol::RSRVersion));
  LOG_DEBUG_STR("m_versions.ap().size()        =" << m_versions.ap().size()        * sizeof(EPA_Protocol::RSRVersion));
}

CacheBuffer::~CacheBuffer()
{
  m_beamletweights().free();
  m_subbandselection().free();
  m_rcusettings().free();
  m_rsusettings().free();
  m_wgsettings().free();
  m_wgsettings.waveforms().free();
  m_subbandstats().free();
  m_beamletstats().free();
  m_xcstats().free();
  m_systemstatus.board().free();
  m_versions.bp().free();
  m_versions.ap().free();
}

void CacheBuffer::reset(void)
{
  //
  // Initialize cache, allocating memory and setting default values
  //
  struct timeval tv;
  tv.tv_sec = 0; tv.tv_usec = 0;
  m_timestamp.set(tv);

  m_beamletweights().resize(BeamletWeights::SINGLE_TIMESTEP, StationSettings::instance()->nrRcus(), MEPHeader::N_BEAMLETS);
  m_beamletweights() = complex<int16>(0,0);

  m_subbandselection().resize(StationSettings::instance()->nrRcus(),
			      MEPHeader::N_LOCAL_XLETS + MEPHeader::N_BEAMLETS);
  m_subbandselection() = 0;

  if (GET_CONFIG("RSPDriver.IDENTITY_WEIGHTS", i))
  {
    // these weights ensure that the beamlet statistics
    // exactly match the subband statistics
    m_beamletweights()(Range::all(), Range::all(), Range::all()) =
      complex<int16>(GET_CONFIG("RSPDriver.BF_GAIN", i), 0);

    //
    // Set default subband selection starting at RSPDriver.FIRST_SUBBAND
    //
    for (int rcu = 0; rcu < m_subbandselection().extent(firstDim); rcu++) {
      for (int sb = 0; sb < MEPHeader::N_BEAMLETS; sb++) {
	m_subbandselection()(rcu, sb + MEPHeader::N_LOCAL_XLETS) = (rcu % MEPHeader::N_POL) +
	  (sb * MEPHeader::N_POL) +
	  (GET_CONFIG("RSPDriver.FIRST_SUBBAND", i) * 2);
      }
    }
  }

  // initialize RCU settings
  m_rcusettings().resize(StationSettings::instance()->nrRcus());

  RCUSettings::Control rcumode;
  rcumode.setMode(RCUSettings::Control::MODE_OFF);
  m_rcusettings() = rcumode;

  // initialize HBA settings
  m_hbasettings().resize(StationSettings::instance()->nrRcus(), MEPHeader::N_HBA_DELAYS);
  m_hbasettings() = 0; // initialize to 0

  // RSU settings
  m_rsusettings().resize(StationSettings::instance()->nrRspBoards());
  RSUSettings::ResetControl 	rsumode;
  rsumode.setRaw(RSUSettings::ResetControl::CTRL_OFF);
  m_rsusettings() = rsumode;

  m_wgsettings().resize(StationSettings::instance()->nrRcus());
  WGSettings::WGRegisterType init;
  init.freq        = 0;
  init.phase       = 0;
  init.ampl        = 0;
  init.nof_samples = 0;
  init.mode = WGSettings::MODE_OFF;
  init.preset = WGSettings::PRESET_SINE;
  m_wgsettings() = init;

  m_wgsettings.waveforms().resize(StationSettings::instance()->nrRcus(), MEPHeader::N_WAVE_SAMPLES);
  m_wgsettings.waveforms() = 0;

  m_subbandstats().resize(StationSettings::instance()->nrRcus(), MEPHeader::N_SUBBANDS);
  m_subbandstats() = 0;

  m_beamletstats().resize(StationSettings::instance()->nrRspBoards() * MEPHeader::N_POL,
			  MEPHeader::N_BEAMLETS);
  m_beamletstats() = 0;

  m_xcstats().resize(MEPHeader::N_POL,
		     MEPHeader::N_POL,
		     StationSettings::instance()->nrBlps(),
		     StationSettings::instance()->nrBlps());

  m_xcstats() = complex<double>(0,0);

  // BoardStatus
  m_systemstatus.board().resize(StationSettings::instance()->nrRspBoards());
  BoardStatus boardinit;
  memset(&boardinit, 0, sizeof(BoardStatus));
  m_systemstatus.board() = boardinit;

  EPA_Protocol::RSRVersion versioninit = { 0, 0, 0 };
  m_versions.bp().resize(StationSettings::instance()->nrRspBoards());
  m_versions.bp() = versioninit;
  m_versions.ap().resize(StationSettings::instance()->nrBlps());
  m_versions.ap() = versioninit;
}

RTC::Timestamp CacheBuffer::getTimestamp() const
{
  return m_timestamp;
}

BeamletWeights& CacheBuffer::getBeamletWeights()
{
  return m_beamletweights;
}

SubbandSelection& CacheBuffer::getSubbandSelection()
{
  return m_subbandselection;
}

RCUSettings& CacheBuffer::getRCUSettings()
{
  return m_rcusettings;
}

HBASettings& CacheBuffer::getHBASettings()
{
  return m_hbasettings;
}

RSUSettings& CacheBuffer::getRSUSettings()
{
  return m_rsusettings;
}

WGSettings& CacheBuffer::getWGSettings()
{
  return m_wgsettings;
}

SystemStatus& CacheBuffer::getSystemStatus()
{
  return m_systemstatus;
}

Statistics& CacheBuffer::getSubbandStats()
{
  return m_subbandstats;
}

Statistics& CacheBuffer::getBeamletStats()
{
  return m_beamletstats;
}

XCStatistics& CacheBuffer::getXCStats()
{
  return m_xcstats;
}

Versions& CacheBuffer::getVersions()
{
  return m_versions;
}

uint32& CacheBuffer::getClock()
{
  return m_clock;
}

void CacheBuffer::setTimestamp(const RTC::Timestamp& timestamp)
{
  m_timestamp = timestamp;
}

/**
 * Cache implementation
 */

Cache& Cache::getInstance()
{
  if (0 == m_instance)
  {
    m_instance = new Cache;
    return *m_instance;
  }
  else return *m_instance;
}

Cache::Cache() : m_front(0), m_back(0)
{
  //
  // initialize preset waveforms
  //
  WGSettings::initWaveformPresets();

  m_front = new CacheBuffer(this); ASSERT(m_front);
  m_back = new CacheBuffer(this);  ASSERT(m_back);

  getState().init(StationSettings::instance()->nrRspBoards(),
		  StationSettings::instance()->nrBlps(),
		  StationSettings::instance()->nrRcus());

  // start by writing the correct clock setting
  Sequencer::getInstance().startSequence(Sequencer::SETCLOCK);
}

Cache::~Cache()
{
  if (m_front) delete m_front;
  if (m_back) delete m_back;
}

void Cache::reset(void)
{
  m_front->reset();
  m_back->reset();
}

void Cache::swapBuffers()
{
  if (GET_CONFIG("RSPDriver.XC_FILL", i)) {
    // fill xcorr array by copying and taking complex conjugate of values mirrored in the diagonal
    Array<complex<double>, 4> xc(m_back->getXCStats()());
    firstIndex  i; secondIndex j; thirdIndex  k; fourthIndex  l;
    xc = where(xc(i,j,k,l)==complex<double>(0,0), conj(xc(j,i,l,k)), xc(i,j,k,l));
  }

  CacheBuffer *tmp = m_front;
  m_front = m_back;
  m_back  = tmp;
}

CacheBuffer& Cache::getFront()
{
  return *m_front;
}

CacheBuffer& Cache::getBack()
{
  return *m_back;
}
