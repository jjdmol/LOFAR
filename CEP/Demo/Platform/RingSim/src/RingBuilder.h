#ifndef _BASESIM_RingBuilder_H
#define _BASESIM_RingBuilder_H

#include "SimulBuilder.h"

template <class DH_T>
class RingBuilder : public SimulBuilder
{
 public:
  RingBuilder(int channels);
  ~RingBuilder();
  void buildSimul(Simul* aSimul);
  WorkHolder* getWorker();
  
 public:
  int itsChannels;
};

#endif

/*******************************************************************************/
#include "Step.h"
#include "Simul.h"
#include "DH_Ring.h"
#include "WH_ToRing.h"
#include "WH_Ring.h"
#include "WH_RingOut.h"
#include "WH_RingSimul.h"

template <class DH_T>
inline RingBuilder<DH_T>::RingBuilder(int channels):
  itsChannels(channels)
{
  cout << "RingBuilder C'tor" << endl; 
}

template <class DH_T>
inline RingBuilder<DH_T>::~RingBuilder() {
}

template <class DH_T>
inline WorkHolder* RingBuilder<DH_T>::getWorker() {
  if (itsWorker==NULL){
    return new WH_RingSimul<DH_T>(itsChannels);
  } else {
    return itsWorker;
  }
  
}

template <class DH_T>
inline void RingBuilder<DH_T>::buildSimul(Simul* aSimul) {
  cout << "Build the RingSimul..." << endl;

  aSimul->setRate(itsChannels+1);

  // Define the interior of the ring
  Step *RingStep[itsChannels];
  Step *RingOutStep[itsChannels];

  for (int stepnr=0; stepnr<itsChannels; stepnr++) {
    RingStep[stepnr] = new Step(new WH_Ring<DH_Test>());
    RingStep[stepnr]->runOnNode(0);
    RingStep[stepnr]->setInRate(itsChannels+1,0); // set inrate for channel 0
    if (stepnr >  0) RingStep[stepnr]->connect(RingStep[stepnr-1],1,1,1); 
    aSimul->addStep (RingStep[stepnr]);
  }
  // connect last element to first one to close the ring.
  RingStep[0]->connect(RingStep[itsChannels-1],1,1,1);
  cout << "Created ring elements" << endl;

  aSimul->connectInputToArray(RingStep,
				itsChannels,
				1); // skip every second DataHolder from the ring elements 

  cout << "connected ring elements to input" << endl;

  for (int stepnr=0; stepnr<itsChannels; stepnr++) {
    RingOutStep[stepnr] = new Step(new WH_RingOut());
    RingOutStep[stepnr]->runOnNode(0);
    aSimul->addStep (RingOutStep[stepnr]);
    RingOutStep[stepnr]->connect(RingStep[stepnr],0,0,1);
    RingOutStep[stepnr]->setOutRate(itsChannels+1);
  }
  aSimul->connectOutputToArray(RingOutStep,
			       itsChannels);
  cout << "Finished RingBuilder::build" << endl;
}

