#include "SelfcalEngineStub.h"
#include <stdlib.h>
#include <iostream.h>

main () {
  SelfcalEngineStub myStub;
  
  float *myparams=new float[8];
  cout << "parameters : ";
  for (int i=0; i<8; i++) {
    // fill with random pattern of numbers [0..100]
    myparams[i] = (100.0*random() / RAND_MAX) +1.0;
    cout << myparams[i] << " ";
    }
  cout << endl;
  myStub.init(8, myparams);
  
  // fill the work definition array
  bool *workdef=new bool[8];
  cout << "workdef    :" ;
  for (int i=0; i<8; i++) {
    int rand100 = (int)(100.*random()/RAND_MAX+1.0);
    workdef[i] = (rand100 > 80 );
    cout << workdef[i] << " ";
  }
  cout << endl;
  
  for (int i=0; i<15; i++) {
    myparams = myStub.Solve(workdef, myparams);
    myStub.dump();
  }
  
}
