// WH_FFT.cpp: implementation of the WH_FFT class.
//
//////////////////////////////////////////////////////////////////////

#include "general.h"
#include "WH_FFT.h"
#include <fstream.h>

#include "four1.h"

short WH_FFT::itsInstanceCnt = 0;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



WH_FFT::WH_FFT (int inputs,
		int outputs):
  WorkHolder (inputs, outputs)
{
  Firewall::Assert(inputs == outputs,
		   __HERE__,
		   "WH_FFT C'tor: inputs and outputs must be the same!");
  
  int ch;
  itsInDataHolders.reserve(inputs);
  itsOutDataHolders.reserve(outputs);

  for (ch = 0; ch < inputs; ch++)    {
    DH_Antenna* aDH = new DH_Antenna();
    itsInDataHolders.push_back(aDH);
  }
  for (ch=0; ch < outputs; ch++) {
    DH_freq* aDH = new DH_freq();
    itsOutDataHolders.push_back(aDH);
  }
  myInstanceCnt = itsInstanceCnt++;
  return;
}



WH_FFT::~WH_FFT ()
{
  int ch;
  for (ch=0;ch<getInputs();ch++) {

  }
  for (ch=0;ch<getOutputs();ch++) {

  }
}

void WH_FFT::process ()
{
  int input;		// loop counters
  
  if (WorkHolder::getProcMode() == Process) {
    float  fft_data[(ANTSAMPLES*2)]; //Data for C++ fft
    for (input = 0; input < getInputs (); input++)
      {
	// Copy input buffer in the right order to fft_data
	for(int freq = 0; freq < (ANTSAMPLES); freq++) {
	  fft_data[(2*freq)    ] = itsInDataHolders[input]->getBuffer()[freq].real();
	  fft_data[((2*freq)+1)] = itsInDataHolders[input]->getBuffer()[freq].imag();
	}


	// Call 1D-fft function four1
	four1(fft_data-1,(ANTSAMPLES),1);

	// Put fft-data back in output buffer
	for(int freq =0; freq < FREQS; freq++) {
	  itsOutDataHolders[input]->getBuffer()[freq] = 
	    DataBufferType(fft_data[(2*(freq))],fft_data[(2*(freq))+1]);
	}
      }
    for (int output = 0; output < getOutputs (); output++)
      { 
	// mind the parentheses!
	itsOutDataHolders[output]->getPacket()->timeStamp =
	  itsInDataHolders[0]->getPacket()->timeStamp ;
      }
  }
  else { // zeroes,ones,infile,skip
    switch (WorkHolder::getProcMode()) {
    case Zeroes :
      // set all output to 0
      for (int output = 0; output < getOutputs (); output++) {
	for (int freq = 0; freq < FREQS; freq++)	  {
	  itsOutDataHolders[output]->getBuffer()[freq] = 0;
	  }
      }
      break;
    case Ones :
      // set all output to 1
      for (int output = 0; output < getOutputs (); output++) {
	for (int freq = 0; freq < FREQS; freq++) {
	  itsOutDataHolders[output]->getBuffer()[freq] = 1;
	}
      }
      break;
    case Infile :
      {
	// read some infile ..
      }     
      break;
    case Skip :
      TRACER(error, "WH_FFT Error: cannot skip proces");
      break; 
    default :
      TRACER(error, "WH_FFT Error: unknown process mode ");
      break;
    }
  }
}


void WH_FFT::dump () const
{
  cout << "WH_FFT Buffer_" ;
//   for (freq = 0; freq < FREQS; freq++)
//     {
//       cout << itsOutDataHolders[0]->getBuffer()[freq] << " " ;
//     }
//   cout << endl;

  if (getInstanceCnt() == FFT_GRAPH) {
    //    cout << "Write FFT output (" << FFT_GRAPH << " to file: outData/FFT.ext" 
    //         << endl
    //	       << "For plotting in Matlab with: plot(load('outData/FFT.ext'))" 
    //         << endl ;
    ofstream outfile("outData/FFT.ext");
    for (int freq = 0; freq < FREQS; freq++)
      { outfile << sqrt(itsOutDataHolders[0]->getBuffer()[freq].real () * 
			itsOutDataHolders[0]->getBuffer()[freq].real () +
			itsOutDataHolders[0]->getBuffer()[freq].imag () *
			itsOutDataHolders[0]->getBuffer()[freq].imag ()) << endl;
      }
  }


}



