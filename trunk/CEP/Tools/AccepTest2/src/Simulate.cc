#include <Common/LofarLogger.h>
#include <tinyCEP/SimulatorParseClass.h>
#include <AH_AccepTest.h>
#include <Transport/TH_MPI.h>

using namespace LOFAR;

int main (int argc, const char** argv)
{
  //INIT_LOGGER("AccepTest2.properties");

  try {
#ifdef HAVE_MPI
    TH_MPI::init(argc, argv);
#endif

    AH_AccepTest simulator;
    simulator.setarg (argc, argv);
    simulator.baseDefine();
    simulator.basePrerun();
    simulator.baseRun(1);
    //simulator.baseDump();
    simulator.baseQuit();
  } catch (LOFAR::Exception ex) {
    cout << "found a known exception: " << ex.what() << endl;

  } catch (...) {
    cout << "Unexpected exception" << endl;
  }
}
