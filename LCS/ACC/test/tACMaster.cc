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

	TH_Socket	TCPto  ("localhost", "", atoi(argv[1]), false);
	TH_Socket	TCPfrom("", "localhost", atoi(argv[1]), true);
	DH_AC_Client.connectBidirectional(DH_AC_Server, TCPto, TCPfrom, true);	// blocking
	DH_AC_Server.init();

	DH_AC_Server.read();
	LOG_INFO ("Connection request received!");
	
	DH_AC_Server.setServerIP (0xC0A80175);
	DH_AC_Server.setServerPort (5051);
	DH_AC_Server.write();

	LOG_DEBUG("Starting tACServer at port 5051");
	int32	res = execl("./tACServer", "tACServer", "5051", NULL);
	cout << "execl = " << res << ", errno = " << errno << strerror(errno) << endl;
}
catch (LOFAR::Exception&	ex) {
	LOG_FATAL_STR("Caught exception: " << ex << endl);
	return (0);
}

}


