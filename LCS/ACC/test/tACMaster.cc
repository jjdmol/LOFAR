#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/hexdump.h>

#include <ACC/DH_AC_Connect.h>
#include <Transport/TH_Socket.h>

using namespace LOFAR;
using namespace LOFAR::ACC;

int main (int argc, char *argv[]) {
	INIT_LOGGER ("default.log_prop");

	if (argc < 2) {
		cout << endl << "Syntax: " << argv[0] << " portnr" << endl;
		return (-1);
	}

try {
	DH_AC_Connect	DH_AC_Client("everybody");
	DH_AC_Connect	DH_AC_Server("myname");
	DH_AC_Client.setID(1);
	DH_AC_Server.setID(2);

	TH_Socket	TCPConnection("localhost", atoi(argv[1]), true);
	DH_AC_Client.connectTo(DH_AC_Server, TCPConnection, true);	// blocking
	DH_AC_Server.connectTo(DH_AC_Client, TCPConnection, true);	// blocking
	DH_AC_Server.init();

	DH_AC_Server.read();
	LOG_INFO ("Connection request received!\n");
	hexdump (static_cast<char*>(DH_AC_Server.getDataPtr()), DH_AC_Server.getDataSize());
	
	DH_AC_Server.setServerIP (0x0A570230);
	DH_AC_Server.setServerPort (5051);
	DH_AC_Server.write();

	LOG_DEBUG("Starting tACServer at port 5051\n");
	execl("tACServer", "tACServer", "5051");
}
catch (LOFAR::Exception&	ex) {
	LOG_FATAL_STR("Caught exception: " << ex << endl);
	return (0);
}

}


