//# testHexdump.cc: Program to test the hexdump functions
//#
//# Copyright (C) 2004
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/LofarLogger.h>	       // contains the API of the LofarLogger
#include <Common/hexdump.h>
#include <Common/StringUtil.h>
#include <cstring>
#include <cstdio>

using namespace LOFAR;


int main (int, char *argv[]) {

	// Read in the log-environment configuration
	// We should always start with this.
	INIT_LOGGER("tHexdump");

	// Show operator were are on the air
	LOG_INFO (formatString("Program %s has started", argv[0]));

	LOG_INFO ("testing hexdump(stdout, ...)");
	char	stdoutTest [80];
	strcpy (stdoutTest, "this should appear on standard out");
	hexdump (stdoutTest, strlen(stdoutTest));

	LOG_INFO ("testing hexdump(FILE*, ...)");
	char	fileTest [80];
	strcpy (fileTest,"this should appear in the  file tHexdump_tmp.txt");
	FILE*	fd = fopen("./tHexdump_tmp.txt", "w");
	hexdump(fd, fileTest, strlen(fileTest));
	fclose(fd);

	LOG_INFO ("testing hexdump(char*, ...)");
	char	charPtrTest [80];
	char	hexdumpInfo [380];	// approx 4.8 * input string size
	strcpy(charPtrTest, "this should appear in the given string");
	hexdump(hexdumpInfo, charPtrTest, strlen(charPtrTest));
	cout << hexdumpInfo << endl;

	LOG_INFO ("testing hexdump(string, ...)");
	string		hexdumpString;
	char		stringTest [80];
	strcpy(stringTest, "this should appear in the C++ string");
	hexdump(hexdumpString, stringTest, strlen(stringTest));
	cout << hexdumpString << endl;

	LOG_INFO("Normal termination of program");
	return (0);
}

