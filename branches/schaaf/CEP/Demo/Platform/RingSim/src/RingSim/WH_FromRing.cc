// WH_FromRing.cpp: implementation of the WH_FromRing class.
//
//////////////////////////////////////////////////////////////////////

#include "WH_FromRing.h"
#include "firewalls.h"

short   WH_FromRing::itsInstanceCnt = 0;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



WH_FromRing::WH_FromRing ():
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
  for (int i=0; i< 10 ; i++) itsDHBuffer[i].getPacket()->SourceID = 0;
  myInstanceCnt = itsInstanceCnt++;
}


WH_FromRing::~WH_FromRing ()
{ 
}

void WH_FromRing::process ()
{
  
  if (itsInDataHolders[0]->getPacket()->destination == getInstanceCnt()) {
  
  short aSourceID = itsInDataHolders[0]->getPacket()->SourceID;
  Firewall::Assert((( aSourceID >= 0) && (aSourceID < 10)),
		   __HERE__,
		   "SourceID out of range");

  Firewall::Assert(itsDHBuffer[aSourceID].getPacket()->SourceID == 0,
		   __HERE__,
		   "Buffer field not empty!! %i %i",
		   aSourceID,
		   getInstanceCnt());
  
  memcpy((void*)itsDHBuffer[aSourceID].getPacket(),
	 (void*)itsInDataHolders[0]->getPacket(), 
	 itsInDataHolders[0]->getDataPacketSize());

  TRACER(monitor,"WH_FromRing " << getInstanceCnt() 
	 << " Filled DHBuffer[" << aSourceID << "] "
	 << itsDHBuffer[aSourceID].getBuffer()[0]);
  }
  
  if (getOutHolder(0)->doHandle()) {
    TRACER(debug, "Processing Data in DHBuffer");
    for (int i=0; i< 10 ; i++) itsDHBuffer[i].getPacket()->SourceID = 0;
  }

}

void WH_FromRing::dump () const
{
  cout << "WH_FromRing Buffer " << getInstanceCnt() << "   ";
  for (int i=0; i<10; i++) {
    cout << " " <<  itsDHBuffer[i].getBuffer()[0] ;
  }
  cout << endl;
}









