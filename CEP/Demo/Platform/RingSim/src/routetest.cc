#include <iostream.h>
#include <stdlib.h>
#include <string>
#include <map>

//#include "Transport.h"
#include "Step.h"
#include "BaseSim.h"
#include "Simul.h"
#include "BaseSim.h"
#include "DH_Test.h"
#include "DH_Ring.h"
#include "WH_Ring.h"
#include "RingBuilder.h"
#include "WH_ToRing.h"
#include "WH_FromRing.h"
#include "WH_Empty.h"
#include "Profiler.h"

int main (int argc, char *argv[])
{

  // initialise MPI environment
  TRANSPORTER::init(argc,argv);
  unsigned int rank = TRANSPORTER::getCurrentRank ();
  unsigned int size = TRANSPORTER::getNumberOfNodes();

  cout << "Ring Test " << rank << " of " << size << "  operational." << flush << endl;

  // create the main Simul; Steps and Simuls will be added to this one
  Simul RingSim(new WH_Empty(),"RingSim",0); //Uses an empty workholder.

  Step *ToRingStep[10];

  // First define the pre-ring steps
  for (int stepnr=0; stepnr<10; stepnr++) {
    ToRingStep[stepnr] = new Step(new WH_ToRing());
    ToRingStep[stepnr]->runOnNode(0);
    ToRingStep[stepnr]->setOutRate(11);
    RingSim.addStep (ToRingStep[stepnr]);
  }

  Simul theRing(RingBuilder<DH_Test>(10));

  theRing.Step::connectInputArray(ToRingStep,10);
  RingSim.addStep(theRing);

  Step *FromRingStep[10];
  for (int stepnr=0; stepnr<10; stepnr++) {
    FromRingStep[stepnr] = new Step(new WH_FromRing());
    FromRingStep[stepnr]->runOnNode(0);
    FromRingStep[stepnr]->setInRate(11);
    RingSim.addStep (FromRingStep[stepnr]);
  }

  theRing.connectOutputArray(FromRingStep,10);
  //RingSim.connectOutputToArray((Step**)(&&step3),1)
   
  //RingSim.resolveComm();

  //  RingSim.ConnectOutputToArray(fft,ELEMENTS);
  //////////////////////////////////////////////////////////////////////
  //
  // Finished configuration 
//////////////////////////////////////////////////////////////

  TRACER(debug,"Ready with definition of configuration");
  Profiler::init();

  cout << endl <<  "Start Process" << endl;    
  for (int i = 0; i < 221; i++) {
    if (i%1 == 0) { // print a dot after every 10 process steps
      TRACER(monitor,"====================== " 
	     << i << " ===============================" );
    }
    if (i==31)  {
      
      Profiler::activate();
    }
    RingSim.process ();
    //   RingSim.dump ();
    if (i==43)   Profiler::deActivate();
    
  }

     cout << endl << "DUMP Data from last Processing step: " << endl;
     RingSim.dump ();  

  cout << endl << "END OF SIMUL on node " << rank << endl;
 
 
  //     close MPI environment
  TRANSPORTER::finalize(); // will do nothing since NOMPI_ is defined in the Makefile
  return 0;
}












