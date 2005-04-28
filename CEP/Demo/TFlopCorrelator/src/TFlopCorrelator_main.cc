//#  TFlopCorrelator_main.cc:
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/RunOnNode.h>
#include <TFlopCorrelator/AH_BGLProcessing.h>


int main (int argc, const char** argv) {

  INIT_LOGGER("AH_BGLProcessing");

  // Note: AH_BGLProcessing is based in tinyCEP only for
  //       compatibility with BGL. Therefore, HAVE_BGL macros are
  //       used to exclude all CEPFrame code for the BGL executable.


  try {

    SETNODE(0,0); // define which node we are.
                  // todo: get these from ACC and maybe MPI

    // The applications are orginised as:
    // 0: input section
    // 1: BGL processing
    // 2: data gathering and torage

    RUNINAPPL(0) {}
    RUNINAPPL(1) {
      AH_BGLProcessing correlator();
      correlator.setarg(argc, argv);    
      correlator.baseDefine();
    }
    RUNINAPPL(2) {
    }
    cout << "defined" << endl;
    



    correlator.basePrerun();
    cout << "init" << endl;
    correlator.baseRun(1);
    cout << "run" << endl;
    correlator.baseDump();
    correlator.baseQuit();


  } catch (std::exception& x) {
    cout << "Unexpected exception" << endl;
    cerr << x.what() << endl; 
  }
  return 0;
