//#  MEPHeader.h: implementation of the MEPHeader class
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

#include "MEPHeader.h"

#include <string.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace EPA_Protocol;
using namespace std;

unsigned int MEPHeader::getSize()
{
  return MEPHeader::SIZE;
}

unsigned int MEPHeader::pack  (void* buffer)
{
  memcpy(buffer, &(this->m_fields), MEPHeader::SIZE);
  return MEPHeader::SIZE;
}

unsigned int MEPHeader::unpack(void *buffer)
{
  memcpy(&(this->m_fields), buffer, MEPHeader::SIZE);
  return MEPHeader::SIZE;
}

void MEPHeader::set(uint8 type,
		    uint8 dstid,
		    uint8 pid,
		    uint8 regid,
		    uint16 size)
{
  memset(&m_fields, 0, sizeof(m_fields));
  m_fields.type = type;
  m_fields.addr.dstid = dstid;
  m_fields.addr.pid = pid;
  m_fields.addr.regid = regid;
  m_fields.size = size;
}
