//#  AllRegisterState.h: implementation of the AllRegisterState class
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

#include <APL/RSP_Protocol/AllRegisterState.h>
#include <APL/RTCCommon/Marshalling.h>

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace RSP_Protocol;

unsigned int AllRegisterState::getSize()
{
  return
      sys_state.getSize()
    + bf_state.getSize()
    + ss_state.getSize()
    + rcusettings_state.getSize()
    + rcuprotocol_state.getSize()
    + rsuclear_state.getSize()
    + diagwgsettings_state.getSize()
    + sst_state.getSize()
    + bst_state.getSize()
    + xst_state.getSize()
    + cdo_state.getSize()
    + bs_state.getSize()
    + tds_state.getSize();
}

unsigned int AllRegisterState::pack  (void* buffer)
{
  unsigned int offset = 0;

  offset = 
      sys_state.pack(buffer)
    + bf_state.pack(buffer)
    + ss_state.pack(buffer)
    + rcusettings_state.pack(buffer)
    + rcuprotocol_state.pack(buffer)
    + rsuclear_state.pack(buffer)
    + diagwgsettings_state.pack(buffer)
    + sst_state.pack(buffer)
    + bst_state.pack(buffer)
    + xst_state.pack(buffer)
    + cdo_state.pack(buffer)
    + bs_state.pack(buffer)
    + tds_state.pack(buffer);

  return offset;
}

unsigned int AllRegisterState::unpack(void *buffer)
{
  unsigned int offset = 0;

  offset = 
      sys_state.unpack(buffer)
    + bf_state.unpack(buffer)
    + ss_state.unpack(buffer)
    + rcusettings_state.unpack(buffer)
    + rcuprotocol_state.unpack(buffer)
    + rsuclear_state.unpack(buffer)
    + diagwgsettings_state.unpack(buffer)
    + sst_state.unpack(buffer)
    + bst_state.unpack(buffer)
    + xst_state.unpack(buffer)
    + cdo_state.unpack(buffer)
    + bs_state.unpack(buffer)
    + tds_state.unpack(buffer);

  return offset;
}
