#include <time.h>
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/hexdump.h>
#include <myACClientFunctions.h>

using namespace LOFAR;
using namespace LOFAR::ACC;


int main (int argc, char *argv[]) {
	INIT_LOGGER ("default.log_prop");

	myACClientFunctions		myACF;
	ACAsyncClient			asyncClient(&myACF, "localhost");
	ApplControlClient*		ACClient = &asyncClient;
	LOG_DEBUG ("Connected to private AC server!");

	// switch to async mode
	sleep (5);
	LOG_DEBUG (formatString("Sending command boot went %s!", 
				ACClient->boot(time(0L), "configID") ? "OK" : "WRONG"));
	ACClient->processACmsgFromServer();

	LOG_DEBUG (formatString("Sending command define went %s!", 
				ACClient->define(0x22334455) ? "OK" : "WRONG"));
	ACClient->processACmsgFromServer();

	LOG_DEBUG (formatString("Sending command init went %s!", 
				ACClient->init(time(0)+30) ? "OK" : "WRONG"));
	ACClient->processACmsgFromServer();

	LOG_DEBUG (formatString("Sending command pause went %s!", 
				ACClient->pause(time(0)+40, 0, "pause??") ? "OK" : "WRONG"));
	ACClient->processACmsgFromServer();

	LOG_DEBUG (formatString("Sending command run went %s!", 
				ACClient->run(0x32547698) ? "OK" : "WRONG"));
	ACClient->processACmsgFromServer();

	LOG_DEBUG (formatString("Sending command pause went %s!", 
				ACClient->pause(0x00110101, 0x00001234, "pause condition") ? "OK" : "WRONG"));
	ACClient->processACmsgFromServer();

	LOG_DEBUG (formatString("Sending command snapshot went %s!", 
				ACClient->snapshot(0x4321, "destination for snapshot") ? "OK" : "WRONG"));

	LOG_DEBUG (formatString("Sending command recover went %s!", 
				ACClient->recover(0x12345678, "recover source") ? "OK" : "WRONG"));
	ACClient->processACmsgFromServer();

	LOG_DEBUG (formatString("Sending command reinit went %s!", 
				ACClient->boot(0x25525775, "reinit configID") ? "OK" : "WRONG"));
	ACClient->processACmsgFromServer();

	LOG_DEBUG (formatString("Sending command quit went %s!", 
				ACClient->quit(0x01020304) ? "OK" : "WRONG"));
	ACClient->processACmsgFromServer();


	LOG_DEBUG (formatString("Command askInfo returned \n[%s]", 
				ACClient->askInfo("Mag ik van jouw ....").c_str()));
	ACClient->processACmsgFromServer();

	sleep (5);

}


