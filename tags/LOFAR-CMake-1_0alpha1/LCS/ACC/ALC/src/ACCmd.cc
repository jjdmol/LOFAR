//#  ACCmd.cc: Names of the ALC commands.
//#
//#  Copyright (C) 2007
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
//#	 Abstract:
//#	 This class implements the command protocol between an application manager
//#	 (MAC for instance) and an Application Controller (=ACC package).
//#	 The AM has the client role, the AC the server role.
//#
//#  $Id$

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

