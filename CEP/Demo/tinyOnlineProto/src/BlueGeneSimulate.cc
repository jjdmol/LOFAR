#include <tinyOnlineProto/BlueGeneDemo.h>
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

void usage(const char** argv) {


  exit(EXIT_FAILURE);
} 

int main (int argc, const char** argv)
{

#ifdef __BLRTS__
  // The BlueGene 

  // Set trace level.
  Debug::initLevels (argc, argv);

#ifdef HAVE_MPI
  MPI_Init(&argc,(char***)&argv);
#else
  cerr << "MPI not configured on BG/L" << endl; 
  exit (EXIT_FAILURE);
#endif

  try {
    BlueGeneDemo simulator(true);
    simulator.setarg (argc, argv);
    
    simulator.baseDefine();
    simulator.basePrerun();
    simulator.baseRun(10); 
    simulator.baseDump();
    simulator.baseQuit();
    
  } catch (...) {
    cout << "Unexpected exception" << endl;
  }
#else
  // The PowerPC frontend node

  if (argc != 2) {
    usage(argv);
  }

  // parse command line: read the number of BlueGene nodes.


  // Set trace level.
  Debug::initLevels (argc, argv);

  try {
    BlueGeneDemo simulator(false);
    simulator.setarg(argc, argv);
    
    try {
      LOFAR::SimulatorParse::parse(simulator);
    } catch (LOFAR::SimulatorParseError x) {
    }
  } catch (...) {
    cout << "Unexpected exception" << endl;
  }
#endif
}
