//#  testHexdump.cc: Program to test the hexdump functions
//#
//#  Copyright (C) 2004
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

#include <Common/LofarLogger.h>	       // contains the API of the LofarLogger
#include <Common/hexdump.h>

using namespace std;
using namespace LOFAR;


int main (int, char *argv[]) {

	// Read in the log-environment configuration
	// We should always start with this.
#ifdef HAVE_LOG4CPLUS
	INIT_LOGGER("tHexdump.log_prop");
#else
	INIT_LOGGER("tHexdump.debug");
#endif

	// Show operator were are on the air
	LOG_INFO (formatString("Program %s has started", argv[0]));

	LOG_INFO ("testing hexdump(stdout, ...)");
	char	stdoutTest [80];
	strcpy (stdoutTest, "this should appear on standard out");
	hexdump (stdoutTest, strlen(stdoutTest));

	LOG_INFO ("testing hexdump(FILE*, ...)");
	char	fileTest [80];
	strcpy (fileTest, "this should appear in the file");
	FILE*	fd = fopen("./tHexdump_tmp.txt", "w");
	hexdump(fd, fileTest, strlen(fileTest));
	fclose(fd);
	system("cat ./tHexdump_tmp.txt");

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

