// WH_RingOut.cpp: implementation of the WH_RingOut class.
//
//////////////////////////////////////////////////////////////////////

#include "WH_RingOut.h"
#include "WH_Ring.h" // need NOTADDRESSED value
#include "Debug.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



WH_RingOut::WH_RingOut (int seqNr):
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


WH_RingOut::~WH_RingOut ()
{ 
  for (int ch=0; ch<getInputs(); ch++) {
    delete itsInDataHolders[ch];
  }
  for (int ch=0; ch<getOutputs(); ch++) {
    delete itsOutDataHolders[ch];
  }
  while (itsQDataHolders.size() > 0) {
      DH_Ring<DH_Test>* aDH = itsQDataHolders.front();
      itsQDataHolders.pop();
      delete aDH;
  }
}

WH_RingOut* WH_RingOut::make (const string&) const
{
  return new WH_RingOut (itsSeqNr);
}

void WH_RingOut::process ()
{
  
  if (itsInDataHolders[0]->getPacket()->destination == itsSeqNr) {
  
  short aSourceID = itsInDataHolders[0]->getPacket()->SourceID;
  AssertStr((( aSourceID >= 0) && (aSourceID < 10)), "SourceID out of range");

  DH_Ring<DH_Test>* aDH = new DH_Ring<DH_Test>();
  memcpy((void*)aDH->getPacket(),
	 (void*)itsInDataHolders[0]->getPacket(), 
	 itsInDataHolders[0]->getDataPacketSize());
  if (aDH->getPacket()->destination != NOTADDRESSED) {
    itsQDataHolders.push(aDH);
  } else {
    delete aDH;
    cout << "Skip push of not-addressed package" << endl;
  }
  cout << "Added to Q " << itsSeqNr << " size now is " << itsQDataHolders.size() 
       << "\t " << aDH->getBuffer()[0] << endl;

  AssertStr(itsQDataHolders.size() <= 50, "Queue length too long!!");

  TRACER1("WH_RingOut " << itsSeqNr 
	  << " Filled DHBuffer[" << aSourceID << "] ");
  }
  
  if (getOutHolder(0)->doHandle())
    if (itsQDataHolders.size() > 0) {
    
      DH_Ring<DH_Test> *aDH;
      aDH = itsQDataHolders.front();
      itsQDataHolders.pop();
//     cout << "Removed from Q " << itsSeqNr << " size now is " << itsQDataHolders.size() 
// 	 << "\t " << aDH->getBuffer()[0] << endl;
    

      memcpy((void*)itsOutDataHolders[0]->getPacket(), 
	     (void*)aDH->getPacket(),
	     itsInDataHolders[0]->getDataPacketSize());
      delete aDH;
    } else {
      itsOutDataHolders[0]->getPacket()->destination = NOTADDRESSED;
    }

}

void WH_RingOut::dump () const
{
  cout << "WH_RingOut Buffer: " ; //<< itsSeqNr << "   ";
  cout << " " <<  itsOutDataHolders[0]->getBuffer()[0] ;
  cout << endl;
  cout << "Q length = " << itsQDataHolders.size() << endl;
}


DH_Ring<DH_Test>* WH_RingOut::getInHolder (int channel)
{
  return itsInDataHolders[channel]; 
}

DH_Ring<DH_Test>* WH_RingOut::getOutHolder (int channel)
{
  return itsOutDataHolders[channel];
}
