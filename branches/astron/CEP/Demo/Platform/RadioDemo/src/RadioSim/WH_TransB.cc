// WH_TransB.cpp: implementation of the WH_TransB class.
//
//////////////////////////////////////////////////////////////////////


#include "WH_TransB.h"
#include "callspectralfft.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


WH_TransB::WH_TransB (int inputs,
		      int outputs):
  WorkHolder (inputs, outputs)
{
  int ch;
  itsInDataHolders.reserve(inputs);
  itsOutDataHolders.reserve(outputs);
  for (ch = 0; ch < inputs; ch++)    {
    DH_beamT* aDH = new DH_beamT();
    itsInDataHolders.push_back(aDH);
  }
  for (ch = 0; ch < outputs; ch++)     {
    DH_beam* aDH = new DH_beam();
    itsOutDataHolders.push_back(aDH);
  }

}


WH_TransB::~WH_TransB ()
{
  int ch;
  for (ch=0;ch<getInputs();ch++) {

  }
  for (ch=0;ch<getOutputs();ch++) {

  }
}

void WH_TransB::process ()
{
  
  if (WorkHolder::getProcMode() == Process) {
    int freq, beam;		// loop counters
    // firewall::Assert(FREQS() != getOutputs()
    //                  ,__HERE__,
    //                  "Array boundary check failed");
    for (freq = 0; freq < FREQS; freq++) {
      for (beam = 0; beam < BEAMS; beam++) {
	itsOutDataHolders[0]->getBuffer()[beam][freq] =
	  itsInDataHolders[freq]->getBuffer()[beam];
      }
    }
    for (int output = 0; output < getOutputs (); output++) {
      itsOutDataHolders[output]->getPacket()->timeStamp =
	itsInDataHolders[0]->getPacket()->timeStamp;
    }    
  } else { // zeroes,ones,infile,skip
    switch (WorkHolder::getProcMode()) {
    case Zeroes :
      // set all output to 0
      for (int freq = 0; freq < FREQS; freq++) {
	for (int beam = 0; beam < BEAMS; beam++) {
	  itsOutDataHolders[0]->getBuffer()[beam][freq] = 0;
	}
      }
      break;
    case Ones :
      // set all output to 1
      for (int freq = 0; freq < FREQS; freq++) {
	  for (int beam = 0; beam < BEAMS; beam++) {
	      itsOutDataHolders[0]->getBuffer()[beam][freq] = 1;
	    }
	}
      break;
    case Infile :
      {
	// read some infile ..
      }     
      break;
    case Skip :
      int freq, beam;		// loop counters
      // cout << "WH_TransB " << getIndex() << " Process"  << endl;
      // firewall::Assert(,__HERE__,
      //                  "Array boundary check failed");
      for (freq = 0; freq < FREQS; freq++) {
	  for (beam = 0; beam < BEAMS; beam++) {
	      itsOutDataHolders[0]->getBuffer()[beam][freq] = 
		itsInDataHolders[beam]->getBuffer()[freq];
	    }
	}
      break; 
    default :
      TRACER(error, "WH_TransB Error: unknown process mode ");
      break;
    }
  }
}

void WH_TransB::dump () const
{
//   cout << "WH_TransB Buffer" << endl;
//   int freq, output;
//   cout << "TransB Time = " << itsOutDataHolders[output]->getPacket()->timeStamp << endl;
//   for (output = 0; output < getOutputs (); output++)
//     {
//       cout << "    Output " << output << ":  ";
//       for (freq = 0; freq < FREQS; freq++)
// 	{
//  	  cout << itsOutDataHolders[beam]->getBuffer()[freq] << " ";
// 	}
//       cout << endl;
//     }
}
