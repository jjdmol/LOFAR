#include <BlueGeneFrontEnd.h>
#include <tinyCEP/SimulatorParseClass.h>
#include <Common/lofar_iostream.h>
#include <Common/Debug.h>


#ifdef HAVE_CORBA
int atexit(void (*function)(void))
{
  return 1;
}
#endif

using namespace LOFAR;

int main (int argc, const char** argv)
{
  Debug::initLevels (argc, argv);

  // The BlueGene 
  try {
    BlueGeneFrontEnd simulator(true);
    simulator.setarg (argc, argv);
    
    simulator.baseDefine();
    simulator.basePrerun();
    simulator.baseRun(10); 
    simulator.baseDump();
    simulator.baseQuit();

  } catch (...) {
    cout << "Unexpected exception" << endl;
  }
}
