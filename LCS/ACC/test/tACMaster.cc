#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/hexdump.h>

#include <netdb.h>		// gethostbyname

#include <sys/types.h>	// 4x = inet_ntop
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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

	for (;true;) {
		while (!DH_AC_Server.read()) {
			LOG_INFO("AFTER READ");
		}
		LOG_INFO ("Connection request received!");
		
#if defined (__APPLE__)
		DH_AC_Server.setServerIP (0xA9FE3264);
#else
		DH_AC_Server.setServerIP (0xC0A80175);
#endif
		DH_AC_Server.setServerPort (3801);
		DH_AC_Server.write();

		LOG_DEBUG("Starting ApplicationController at port 3801");
		int32	res = system("(cd ../src ; ./ApplController AC.param) &");
		cout << "result = " << res << ", errno = " << errno 
												   << strerror(errno) << endl;
	}

	return(0);
}
catch (LOFAR::Exception&	ex) {
	LOG_FATAL_STR("Caught exception: " << ex << endl);
	return (0);
}

}


