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

  aSimul->runOnNode(0);
  aSimul->setRate(itsChannels+1);

  // Define the interior of the ring
  Step *RingStep[itsChannels];
  Step *RingOutStep[itsChannels];

  int firstringstart=0;
  int firstringend = itsChannels/2-1;
  int ringsize = firstringend-firstringstart+1;
  for (int stepnr=firstringstart; stepnr<=firstringend; stepnr++) {
    RingStep[stepnr] = new Step(new WH_Ring<DH_Test>());
    RingStep[stepnr]->runOnNode(stepnr+1);
    RingStep[stepnr]->setInRate(itsChannels+1,0); // set inrate for channel 0
    if (stepnr >  firstringstart) {
      RingStep[stepnr]->connect(RingStep[stepnr-1],1,1,1); //fwd loop 
      cout << stepnr << "<--" << stepnr-1 << endl;
    }
    aSimul->addStep (RingStep[stepnr]);
    // Prevent deadlock in first step
    // after the first step, this flag will be set back to normal (in WH_Ring::process())
    RingStep[stepnr]->getWorker()->getInHolder(2)->setRead(false); // switch off read from second ring
  }
  
  // Prevent deadlock in first step
  // after the first step, this flag will be set back to normal (in WH_Ring::process())
  RingStep[firstringstart]->getWorker()->getInHolder(1)->setRead(false);


  // connect last element to first one to close the ring.
  RingStep[firstringstart]->connect(RingStep[firstringend],1,1,1); // close forwards loop
  cout << firstringstart << "<--" << firstringend << endl;
  cout << "Created first ring" << endl;

  int secondringstart=itsChannels/2;
  int secondringend = itsChannels-1;
  for (int stepnr=secondringstart; stepnr<=secondringend; stepnr++) {
    RingStep[stepnr] = new Step(new WH_Ring<DH_Test>());
    RingStep[stepnr]->runOnNode(stepnr-secondringstart+1);
    RingStep[stepnr]->setInRate(itsChannels+1,0); // set inrate for channel 0
    if (stepnr >  secondringstart) {
      RingStep[stepnr]->connect(RingStep[stepnr-1],1,1,1); //fwd loop 
      cout << stepnr << "<--" << stepnr-1 << endl;
    }
    aSimul->addStep (RingStep[stepnr]);
  }
  // Prevent deadlock in first step
  // after the first step, this flag will be set back to normal (in WH_Ring::process())
  RingStep[secondringstart]->getWorker()->getInHolder(1)->setRead(false);

  // connect last element to first one to close the ring.
  RingStep[secondringstart]->connect(RingStep[secondringend],1,1,1); // close forwards loop
  cout << secondringstart << "<--" << secondringend << endl;

  cout << "Created second ring" << endl;

  cout << "Connecting the rings to each other " << endl;
  for (int stepnr=0; stepnr <= firstringend; stepnr++) {
    cout << "connect  " << stepnr+ringsize << "  --  " << stepnr << endl;
    RingStep[stepnr+ringsize]->connect(RingStep[stepnr],2,2,1); 
    cout << "reverse " << endl;
    RingStep[stepnr]->connect(RingStep[stepnr+ringsize],2,2,1); 
  }

  cout << "Created ring elements" << endl;

  aSimul->connectInputToArray(RingStep,
				itsChannels,
				2); // only connect the first DataHolder from the ring elements 

  cout << "connected ring elements to input" << endl;

  for (int stepnr=0; stepnr<itsChannels; stepnr++) {
    RingOutStep[stepnr] = new Step(new WH_RingOut());
    if (stepnr <= firstringend) {
      RingOutStep[stepnr]->runOnNode(0);
    } else {
      RingOutStep[stepnr]->runOnNode(0);
    }
    aSimul->addStep (RingOutStep[stepnr]);
    RingOutStep[stepnr]->connect(RingStep[stepnr],0,0,1);
    RingOutStep[stepnr]->setOutRate(itsChannels+1);
  }

  //  aSimul->connectOutputToArray(RingOutStep,
  //			       itsChannels);
  cout << "Finished RingBuilder::build" << endl;
}

