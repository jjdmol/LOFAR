#include <lofar_config.h>

#include <time.h>
#include <libgen.h>			// basename
#include <Common/hexdump.h>
#include <Common/LofarLogger.h>
#include <ACC/ProcControlComm.h>
#include <ACC/ParameterSet.h>
#include <Transport/TH_Socket.h>

using namespace LOFAR;
using namespace LOFAR::ACC;


int main (int argc, char *argv[]) {

	INIT_LOGGER(basename(argv[0]));
	LOG_INFO_STR("Starting up: " << argv[0]);

	if (argc < 2) {
		LOG_FATAL_STR("Syntax: " << argv[0] << " parameterfile");
		return (1);
	}

	ParameterSet	myPS(argv[1]);
	string			procID = myPS.getString("process.name");

	DH_ProcControl		DH_Client;
	DH_ProcControl		DH_Server;
	DH_Client.setID(500);
	DH_Server.setID(501);
	TH_Socket	TCPto   ("localhost", "", 3802, false);
	TH_Socket	TCPfrom ("", "localhost", 3802, true);

	LOG_DEBUG("Trying to connect to AC at localhost, 3802");
	DH_Client.connectBidirectional(DH_Server, TCPto, TCPfrom, true);
	DH_Client.init();

	// init random generator with some value for testing
	uint32 seed = 0;
	for (uint16 i = 0; i < procID.length(); ++i) {
		seed += procID.data()[i];
	}
	cout << "seed=" << seed << endl;
	srand(seed);

	sleep(rand() % 5);			// simulate we are busy
	DH_Client.setCommand (PCCmdStart);		// ready to receive data
	DH_Client.setOptions (procID);			// tell AC who we are.
	DH_Client.write();

	// wait for a command
	PCCmd	command;
	do {
		DH_Client.read();		// sync client!

		command  		 = DH_Client.getCommand();
		time_t	waitTime = DH_Client.getWaitTime();
		string	options  = DH_Client.getOptions();
		LOG_DEBUG_STR("Received command: " << command << " with waitTime= " 
						<< waitTime << " and options: " << options);

		// tell after a while we are ready with this command
		sleep (rand()%10);
		DH_Client.setCommand(static_cast<PCCmd>(DH_Client.getCommand() ^ PCCmdResult));
		DH_Client.setResult (PcCmdMaskOk);
		DH_Client.write();

	} while (command != PCCmdQuit);

	LOG_DEBUG("Disconnecting");

	return (0);
}


