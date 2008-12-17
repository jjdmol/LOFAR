#include <time.h>
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <Common/hexdump.h>
#include <myACClientFunctions.h>

using namespace LOFAR;
using namespace LOFAR::ACC::ALC;


int main (int argc, char *argv[]) {
	INIT_LOGGER ("default.log_prop");

	myACClientFunctions		myACF;
	ACAsyncClient			asyncClient(&myACF, "myUniqName", 10, 100);
	ApplControlClient*		ACClient = &asyncClient;
	LOG_DEBUG ("Connected to private AC server!");

	// switch to async mode

	// BOOT
	LOG_DEBUG("Sending boot command over 5 seconds");
	sleep (5);
	LOG_DEBUG (formatString("Sending command boot went %s!", 
				ACClient->boot(time(0L), "configID") ? "OK" : "WRONG"));
	LOG_DEBUG("Waiting for result from boot command");
	ACClient->processACmsgFromServer();
	LOG_DEBUG("Parameter subset must have been made by now");

	// DEFINE
	LOG_DEBUG("Sending define command over 10 seconds");
	sleep (10);
	LOG_DEBUG (formatString("Sending command define went %s!", 
				ACClient->define(time(0L)) ? "OK" : "WRONG"));
	LOG_DEBUG("Waiting for result from define command");
	ACClient->processACmsgFromServer();
	LOG_DEBUG("Application processes must be running by now");

	// QUIT
	LOG_DEBUG("Sending quit command over 10 seconds");
	sleep (30);
	LOG_DEBUG (formatString("Sending command quit went %s!", 
				ACClient->quit(0) ? "OK" : "WRONG"));
	LOG_DEBUG("Waiting for result from quit command");
	ACClient->processACmsgFromServer();
	LOG_DEBUG("Application processes must be killed by now");

#if 0
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



	LOG_DEBUG (formatString("Command askInfo returned \n[%s]", 
				ACClient->askInfo("Mag ik van jouw ....").c_str()));
	ACClient->processACmsgFromServer();

	sleep (5);
#endif
	LOG_DEBUG ("Exiting over 15 seconds");
	sleep (15);

}


