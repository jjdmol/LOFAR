// WH_TransF.cpp: implementation of the WH_TransF class.
//
//////////////////////////////////////////////////////////////////////



#include "WH_TransF.h"
#include "callspectralfft.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



WH_TransF::WH_TransF (int inputs,
		      int outputs):
WorkHolder (inputs, outputs)
{
  int ch;
  itsInDataHolders.reserve(inputs);
  itsOutDataHolders.reserve(outputs);
  for (ch = 0; ch < inputs; ch++) {
    DH_freq* aDH = new DH_freq();
    itsInDataHolders.push_back(aDH);
  }
  for (ch = 0; ch < outputs; ch++){
    DH_freqT* aDH = new DH_freqT();
    itsOutDataHolders.push_back(aDH);
  }
}


WH_TransF::~WH_TransF ()
{
  int ch;
  for (ch=0;ch<getInputs();ch++) {
   
  }
  for (ch=0;ch<getOutputs();ch++) {
   
  }
}

void WH_TransF::process ()
{
  if (WorkHolder::getProcMode() == Process) {
    int input, freq;		// loop counters
    //  cout << "WH_TransF " << getIndex() << " Process"  << endl;

    // firewall::Assert(,__HERE__,
    //                  "Array boundary check failed");
    for (input = 0; input < getInputs (); input++)      {
	for (freq = 0; freq < FREQS; freq++)	  {
	    itsOutDataHolders[freq]->getBuffer()[input] = itsInDataHolders[input]->getBuffer()[freq];
	  }
      }
    for (int output = 0; output < getOutputs (); output++)      {	
	itsOutDataHolders[output]->getPacket()->timeStamp =
	  itsInDataHolders[0]->getPacket()->timeStamp; //copy timestamp
      }

  }
  else { // zeroes,ones,infile,skip
    switch (WorkHolder::getProcMode()) {
    case Zeroes :
      break;
    case Ones :
      // set all output to 1
      break;
    case Infile :      
      // read some infile ..
      break;
    case Skip :
      break; 
    default :
      TRACER(error, "WH_TransF Error: unknown process mode ");
      break;
    }
  }
}

void WH_TransF::dump () const
{
  //  cout << "WH_TransF Buffer" << endl;
//   int element, output;
//   for (output = 0; output < getOutputs (); output++)
//     {
//       cout << "    Output " << output << ":  ";
//       for (element = 0; element < ELEMENTS; element++)
// 	{
// 	  cout << getOutBuffer (output)[element].real () << "+"
// 	    << getOutBuffer (output)[element].imag () << "i ";
// 	}
//       cout << endl;
//     }
}
