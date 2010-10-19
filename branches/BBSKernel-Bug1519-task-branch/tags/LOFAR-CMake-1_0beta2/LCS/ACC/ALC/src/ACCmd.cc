//# ACCmd.cc: Names of the ALC commands.
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
#include <ALC/ACCmd.h>


namespace LOFAR {
  namespace ACC {
    namespace ALC {

static	char*	AlcCmdNames[] = {
		"No command",	"Boot",
		"Quit", 		"Define",
		"Init", 		"Pause",
		"Run", 			"Release",
		"Snapshot",		"Recover",
		"Reinit",
		"Info",			"Answer",
		"Report",		"Async",
		"CancelQueue"
};

string	ACCmdName (ACCmd		ACcmdValue) {
	ACCmd		orgCmd(orgACCmd(ACcmdValue));
	uint16		cmdValue((uint16)orgCmd);

	if (cmdValue > sizeof (AlcCmdNames)) {
		return ("Unknown command");
	}

	string	name("AC");
	name += AlcCmdNames[cmdValue];
	if (isALCresult(ACcmdValue)) {
		name += "Result";
	}

	return (name);
}


    } // namespace ALC
  } // namespace ACC
} // namespace LOFAR

