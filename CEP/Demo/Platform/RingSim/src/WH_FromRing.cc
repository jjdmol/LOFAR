// WH_FromRing.cpp: implementation of the WH_FromRing class.
//
//////////////////////////////////////////////////////////////////////

#include "WH_FromRing.h"
#include "WH_Ring.h" // need definition of NOTADDRESSED ??
#include "Step.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

WH_FromRing::WH_FromRing (int seqNr):
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


WH_FromRing::~WH_FromRing ()
{ 
  for (int ch=0; ch<getInputs(); ch++) {
    delete itsInDataHolders[ch];
  }
  for (int ch=0; ch<getOutputs(); ch++) {
    delete itsOutDataHolders[ch];
  }
}

WH_FromRing* WH_FromRing::make (const string&) const
{
  return new WH_FromRing (itsSeqNr);
}

void WH_FromRing::process ()
{

  if (getInHolder(0)->doHandle()) {
    if (itsInDataHolders[0]->getPacket()->destination != NOTADDRESSED) {
    cout << "WH_FromRing " << itsSeqNr << " Received: " 
	 << itsInDataHolders[0]->getBuffer()[0] << " From " 
	 << itsInDataHolders[0]->getPacket()->destination << endl;
    }
  }
}

void WH_FromRing::dump () const
{
  cout << "WH_FomRing Buffer " //<< itsSeqNr 
     << " InBuffer[0] = " <<  itsInDataHolders[0]->getBuffer()[0] << endl;
}


DH_Ring<DH_Test>* WH_FromRing::getInHolder (int channel)
{ 
  return itsInDataHolders[channel]; 
}

DH_Ring<DH_Test>* WH_FromRing::getOutHolder (int channel)
{ 
  return itsOutDataHolders[channel];
}
