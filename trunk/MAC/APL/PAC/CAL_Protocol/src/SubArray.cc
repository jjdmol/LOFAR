//#  -*- mode: c++ -*-
//#  SubArray.cc: implementation of the SubArray class
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

#include "SubArray.h"
#include "CalibrationInterface.h"
#include <Common/LofarLogger.h>
#include <APL/RTCCommon/Marshalling.h>

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace CAL;
    
SubArray::SubArray() : AntennaArray(), m_spw("undefined", 0, 0, 0, 0)
{
  m_result[FRONT] = 0;
  m_result[BACK] = 0;
}

SubArray::SubArray(string                 name,
		   const Array<double,1>& geoloc,
		   const Array<double,3>& pos,
		   const Array<bool, 2>&  select,
		   double sampling_frequency,
		   int nyquist_zone,
		   int nsubbands,
		   uint8 rcucontrol) :
  AntennaArray(name, geoloc, pos),
  m_spw(name + "_spw", sampling_frequency, nyquist_zone, nsubbands, rcucontrol)
{
  // assert sizes
  ASSERT(select.extent(firstDim) == pos.extent(firstDim)
	 && select.extent(secondDim) == pos.extent(secondDim)
	 && pos.extent(thirdDim) == 3);

  // make array at least big enough
  m_rcuindex.resize(pos.extent(firstDim), pos.extent(secondDim));

  for (int i = 0; i < pos.extent(firstDim); i++)
    {
      for (int pol = 0; pol < pos.extent(secondDim); pol++)
	{
	  if (select(i,pol)) {
	    m_rcuindex(i,pol) = (i * pos.extent(secondDim)) + pol;
	  } else {
	    m_rcuindex(i,pol) = -1;
	  }
	}
    }

  // TODO: compact array by removing antennas of which both polarizations have not been selected

  // create calibration result objects
  m_result[FRONT] = new AntennaGains(pos.extent(firstDim), pos.extent(secondDim), m_spw.getNumSubbands());
  m_result[BACK]  = new AntennaGains(pos.extent(firstDim), pos.extent(secondDim), m_spw.getNumSubbands());
  ASSERT(m_result[FRONT] && m_result[BACK]);
}

SubArray::~SubArray()
{
  if (m_result[FRONT]) delete m_result[FRONT];
  if (m_result[BACK])  delete m_result[BACK];
}

void SubArray::calibrate(CalibrationInterface* cal, const ACC& acc)
{
  ASSERT(m_result[FRONT]);

  if (cal) cal->calibrate(*this, acc, *m_result[FRONT]);

  m_result[FRONT]->setDone();
}

bool SubArray::getGains(AntennaGains*& cal, int buffer)
{
  cal = 0;
  ASSERT(buffer >= FRONT && buffer <= BACK && m_result[buffer]);

  cal = m_result[buffer];
  
  return m_result[buffer]->isDone();
}

void SubArray::abortCalibration()
{}

const SpectralWindow& SubArray::getSPW() const
{
  return m_spw;
}

SubArray& SubArray::operator=(const SubArray& rhs)
{
  if (this != &rhs) {
    // base-class assignment
    AntennaArray::operator=(rhs);

    // assign spectral window
    m_spw = rhs.getSPW();

    // clear m_result pointers
    this->m_result[FRONT] = 0;
    this->m_result[BACK] = 0;
  }

  return *this;
}

bool SubArray::isDone()
{
  ASSERT(m_result[FRONT]);
  return m_result[FRONT]->isDone();
}

void SubArray::clearDone()
{
  ASSERT(m_result[FRONT]);
  m_result[FRONT]->setDone(false);
}

unsigned int SubArray::getSize()
{
  return
      MSH_STRING_SIZE(m_name)
    + MSH_ARRAY_SIZE (m_pos,      double)
    + MSH_ARRAY_SIZE (m_rcuindex, int16)
    + m_spw.getSize();
}

unsigned int SubArray::pack(void* buffer)
{
  unsigned int offset = 0;

  MSH_PACK_STRING(buffer, offset, m_name);
  MSH_PACK_ARRAY(buffer,  offset, m_pos,      double);
  MSH_PACK_ARRAY(buffer,  offset, m_rcuindex, int16);
  offset += m_spw.pack(((char*)buffer) + offset);

  return offset;
}

unsigned int SubArray::unpack(void* buffer)
{
  unsigned int offset = 0;

  MSH_UNPACK_STRING(buffer, offset, m_name);
  MSH_UNPACK_ARRAY(buffer,  offset, m_pos,      double, 3);
  MSH_UNPACK_ARRAY(buffer,  offset, m_rcuindex, int16,  2);
  offset += m_spw.unpack(((char*)buffer) + offset);

  return offset;
}

SubArrays::SubArrays()
{}

SubArrays::~SubArrays()
{
  // delete subarrays from m_new_arrays map and clear map
  for (map<string, SubArray*>::const_iterator it = m_new_arrays.begin();
       it != m_new_arrays.begin(); ++it)
  {
    if ((*it).second) delete (*it).second;
  }
  m_new_arrays.clear();

  // delete subarrays from m_arrays map and clear map
  for (map<string, SubArray*>::const_iterator it = m_arrays.begin();
       it != m_arrays.end(); ++it)
  {
    if ((*it).second) delete (*it).second;
  }
  m_arrays.clear();

  // clear m_dead_arrays list, subarrays have already been delete
  // because they were also in the m_arrays map
  m_dead_arrays.clear();
}

void SubArrays::schedule_add(SubArray* array)
{
  if (array) {
    m_new_arrays[array->getName()] = array;
  }
}

bool SubArrays::schedule_remove(string name)
{
  // find in m_new_arrays
  map<string,SubArray*>::iterator it = m_new_arrays.find(name);

  // if found then remove
  if (it != m_new_arrays.end()) {
    delete (*it).second;
    m_new_arrays.erase(it);
    return true;
  }

  // if not found in m_new_arrays, try to find in m_arrays
  it = m_arrays.find(name);

  // if found then move to m_dead_arrays
  if (it != m_arrays.end()) {
    m_dead_arrays.push_back((*it).second);
    return true;
  }

  return false;
}

bool SubArrays::schedule_remove(SubArray*& subarray)
{
  return (subarray?schedule_remove(subarray->getName()):false);
}

void SubArrays::creator()
{
  /**
   * New subarrays are listed in m_new_arrays.
   * This method moves these subarrays to the m_arrays map.
   */
  for (map<string, SubArray*>::const_iterator it = m_new_arrays.begin(); it != m_new_arrays.end(); ++it) {
    m_arrays[(*it).second->getName()] = (*it).second; // add to m_arrays
  }
  m_new_arrays.clear(); // clear m_new_array
}

void SubArrays::undertaker()
{
  /**
   * Subarrays that should be removed are listed in m_dead_arrays.
   * This method deletes these subarrays and removes them from the m_arrays.
   * The m_dead_arrays list is cleared when done.
   */
  map<string, SubArray*>::iterator findit;
  for (list<SubArray*>::const_iterator it = m_dead_arrays.begin(); it != m_dead_arrays.end(); ++it) {

    /* Remove from the m_arrays map*/
    findit = m_arrays.find((*it)->getName());
    if (findit != m_arrays.end()) {
      m_arrays.erase(findit);
    } else {
      LOG_FATAL_STR("trying to remove non-existing subarray " << (*findit).second->getName());
      exit(EXIT_FAILURE);
    }

    delete (*it);
  }
  m_dead_arrays.clear();
}

SubArray* SubArrays::getByName(std::string name)
{
  // find in m_new_arrays
  map<string,SubArray*>::const_iterator it = m_new_arrays.find(name);

  if (it != m_new_arrays.end()) {
    return (*it).second;
  }

  // if not found in m_new_arrays, try to find in m_arrays
  it = m_arrays.find(name);

  if (it != m_arrays.end()) {
    return (*it).second;
  }
  return 0;
}

void SubArrays::updateAll()
{
  for (map<string, SubArray*>::const_iterator it = m_arrays.begin();
       it != m_arrays.end(); ++it)
  {
    SubArray* subarray = (*it).second;
    
    // notify subarrays that have completed calibration
    if (subarray) {
      if (subarray->isDone()) {
	subarray->notify();
	subarray->clearDone(); // we've notified all subscribers, clear done flag
      }
    }
  }
}

void SubArrays::calibrate(CalibrationInterface* cal, ACC& acc)
{
  bool done = false;

  ASSERT(0 != cal);

  mutex_lock();
  if (acc.isValid()) {
    for (map<string, SubArray*>::const_iterator it = m_arrays.begin();
	 it != m_arrays.end(); ++it)
      {
	SubArray* subarray = (*it).second;
	ASSERT(0 != subarray);

	if (!subarray->isDone()) {
	  LOG_INFO_STR("start calibration of subarray: " << subarray->getName());
	  subarray->calibrate(cal, acc);
	  LOG_INFO_STR("finished calibration of subarray: " << subarray->getName());
	  done = true;
	}
      }
  }
  mutex_unlock();

  //
  // prevent reuse of this acc by next calibrate call
  // TODO: this should be done elsewhere once calibrate
  // is running in its own thread
  //
  if (done) acc.invalidate();
}

bool SubArrays::remove(string name)
{
  // find SubArray
  map<string,SubArray*>::iterator it = m_arrays.find(name);

  // if found then remove
  if (it != m_arrays.end()) {
    if ((*it).second) delete ((*it).second);
    m_arrays.erase(it);
    return true;
  }

  return false;
}

bool SubArrays::remove(SubArray*& subarray)
{
  return (subarray?remove(subarray->getName()):false);
}

