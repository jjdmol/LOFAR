#include <BlueGeneFrontEnd.h>
#include <tinyCEP/SimulatorParseClass.h>
#include <Common/lofar_iostream.h>
#include <Common/Debug.h>
#include <Common/LofarLogger.h>

#ifdef HAVE_CORBA
int atexit(void (*function)(void))
{
  return 1;
}
#endif

using namespace LOFAR;

int main (int argc, const char** argv)
{
  bool isFrontEnd = true;

  Debug::initLevels (argc, argv);
  // The BlueGene 
  try {
    cout << argc << endl;
    if (argc >= 2 && !strcmp(argv[1], "-b")) {
      cout << "argument -b recognized"<< endl;
      isFrontEnd = false;
      
      INIT_LOGGER("BGlogger.prop");
      
    } else {

      INIT_LOGGER("BGlogger.prop");

    }

    BlueGeneFrontEnd simulator(isFrontEnd);

    simulator.setarg (argc, argv);
    
    simulator.baseDefine();
    simulator.basePrerun();
    simulator.baseRun(RUNS); 
    simulator.baseDump();
    simulator.baseQuit();

  } catch (...) {
    cout << "Unexpected exception" << endl;
  }
}
