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
#include "RSPDriverTask.h"

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

  m_subbandselection.resize(RSPDriverTask::N_RCU);
  m_rcusettings.resize(RSPDriverTask::N_RCU);
  m_wgsettings.resize(RSPDriverTask::N_RCU);
  m_statistics.resize(RSPDriverTask::N_RCU);

  m_beamletweights.weights().resize(BeamletWeights::SINGLE_TIMESTEP,
				    RSPDriverTask::N_RCU,
				    N_BEAMLETS);
  m_beamletweights.weights()(Range::all(), Range::all(), Range::all()) = complex<int16>(0,0);
}

CacheBuffer::~CacheBuffer()
{
  m_subbandselection.free();
  m_rcusettings.free();
  m_wgsettings.free();
  m_statistics.free();
}

BeamletWeights&   CacheBuffer::getBeamletWeights()
{
  return m_beamletweights;
}

SubbandSelection& CacheBuffer::getSubbandSelection(int rcu)
{
  return m_subbandselection(rcu);
}

RCUSettings&      CacheBuffer::getRCUSettings(int rcu)
{
  return m_rcusettings(rcu);
}

WGSettings&       CacheBuffer::getWGSettings(int rcu)
{
  return m_wgsettings(rcu);
}

SystemStatus&     CacheBuffer::getSystemStatus()
{
  return m_systemstatus;
}

Statistics&       CacheBuffer::getStatistics(int rcu)
{
  return m_statistics(rcu);
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
    m_instance = new Cache();
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


