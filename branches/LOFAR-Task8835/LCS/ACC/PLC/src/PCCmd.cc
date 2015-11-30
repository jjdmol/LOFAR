//# PCCmd.cc: Names of the PLC commands.
//#
//# Copyright (C) 2007
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>

//# Includes
#include <PLC/PCCmd.h>


namespace LOFAR {
  namespace ACC {
    namespace PLC {

static	const char*	PlcCmdNames[] = {
		"Boot",
		"Quit", 		"Define",
		"Init", 		"Pause",
		"Run", 			"Release",
		"Snapshot",		"Recover",
		"Reinit",		"Params",
		"Info",			"Answer",
		"Report",		"Async"
};

//
// PCCmdName(value)
//
string	PCCmdName (PCCmd		PCcmdValue) {
	PCCmd		orgCmd(orgPCCmd(PCcmdValue));
	uint16		cmdValue((uint16)orgCmd - (uint16)(PCCmdBoot));

	if (cmdValue > sizeof (PlcCmdNames)) {
		return ("Unknown command");
	}

	string	name("PC");
	name += PlcCmdNames[cmdValue];
	if (isPLCresult(PCcmdValue)) {
		name += "Result";
	}

	return (name);
}


    } // namespace PLC
  } // namespace ACC
} // namespace LOFAR

