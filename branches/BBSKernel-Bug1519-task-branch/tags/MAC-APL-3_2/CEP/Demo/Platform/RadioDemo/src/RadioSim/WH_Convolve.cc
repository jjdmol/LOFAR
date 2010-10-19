// WH_Convolve.cpp: implementation of the WH_Convolve class.
//
//////////////////////////////////////////////////////////////////////


#include "WH_Convolve.h"
#include "callspectralfft.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



WH_Convolve::WH_Convolve (int inputs,
		      int outputs):
  WorkHolder (inputs, outputs)
{
  int ch;
  itsInDataHolders.reserve(inputs);
  itsOutDataHolders.reserve(outputs);
  for (ch = 0; ch < inputs; ch++){
    DH_Corr* aDH = new DH_Corr();
    itsInDataHolders.push_back(aDH);
  }
  
  for (ch = 0; ch < outputs; ch++)    {
    DH_Corr* aDH = new DH_Corr();
    itsOutDataHolders.push_back(aDH);
  }
}


WH_Convolve::~WH_Convolve ()
{
  int ch;
  for (ch=0;ch<getInputs();ch++) {
   
  }
  for (ch=0;ch<getOutputs();ch++) {
   
  }
}

void WH_Convolve::process ()
{

  if (WorkHolder::getProcMode() == Process) {
     for (int station1 = 0; station1 < STATIONS; station1++) {
	for (int station2 = 0; station2 < station1; station2++) {
	    for (int freq = 0; freq < FREQS; freq++) {	
		itsOutDataHolders[0]->getBuffer()[station1][station2][freq] = 
		  DataBufferType(itsInDataHolders[0]->getBuffer()[station1][station2][freq].real(),
				 itsInDataHolders[0]->getBuffer()[station1][station2][freq].imag());
	      }
	  }
      }
  }
  else { // zeroes,ones,infile,skip
    switch (WorkHolder::getProcMode()) {
    case Zeroes :
      // set all output to 0
    for (int input = 0; input < getInputs (); input++) {
	for (int beam = 0; beam < BEAMS; beam++) {
	  //
	  }
      }
    break;
    case Ones :
      // set all output to 1
    for (int input = 0; input < getInputs (); input++) {
	for (int beam = 0; beam < BEAMS; beam++) {
	  //
	  }
      }
      break;
    case Infile :
      {
	// read some infile ..
      }     
      break;
    case Skip :
      for (int station1 = 0; station1 < STATIONS; station1++) {
	for (int station2 = 0; station2 < station1; station2++) {
	  for (int freq = 0; freq < FREQS; freq++) {	
	    itsOutDataHolders[0]->getBuffer()[station1][station2][freq] = 
	      DataBufferType(itsInDataHolders[0]->getBuffer()[station1][station2][freq].real(),
			     itsInDataHolders[0]->getBuffer()[station1][station2][freq].imag());
	  }
	}
      }
    default :
      TRACER(error, "WH_Convolve Error: unknown process mode ");
      break;
    }
  }
}

void WH_Convolve::dump () const
{
//  cout << "WH_Convolve Buffer_" << getIndex () << ":  " << endl;
//  for (int station1 = 0; station1 < STATIONS; station1++)
//     {
//       for (int station2 = 0; station2 < station1; station2++)
// 	{
// 	  cout << "Station" << station1 << " X " << station2 << " : ";
// 	  for (int freq = 0; freq < BEAMS; freq++)
// 	    {
// 	      cout << itsOutDataHolders[0]->getBuffer()[station1][station2][freq] << " ";
// 	    }
// 	  cout << endl;
// 	}
//     }
//   cout << endl;
}

