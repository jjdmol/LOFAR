//#include <time.h>
#include <lofar_config.h>
#include <Common/LofarLogger.h>
//#include <Common/hexdump.h>
//#include <myACClientFunctions.h>
#include <ALC/ACSyncClient.h>

using namespace LOFAR;
using namespace LOFAR::ACC;
using namespace LOFAR::ACC::ALC;

// This program is a commandline client to the ApplicationController

void printUsage() {
  cout << "This program is a commandline client to ACC. It can be used from shell scripts." << endl;
  cout << endl;
  cout << "  Usage :" << endl;
  cout << "    ACcli <unique-id> <command> [options]" << endl;
  cout << "    " << endl;
  cout << "  where" << endl;
  cout << "    <unique-id> is the unique identifier used to connect to a particular process. It should be equal to the identifier used when the process was started using ACcli." << endl;
  cout << "    <command> is the command to be given to the process. It can be any of:" << endl;
  cout << "      boot <paramfile>   get ready for starting the process. The next option should be the param file for that process." << endl;
  cout << "      define             start the process and give it the define command" << endl;
  cout << "      init   " << endl;
  cout << "      run    " << endl;
  cout << "      cancel " << endl;
  cout << "      pause <condition>  tell the program to pause when the condition is true. The program itself must be able to recognize the string." << endl;
  cout << "      stop   " << endl;
  cout << "    " << endl;
}

int main (int argc, char *argv[]) {
  INIT_LOGGER ("ACcli");

  if (argc < 3) {
    printUsage();
    return 1;
  }

  string myUniqueName(argv[1]);
  string command(argv[2]);

  bool returnValue = true;

  // Number of processes the user will start
  // uint16	itsNrProcs;
  // Expected lifetime of application in minutes
  // uint32	itsLifetime;
  // Activity of AC (1/2/3: low/medium/high)
  // uint16	itsActivityLevel;
  // Architecture code (0 = Intel, 1 = Blue Gene)
  // uint16	itsArchitecture;

  // Connect to AC
  //ACSyncClient ACClient(myUniqueName, nProcs, lifeTime, activityLevel, arch);
  ACSyncClient ACClient(myUniqueName, 10, 100, 1, 0);

  if (command == "boot"){
    // Boot(parameterfile): start nodes
    if (argc < 4) {
      printUsage();
      return 1;
    }
    returnValue = ACClient.boot(time(0L), argv[3]);

  } else if (command == "define") {
    // Define : start processes
    returnValue = ACClient.define(time(0L));

  } else if (command == "init") {
    // Init: let AP connect to each other
    returnValue = ACClient.init(time(0L));

  } else if (command == "run") {
    // Run: do work
    returnValue = ACClient.run(time(0L));

  } else if (command == "cancel") {
    // Cancel Command queue
    returnValue = ACClient.cancelCmdQueue();

  } else if (command == "pause") {
    // Pause(condition,waitTime)
    if (argc < 4) {
      printUsage();
      return 1;
    }
    returnValue = ACClient.pause(time(0), 30, argv[3]);

  } else if (command == "stop") {
    // Quit: stop processes
    returnValue = ACClient.quit(time(0));

  } else {
    printUsage();
  }

  return returnValue;
}

