// WH_DataProc.cpp: implementation of the WH_DataProc class.
//
//////////////////////////////////////////////////////////////////////

#include "WH_DataProc.h"
#include <list>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



WH_DataProc::WH_DataProc (int inputs,int outputs):
WorkHolder (inputs, outputs)
{
  int ch;
  itsInDataHolders.reserve(inputs);
  itsOutDataHolders.reserve(outputs);
  for (ch = 0; ch < inputs; ch++){
    DH_beam* aDH = new DH_beam();
    itsInDataHolders.push_back(aDH);
  }
  for (ch = 0; ch < outputs; ch++)    {
    DH_Corr* aDH = new DH_Corr();
    itsOutDataHolders.push_back(aDH);
  }
}


WH_DataProc::~WH_DataProc ()
{ 
}


void WH_DataProc::process ()
{
}

void WH_DataProc::dump () const
{
  cout << "DataProc In Time    = " << itsInDataHolders[0]->getPacket()->timeStamp << endl;
//   cout << "WH_DataProc Buffer_" <<endl;
//   for (int station1 = 0; station1 < STATIONS; station1++)
//     {
//       for (int station2 = 0; station2 < station1; station2++)
// 	{
// 	  for (int freq = 0; freq < FREQS; freq++)
// 	    {	
//  	      cout << ((DH_Corr *) (this->getOutHolder (0)))->dataPacket->Buffer[station1][station2][freq].real () 
// 		   << "+" 
// 		   << ((DH_Corr *) (this->getOutHolder (0)))->dataPacket->Buffer[station1][station2][freq].imag () 
// 		   << "i, ";

// 	    }
// 	}
//     }
}
