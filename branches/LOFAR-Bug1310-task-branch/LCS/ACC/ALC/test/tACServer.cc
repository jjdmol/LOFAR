#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/hexdump.h>

#include <ALC/ApplControlServer.h>
#include <cstdlib>


using namespace LOFAR;
using namespace LOFAR::ACC::ALC;

class myACImpl : public ApplControl {
public:
//	myACImpl ();
//	~myACImpl();

	bool	boot(const time_t	scheduleTime, const string& configID) const {
		cout << "Boot function called with:" << scheduleTime << endl;
		return (true);
	}
	bool	define(const time_t	scheduleTime) const {
		cout << "Define function called with:" << scheduleTime << endl;
		return (true);
	}
	bool	init(const time_t	scheduleTime) const {
		cout << "Init function called with:" << scheduleTime << endl;
		return (true);
	}
	bool	run(const time_t	scheduleTime) const {
		cout << "Run function called with:" << scheduleTime << endl;
		return (true);
	}
	bool	pause(const time_t	scheduleTime, const time_t waitTime, 
											  const string& configID) const {
		cout << "Pause function called with:" << scheduleTime << endl;
		return (true);
	}
	bool	release(const time_t	scheduleTime) const {
		cout << "Release function called with:" << scheduleTime << endl;
		return (true);
	}
	bool	quit(const time_t	scheduleTime) const {
		cout << "Quit function called with:" << scheduleTime << endl;
		return (true);
	}
	bool	shutdown(const time_t	scheduleTime) const {
		cout << "Shutdown function called with:" << scheduleTime << endl;
		return (true);
	}
	bool	snapshot(const time_t	scheduleTime, const string& dest) const {
		cout << "Snapshot function called with:" << scheduleTime << endl;
		return (true);
	}
	bool	recover(const time_t	scheduleTime, const string& source) const {
		cout << "Recover function called with:" << scheduleTime << endl;
		return (true);
	}
	bool	reinit(const time_t	scheduleTime, const string& configID) const {
		cout << "Reinit function called with:" << scheduleTime << endl;
		return (true);
	}
	bool	cancelCmdQueue() const {
		cout << "cancelCmdQueue function called" << endl;
		return (true);
	}

	string	askInfo(const string& keylist) const {
		cout << "askInfo function called with:" << keylist << endl;
		return ("Answer from ACimpl class");
	}
};	// class myACImpl

int main (int argc, char *argv[]) {
	INIT_LOGGER ("default.log_prop");

	if (argc != 2) {
		cout << "Syntax: tACServer portnr" << endl;
		return (1);
	}

	try {
		uint16	portNr = atoi(argv[1]);
		myACImpl				ACImpl;
		ApplControlServer		serverStub(portNr, &ACImpl);

		LOG_INFO("Entering main 'while' loop");
		DH_ApplControl*		activeDH;
		while (serverStub.pollForMessage()) {
			activeDH = serverStub.getDataHolder();
			serverStub.handleMessage(activeDH);
		}
		LOG_INFO("Connection with client was closed");

	}
	catch (LOFAR::Exception&	ex) {
		LOG_FATAL_STR("Caught exception: " << ex << endl);
		return (0);
	}

	return (1);	
}


