//# tAddress.cc: test program for the Address class
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
#include <LACE/FileAddress.h>
#include <LACE/InetAddress.h>

using namespace LOFAR;
using namespace LOFAR::LACE;
using namespace std;

bool DeviceTest()
{
	cout << "\nCreating a blank Address...\n";
	Address		myDA(0);

	cout << "myDA.type  : " << myDA.getType() << endl;
	cout << "myDA.isSet : " << (myDA.isSet() ? "true" : "false") << endl;

	Address		myEqualDA(0);
	Address		myDifferentDA(1);
	cout << "equal      : " << (myDA == myEqualDA ? 	"true" : "false") << endl;
	cout << "equal      : " << (myDA != myDifferentDA ? "true" : "false") << endl;
	cout << "Not equal  : " << (myDA != myEqualDA ? 	"true" : "false") << endl;
	cout << "Not equal  : " << (myDA == myDifferentDA ? "true" : "false") << endl;

	cout << "Devicename : " << myDA.deviceName() << endl;

	return (true);
}


bool InetTest()
{
	cout << "\nTesting the derived InetAddress...\n";
	InetAddress		myIA1;
	cout << "myIA.type  = " << myIA1.getType() << endl;
	cout << "myIA.isSet = " << (myIA1.isSet() ? "true" : "false") << endl;

	cout << "Setting portnr to 0\n";
	ASSERT(myIA1.set(0) != 0);
	cout << "DeviceName : " << myIA1.deviceName() << endl;

	cout << "Setting portnr to 1234\n";
	ASSERT(myIA1.set(1234) == 0);
	cout << "DeviceName : " << myIA1.deviceName() << endl;

	cout << "Setting device to 1234@nohost\n";
	ASSERT(myIA1.set(1234, "nohost") != 0);
	cout << "DeviceName : " << myIA1.deviceName() << endl;

	cout << "Setting device to 1234@lofar17\n";
	ASSERT(myIA1.set(1234, "lofar17") == 0);
	cout << "DeviceName : " << myIA1.deviceName() << endl;

	cout << "Setting device to 1234@lofar17:whatProtocol\n";
	ASSERT(myIA1.set(1234, "lofar17", "whatProtocol") != 0);
	cout << "DeviceName : " << myIA1.deviceName() << endl;

	cout << "Setting device to 1234@lofar17:udp\n";
	ASSERT(myIA1.set(1234, "lofar17", "udp") == 0);
	cout << "DeviceName : " << myIA1.deviceName() << endl;

	InetAddress		myIA2;
	cout << "Setting device to whatService@lofar17:tcp\n";
	ASSERT(myIA2.set("whatService", "lofar17", "tcp") != 0);
	cout << "DeviceName : " << myIA2.deviceName() << endl;

	cout << "Setting device to ssh@lofar17:tcp\n";
	ASSERT(myIA2.set("ssh", "lofar17", "udp") == 0);
	cout << "DeviceName : " << myIA2.deviceName() << endl;

	InetAddress		myIA1copy;
	myIA1copy = myIA1;
	cout << "equal      : " << (myIA1 == myIA1copy ? 	"true" : "false") << endl;
	cout << "equal      : " << (myIA1 != myIA2 ? 		"true" : "false") << endl;
	cout << "Not equal  : " << (myIA1 != myIA1copy ? 	"true" : "false") << endl;
	cout << "Not equal  : " << (myIA1 == myIA2 ? 		"true" : "false") << endl;

	return (true);
}


bool FileTest()
{
	cout << "\nCreating a derived FileAddress...\n";

	FileAddress		myFA1;

	cout << "Setting filename to whatFile:r\n";
	ASSERT(myFA1.set("whatFile", "r") != 0);		// readonly, may not be created
	cout << "FileName : " << myFA1.deviceName() << endl;
	
	cout << "Setting filename to whatFile:w\n";
	ASSERT(myFA1.set("whatFile", "w") == 0);		// write + truncate + create
	cout << "FileName : " << myFA1.deviceName() << endl;
	
	cout << "Setting filename to whatFile:r+\n";
	ASSERT(myFA1.set("whatFile", "r+") != 0);		// read + write, NO create
	cout << "FileName : " << myFA1.deviceName() << endl;
	
	FileAddress		myFA2;
	cout << "Setting filename to /usr/include/errno.h:r\n";
	ASSERT(myFA2.set("/usr/include/errno.h", "r") == 0);
	cout << "FileName : " << myFA2.deviceName() << endl;
	
	cout << "Setting filename to /whatDir/myFile:w\n";
	ASSERT(myFA2.set("whatDir/myFile", "w") != 0);
	cout << "FileName : " << myFA2.deviceName() << endl;
	
	FileAddress		myFA1copy;
	myFA1copy = myFA1;
	cout << "equal      : " << (myFA1 == myFA1copy ? 	"true" : "false") << endl;
	cout << "equal      : " << (myFA1 != myFA2 ? 		"true" : "false") << endl;
	cout << "Not equal  : " << (myFA1 != myFA1copy ? 	"true" : "false") << endl;
	cout << "Not equal  : " << (myFA1 == myFA2 ? 		"true" : "false") << endl;

	return (true);
}


int main()
{
  INIT_LOGGER("tAddress");

  bool result = 
    DeviceTest()    &&
    FileTest() 		&&
    InetTest();

  return (result ? 0 : 1);
}
