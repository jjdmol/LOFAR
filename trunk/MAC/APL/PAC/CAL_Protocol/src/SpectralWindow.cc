//#  -*- mode: c++ -*-
//#  SpectralWindow.cc: implementation of the SpectralWindow class
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

#include "SpectralWindow.h"

#include <blitz/array.h>
#include <sstream>

#include <Marshalling.h>

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace CAL;

#define SPW_DIMS    2
#define SPW_NPARAMS 3 /* sampling_freq, nyquist_zone, numsubbands */

SpectralWindow::SpectralWindow() :
  m_name("undefined"), m_sampling_freq(0), m_nyquist_zone(0), m_numsubbands(0)
{
}

SpectralWindow::~SpectralWindow()
{
}

double SpectralWindow::getSubbandFreq(int subband) const
{
  ASSERT(m_numsubbands);
  ASSERT(subband >= 0 && subband <= m_numsubbands);

  return (subband % m_numsubbands) * getSubbandWidth();
}

unsigned int SpectralWindow::getSize() const
{
  return MSH_STRING_SIZE(m_name) + sizeof(double) + sizeof(uint16) + sizeof(uint16);
}

unsigned int SpectralWindow::pack(void* buffer) const
{
  unsigned int offset = 0;

  MSH_PACK_STRING(buffer, offset, m_name);
  memcpy(((char*)buffer) + offset, &m_sampling_freq, sizeof(double));
  offset += sizeof(double);
  memcpy(((char*)buffer) + offset, &m_nyquist_zone, sizeof(uint16));
  offset += sizeof(uint16);
  memcpy(((char*)buffer) + offset, &m_numsubbands, sizeof(uint16));
  offset += sizeof(uint16);

  return offset;
}

unsigned int SpectralWindow::unpack(void* buffer)
{
  unsigned int offset = 0;

  MSH_UNPACK_STRING(buffer, offset, m_name);
  memcpy(&m_sampling_freq, ((char*)buffer) + offset, sizeof(double));
  offset += sizeof(double);
  memcpy(&m_nyquist_zone, ((char*)buffer) + offset, sizeof(uint16));
  offset += sizeof(uint16);
  memcpy(&m_numsubbands, ((char*)buffer) + offset, sizeof(uint16));
  offset += sizeof(uint16);

  return offset;
}
