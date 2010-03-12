//#  -*- mode: c++ -*-
//#  AntennaGains.cc: implementation of the AntennaGains class.
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
//#  $Id: AntennaGains.cc 12256 2008-11-26 10:54:14Z overeem $

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <APL/ICAL_Protocol/AntennaGains.h>
#include <APL/RTCCommon/MarshallBlitz.h>
#include <Common/LofarTypes.h>

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace CAL;

AntennaGains::AntennaGains() : m_mutex(new pthread_mutex_t)
{
  ASSERT(m_mutex);
  pthread_mutex_init((pthread_mutex_t*)m_mutex, 0);
  lock();

  m_done = false;

  m_gains.resize(1,1,1);
  m_gains = 0;
  
  m_quality.resize(1,1,1);
  m_quality = 0;

  unlock();
}

AntennaGains::AntennaGains(int nantennas, int npol, int nsubbands) : m_mutex(new pthread_mutex_t)
{
  ASSERT(m_mutex);
  pthread_mutex_init((pthread_mutex_t*)m_mutex, 0);
  lock(); m_done = false; unlock();

  if (nantennas < 0 || npol < 0 || nsubbands < 0) {
      nantennas = 0;
      npol = 0;
      nsubbands = 0;
    }
   
  m_gains.resize(nantennas, npol, nsubbands);
  m_gains = 1;

  m_quality.resize(nantennas, npol, nsubbands);
  m_quality = 1;
}

AntennaGains::~AntennaGains()
{
  if (m_mutex) 
	delete m_mutex;
}

unsigned int AntennaGains::getSize()
{
  return 
      MSH_ARRAY_SIZE(m_gains, complex<double>)
    + MSH_ARRAY_SIZE(m_quality, double)
    + sizeof(bool);
}

unsigned int AntennaGains::pack(void* buffer)
{
  unsigned int offset = 0;

  lock();
  MSH_PACK_ARRAY(buffer, offset, m_gains, complex<double>);
  MSH_PACK_ARRAY(buffer, offset, m_quality, double);
  memcpy((char*)buffer + offset, &m_done, sizeof(bool));
  offset += sizeof(bool);
  unlock();

  return offset;
}

unsigned int AntennaGains::unpack(void* buffer)
{
  unsigned int offset = 0;

  lock();
  MSH_UNPACK_ARRAY(buffer, offset, m_gains, complex<double>, 3);
  MSH_UNPACK_ARRAY(buffer, offset, m_quality, double, 3);
  memcpy(&m_done, (char*)buffer + offset, sizeof(bool));
  offset += sizeof(bool);
  unlock();

  return offset;
}

AntennaGains& AntennaGains::operator=(const AntennaGains& rhs)
{
  if (this != &rhs) {
    lock(); rhs.lock();

    m_gains.resize(rhs.m_gains.shape());
    m_gains = rhs.m_gains.copy();
    
    m_quality.resize(rhs.m_quality.shape());
    m_quality = rhs.m_quality.copy();
    
    m_done = rhs.m_done;
    rhs.unlock(); unlock();
  }
  return *this;
}

