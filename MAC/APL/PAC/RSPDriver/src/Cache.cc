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
			    GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i),
			    MEPHeader::N_BEAMLETS,
			    MEPHeader::N_POL);
  m_subbandselection().resize(GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i),
			      MEPHeader::N_BEAMLETS * MEPHeader::N_POL);

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

    //
    // Set subbands selection in increasing value
    //
    secondIndex i;
    m_subbandselection()(Range::all(), Range::all()) = i;
  }

  m_rcusettings().resize(GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL);
  m_rcusettings() = RCUSettings::RCURegisterType();

  m_wgsettings().resize(GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL);
  
  WGSettings::WGRegisterType init;
  init.freq        = 0;
  init.phase       = 0;
  init.ampl        = 0;
  init.nof_samples = 0;
  init.mode = WGSettings::MODE_OFF;
  init._pad = 0;
  m_wgsettings() = init;

  m_subbandstats().resize(Statistics::N_STAT_TYPES / 2,
			  GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL,
			  MEPHeader::N_SUBBANDS);
  m_subbandstats() = 0;

  m_beamletstats().resize(Statistics::N_STAT_TYPES / 2,
			  GET_CONFIG("RS.N_RSPBOARDS", i) * MEPHeader::N_POL,
			  MEPHeader::N_BEAMLETS);
  m_beamletstats() = 0;

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
}

CacheBuffer::~CacheBuffer()
{
  m_beamletweights().free();
  m_subbandselection().free();
  m_rcusettings().free();
  m_wgsettings().free();
  m_subbandstats().free();
  m_beamletstats().free();
  m_systemstatus.board().free();
  m_systemstatus.rcu().free();
  m_versions.rsp().free();
  m_versions.bp().free();
  m_versions.ap().free();
}

RSP_Protocol::Timestamp CacheBuffer::getTimestamp() const
{
  return m_timestamp;
}

BeamletWeights&   CacheBuffer::getBeamletWeights()
{
  return m_beamletweights;
}

SubbandSelection& CacheBuffer::getSubbandSelection()
{
  return m_subbandselection;
}

RCUSettings&      CacheBuffer::getRCUSettings()
{
  return m_rcusettings;
}

WGSettings&       CacheBuffer::getWGSettings()
{
  return m_wgsettings;
}

SystemStatus&     CacheBuffer::getSystemStatus()
{
  return m_systemstatus;
}

Statistics&       CacheBuffer::getSubbandStats()
{
  return m_subbandstats;
}

Statistics&       CacheBuffer::getBeamletStats()
{
  return m_beamletstats;
}

Versions&         CacheBuffer::getVersions()
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
