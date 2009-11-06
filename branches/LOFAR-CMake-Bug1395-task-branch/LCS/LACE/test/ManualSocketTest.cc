//# ManualSocketTest.cc: Manual program to test Net/Socket code
//#
//# Copyright (C) 2002-2004
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
#include <Common/lofar_string.h>
#include <Common/LofarLogger.h>
#include <LACE/SocketAcceptor.h>
#include <LACE/SocketConnector.h>
#include <errno.h>

using namespace LOFAR;
using namespace LOFAR::LACE;


static bool				serverRole;
static InetAddress		theAddress;
static SocketAcceptor	theAcceptor;
static SocketConnector	theConnector;
static SocketStream		dataStream;

//
// doConnect
//
void doConnect()
{
	if (dataStream.isConnected()) {
		cout << "Already connected" << endl;
		return;
	}

	int32	waitMs;
	cout << "Wait how many millisecs: ";
	cin >> waitMs;

	int		error;
	if (serverRole) {
		if ((error = theAcceptor.open(theAddress)) != 0) {
			LOG_INFO_STR("Opening listener failed with error=" << error << " and errno=" << errno);
		}
		do {
			error = theAcceptor.accept(dataStream, waitMs);
			if (error < 0) {
				LOG_INFO_STR("Accept failed with error=" << error << " and errno=" << errno);
			}
		} while (error != 0);
	}
	else {
		// open client socket
		do {
			error = theConnector.connect(dataStream, theAddress, waitMs);
			if (error < 0) {
				LOG_INFO_STR("Connect failed with error=" << error << " and errno=" << errno);
			}
		} while (error != 0);
	}
}

void doRead()
{
	int32	nrBytes;
	int32	bytesRead;
	char	buf [1025];

	cout << "Wait for how many bytes [max 1024]: ";
	cin	>> nrBytes;

	if (nrBytes < 0)		nrBytes = 1;
	if (nrBytes > 1024)		nrBytes = 1024;

	bytesRead = dataStream.read(buf, nrBytes);
	cout << "read returned value " << bytesRead << endl;

	if (bytesRead < 0) {
		cout << "error: " << strerror(errno);
		return;
	}

	buf[bytesRead] = '\0';
	cout << "received:[" << buf << "]" << endl;

}

void doWrite()
{
	int32	bytesWrtn;
	string	buf;

	cout << "Type message to send to other side: ";
	cin	>> buf;

	bytesWrtn = dataStream.write(buf.c_str(), buf.length());
	cout << "write returned value " << bytesWrtn << endl;

	if (bytesWrtn < 0) {
		cout << "error: " << strerror(errno);
		return;
	}
}

void showMenu()
{
	cout << endl << endl << endl;
	if (serverRole) 
		cout << "Serversocket is ";
	else
		cout << "Clientsocket is ";
	if (dataStream.isConnected()) {
		cout << "connected (";
		if (!dataStream.getBlocking())
			cout << "NON";
		cout << "blocking) at port " << theAddress.portNr() << endl;
	}
	else {
		cout << "not yet connected" << endl;
	}

	cout << "Commands:" << endl;
	cout << "c		Connect to other side" << endl;
	cout << "n		set nonblockingmode" << endl;
	cout << "b		set blockingmode" << endl << endl;;

	if (dataStream.isConnected())  {
		cout << "w		write some data" << endl;
		cout << "r		read some data" << endl;
	}

	cout << endl << "q		quit" << endl;
	cout << endl << "Enter letter of your choice: ";
}


char getMenuChoice()
{
	char	choice;

	for (;;) {
		showMenu();
		cin >> choice;
		switch (choice) {
			case 'c':
			case 'n':
			case 'b':
			case 'q':
				return (choice);

			case 'r':
			case 'w':
				if (dataStream.isConnected())
					return (choice);
				break;
		}
		cout << "Sorry wrong choice" << endl;
	}
}

int main (int32	argc, char*argv[]) {

	INIT_LOGGER("ManualSocketTest");
  try {	
	switch (argc) {
	case 2:		// server
		theAddress.set(atoi(argv[1]), "localhost", "TCP");
		serverRole = true;
		break;
	case 3:
		theAddress.set(atoi(argv[1]), argv[2], "TCP");
		serverRole = false;
		break;
	default:
		cout <<"Syntax: " << argv[0] << " hostname port  - be a client" << endl;
		cout <<"        " << argv[0] << " port           - be a server" << endl;
		return (-1);
	}

	char		theChoice = ' ';
	while (theChoice != 'q') {
		switch (theChoice = getMenuChoice()) {
		case 'c':
			doConnect();
			break;
		case 'n':
			dataStream.setBlocking(false);
			if (serverRole)
				theAcceptor.setBlocking(false);
			break;
		case 'b':
			dataStream.setBlocking(true);
			if (serverRole)
				theAcceptor.setBlocking(true);
			break;
		case 'r':
			doRead();
			break;
		case 'w':
			doWrite();
			break;
		case 'x':
			break;
		}
	}

	// server has seperate sockets (clients has copied the pointer)
	dataStream.close();

	if (serverRole) {
		theAcceptor.close();
	}

	return (0);
  }
  catch (Exception&		ex) {
	cout << "EXECEPTION: " << ex.what() << endl;
	return (1);
  }
}
