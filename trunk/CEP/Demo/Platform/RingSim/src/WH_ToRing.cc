// WH_ToRing.cpp: implementation of the WH_ToRing class.
//
//////////////////////////////////////////////////////////////////////

#include "WH_ToRing.h"
#include "WH_Ring.h" // need NOTADDRESSED
#include "Step.h"

short WH_ToRing::itsInstanceCnt = 0;
short WH_ToRing::BeamNr=-1;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



WH_ToRing::WH_ToRing ():
WorkHolder (1,1)
{
  itsInDataHolders.reserve(1);
  itsOutDataHolders.reserve(1);
  
  for (int ch = 0; ch < getInputs(); ch++)    {
    DH_Ring<DH_Test>* aDH = new DH_Ring<DH_Test>();
    itsInDataHolders.push_back(aDH);
  }
  
  for (int ch = 0; ch < getOutputs(); ch++)    {
    DH_Ring<DH_Test>* aDH = new DH_Ring<DH_Test>();
    itsOutDataHolders.push_back(aDH);
  }

  myInstanceCnt = itsInstanceCnt++;
}


WH_ToRing::~WH_ToRing ()
{ 
}

void WH_ToRing::process ()
{
  if (getOutHolder(0)->doHandle() && BeamNr < 5) {
    if (getInstanceCnt() == 0) BeamNr++;

    itsOutDataHolders[0]->getBuffer()[0] = BeamNr+100*getInstanceCnt();
    itsOutDataHolders[0]->getPacket()->destination = 2*BeamNr;
    while (itsOutDataHolders[0]->getPacket()->destination >= 10) {
      itsOutDataHolders[0]->getPacket()->destination -= 10;
    }
    itsOutDataHolders[0]->getPacket()->SourceID = getInstanceCnt();
    cout << "WH_ToRing " << getInstanceCnt() << " Send: " 
	 << itsOutDataHolders[0]->getBuffer()[0] << " To " 
	 << itsOutDataHolders[0]->getPacket()->destination << endl;
  } else {
    itsOutDataHolders[0]->getPacket()->destination = NOTADDRESSED;
  }

}

void WH_ToRing::dump () const
{
  //cout << "WH_ToRing Buffer " << getInstanceCnt() << " Timestamp  = " 
//        << itsOutDataHolders[0]->getPacket()->timeStamp 
//        << " Buffer[0] = " <<  itsOutDataHolders[0]->getBuffer()[0] << endl;
}









