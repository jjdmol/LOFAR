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
#include "WH_FromRing.h"

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
inline void RingBuilder<DH_T>::buildSimul(Simul* aSimul) {
  cout << "Build the RingSimul..." << endl;

  // Define the interior of the ring
  Step *RingStep[itsChannels];
  Step *ToRingStep[itsChannels];
  Step *FromRingStep[itsChannels];

  // First define the pre-ring steps
  for (int stepnr=0; stepnr<itsChannels; stepnr++) {
    ToRingStep[stepnr] = new Step(new WH_ToRing());
    ToRingStep[stepnr]->runOnNode(0);
    ToRingStep[stepnr]->setOutRate(itsChannels+1);
    aSimul->addStep (ToRingStep[stepnr]);
  }

  for (int stepnr=0; stepnr<itsChannels; stepnr++) {
    RingStep[stepnr] = new Step(new WH_Ring<DH_Test>());
    RingStep[stepnr]->runOnNode(0);
    RingStep[stepnr]->setInRate(itsChannels+1,0); // set inrate for channel 0
    if (stepnr >  0) RingStep[stepnr]->connect(RingStep[stepnr-1],1,1,1); 
    aSimul->addStep (RingStep[stepnr]);
    RingStep[stepnr]->connect(ToRingStep[stepnr],0,0,1);
  }
  RingStep[0]->connect(RingStep[itsChannels-1],1,1,1);
  
  for (int stepnr=0; stepnr<itsChannels; stepnr++) {
    FromRingStep[stepnr] = new Step(new WH_FromRing());
    FromRingStep[stepnr]->runOnNode(0);
    aSimul->addStep (FromRingStep[stepnr]);
    FromRingStep[stepnr]->connect(RingStep[stepnr],0,0,1);
    FromRingStep[stepnr]->setOutRate(itsChannels+1);
  }
}
