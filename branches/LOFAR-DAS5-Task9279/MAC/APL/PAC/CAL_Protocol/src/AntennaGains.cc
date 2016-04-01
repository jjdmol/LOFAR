//#  -*- mode: c++ -*-
//#  AntennaGains.cc: implementation of the AntennaGains class.
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

#include <APL/CAL_Protocol/AntennaGains.h>
#include <APL/RTCCommon/MarshallBlitz.h>
#include <Common/LofarTypes.h>

using namespace std;
using namespace blitz;
namespace LOFAR {
    namespace CAL {

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

AntennaGains::AntennaGains(uint nrRCUs, uint nsubbands) : m_mutex(new pthread_mutex_t)
{
    ASSERT(m_mutex);
    pthread_mutex_init((pthread_mutex_t*)m_mutex, 0);
    lock(); m_done = false; unlock();

    m_gains.resize(nrRCUs, nsubbands);
    m_gains = 1;

    m_quality.resize(nrRCUs, nsubbands);
    m_quality = 1;
}

AntennaGains::~AntennaGains()
{
    if (m_mutex)
    delete m_mutex;
}

//
// clone()
//
AntennaGains* AntennaGains::clone() const
{
    AntennaGains*   theClone = new AntennaGains(m_gains.extent(firstDim), m_gains.extent(secondDim));
    ASSERTSTR(theClone, "Could not clone the AntennaGains class");

    theClone->m_gains   = m_gains;
    theClone->m_quality = m_quality;
    theClone->m_done    = m_done;

    return (theClone);
}

size_t AntennaGains::getSize() const
{
    return
            MSH_size(m_gains)
        + MSH_size(m_quality)
        + sizeof(bool);
}

size_t AntennaGains::pack(char* buffer) const
{
    size_t offset = 0;

    lock();
    offset = MSH_pack(buffer, offset, m_gains);
    offset = MSH_pack(buffer, offset, m_quality);
    memcpy((char*)buffer + offset, &m_done, sizeof(bool));
    offset += sizeof(bool);
    unlock();

    return offset;
}

size_t AntennaGains::unpack(const char* buffer)
{
    size_t offset = 0;

    lock();
    offset = MSH_unpack(buffer, offset, m_gains);
    offset = MSH_unpack(buffer, offset, m_quality);
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

    } // namespace CAL
} // namespace LOFAR
