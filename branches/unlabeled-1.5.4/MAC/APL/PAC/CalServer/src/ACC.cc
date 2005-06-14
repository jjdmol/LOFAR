//#  -*- mode: c++ -*-
//#  ACC.cc: implementation of the Auto Correlation Cube class
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

#include "ACC.h"
#include <blitz/array.h>
#include <fstream>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace CAL;
using namespace RTC;

ACC::ACC(int nsubbands, int nantennas, int npol) :
  SharedResource(1,1), // at most one reader or one writer
  m_acc(nsubbands, npol, npol, nantennas, nantennas),
  m_time(nsubbands), m_valid(false)
{
}

ACC::~ACC()
{
}

const Array<complex<double>, 2> ACC::getACM(int subband, int pol1, int pol2, Timestamp& timestamp) const
{
  Range all = Range::all();
  timestamp = Timestamp(0,0);

  // check range of subband argument
  ASSERT(subband >= 0 && subband < m_acc.extent(firstDim));
  ASSERT(pol1 == 0 || pol1 == 1);
  ASSERT(pol2 == 0 || pol2 == 1);

  timestamp = m_time(subband);

  return m_acc(subband, pol1, pol2, all, all);
}

void ACC::updateACM(int subband, Timestamp timestamp, Array<complex<double>, 4>& newacm)
{
  Range all = Range::all();

  LOG_DEBUG_STR("m_acc.shape()=" << m_acc.shape());
  LOG_DEBUG_STR("newacm.shape()=" << newacm.shape());

  ASSERT(newacm.extent(firstDim)  == m_acc.extent(secondDim));
  ASSERT(newacm.extent(secondDim) == m_acc.extent(thirdDim));
  ASSERT(newacm.extent(thirdDim)  == m_acc.extent(fourthDim));
  ASSERT(newacm.extent(fourthDim) == m_acc.extent(fifthDim));

  m_acc(subband, all, all, all, all) = newacm;
  m_time(subband) = timestamp;
}

void ACC::setACC(blitz::Array<std::complex<double>, 5>& acc)
{
  m_acc.reference(acc);
  m_time.resize(acc.extent(firstDim));
  m_time = Timestamp(0,0);
}

ACCs::ACCs(int nsubbands, int nantennas, int npol) : 
	m_front(0), m_back(1)
{
  m_buffer[m_front] = new ACC(nsubbands, nantennas, npol);
  m_buffer[m_back]  = new ACC(nsubbands, nantennas, npol);
}

ACCs::~ACCs()
{
  delete m_buffer[m_front];
  delete m_buffer[m_back];
}

ACC& ACCs::getFront() const
{
  return *m_buffer[m_front];
}

ACC& ACCs::getBack() const
{
  return *m_buffer[m_back];
}

void ACCs::swap()
{
  int tmp = m_front;
  m_front = m_back;
  m_back = tmp;
}

const ACC* ACCLoader::loadFromFile(string filename)
{
  ACC* acc = 0;
  Array<complex<double>, 5> acc_array;

  ifstream accstream(filename.c_str());
  
  if (accstream.is_open())
    {
      accstream >> acc_array;
    }

  acc = new ACC();
  acc->setACC(acc_array);

  return acc;
}



