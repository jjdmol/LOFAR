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

#include "Cache.h"
#include <PSAccess.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <blitz/array.h>

using namespace RSP;
using namespace LOFAR;
using namespace RSP_Protocol;
using namespace blitz;
using namespace RTC;

/**
 * Instance pointer for the Cache singleton class.
 */
Cache* Cache::m_instance = 0;

/**
 * CacheBuffer implementation
 */

CacheBuffer::CacheBuffer()
{
  struct timeval tv;
  tv.tv_sec = 0; tv.tv_usec = 0;
  m_timestamp.set(tv);

  m_beamletweights().resize(BeamletWeights::SINGLE_TIMESTEP,
			    GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL,
			    MEPHeader::N_BEAMLETS);
  m_subbandselection().resize(GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL,
			      MEPHeader::N_BEAMLETS);

  if (!GET_CONFIG("RSPDriver.IDENTITY_WEIGHTS", i))
  {
    m_beamletweights()(Range::all(), Range::all(), Range::all()) =
      complex<int16>(0, 0);
    m_subbandselection() = 0;
  }
  else
  {
    // these weights ensure that the beamlet statistics
    // exactly match the subband statistics
    m_beamletweights()(Range::all(), Range::all(), Range::all()) =
      complex<int16>(0x4000, 0);

    // set weights on first 8 beamlets for cross correlation
    m_beamletweights()(Range::all(), Range::all(), Range(0, GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL - 1)) =
      complex<int16>(0,0);

    for (int i = 0; i < GET_CONFIG("RS.N_RSPBOARDS", i)* GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL; i++) {
      LOG_INFO_STR("setting " << i);
      m_beamletweights()(0, i, i) = complex<int16>(0x4000,0);
    }

    LOG_INFO_STR("m_beamletweights=" << m_beamletweights()(0, Range::all(), Range::all()));


    //
    // Set subbands selection in increasing value
    //
    m_subbandselection()(Range::all(), Range::all()) = (tensor::j + GET_CONFIG("RSPDriver.FIRST_SUBBAND", i)
							% (MEPHeader::N_SUBBANDS * MEPHeader::N_POL));

    LOG_WARN_STR("m_subbandselection()=" << m_subbandselection());
  }

  m_rcusettings().resize(GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL);
  m_rcusettings() = RCUSettings::RCURegisterType();

  if (GET_CONFIG("RSPDriver.SWAP_RCU_BITS", i))
  {
    for (int i = 0; i < GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL; i++)
    {
      m_rcusettings()(i).value = 82;
    }
  }
  else
  {
    for (int i = 0; i < GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL; i++)
    {
      m_rcusettings()(i).value = 0xB9; // low band, no filter
    }
  }
  
  m_wgsettings().resize(GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL);
  
  WGSettings::WGRegisterType init;
  init.freq        = 0;
  init.phase       = 0;
  init.ampl        = 0;
  init.nof_samples = 0;
  init.mode = WGSettings::MODE_OFF;
  init.preset = WGSettings::PRESET_SINE;
  m_wgsettings() = init;

  m_wgsettings.waveforms().resize(GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL,
				  N_WAVE_SAMPLES);
  m_wgsettings.waveforms() = 0;

  m_subbandstats().resize(GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL,
			  MEPHeader::N_SUBBANDS);
  m_subbandstats() = 0;

  m_beamletstats().resize(GET_CONFIG("RS.N_RSPBOARDS", i) * MEPHeader::N_POL,
			  MEPHeader::N_BEAMLETS);
  m_beamletstats() = 0;

  m_xcstats().resize(MEPHeader::N_POL,
		     MEPHeader::N_POL,
		     GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i),
		     GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i));

  m_xcstats() = complex<double>(0,0);

  m_systemstatus.board().resize(GET_CONFIG("RS.N_RSPBOARDS", i));
  m_systemstatus.rcu().resize(GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL);

  BoardStatus boardinit;
  RCUStatus   rcuinit;

  memset(&boardinit, 0, sizeof(BoardStatus));
  memset(&rcuinit,   0, sizeof(RCUStatus));
  
  m_systemstatus.board() = boardinit;
  m_systemstatus.rcu()   = rcuinit;

  m_versions.rsp().resize(GET_CONFIG("RS.N_RSPBOARDS", i));
  m_versions.rsp() = 0;
  m_versions.bp().resize(GET_CONFIG("RS.N_RSPBOARDS", i));
  m_versions.bp() = 0;
  m_versions.ap().resize(GET_CONFIG("RS.N_RSPBOARDS", i));
  m_versions.ap() = 0;

  // print sizes of the cache
  LOG_DEBUG_STR("m_beamletweights().size()     =" << m_beamletweights().size()     * sizeof(complex<int16>));
  LOG_DEBUG_STR("m_subbandselection().size()   =" << m_subbandselection().size()   * sizeof(uint16));
  LOG_DEBUG_STR("m_rcusettings().size()        =" << m_rcusettings().size()        * sizeof(uint8));
  LOG_DEBUG_STR("m_wgsettings().size()         =" << m_wgsettings().size()         * sizeof(WGSettings::WGRegisterType));
  LOG_DEBUG_STR("m_subbandstats().size()       =" << m_subbandstats().size()       * sizeof(uint16));
  LOG_DEBUG_STR("m_beamletstats().size()       =" << m_beamletstats().size()       * sizeof(double));
  LOG_DEBUG_STR("m_xcstats().size()            =" << m_xcstats().size()            * sizeof(complex<double>));
  LOG_DEBUG_STR("m_systemstatus.board().size() =" << m_systemstatus.board().size() * sizeof(EPA_Protocol::BoardStatus));
  LOG_DEBUG_STR("m_systemstatus.rcu().size()   =" << m_systemstatus.rcu().size()   * sizeof(EPA_Protocol::RCUStatus));
  LOG_DEBUG_STR("m_versions.rsp().size()       =" << m_versions.rsp().size()       * sizeof(uint8));
  LOG_DEBUG_STR("m_versions.bp().size()        =" << m_versions.bp().size()        * sizeof(uint8));
  LOG_DEBUG_STR("m_versions.ap().size()        =" << m_versions.ap().size()        * sizeof(uint8));
}

CacheBuffer::~CacheBuffer()
{
  m_beamletweights().free();
  m_subbandselection().free();
  m_rcusettings().free();
  m_wgsettings().free();
  m_wgsettings.waveforms().free();
  m_subbandstats().free();
  m_beamletstats().free();
  m_xcstats().free();
  m_systemstatus.board().free();
  m_systemstatus.rcu().free();
  m_versions.rsp().free();
  m_versions.bp().free();
  m_versions.ap().free();
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

  m_front = new CacheBuffer();
  m_back = new CacheBuffer();
}

Cache::~Cache()
{
  if (m_front) delete m_front;
  if (m_back) delete m_back;
}

void Cache::swapBuffers()
{
  // clear modified flags on back buffer
  m_back->getWGSettings().clearModified();

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
