#include <time.h>
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/hexdump.h>
#include <myACClientFunctions.h>
#include <ACC/ACSyncClient.h>

using namespace LOFAR;
using namespace LOFAR::ACC;

myACClientFunctions		myACF;
ApplControlClient*		ACClient    = 0;
bool					connected   = false;
bool					IsSync		= false;

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
		ACClient = new ACSyncClient("localhost");
		IsSync = true;
	}
	else {
		ACClient = new ACAsyncClient(&myACF, "localhost");
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
	cout << "Sending boot command" << endl;
	bool	result = ACClient->boot(time(0L), "configID");
	if (!IsSync && result) {
		cout << "Waiting for result from boot command" << endl;
		ACClient->processACmsgFromServer();
	}

	cout << "Parameter subsets must have been made by now" << endl;

}

// Define : start processes
void doDefine()
{
	cout << "Sending define command" << endl;
	bool	result = ACClient->define(time(0L));
	if (!IsSync && result) {
		cout << "Waiting for result from define command" << endl;
		ACClient->processACmsgFromServer();
	}
	cout << "Application processes must be running by now" << endl;
}

// Init: let AP connect to each other
void doInit()
{
	cout << "Sending init command" << endl;
	bool	result = ACClient->init(time(0L));
	if (!IsSync && result) {
		cout << "Waiting for result from init command" << endl;
		ACClient->processACmsgFromServer();
	}
	cout << "Application processes must mutual connected by now" << endl;
}

// Run: do work
void doRun()
{
	cout << "Sending run command" << endl;
	bool	result = ACClient->run(time(0L));
	if (!IsSync && result) {
		cout << "Waiting for result from run command" << endl;
		ACClient->processACmsgFromServer();
	}
	cout << "Application processes must running now" << endl;

}

// Pause(condition,waitTime)
void doPause()
{
	cout << "NOT YET IMPLEMENTED IN THIS PROG" << endl;
	sleep (2);  
}

// Quit: stop processes
void doStop()
{
	cout << "Sending quit command" << endl;
	bool	result = ACClient->quit(0);
	if (!IsSync && result) {
		cout << "Waiting for result from quit command" << endl;
		ACClient->processACmsgFromServer();
	}
	cout << "Application processes must be killed by now" << endl;

}


void showMenu()
{
	cout << endl << endl << endl;
	cout << formatString("%s connection with AC\n\n", 
								connected ? (IsSync ? "Sync" : "Async")  : "Not");
		
	
	cout << "Commands" << endl;
	cout << "C		Connect to AC" << endl;
	cout << "D		Disconnect from AC" << endl << endl;

	cout << "b		Boot(parameterfile): start nodes" << endl;
	cout << "d		Define : start processes" << endl;
	cout << "i		Init: let AP connect to each other" << endl;
	cout << "r		Run: do work" << endl;
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
		case 'b':	doBoot();			break;
		case 'd':	doDefine();			break;
		case 'i':	doInit();			break;
		case 'r':	doRun();			break;
		case 'p':	doPause();			break;
		case 's':	doStop();			break;
		}
	}

	doDisconnect();

#if 0
	LOG_DEBUG (formatString("Sending command pause went %s!", 
				ACClient->pause(time(0)+40, 0, "pause condition") ? "OK" : "WRONG"));
	ACClient->processACmsgFromServer();


	LOG_DEBUG (formatString("Sending command snapshot went %s!", 
				ACClient->snapshot(0x4321, "destination for snapshot") ? "OK" : "WRONG"));
	ACClient->processACmsgFromServer();


	LOG_DEBUG (formatString("Sending command recover went %s!", 
				ACClient->recover(0x12345678, "recover source") ? "OK" : "WRONG"));
	ACClient->processACmsgFromServer();


	LOG_DEBUG (formatString("Sending command reinit went %s!", 
				ACClient->boot(0x25525775, "reinit configID") ? "OK" : "WRONG"));
	ACClient->processACmsgFromServer();


	LOG_DEBUG (formatString("Command askInfo returned \n[%s]", 
				ACClient->askInfo("Mag ik van jouw ....").c_str()));
	ACClient->processACmsgFromServer();

#endif

}


