// WH_CPInput.cpp: implementation of the WH_CPInput class.
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include "WH_CPInput.h"
#include "callspectralfft.h"
#include <fstream.h>



unsigned int WH_CPInput::itsCurrentTimeStamp=0;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



WH_CPInput::WH_CPInput (int inputs,
		  int outputs):
  WorkHolder (inputs, outputs)
{
  int ch;
  itsInDataHolders.reserve(inputs);
  itsOutDataHolders.reserve(outputs);
  for (ch = 0; ch < inputs; ch++)    {
    DH_beam* aDH = new DH_beam();
    itsInDataHolders.push_back(aDH);
  }
  for (ch = 0; ch < outputs; ch++)     {
    DH_BeamBand* aDH = new DH_BeamBand();
    itsOutDataHolders.push_back(aDH);
    }
}


WH_CPInput::~WH_CPInput ()
{
}

void WH_CPInput::process () {
  TRACER(debug,"WH_CPInput Process");

  if (getProcMode() == Process) {
    int beam;		        // loop counters
    
      for (beam = 0; beam < BEAMS; beam++) {
	for (int freq=0; freq<FREQS; freq++) {
	  itsOutDataHolders[beam*FREQBANDS+freq/FREQBANDWIDTH]
	    ->getBuffer()[freq%FREQBANDWIDTH] = 
	    itsInDataHolders[0]->getBuffer()[beam][freq];
	}
      }
      
      for (int output = 0; output < getOutputs (); output++) {
	itsOutDataHolders[output]->getPacket()->timeStamp =
	  itsInDataHolders[0]->getPacket()->timeStamp;    
      }
  } else { // zeroes,ones,infile,skip
  
    switch (getProcMode()) {
    case Zeroes :
      cout << "WH_CPInput ProcMode zeroes not implemented" << endl;
      break;
    case Ones :
      cout << "WH_CPInput ProcMode ones not implemented" << endl;
      break;
    case Infile :
      cout << "WH_CPInput ProcMode infile not implemented" << endl;
      break;
    case Skip :
      cout << "WH_CPInput ProcMode skip not implemented" << endl;
      break; 
    default :
      TRACER(error, "WH_CPInput Error: unknown process mode ");
      break;
    }
  }
}


void WH_CPInput::dump () const
{
//   cout << "WH_CPInput :  ";
//   int freq, output;
//   for (output = 0; output < getOutputs (); output++) {
//       cout << "    Output (Beam) " << output << ":  ";
//       for (freq = 0; freq < FREQS; freq++) {
// 	cout << itsOutDataHolders[output]->getBuffer()[0][freq] << " ";
// 	}
//       cout << endl;
//     }
 
  ofstream outfile("outData/CentralStation0"); // create
  outfile << "#Frequency spectrum for beam 0 in station 0" << endl;
  for (int freq=0; freq<FREQS; freq++) {
    outfile << freq << " "
      //<< abs(itsOutDataHolders[0]->getBuffer()[freq])
	    << endl;
  }
}



