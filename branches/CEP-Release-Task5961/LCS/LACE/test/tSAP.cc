//# tSAP.cc: test program for the DeviceAddress class
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
#include <LACE/SocketSAP.h>

using namespace LOFAR;
using namespace LOFAR::LACE;
using namespace std;

bool SocketTest()
{
	// There is not much we can test as long as there is no
	// connector and acceptor.

	cout << "\nTesting the 'open' function of a UDP Socket...\n";
	InetAddress		myIA1;
	cout << "Setting device to 1234@lofar17:udp\n";
	ASSERT(myIA1.set(1234, "lofar17", "udp") == 0);
	cout << "DeviceName : " << myIA1.deviceName() << endl;

	cout << "\nOpening socket" << endl;
	SocketSAP		mySock;
	mySock.doOpen(myIA1);
	mySock.close();


	cout << "\nTesting the 'open' function of a TCP Socket...\n";
	InetAddress		myIA2;
	cout << "Setting device to ssh@lofar17:tcp\n";
	ASSERT(myIA2.set("ssh", "lofar17", "tcp") == 0);
	cout << "DeviceName : " << myIA2.deviceName() << endl;

	cout << "\nOpening socket" << endl;
	mySock.doOpen(myIA2);
	mySock.close();

	return (true);
}


bool FileTest()
{
	cout << "\nTesting the File class...\n";

	FileAddress		myFA1;
	cout << "Setting filename to testFile:w\n";
	ASSERT(myFA1.set("testFile", "w") == 0);		// write + truncate + create
	cout << "FileName : " << myFA1.deviceName() << endl;
	cout << "AddrName : " << (char*)myFA1.getAddress() << endl;

	File		myFile;
	ASSERT(myFile.open(myFA1) > 0);
	cout << "pointer is at: " << myFile.tell() << endl;

	cout << "\nWriting 'Happy xmas' to the file" << endl;
	char*	hw = { "Happy xmas" };
	myFile.write(hw, strlen(hw));
	int		firstLen = myFile.tell();
	cout << "pointer is at: " << firstLen << endl;
	cout << "closing file." << endl;

	cout << "\nReopening file for append" << endl;
	ASSERT(myFA1.set("testFile", "a") == 0);		// write + append + create
	myFile.open(myFA1);
	cout << "pointer is at: " << myFile.tell() << endl;

	cout << "\nAppending ' to you all!' to the file" << endl;
	string	tya(" to you all!");
	myFile.write(tya.c_str(), tya.size());
	cout << "pointer is at: " << myFile.tell() << endl;
	cout << "closing file." << endl;

	cout << "\nReopening file for read" << endl;
	ASSERT(myFA1.set("testFile", "r") == 0);		// read
	myFile.open(myFA1);
	cout << "pointer is at: " << myFile.tell() << endl;
	
	cout << "\nReading contents of the file" << endl;
	char	buffer [1024];
	int	nrBytes = myFile.read(buffer, 1023);
	buffer[nrBytes] = '\0';
	cout << "contents : " << buffer << endl;
	
	cout << "\nReposition at postion " << ++firstLen << endl;
	myFile.seek(firstLen, SEEK_SET);
	nrBytes = myFile.read(buffer, 1023);
	buffer[nrBytes] = '\0';
	cout << "contents : " << buffer << endl;
	myFile.close();

	cout << "\nDeleting file" << endl;
	myFile.remove();

	return (true);
}


int main()
{
  INIT_LOGGER("tSAP");

  bool result = 
    FileTest() 		&&
    SocketTest();

  return (result ? 0 : 1);
}
