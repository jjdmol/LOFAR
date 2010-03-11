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
//#  $Id: ACC.cc 6967 2005-10-31 16:28:09Z wierenga $

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <APL/CAL_Protocol/ACC.h>
#include <blitz/array.h>
#include <fstream>
#include <stdio.h>

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace CAL;
using namespace RTC;

ACC::ACC(int nsubbands, int nantennas, int npol) :
  SharedResource(1,1), // at most one reader or one writer
  m_acc(nsubbands, npol, npol, nantennas, nantennas),
  m_time(nsubbands), m_valid(false), m_antenna_count(0)
{
}

ACC::~ACC()
{
}

void ACC::setSelection(const blitz::Array<bool, 2>& antenna_selection)
{
  ASSERT(antenna_selection.extent(firstDim)  == m_acc.extent(fourthDim));
  ASSERT(antenna_selection.extent(secondDim) == m_acc.extent(secondDim));
  m_antenna_selection = antenna_selection;

  m_antenna_count = 0;
  for (int i = 0; i < m_antenna_selection.extent(firstDim); ++i) {
    if (sum(m_antenna_selection(i, Range::all())) > 0) m_antenna_count++;
  }
}

const Array<complex<double>, 2> ACC::getACM(int subband, int pol1, int pol2, Timestamp& timestamp)
{
  Range all = Range::all();
  timestamp = Timestamp(0,0);

  // check range of subband argument
  ASSERT(subband >= 0 && subband < m_acc.extent(firstDim));
  ASSERT(pol1 == 0 || pol1 == 1);
  ASSERT(pol2 == 0 || pol2 == 1);

  timestamp = m_time(subband);

  if (m_antenna_count == m_acc.extent(fourthDim)) {

    // return slice of the full ACC
    return m_acc(subband, pol1, pol2, all, all);

  } else {

    // make selection
    m_current_acm.resize(m_antenna_count, m_antenna_count);
    int k = 0;
    for (int i = 0; i < m_acc.extent(fourthDim); ++i) {
      if (sum(m_antenna_selection(i, Range::all())) > 0) {
	int l = 0;
	for (int j = 0; j < m_acc.extent(fifthDim); ++j) {
	  if (sum(m_antenna_selection(j, Range::all())) > 0) {
	    m_current_acm(k, l) = m_acc(subband, pol1, pol2, i, j);
	  }
	  l++;
	}
	k++;
      }
    }

    return m_current_acm;
  }
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

  // select all antennas
  m_antenna_count = m_acc.extent(fourthDim);
  m_antenna_selection.resize(m_acc.extent(fourthDim), m_acc.extent(secondDim));
  m_antenna_selection = true;
}

int ACC::getFromFile(string filename)
{
  Array<complex<double>, 5> acc_array;

  LOG_INFO_STR("Attempting to read ACC array with shape='" << m_acc.shape() << "' from '" << filename);

  ifstream accstream(filename.c_str());
  
  if (accstream.is_open()) {
    accstream >> acc_array;
  } else {
    LOG_WARN_STR("Failed to open file: " << filename);
    return -1;
  }

  for (int i = 0; i < 5; i++) {
    ASSERT(acc_array.extent(i) == m_acc.extent(i));
  }

  setACC(acc_array);

  LOG_INFO_STR("Done reading ACC array");

  return 0;
}

int ACC::getFromBinaryFile(string filename)
{
  LOG_INFO_STR("Attempting to read binary ACC array with shape='" << m_acc.shape() << "' from '" << filename);

  FILE* f = fopen(filename.c_str(), "r");

  if (!f) {
    LOG_WARN_STR("Failed to open file: " << filename);
    return -1;
  }

  size_t nread = fread(m_acc.data(), sizeof(complex<double>), m_acc.size(), f);
  if (nread != (size_t)m_acc.size()) {
    LOG_WARN_STR("Warning: read " << nread << " items but expected " << m_acc.size());
    fclose(f);
    return -1;
  }

  fclose(f);

  LOG_INFO_STR("Done reading ACC array");

  return 0;
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




