#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/hexdump.h>
#include <ACC/ApplControlClient.h>

using namespace LOFAR;
using namespace LOFAR::ACC;

string	mySupplyInfo(const string&	keyList)
{
	// TODO
	return ("supplyInfo from ACClient was called");
}

void	handleAnswerMessage(const string&	answer)
{
	// TODO
	cout << "handleAnswerMessage from ACClient was called";
}

void	handleAckMessage()
{
	// TODO
	cout << "handleAckMessage was called\n";
}


int main (int argc, char *argv[]) {
	INIT_LOGGER ("default.log_prop");

	ApplControlClient	APConn("localhost");
	LOG_DEBUG ("Connected to private AC server!");

	// switch to async mode
	APConn.useAsyncMode (mySupplyInfo, handleAnswerMessage, handleAckMessage);
	LOG_DEBUG ("AsyncMode activated");
	sleep (5);
	LOG_DEBUG (formatString("Command boot went %s!!!", 
				APConn.boot(0x12345678, "configID") ? "OK" : "WRONG"));

	LOG_DEBUG (formatString("Command define went %s!!!", 
				APConn.define(0x22334455, "defineFile") ? "OK" : "WRONG"));

	LOG_DEBUG ("Sending ping request");
	APConn.ping();

	LOG_DEBUG (formatString("Command init went %s!!!", 
				APConn.init(0x55447766) ? "OK" : "WRONG"));

	LOG_DEBUG (formatString("Command run went %s!!!", 
				APConn.run(0x32547698) ? "OK" : "WRONG"));

	LOG_DEBUG (formatString("Command pause went %s!!!", 
				APConn.pause(0x00110101, "pause condition") ? "OK" : "WRONG"));

	LOG_DEBUG (formatString("Command snapshot went %s!!!", 
				APConn.snapshot(0x4321, "destination for snapshot") ? "OK" : "WRONG"));

	LOG_DEBUG (formatString("Command recover went %s!!!", 
				APConn.recover(0x12345678, "recover source") ? "OK" : "WRONG"));

	LOG_DEBUG (formatString("Command reinit went %s!!!", 
				APConn.boot(0x25525775, "reinit configID") ? "OK" : "WRONG"));

	LOG_DEBUG (formatString("Command quit went %s!!!", 
				APConn.quit() ? "OK" : "WRONG"));


	LOG_DEBUG (formatString("Command askInfo returned \n[%s]", 
				APConn.askInfo("Mag ik van jouw ....").c_str()));

	sleep (5);

}


