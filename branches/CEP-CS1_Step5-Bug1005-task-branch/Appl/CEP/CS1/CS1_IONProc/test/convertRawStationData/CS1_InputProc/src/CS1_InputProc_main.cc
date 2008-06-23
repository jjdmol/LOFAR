//#  CS1_InputProc_main.cc:
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
#include <Common/LofarLocators.h>
#include <tinyCEP/ApplicationHolderController.h>
#include <CS1_InputProc/ACCmain_InputProc.h>
#include <CS1_Interface/CS1_Parset.h>
#include <CS1_InputProc/InputSection.h>


using namespace LOFAR;
using namespace LOFAR::CS1;


static unsigned  nrInputSectionRuns;

int main(int argc, char* argv[]) {

  ConfigLocator aCL;
  string        progName = basename(argv[0]);
  string        logPropFile(progName + ".log_prop");
  INIT_LOGGER (aCL.locate(logPropFile).c_str());
  LOG_DEBUG_STR("Initialized logsystem with: " << aCL.locate(logPropFile));

  std::clog << "trying to use " << argv[1] << " as ParameterSet" << std::endl;
  ACC::APS::ParameterSet parameterSet(argv[1]);
  CS1_Parset cs1_parset(&parameterSet);
  
  cs1_parset.adoptFile("OLAP.parset");
  
  cout << atoi(argv[2]) << endl;


  nrInputSectionRuns = atoi(argv[2]) * cs1_parset.nrCoresPerPset() / cs1_parset.nrSubbandsPerPset();
  try {
    InputSection inputSection;

    inputSection.preprocess(&cs1_parset);
    
    for (unsigned run = 0; run < nrInputSectionRuns; run ++)
      inputSection.process();

    inputSection.postprocess();
  
  } catch (Exception &ex) {
    std::cerr << "input thread caught Exception: " << ex << std::endl;
  } catch (std::exception &ex) {
    std::cerr << "input thread caught std::exception: " << ex.what() << std::endl;
  } catch (...) {
    std::cerr << "input thread caught non-std::exception: " << std::endl;
  }
  std::clog << "input thread finished" << std::endl;
}

