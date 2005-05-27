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

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace CAL;
    
SubArray::SubArray(string                 name,
		   const Array<double,3>& pos,
		   const Array<bool, 2>&  select,
		   double sampling_frequency,
		   int nyquist_zone,
		   int nsubbands) :
  AntennaArray(name, pos),
  m_spw(name, sampling_frequency, nyquist_zone, nsubbands)
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

  m_result[FRONT]->setComplete();
}

bool SubArray::getGains(const AntennaGains*& cal, int buffer)
{
  ASSERT(m_result[buffer]);
  cal = 0;

  if (buffer >= FRONT && buffer <= BACK)
  {
    cal = m_result[buffer];
  
    return m_result[buffer]->isComplete();
  }

  return false;
}

void SubArray::abortCalibration()
{}

const SpectralWindow& SubArray::getSPW() const
{
  return m_spw;
}

bool SubArray::isDone()
{
  ASSERT(m_result[FRONT]);
  return m_result[FRONT]->isComplete();
}

SubArrays::SubArrays()
{}

SubArrays::~SubArrays()
{
  for (map<string, SubArray*>::const_iterator it = m_arrays.begin();
       it != m_arrays.end(); it++)
  {
    if ((*it).second) delete (*it).second;
  }
}

void SubArrays::add(SubArray* array)
{
  if (array) {
    m_arrays[array->getName()] = array;
  }
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

SubArray* SubArrays::getByName(std::string name)
{
  return m_arrays[name];
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
      }
    }
  }
}
