//#  tAntennaField.cc
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
//#  $Id: tAntennaField.cc 15222 2010-03-15 14:27:41Z loose $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <ApplCommon/AntennaField.h>

using namespace LOFAR;

int main (int, char* argv[]) 
{
	INIT_VAR_LOGGER(argv[0], argv[0]);

	AntennaField	theAP("tAntennaField.in");	// read the tAntennaField.in file into memory

	// Show the names of the sets.
	LOG_DEBUG_STR("The tAntennaField.in file containes the following definitions:");
	LOG_DEBUG_STR("LBA count        : " << theAP.nrAnts("LBA"));
	LOG_DEBUG_STR("LBA centre       : " << theAP.Centre("LBA"));
	LOG_DEBUG_STR("LBA normVector   : " << theAP.normVector("LBA"));
	LOG_DEBUG_STR("LBA rot.Matrix   : " << theAP.rotationMatrix("LBA"));
	LOG_DEBUG_STR("LBA Ant positions: " << theAP.AntPos("LBA"));
	LOG_DEBUG_STR("LBA RCU positions: " << theAP.RCUPos("LBA"));
	LOG_DEBUG_STR("LBA RCU lengths  : " << theAP.RCULengths("LBA"));
	
	LOG_DEBUG_STR("HBA count        : " << theAP.nrAnts("HBA"));
	LOG_DEBUG_STR("HBA centre       : " << theAP.Centre("HBA"));
	LOG_DEBUG_STR("HBA normVector   : " << theAP.normVector("HBA"));
	LOG_DEBUG_STR("HBA rot.Matrix   : " << theAP.rotationMatrix("HBA"));
	LOG_DEBUG_STR("HBA Ant positions: " << theAP.AntPos("HBA"));
	LOG_DEBUG_STR("HBA RCU positions: " << theAP.RCUPos("HBA"));
	LOG_DEBUG_STR("HBA RCU lengths  : " << theAP.RCULengths("HBA"));
	
	LOG_DEBUG_STR("HBA0 count        : " << theAP.nrAnts("HBA0"));
	LOG_DEBUG_STR("HBA0 centre       : " << theAP.Centre("HBA0"));
	LOG_DEBUG_STR("HBA0 normVector   : " << theAP.normVector("HBA0"));
	LOG_DEBUG_STR("HBA0 rot.Matrix   : " << theAP.rotationMatrix("HBA0"));
	LOG_DEBUG_STR("HBA0 Ant positions: " << theAP.AntPos("HBA0"));
	LOG_DEBUG_STR("HBA0 RCU positions: " << theAP.RCUPos("HBA0"));
	LOG_DEBUG_STR("HBA0 RCU lengths  : " << theAP.RCULengths("HBA0"));

	LOG_DEBUG_STR("HBA1 count        : " << theAP.nrAnts("HBA1"));
	LOG_DEBUG_STR("HBA1 centre       : " << theAP.Centre("HBA1"));
	LOG_DEBUG_STR("HBA1 normVector   : " << theAP.normVector("HBA1"));
	LOG_DEBUG_STR("HBA1 rot.Matrix   : " << theAP.rotationMatrix("HBA1"));
	LOG_DEBUG_STR("HBA1 Ant positions: " << theAP.AntPos("HBA1"));
	LOG_DEBUG_STR("HBA1 RCU positions: " << theAP.RCUPos("HBA1"));
	LOG_DEBUG_STR("HBA1 RCU lengths  : " << theAP.RCULengths("HBA1"));

	return (0);
}

