#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/Debug.h>			// some source still uses this

#include <DH_AC_Connect.h>
#include <Transport/TH_Socket.h>

using namespace LOFAR;

void hexdump (char	*memblock, long	size)
{
#define BTS_P_LINE		16
	long	left, index;
	char	c;

	for (left = 0; left < size; left += BTS_P_LINE) {
		printf ("%04lX: ", left);
		for (index = 0; index < BTS_P_LINE; index ++) {
			if (index == BTS_P_LINE / 2)			/* add extra space in	*/
				printf (" ");					/* the middle o.t. line	*/

			if (left + index < size)
				printf ("%02X ", (unsigned char) memblock [left + index]);
			else
				printf ("   ");
		}

		for (index = 0; index < BTS_P_LINE; index ++) {	/* print char if	*/
			if (left + index < size) {					/* printable char	*/
				c = memblock [left + index];
				if (c < 0x20 || c > 0x7e)
					printf (".");
				else
					printf ("%c", c);
			}
		}
		printf ("\n");
	}
}

int main (int argc, char *argv[]) {
	INIT_LOGGER ("default.log_prop");
	Debug::loadLevels("default.debug");

	if (argc < 2) {
		cout << endl << "Syntax: " << argv[0] << " portnr" << endl;
		return (-1);
	}

try {
	DH_AC_Connect	DH_AC_Client("everybody");
	DH_AC_Connect	DH_AC_Server("myname");
	DH_AC_Client.setID(1);
	DH_AC_Server.setID(2);

cout << "jippie1\n";
	TH_Socket	TCPConnection("localhost", "obsolete", atoi(argv[1]), false);
cout << "jippie2\n";
	DH_AC_Client.connectTo(DH_AC_Server, TCPConnection, true);	// blocking
cout << "jippie3\n";
	DH_AC_Server.connectTo(DH_AC_Client, TCPConnection, true);	// blocking
cout << "jippie4\n";
	DH_AC_Client.init(); 	// ???
cout << "jippie5\n";
	DH_AC_Server.init();
cout << "jippie6\n";

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


