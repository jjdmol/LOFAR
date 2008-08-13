//#  tConnector.cc: test program for the DeviceAddress class
//#
//#  Copyright (C) 2008
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/StringUtil.h>
#include <Common/LofarLogger.h>
#include <LACE/File.h>
#include <LACE/SocketConnector.h>
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
	SocketConnector	myConnector;

	if (myIA.set(2313, "localhost", "tcp") != 0) {
		THROW(Exception, "set Address failed");
	}
	if (myConnector.connect(myStream, myIA) != 0) {
		THROW(Exception, "Connect failed");
	}

	return (true);
}


bool FileTest()
{
	cout << "\nTesting the File class...\n";

	return (true);
}


int main()
{
  INIT_LOGGER("tConnector");

  bool result = 
    FileTest() 		&&
    SocketTest();

  return (result ? 0 : 1);
}
