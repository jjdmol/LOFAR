#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/hexdump.h>

#include <ACC/ApplControlServer.h>


using namespace LOFAR;
using namespace LOFAR::ACC;

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

		while (ServerStub.processACmsgFromClient()) {
			;
		}
		LOG_INFO("Connection with client was closed");

	}
	catch (LOFAR::Exception&	ex) {
		LOG_FATAL_STR("Caught exception: " << ex << endl);
		return (0);
	}

	return (1);	
}


