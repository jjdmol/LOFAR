// WH_RingOut.cpp: implementation of the WH_RingOut class.
//
//////////////////////////////////////////////////////////////////////

#include "WH_RingOut.h"
#include "firewalls.h"

short   WH_RingOut::itsInstanceCnt = 0;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



WH_RingOut::WH_RingOut ():
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


WH_RingOut::~WH_RingOut ()
{ 
}

void WH_RingOut::process ()
{
  
  if (itsInDataHolders[0]->getPacket()->destination == getInstanceCnt()) {
  
  short aSourceID = itsInDataHolders[0]->getPacket()->SourceID;
  Firewall::Assert((( aSourceID >= 0) && (aSourceID < 10)),
		   __HERE__,
		   "SourceID out of range");

  DH_Ring<DH_Test>* aDH = new DH_Ring<DH_Test>();
  memcpy((void*)aDH->getPacket(),
	 (void*)itsInDataHolders[0]->getPacket(), 
	 itsInDataHolders[0]->getDataPacketSize());
  itsQDataHolders.push(aDH);
  //  cout << "Added to Q " << getInstanceCnt() << " size now is " << itsQDataHolders.size() 
  //     << "\t " << aDH->getBuffer()[0] << endl;

  Firewall::Assert(itsQDataHolders.size() <= 20,
		   __HERE__,
		   "Queue length too long!! %i",itsQDataHolders.size());

  TRACER(monitor,"WH_RingOut " << getInstanceCnt() 
	 << " Filled DHBuffer[" << aSourceID << "] ");
  }
  
  if (getOutHolder(0)->doHandle() && itsQDataHolders.size() > 0) {
    
    DH_Ring<DH_Test> *aDH;
    aDH = itsQDataHolders.front();
    itsQDataHolders.pop();
//     cout << "Removed from Q " << getInstanceCnt() << " size now is " << itsQDataHolders.size() 
// 	 << "\t " << aDH->getBuffer()[0] << endl;
    

    memcpy((void*)itsOutDataHolders[0]->getPacket(), 
	   (void*)aDH->getPacket(),
	   itsInDataHolders[0]->getDataPacketSize());
  }

}

void WH_RingOut::dump () const
{
  cout << "WH_RingOut Buffer: " << getInstanceCnt() << "   ";
  cout << " " <<  itsOutDataHolders[0]->getBuffer()[0] ;
  cout << endl;
}









