//#  SystemStatus.h: implementation of the SystemStatus class
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

#include "SystemStatus.h"
#include "EPA_Protocol.ph"
#include "MEPHeader.h"
#include "Marshalling.h"
#include <blitz/array.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
using namespace LOFAR;

using namespace RSP_Protocol;
using namespace EPA_Protocol;
using namespace std;
using namespace blitz;

unsigned int SystemStatus::getSize()
{
  return
      MSH_ARRAY_SIZE(m_board_status, EPA_Protocol::BoardStatus)
    + MSH_ARRAY_SIZE(m_rcu_status,   EPA_Protocol::RCUStatus);
}

unsigned int SystemStatus::pack  (void* buffer)
{
  unsigned int offset = 0;
  
  MSH_PACK_ARRAY(buffer, offset, m_board_status, EPA_Protocol::BoardStatus);
  MSH_PACK_ARRAY(buffer, offset, m_rcu_status,   EPA_Protocol::RCUStatus);

  return offset;
}

unsigned int SystemStatus::unpack(void *buffer)
{
  unsigned int offset = 0;
  
  MSH_UNPACK_ARRAY(buffer, offset, m_board_status, EPA_Protocol::BoardStatus, 1);
  MSH_UNPACK_ARRAY(buffer, offset, m_rcu_status,   EPA_Protocol::RCUStatus,   1);

  return offset;
}
