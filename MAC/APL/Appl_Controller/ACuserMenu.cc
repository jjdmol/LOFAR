//#  ACUserMenu.cc: Menu-program for manual testing an ACC controlled application.
//#
//#  Copyright (C) 2004-2007
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

#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <Common/hexdump.h>
#include <Common/StringUtil.h>
#include <ALC/ACSyncClient.h>
#include <time.h>
#include "myACClientFunctions.h"

using namespace LOFAR;
using namespace LOFAR::ACC;
using namespace LOFAR::ACC::ALC;

myACClientFunctions		myACF;
ApplControlClient*		ACClient    = 0;
bool					connected   = false;
bool					IsSync		= false;
time_t					delayTime	= 0;
string					paramfile ("Observation-CygA.param");

// Note: function does nothin on sync-connections
void waitForAnswer()
{
	if (!IsSync) {
		cout << "Waiting for result from command" << endl;
		while (!ACClient->processACmsgFromServer()) {
			;
		}
	}

	if (delayTime) {
		cout << "Command is placed on stack, waiting for real result" << endl;
		while(!ACClient->processACmsgFromServer()) {
			;
		}
	}
}


// enter delayTime
void doDelayTime()
{
	cout << "Enter delaytime in seconds: ";
	string	timeString;
	cin >> timeString;
	delayTime = atol(timeString.c_str());
}

// Connect to AC
void doConnect()
{
	if (connected) {
		cout << "Already connected!" << endl;
		sleep (2);
		return;
	}

	cout << "(A)sync connection or (S)ync connection? ";
	char	comType;
	cin >> comType;
	
	if (comType == 's' || comType == 'S') {
		ACClient = new ACSyncClient("myUniqName", 10, 100, 1, 0);
		IsSync = true;
	}
	else {
		ACClient = new ACAsyncClient(&myACF, "myUniqName", 10, 100, 1, 0);
		IsSync = false;
	}

	connected   = true;
}

// Disconnect from AC
void doDisconnect()
{
	if (ACClient) {
		delete ACClient;
	}

	connected = false;
}

// Boot(parameterfile): start nodes
void doBoot()
{
	cout << "Enter name of parameterfile (" << paramfile << "): ";
	string	aParFile;
	cin.clear();
	cin >> aParFile;
	if (aParFile.length() > 2) {
		paramfile = aParFile;
	}

	cout << "Sending boot command" << endl;
	bool	result = ACClient->boot(time(0L)+delayTime, paramfile);
	if (result) {
		waitForAnswer();
	}
	if (result) {
		cout << "Application processes should be running now" << endl;
	}
	else {
		cout << "ERROR during boot command" << endl;
		sleep (2);
	}

}

// Define : start processes
void doDefine()
{
	cout << "Sending define command" << endl;
	bool result = ACClient->define(time(0L)+delayTime);
	if (result) {
		waitForAnswer();
	}
	if (result) {
		cout << "Application processes should have claimed the resources" << endl;
	}
	else {
		cout << "ERROR during define command" << endl;
		sleep (2);
	}
}

// Init: let AP connect to each other
void doInit()
{
	cout << "Sending init command" << endl;
	bool result = ACClient->init(time(0L)+delayTime);
	if (result) {
		waitForAnswer();
	}
	if (result) {
		cout << "Application processes should be in hot-standby mode" << endl;
	}
	else {
		cout << "ERROR during init command" << endl;
		sleep (2);
	}
}

// Run: do work
void doRun()
{
	cout << "Sending run command" << endl;
	bool result = ACClient->run(time(0L)+delayTime);
	if (result) {
		waitForAnswer();
	}
	if (result) {
		cout << "Application processes must running now" << endl;
	}
	else {
		cout << "ERROR during run command" << endl;
		sleep (2);
	}
}

// Cancel Command queue
void doCancel()
{
	cout << "Sending CancelQueue command" << endl;
	bool result = ACClient->cancelCmdQueue();
	if (result) {
		waitForAnswer();
	}
	if (result) {
		cout << "Queue should be flushed by now." << endl;
	}
	else {
		cout << "ERROR during cancelqueue command" << endl;
		sleep (2);
	}
}

// Pause(condition,waitTime)
void doPause()
{
	string	answer;
	string	option;

	// ask user condition
	answer.clear();
	while (answer.empty()) {
		cout << "What condition must be used? " << endl;
		cout << "N	now -> immediately" << endl;
		cout << "A	asap -> without corrupting anything" << endl;
		cout << "T	timestamp -> at some defined time" << endl;
		getline(cin, answer);
		if (answer == "N") {
			option="now";
		}
		else if (answer == "A") {
			option = "asap";
		}
		else if (answer == "T") {
			cout << "Over how many seconds should the application stop?" << endl;
			cout << "  ( based on the timestamp off the datasamples)  : " << endl;
			answer.clear();
			getline(cin, answer);
			uint32	sampleTime = time(0L) + atol(answer.c_str());
			option = "timestamp=" + toString(sampleTime);
		}
	} 
	
	cout << "Sending pause(" << option << ") command" << endl;
	bool result = ACClient->pause(time(0)+delayTime,30,option);
	if (result) {
		waitForAnswer();
	}
	if (result) {
		cout << "Application processes should be paused by now" << endl;
	}
	else {
		cout << "ERROR during pause command" << endl;
		sleep (2);
	}
}

// Release: free resources
void doRelease()
{
	cout << "Sending release command" << endl;
	bool result = ACClient->release(time(0)+delayTime);
	if (result) {
		waitForAnswer();
	}
	if (result) {
		cout << "Application processes released their resources." << endl;
	}
	else {
		cout << "ERROR during release command" << endl;
		sleep (2);
	}

}


// Quit: stop processes
void doStop()
{
	cout << "Sending quit command" << endl;
	bool result = ACClient->quit(time(0)+delayTime);
	if (result) {
		waitForAnswer();
	}
	if (result) {
		cout << "Application processes should be killed by now" << endl;
	}
	else {
		cout << "ERROR during quit command" << endl;
		sleep (2);
	}

}


void showMenu()
{
	cout << endl << endl << endl;
	cout << formatString("%s connection with AC\n", 
							connected ? (IsSync ? "Sync" : "Async") : "No");
	cout << "Time delay: " << delayTime << endl << endl;
		
	
	cout << "Commands" << endl;
	if (!connected) {
		cout << "c   Connect to AC" << endl << endl;
	}
	else {
		cout << "D   Disconnect from AC" << endl << endl;
	
		cout << "T   Set delayTime for commands" << endl << endl;

		cout << "b   Boot(parameterfile): start processes" << endl;
		cout << "d   Define : allocate resources [CLAIM]" << endl;
		cout << "i   Init   : Go to hot-standby  [PREPARE]" << endl;
		cout << "r   Run    : do work            [RESUME]" << endl;
		cout << "p   Pause(condition,waitTime)   [SUSPEND]" << endl;
		cout << "R   Release: free resources     [RELEASE]" << endl;
		cout << "s   Quit   : stop processes     [QUIT]" << endl << endl;
		cout << "c   Cancel : Command Queue" << endl;
	}

	cout << "q   Quit this program" << endl << endl;

	cout << "Enter letter of your choice: ";
}

int main (int argc, char *argv[]) {
	ConfigLocator	aCL;
	string			progName(basename(argv[0]));
#ifdef HAVE_LOG4CPLUS
        string			logPropFile(progName + ".log_prop");
	INIT_LOGGER (aCL.locate(logPropFile).c_str());
#else
        string logPropFile(progName + ".debug");
        INIT_LOGGER (aCL.locate(logPropFile).c_str());	
#endif

	char		aChoice = ' ';
	while (aChoice != 'q') {
		showMenu();
		cin >> aChoice;
		switch (aChoice) {
		case 'C':
		case 'c':	if (!connected) {
						doConnect();
					}
					else {
						doCancel();
					}
					break;
		case 'D':	doDisconnect();		break;
		case 'T':	doDelayTime();		break;
		case 'b':	doBoot();			break;
		case 'd':	doDefine();			break;
		case 'i':	doInit();			break;
		case 'r':	doRun();			break;
		case 'p':	doPause();			break;
		case 'R':	doRelease();		break;
		case 's':	doStop();			break;
		}
	}

	doDisconnect();

}


