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
#include <APL/RTCCommon/MarshallBlitz.h>

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
    + hbaprotocol_state.getSize()
    + rsuclear_state.getSize()
    + diagwgsettings_state.getSize()
    + sst_state.getSize()
    + bst_state.getSize()
    + xst_state.getSize()
    + cdo_state.getSize()
    + bs_state.getSize()
    + tdclear_state.getSize()
    + tdwrite_state.getSize()
    + tdread_state.getSize()
    + rad_state.getSize()
    + ts_state.getSize()
    + tdstatuswrite_state.getSize()
    + tdstatusread_state.getSize()
    + tbbsettings_state.getSize()
    + tbbbandsel_state.getSize()
    + bypasssettings_state.getSize()
	+ rawdatawrite_state.getSize()
	+ rawdataread_state.getSize();
}

unsigned int AllRegisterState::pack  (void* buffer)
{
	unsigned int offset = 0;

	offset += sys_state.pack((char*)buffer + offset);
	offset += bf_state.pack((char*)buffer + offset);
	offset += ss_state.pack((char*)buffer + offset);
	offset += rcusettings_state.pack((char*)buffer + offset);
	offset += rcuprotocol_state.pack((char*)buffer + offset);
	offset += hbaprotocol_state.pack((char*)buffer + offset);
	offset += rsuclear_state.pack((char*)buffer + offset);
	offset += diagwgsettings_state.pack((char*)buffer + offset);
	offset += sst_state.pack((char*)buffer + offset);
	offset += bst_state.pack((char*)buffer + offset);
	offset += xst_state.pack((char*)buffer + offset);
	offset += cdo_state.pack((char*)buffer + offset);
	offset += bs_state.pack((char*)buffer + offset);
	offset += tdclear_state.pack((char*)buffer + offset);
	offset += tdwrite_state.pack((char*)buffer + offset);
	offset += tdread_state.pack((char*)buffer + offset);
	offset += rad_state.pack((char*)buffer + offset);
	offset += ts_state.pack((char*)buffer + offset);
	offset += tdstatuswrite_state.pack((char*)buffer + offset);
	offset += tdstatusread_state.pack((char*)buffer + offset);
	offset += tbbsettings_state.pack((char*)buffer + offset);
	offset += tbbbandsel_state.pack((char*)buffer + offset);
	offset += bypasssettings_state.pack((char*)buffer + offset);
	offset += rawdatawrite_state.pack((char*)buffer + offset);
	offset += rawdataread_state.pack((char*)buffer + offset);

	return (offset);
}

unsigned int AllRegisterState::unpack(void *buffer)
{
	unsigned int offset = 0;

	offset += sys_state.unpack((char*)buffer + offset);
	offset += bf_state.unpack((char*)buffer + offset);
	offset += ss_state.unpack((char*)buffer + offset);
	offset += rcusettings_state.unpack((char*)buffer + offset);
	offset += rcuprotocol_state.unpack((char*)buffer + offset);
	offset += hbaprotocol_state.unpack((char*)buffer + offset);
	offset += rsuclear_state.unpack((char*)buffer + offset);
	offset += diagwgsettings_state.unpack((char*)buffer + offset);
	offset += sst_state.unpack((char*)buffer + offset);
	offset += bst_state.unpack((char*)buffer + offset);
	offset += xst_state.unpack((char*)buffer + offset);
	offset += cdo_state.unpack((char*)buffer + offset);
	offset += bs_state.unpack((char*)buffer + offset);
	offset += tdclear_state.unpack((char*)buffer + offset);
	offset += tdwrite_state.unpack((char*)buffer + offset);
	offset += tdread_state.unpack((char*)buffer + offset);
	offset += rad_state.unpack((char*)buffer + offset);
	offset += ts_state.unpack((char*)buffer + offset);
	offset += tdstatuswrite_state.unpack((char*)buffer + offset);
	offset += tdstatusread_state.unpack((char*)buffer + offset);
	offset += tbbsettings_state.unpack((char*)buffer + offset);
	offset += tbbbandsel_state.unpack((char*)buffer + offset);
	offset += bypasssettings_state.unpack((char*)buffer + offset);
	offset += rawdatawrite_state.unpack((char*)buffer + offset);
	offset += rawdataread_state.unpack((char*)buffer + offset);

	return (offset);
}
