#include <time.h>
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/hexdump.h>
#include <Transport/TH_Socket.h>
#include <ACC/ProcControlComm.h>

using namespace LOFAR;
using namespace LOFAR::ACC;


int main (int argc, char *argv[]) {
	INIT_LOGGER ("default.log_prop");

	DH_ProcControl		DH_Client;
	DH_ProcControl		DH_Server;
	DH_Client.setID(500);
	DH_Server.setID(501);
	TH_Socket	TCPto   ("localhost", "", 3802, false);
	TH_Socket	TCPfrom ("", "localhost", 3802, true);

	LOG_DEBUG("Trying to connect to AC at localhost, 3802");
	DH_Client.connectBidirectional(DH_Server, TCPto, TCPfrom, true);
	DH_Client.init();

	sleep(2);			// simulate we are busy
	DH_Client.setCommand (PCCmdStart);		// ready to receive data
	DH_Client.setOptions (argv[0]);			// tell AC who we are.
	DH_Client.write();

	// wait for a command
	DH_Client.read();
	hexdump (DH_Client.getDataPtr(), DH_Client.getDataSize());
	// tell after a while we are ready with this command
	sleep (3);
	DH_Client.setCommand(static_cast<PCCmd>(DH_Client.getCommand() ^ PCCmdResult));
	DH_Client.setResult (PcCmdMaskOk);
	DH_Client.write();

	// stay alive a while.
	sleep (10);

	LOG_DEBUG("Disconnecting");

}


