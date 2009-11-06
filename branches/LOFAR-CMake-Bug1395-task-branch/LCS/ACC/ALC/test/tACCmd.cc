//# tACCmd.cc: 
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
#include <Common/StringUtil.h>
#include <ALC/ACCmd.h>

using namespace LOFAR;
using namespace LOFAR::ACC::ALC;

#define NAME_TEST(cmd)	cout <<	"value: " << (cmd) << " = " << ACCmdName(cmd) << endl;

int main (int /*argc*/, char** /* argv*/) {
	NAME_TEST(ACCmdNone);
	NAME_TEST(ACCmdBoot);
	NAME_TEST(ACCmdQuit);
	NAME_TEST(ACCmdDefine);
	NAME_TEST(ACCmdInit);
	NAME_TEST(ACCmdPause);
	NAME_TEST(ACCmdRun);
	NAME_TEST(ACCmdRelease);
	NAME_TEST(ACCmdSnapshot);
	NAME_TEST(ACCmdRecover);
	NAME_TEST(ACCmdReinit);
	NAME_TEST(ACCmdInfo);
	NAME_TEST(ACCmdAnswer);
	NAME_TEST(ACCmdReport);
	NAME_TEST(ACCmdAsync);
	NAME_TEST(ACCmdCancelQueue);
	NAME_TEST(ACCmdResult);

	NAME_TEST((ACCmd)(ACCmdNone | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdBoot | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdQuit | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdDefine | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdInit | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdPause | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdRun | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdRelease | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdSnapshot | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdRecover | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdReinit | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdInfo | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdAnswer | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdReport | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdAsync | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdCancelQueue | ACCmdResult));

	return (0);
}

