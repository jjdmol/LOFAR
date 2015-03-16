//#  MEPData.h: implementation of the MEPData class
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

#include <APL/RSP_Protocol/MEPData.h>

#include <string.h>

using namespace LOFAR;
using namespace EPA_Protocol;
using namespace std;

size_t MEPData::getSize() const
{
  return m_count;
}

size_t MEPData::pack  (char* buffer) const
{
  memcpy(buffer, m_dataptr, m_count);
  return m_count;
}

size_t MEPData::unpack(const char* buffer)
{
  if (m_count && m_dataptr) {
    memcpy(m_dataptr, buffer, m_count);
  }
  return m_count;
}

void MEPData::setBuffer(void* buf, size_t count)
{
  m_dataptr = buf;
  m_count   = count;
}

void* MEPData::getBuffer() const
{
  return m_dataptr;
}

size_t MEPData::getDataLen() const
{
  return m_count;
}

