//# tSystemUtil.cc: test program for the SystemUtil source
//#
//# Copyright (C) 2006-2007
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

#include <Common/StringUtil.h>
#include <Common/SystemUtil.h>
#include <Common/hexdump.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace LOFAR;
using namespace std;

int main()
{
	cout << "My short hostname is: " << myHostname(false) << endl;
	cout << "My long hostname is : " << myHostname(true) << endl;
	uint32	address = myIPV4Address();
	cout << formatString("My IPV4 address is  : %08lX\n", ntohl(address));
	hexdump ((char*) &address, sizeof(uint32));
	
	return (0);
}
