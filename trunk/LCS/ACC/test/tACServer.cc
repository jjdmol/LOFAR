#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/Debug.h>			// some source still uses this

#include <ApplControlServer.h>


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

void ping ()
{
	cout << "Function 'ping' called" << endl;
}

bool noParam ()
{
	cout << "Function 'noparam' called" << endl;
	return (true);
}

bool	singleParam(const time_t	scheduleTime) {
	cout << "SingleParam function called with:" << scheduleTime << endl;
	return (true);
}

bool	doubleParam(const time_t	scheduleTime, const string& options) {
	cout << "DoubleParam function called with:" << scheduleTime 
		 <<	"," << options << endl;
	return (true);
}

string	supplyInfo(const string& keylist) {
	return ("No information available yet");
}

void handleAckMessage()
{
	cout << "Ack message received!" << endl;
}

int main (int argc, char *argv[]) {
	INIT_LOGGER ("default.log_prop");
	Debug::loadLevels("default.debug");

	try {
		ApplCtrlFunctions		ACF(doubleParam,
									doubleParam, 
									singleParam,
									singleParam,
									doubleParam,
									noParam,
									doubleParam,
									doubleParam,
									doubleParam,
									ping,
									supplyInfo);
//		ACF.handleAckMessage = handleAckMessage;

		uint16	portNr = atoi(argv[1]);
		ApplControlServer		ServerStub(portNr, ACF);

		for (;;) {
			ServerStub.processACmsgFromClient();
		}
	}
	catch (LOFAR::Exception&	ex) {
		LOG_FATAL_STR("Caught exception: " << ex << endl);
		return (0);
	}

}


