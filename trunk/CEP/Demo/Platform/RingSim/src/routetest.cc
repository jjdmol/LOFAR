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

main (int argc, char *argv[])
{

  // initialise MPI environment
  TRANSPORTER::init(argc,argv);
  unsigned int rank = TRANSPORTER::getCurrentRank ();
  unsigned int size = TRANSPORTER::getNumberOfNodes();

  cout << "Ring Test " << rank << " of " << size << "  operational." << flush << endl;

  // create the main Simul; Steps and Simuls will be added to this one
  Simul RingSim(new WH_Empty(),"RingSim",0); //Uses an empty workholder.
  Simul theRing(new RingBuilder<DH_Test>(10));
  RingSim.addStep(&theRing);

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












