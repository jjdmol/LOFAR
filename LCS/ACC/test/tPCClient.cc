#include <time.h>
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/hexdump.h>
#include <Transport/TH_Socket.h>
#include <ACC/DH_ProcessControl.h>

using namespace LOFAR;
using namespace LOFAR::ACC;


int main (int argc, char *argv[]) {
	INIT_LOGGER ("default.log_prop");

	DH_ProcessControl		DH_Client;
	DH_ProcessControl		DH_Server;
	DH_Client.setID(500);
	DH_Server.setID(501);
	TH_Socket	TCPto   ("localhost", "", 3802, false);
	TH_Socket	TCPfrom ("", "localhost", 3802, true);

	LOG_DEBUG("Trying to connect to AC at localhost, 3802");
	DH_Client.connectBidirectional(DH_Server, TCPto, TCPfrom, true);
	DH_Client.init();

	sleep (10);

	LOG_DEBUG("Disconnecting");

}


