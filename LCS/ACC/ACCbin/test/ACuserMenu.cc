#include <time.h>
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/hexdump.h>
#include <myACClientFunctions.h>
#include <ALC/ACSyncClient.h>

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
		cout << "Parameter subsets must have been made by now" << endl;
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
		cout << "Application processes must be running by now" << endl;
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
		cout << "Application processes must mutual connected by now" << endl;
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
		cout << "Queue must be flushed by now." << endl;
	}
	else {
		cout << "ERROR during cancelqueue command" << endl;
		sleep (2);
	}
}

// Pause(condition,waitTime)
void doPause()
{
	cout << "Sending pause command" << endl;
	bool result = ACClient->pause(time(0)+delayTime,30,"");
	if (result) {
		waitForAnswer();
	}
	if (result) {
		cout << "Application processes must be paused by now" << endl;
	}
	else {
		cout << "ERROR during pause command" << endl;
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
		cout << "Application processes must be killed by now" << endl;
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
	cout << "C		Connect to AC" << endl;
	cout << "D		Disconnect from AC" << endl << endl;
	
	cout << "T		Set delayTime for commands" << endl << endl;

	cout << "b		Boot(parameterfile): start nodes" << endl;
	cout << "d		Define : start processes" << endl;
	cout << "i		Init: let AP connect to each other" << endl;
	cout << "r		Run: do work" << endl;
	cout << "c		Cancel Command Queue" << endl;
	cout << "p		Pause(condition,waitTime)" << endl;
	cout << "s		Quit: stop processes" << endl << endl;

	cout << "q		Quit this program" << endl << endl;
	cout << "Enter letter of your choice: ";
}

int main (int argc, char *argv[]) {
	INIT_LOGGER ("default");

	char		aChoice = ' ';
	while (aChoice != 'q') {
		showMenu();
		cin >> aChoice;
		switch (aChoice) {
		case 'C':	doConnect();	 	break;
		case 'D':	doDisconnect();		break;
		case 'T':	doDelayTime();		break;
		case 'b':	doBoot();			break;
		case 'd':	doDefine();			break;
		case 'i':	doInit();			break;
		case 'r':	doRun();			break;
		case 'c':	doCancel();			break;
		case 'p':	doPause();			break;
		case 's':	doStop();			break;
		}
	}

	doDisconnect();

}


