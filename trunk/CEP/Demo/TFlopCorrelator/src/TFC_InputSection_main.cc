//#  TFC_InputSection_main.cc:
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/lofar_iostream.h> 
#include <Common/LofarLogger.h>

#include <Common/RunOnNode.h>
#include <TFlopCorrelator/AH_InputSection.h>

using namespace LOFAR;

int main (int argc, const char** argv) {

  INIT_LOGGER("TFC_InputSection");

  try {
    AH_InputSection myAH();
    myAH.setarg(argc, argv);
    myAH.baseDefine();
    cout << "defined" << endl;
    Profiler::init();
    myAH.basePrerun();
    cout << "init done" << endl;
    Profiler::activate();
    cout << "run" << endl;
    myAH.baseRun(kvm.getInt("runsteps",1));
    cout << "run complete" << endl;
    myAH.baseDump();
    myAH.baseQuit();
    Profiler::deActivate();

  } catch (std::exception& x) {
    cout << "Unexpected exception" << endl;
    cerr << x.what() << endl; 
  }
  return 0;
}
