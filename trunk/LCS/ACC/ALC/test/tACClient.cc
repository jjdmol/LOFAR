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
	LOG_DEBUG (formatString("Command boot went %s!!!", 
				ACClient->boot(time(0L), "configID") ? "OK" : "WRONG"));

	LOG_DEBUG (formatString("Command define went %s!!!", 
				ACClient->define(0x22334455) ? "OK" : "WRONG"));

	LOG_DEBUG (formatString("Command init went %s!!!", 
				ACClient->init(time(0)+30) ? "OK" : "WRONG"));

	LOG_DEBUG (formatString("Command pause went %s!!!", 
				ACClient->pause(time(0)+40, 0, "pause??") ? "OK" : "WRONG"));

	LOG_DEBUG (formatString("Command run went %s!!!", 
				ACClient->run(0x32547698) ? "OK" : "WRONG"));

	LOG_DEBUG (formatString("Command pause went %s!!!", 
				ACClient->pause(0x00110101, 0x00001234, "pause condition") ? "OK" : "WRONG"));

	LOG_DEBUG (formatString("Command snapshot went %s!!!", 
				ACClient->snapshot(0x4321, "destination for snapshot") ? "OK" : "WRONG"));

	LOG_DEBUG (formatString("Command recover went %s!!!", 
				ACClient->recover(0x12345678, "recover source") ? "OK" : "WRONG"));

	LOG_DEBUG (formatString("Command reinit went %s!!!", 
				ACClient->boot(0x25525775, "reinit configID") ? "OK" : "WRONG"));

	LOG_DEBUG (formatString("Command quit went %s!!!", 
				ACClient->quit(0x01020304) ? "OK" : "WRONG"));


	LOG_DEBUG (formatString("Command askInfo returned \n[%s]", 
				ACClient->askInfo("Mag ik van jouw ....").c_str()));

	sleep (5);

}


