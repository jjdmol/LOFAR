// WH_ToRing.cpp: implementation of the WH_ToRing class.
//
//////////////////////////////////////////////////////////////////////

#include "WH_ToRing.h"
#include "WH_Ring.h" // need NOTADDRESSED
#include "Step.h"

short WH_ToRing::theirBeamNr=-1;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


WH_ToRing::WH_ToRing (int seqNr):
  WorkHolder (1,1),
  itsSeqNr   (seqNr)
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
}


WH_ToRing::~WH_ToRing ()
{
  for (int ch=0; ch<getInputs(); ch++) {
    delete itsInDataHolders[ch];
  }
  for (int ch=0; ch<getOutputs(); ch++) {
    delete itsOutDataHolders[ch];
  }
}

WH_ToRing* WH_ToRing::make (const string&) const
{
  return new WH_ToRing (itsSeqNr);
}

void WH_ToRing::process ()
{
  if (getOutHolder(0)->doHandle() && theirBeamNr < 5) {
    if (itsSeqNr == 0) theirBeamNr++;

    itsOutDataHolders[0]->getBuffer()[0] = theirBeamNr+100*itsSeqNr;
    itsOutDataHolders[0]->getPacket()->destination = 2*theirBeamNr;
    while (itsOutDataHolders[0]->getPacket()->destination >= 10) {
      itsOutDataHolders[0]->getPacket()->destination -= 10;
    }
    itsOutDataHolders[0]->getPacket()->SourceID = itsSeqNr;
    cout << "WH_ToRing " << itsSeqNr << " Send: " 
	 << itsOutDataHolders[0]->getBuffer()[0] << " To " 
	 << itsOutDataHolders[0]->getPacket()->destination << endl;
  } else {
    itsOutDataHolders[0]->getPacket()->destination = NOTADDRESSED;
  }

}

void WH_ToRing::dump () const
{
  //cout << "WH_ToRing Buffer " << itsSeqNr << " Timestamp  = " 
//        << itsOutDataHolders[0]->getPacket()->timeStamp 
//        << " Buffer[0] = " <<  itsOutDataHolders[0]->getBuffer()[0] << endl;
}


DH_Ring<DH_Test>* WH_ToRing::getInHolder (int channel)
{
  return itsInDataHolders[channel]; 
}

DH_Ring<DH_Test>* WH_ToRing::getOutHolder (int channel)
{
  return itsOutDataHolders[channel];
}
