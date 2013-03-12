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

size_t AllRegisterState::getSize() const
{
  return
      sys_state.getSize()
    + bf_state.getSize()
    + ss_state.getSize()
    + rcusettings_state.getSize()
    + rcuprotocol_state.getSize()
    + itsRcuReadState.getSize()
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
    + itsCRstate.getSize()
    + ts_state.getSize()
    + tdstatuswrite_state.getSize()
    + tdstatusread_state.getSize()
    + tbbsettings_state.getSize()
    + tbbbandsel_state.getSize()
    + bypasssettings_state.getSize()
	+ rawdatawrite_state.getSize()
	+ rawdataread_state.getSize()
	+ itsSerdesWriteState.getSize()
	+ itsSerdesReadState.getSize()
	+ itsBitModeWriteState.getSize();
}

size_t AllRegisterState::pack  (char* buffer) const
{
	size_t offset = 0;

	offset += sys_state.pack(buffer + offset);
	offset += bf_state.pack(buffer + offset);
	offset += ss_state.pack(buffer + offset);
	offset += rcusettings_state.pack(buffer + offset);
	offset += rcuprotocol_state.pack(buffer + offset);
	offset += itsRcuReadState.pack(buffer + offset);
	offset += hbaprotocol_state.pack(buffer + offset);
	offset += rsuclear_state.pack(buffer + offset);
	offset += diagwgsettings_state.pack(buffer + offset);
	offset += sst_state.pack(buffer + offset);
	offset += bst_state.pack(buffer + offset);
	offset += xst_state.pack(buffer + offset);
	offset += cdo_state.pack(buffer + offset);
	offset += bs_state.pack(buffer + offset);
	offset += tdclear_state.pack(buffer + offset);
	offset += tdwrite_state.pack(buffer + offset);
	offset += tdread_state.pack(buffer + offset);
	offset += rad_state.pack(buffer + offset);
	offset += itsCRstate.pack(buffer + offset);
	offset += ts_state.pack(buffer + offset);
	offset += tdstatuswrite_state.pack(buffer + offset);
	offset += tdstatusread_state.pack(buffer + offset);
	offset += tbbsettings_state.pack(buffer + offset);
	offset += tbbbandsel_state.pack(buffer + offset);
	offset += bypasssettings_state.pack(buffer + offset);
	offset += rawdatawrite_state.pack(buffer + offset);
	offset += rawdataread_state.pack(buffer + offset);
	offset += itsSerdesWriteState.pack(buffer + offset);
	offset += itsSerdesReadState.pack(buffer + offset);
	offset += itsBitModeWriteState.pack(buffer + offset);

	return (offset);
}

size_t AllRegisterState::unpack(const char *buffer)
{
	size_t offset = 0;

	offset += sys_state.unpack(buffer + offset);
	offset += bf_state.unpack(buffer + offset);
	offset += ss_state.unpack(buffer + offset);
	offset += rcusettings_state.unpack(buffer + offset);
	offset += rcuprotocol_state.unpack(buffer + offset);
	offset += itsRcuReadState.unpack(buffer + offset);
	offset += hbaprotocol_state.unpack(buffer + offset);
	offset += rsuclear_state.unpack(buffer + offset);
	offset += diagwgsettings_state.unpack(buffer + offset);
	offset += sst_state.unpack(buffer + offset);
	offset += bst_state.unpack(buffer + offset);
	offset += xst_state.unpack(buffer + offset);
	offset += cdo_state.unpack(buffer + offset);
	offset += bs_state.unpack(buffer + offset);
	offset += tdclear_state.unpack(buffer + offset);
	offset += tdwrite_state.unpack(buffer + offset);
	offset += tdread_state.unpack(buffer + offset);
	offset += rad_state.unpack(buffer + offset);
	offset += itsCRstate.unpack(buffer + offset);
	offset += ts_state.unpack(buffer + offset);
	offset += tdstatuswrite_state.unpack(buffer + offset);
	offset += tdstatusread_state.unpack(buffer + offset);
	offset += tbbsettings_state.unpack(buffer + offset);
	offset += tbbbandsel_state.unpack(buffer + offset);
	offset += bypasssettings_state.unpack(buffer + offset);
	offset += rawdatawrite_state.unpack(buffer + offset);
	offset += rawdataread_state.unpack(buffer + offset);
	offset += itsSerdesWriteState.unpack(buffer + offset);
	offset += itsSerdesReadState.unpack(buffer + offset);
	offset += itsBitModeWriteState.unpack(buffer + offset);

	return (offset);
}
