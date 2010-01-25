//#  tAntennaSet.cc
//#
//#  Copyright (C) 2008
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <APL/APLCommon/AntennaPos.h>

using namespace LOFAR;
using namespace LOFAR::APLCommon;

int main (int	argc, char* argv[]) 
{
	INIT_VAR_LOGGER(argv[0], argv[0]);

	AntennaPos	theAP("AntennaPos1.conf");	// read the AntennaPos.conf file into memory

	// Show the names of the sets.
	LOG_DEBUG_STR("The AntennaPos1.conf file containes the following definitions:");
	LOG_DEBUG_STR("LBA count        : " << theAP.nrLBAs());
	LOG_DEBUG_STR("LBA centre       : " << theAP.LBACentre());
	LOG_DEBUG_STR("LBA Ant positions: " << theAP.LBAAntPos());
	LOG_DEBUG_STR("LBA RCU positions: " << theAP.LBARCUPos());
	LOG_DEBUG_STR("LBA RCU lengths  : " << theAP.LBARCULengths());
	
	LOG_DEBUG_STR("HBA count        : " << theAP.nrHBAs());
	LOG_DEBUG_STR("HBA centre       : " << theAP.HBACentre());
	LOG_DEBUG_STR("HBA Ant positions: " << theAP.HBAAntPos());
	LOG_DEBUG_STR("HBA RCU positions: " << theAP.HBARCUPos());
	LOG_DEBUG_STR("HBA RCU lengths  : " << theAP.HBARCULengths());

	return (0);
}

