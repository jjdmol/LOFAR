// WH_Station.cpp: implementation of the WH_Station class.
//
//////////////////////////////////////////////////////////////////////

#include <list>
#include <fstream.h>

#include "WH_Station.h"

short WH_Station::itsInstanceCnt = 0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



WH_Station::WH_Station (int inputs,int outputs):
WorkHolder (inputs, outputs)
{

  itsInDataHolders.reserve(inputs);
  itsOutDataHolders.reserve(outputs);

  int channel;  
  for (channel = 0; channel < inputs; channel++)    {
    DH_Antenna* aDH = new DH_Antenna();
    itsInDataHolders.push_back(aDH);
  }
  for (channel = 0; channel < outputs; channel++)    {
    DH_beam* aDH = new DH_beam();
    itsOutDataHolders.push_back(aDH);
  }
  myInstanceCnt = itsInstanceCnt++;
}


WH_Station::~WH_Station () {
  int ch;
  for (ch=0;ch<getInputs();ch++) {

  }
  for (ch=0;ch<getOutputs();ch++) {
  }
}

void WH_Station::process () {
}

void WH_Station::dump () {
//   cout << "WH_Station Buffer_" << getIndex () <<":  ";
//   cout << endl;
  cout << "Station Time    = " 
       << itsOutDataHolders[0]->getPacket()->timeStamp 
       << endl;
//   int freq, output;
//   for (output = 0; output < getOutputs (); output++)
//     {
//       //      cout << "    Output " << output << ":  ";
//       for (freq = 0; freq < FREQS; freq++)
// 	{
// 	  cout << itsOutDataHolders[output]->getBuffer()[freq] << " ";
// 	}
//       cout << endl;
//     }
   
   if (getInstanceCnt() == 0) {
     ofstream outfile("outData/Station0"); // create
     outfile << "#Frequency spectrum for beam 0 in station 0" << endl;
     int beam=0;
     for (int freq=0; freq<FREQS; freq++) {
       outfile << freq << " "
	       << (itsOutDataHolders[0]->getBuffer()[beam][freq].real () * 
		   itsOutDataHolders[0]->getBuffer()[beam][freq].real () +
		   itsOutDataHolders[0]->getBuffer()[beam][freq].imag () *
		   itsOutDataHolders[0]->getBuffer()[beam][freq].imag ())  
	       << endl;
     }
   }
}

