//# tPCCmd.cc: 
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
#include <PLC/PCCmd.h>

using namespace LOFAR;
using namespace LOFAR::ACC::PLC;

#define NAME_TEST(cmd)	cout <<	"value: " << (cmd) << " = " << PCCmdName(cmd) << endl;

int main (int /*argc*/, char** /*argv*/) {
	NAME_TEST(PCCmdNone);
	NAME_TEST(PCCmdBoot);
	NAME_TEST(PCCmdQuit);
	NAME_TEST(PCCmdDefine);
	NAME_TEST(PCCmdInit);
	NAME_TEST(PCCmdPause);
	NAME_TEST(PCCmdRun);
	NAME_TEST(PCCmdRelease);
	NAME_TEST(PCCmdSnapshot);
	NAME_TEST(PCCmdRecover);
	NAME_TEST(PCCmdReinit);
	NAME_TEST(PCCmdParams);
	NAME_TEST(PCCmdInfo);
	NAME_TEST(PCCmdAnswer);
	NAME_TEST(PCCmdReport);
	NAME_TEST(PCCmdAsync);
	NAME_TEST(PCCmdResult);

	NAME_TEST((PCCmd)(PCCmdNone | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdBoot | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdQuit | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdDefine | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdInit | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdPause | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdRun | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdRelease | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdSnapshot | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdRecover | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdReinit | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdParams | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdInfo | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdAnswer | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdReport | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdAsync | PCCmdResult));

	return (0);
}

