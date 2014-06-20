//# ACCmd.h: Values and names of the AC commands.
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

#ifndef LOFAR_ALC_AC_CMD_H
#define LOFAR_ALC_AC_CMD_H

// \file
// Values and names of the ALC commands

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>

namespace LOFAR {
  namespace ACC {
    namespace ALC {
// \addtogroup ALC
// @{


// The ACCmd enumeration is a list of command(numbers) that are used to
// tell the ApplControl server-side what command should be invoked.
// In a result message the command value is OR'ed with the \c ACCmdResult
// mask.
enum ACCmd {    ACCmdNone = 0, ACCmdBoot, 
				ACCmdQuit, 
				ACCmdDefine,   ACCmdInit,
				ACCmdPause,    ACCmdRun, 
				ACCmdRelease,
				ACCmdSnapshot, ACCmdRecover, 
				ACCmdReinit, 
				ACCmdInfo,     ACCmdAnswer,
				ACCmdReport,   ACCmdAsync,
			    ACCmdCancelQueue,
				ACCmdResult = 0x1000
};

// Return the name of the given command.
string 	ACCmdName	(ACCmd		ACCmdValue);

// Return whether or not the value represents a result.
inline	bool	isALCresult	(ACCmd		ACCmdValue)
{
	return (ACCmdValue & ACCmdResult);
}

// Return ACCmd to which the given result belongs.
inline ACCmd	orgACCmd	(ACCmd		cmdOrResult)
{
	return ((ACCmd)(cmdOrResult & (uint16)~ACCmdResult));
}

// @} addgroup
    } // namespace ALC
  } // namespace ACC
} // namespace LOFAR

#endif
