// WH_FromRing.cpp: implementation of the WH_FromRing class.
//
//////////////////////////////////////////////////////////////////////

#include "WH_FromRing.h"
#include "Step.h"

short WH_FromRing::itsInstanceCnt = 0;

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
  myInstanceCnt = itsInstanceCnt++;
}


WH_FromRing::~WH_FromRing ()
{ 
}

void WH_FromRing::process ()
{
  if (getInHolder(0)->doHandle()) {

    cout << "WH_FromRing " << getInstanceCnt() << " Received: " 
	 << itsInDataHolders[0]->getBuffer()[0] << " From " 
	 << itsInDataHolders[0]->getPacket()->destination << endl;
  }

}

void WH_FromRing::dump () const
{
cout << "WH_FomRing Buffer " << getInstanceCnt() 
     << " InBuffer[0] = " <<  itsInDataHolders[0]->getBuffer()[0] << endl;
}









