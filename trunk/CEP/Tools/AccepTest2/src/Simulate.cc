#include <Common/LofarLogger.h>
#include <tinyCEP/SimulatorParseClass.h>
#include <AH_testWHs.h>

using namespace LOFAR;

int main (int argc, const char** argv)
{
  //INIT_LOGGER("AccepTest2.properties");

  try {
    cout<<"Making simulator ..."<<flush;
    AH_testWHs simulator;
    cout<<"ok"<<endl;
    cout<<"Reading args ..."<<flush;
    simulator.setarg (argc, argv);
    cout<<"ok"<<endl;
    cout<<"baseDefine ..."<<flush;
    simulator.baseDefine();
    cout<<"ok"<<endl;
    cout<<"basePrerun ..."<<flush;
    simulator.basePrerun();
    cout<<"ok"<<endl;
    cout<<"baseRun ..."<<flush;
    simulator.baseRun(10);
    cout<<"ok"<<endl;
    //cout<<"baseDump ..."<<flush;
    //simulator.baseDump();
    //cout<<"ok"<<endl;
    cout<<"baseQuit ..."<<flush;
    simulator.baseQuit();
    cout<<"ok"<<endl;
  } catch (...) {
    cout << "Unexpected exception" << endl;
  }
}
