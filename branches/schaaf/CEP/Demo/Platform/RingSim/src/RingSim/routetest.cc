#include <iostream.h>
#include <stdlib.h>
#include <string>
#include <map>

//#include "Transport.h"
#include "Step.h"
#include "BaseSim.h"
#include "Simul.h"
#include "BaseSim.h"
#include "DH_Ring.h"
#include "WH_Ring.h"
#include "WH_ToRing.h"
#include "WH_FromRing.h"
#include "WH_Empty.h"
#include "Profiler.h"

main (int argc, char *argv[])
{

  // initialise MPI environment
  TRANSPORTER::init(argc,argv);
  unsigned int rank = TRANSPORTER::getCurrentRank ();
  unsigned int size = TRANSPORTER::getNumberOfNodes();

  cout << "Ring Test " << rank << " of " << size << "  operational." << flush << endl;

  // create the main Simul; Steps and Simuls will be added to this one
  Simul RingSim(new WH_Empty(),"RingSim",0); //Uses an empty workholder.
  RingSim.runOnNode(0);
  
  Step *RingStep[10];
  Step *ToRingStep[10];
  Step *FromRingStep[10];

  // First define the pre-ring steps
  for (int stepnr=0; stepnr<10; stepnr++) {
    ToRingStep[stepnr] = new Step(new WH_ToRing());
    ToRingStep[stepnr]->runOnNode(0);
    ToRingStep[stepnr]->setOutRate(11);
    RingSim.addStep (ToRingStep[stepnr]);
  }

  for (int stepnr=0; stepnr<10; stepnr++) {
    RingStep[stepnr] = new Step(new WH_Ring<DH_Test>());
    RingStep[stepnr]->runOnNode(0);
    RingStep[stepnr]->setInRate(11,0); // set inrate for channel 0
    if (stepnr >  0) RingStep[stepnr]->connect(RingStep[stepnr-1],1,1,1); 
    RingSim.addStep (RingStep[stepnr]);
    RingStep[stepnr]->connect(ToRingStep[stepnr],0,0,1);
  }
  RingStep[0]->connect(RingStep[9],1,1,1);
  
  // First define the post-ring steps
  for (int stepnr=0; stepnr<10; stepnr++) {
    FromRingStep[stepnr] = new Step(new WH_FromRing());
    FromRingStep[stepnr]->runOnNode(0);
    RingSim.addStep (FromRingStep[stepnr]);
    FromRingStep[stepnr]->connect(RingStep[stepnr],0,0,1);
    FromRingStep[stepnr]->setOutRate(11);
  }
  //RingSim.connectOutputToArra((Step**)(&&step3),1)
   
  //RingSim.resolveComm();

  //  RingSim.ConnectOutputToArray(fft,ELEMENTS);
  //////////////////////////////////////////////////////////////////////
  //
  // Finished configuration 
//////////////////////////////////////////////////////////////

  TRACER(debug,"Ready with definition of configuration");
  Profiler::init();

  cout << endl <<  "Start Process" << endl;    
  for (int i = 0; i < 121; i++) {
    if (i%1 == 0) { // print a dot after every 10 process steps
      TRACER(monitor,"====================== " 
	     << i << " ===============================" );
    }
    if (i==3)   Profiler::activate();
    RingSim.process ();
    //   RingSim.dump ();
    if (i==3)   Profiler::deActivate();
    
  }

     cout << endl << "DUMP Data from last Processing step: " << endl;
     RingSim.dump ();  

  cout << endl << "END OF SIMUL on node " << rank << endl;
 
 
  //     close MPI environment
  TRANSPORTER::finalize(); // will do nothing since NOMPI_ is defined in the Makefile
  return 0;
}












