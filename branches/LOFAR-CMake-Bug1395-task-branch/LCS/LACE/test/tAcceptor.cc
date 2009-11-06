//# tAcceptor.cc: test program for the DeviceAddress class
//#
//# Copyright (C) 2008
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
#include <Common/LofarLogger.h>
#include <LACE/File.h>
#include <LACE/SocketAcceptor.h>
//#include <LACE/InetAddress.h>

using namespace LOFAR;
using namespace LOFAR::LACE;
using namespace std;

bool SocketTest()
{
	// NOTE: these tests assume a listener at 2345;

	cout << "\nOpening socket" << endl;
	InetAddress		myIA;
	SocketStream	myStream;
	SocketAcceptor	myAcceptor;

	cout << "setting address ..." << endl;
	if (myIA.set(2313, "localhost", "tcp") != 0) {
		THROW(Exception, "set Address failed");
	}
	cout << "opening listener ..." << endl;
	if (myAcceptor.open(myIA) != 0) {
		THROW(Exception, "Opening listener failed");
	}
	cout << "accepting connections ..." << endl;
	if (myAcceptor.accept(myStream) != 0) {
		THROW(Exception, "Accept failed");
	}

	sleep (4);
//	myStream.read/write(...);

	myStream.close();


	myAcceptor.close();

	return (true);
}


bool FileTest()
{
	cout << "\nTesting the File class...\n";

	return (true);
}


int main()
{
  INIT_LOGGER("tAcceptor");

  bool result = 
    FileTest() 		&&
    SocketTest();

  return (result ? 0 : 1);
}
