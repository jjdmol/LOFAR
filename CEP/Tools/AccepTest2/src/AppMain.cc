#include <Common/LofarLogger.h>
#include <tinyCEP/SimulatorParseClass.h>
#include <AH_AccepTest.h>
#include <Transport/TH_MPI.h>

using namespace LOFAR;

int main (int argc, const char** argv)
{
  INIT_LOGGER("accepTest2.log_prop");

  try {
#ifdef HAVE_MPI
    TH_MPI::init(argc, argv);
#endif

    AH_AccepTest app;
    app.setarg (argc, argv);
    app.baseDefine();
    app.basePrerun();
    app.baseRun(1);
    //app.baseDump();
    app.baseQuit();
  } catch (LOFAR::Exception ex) {
    cout << "found a known exception: " << ex.what() << endl;

  } catch (...) {
    cout << "Unexpected exception" << endl;
  }
}
