// WH_Integrator.cpp: implementation of the WH_Integrator class.
//
//////////////////////////////////////////////////////////////////////


#include "WH_Integrator.h"
#include "callspectralfft.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



WH_Integrator::WH_Integrator (int inputs,
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


WH_Integrator::~WH_Integrator ()
{
  int ch;
  for (ch=0;ch<getInputs();ch++) {
   
  }
  for (ch=0;ch<getOutputs();ch++) {
   
  }
}

void WH_Integrator::process ()
{
  if (WorkHolder::getProcMode() == Process) {
     for (int station1 = 0; station1 < STATIONS; station1++) {
	for (int station2 = 0; station2 < station1; station2++) {
	    for (int freq = 0; freq < FREQS; freq++) {	
	    itsOutDataHolders[0]->getBuffer()[station1][station2][freq] = 
		DataBufferType (itsInDataHolders[0]->getBuffer()[station1][station2][freq].real(),
				itsInDataHolders[0]->getBuffer()[station1][station2][freq].imag()); 
	      }
	  }
      }
  }
  else { // zeroes,ones,infile,skip
    switch (WorkHolder::getProcMode()) {
    case Zeroes :
      // set all output to 0
      for (int input = 0; input < getInputs (); input++)       {
	for (int beam = 0; beam < BEAMS; beam++)	  {
          //
	}
      }
      break;
    case Ones :
      // set all output to 1
      for (int input = 0; input < getInputs (); input++)       {
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
      for (int station1 = 0; station1 < STATIONS; station1++)	{
	for (int station2 = 0; station2 < station1; station2++) 	    {
	  for (int freq = 0; freq < FREQS; freq++) 		{	
	    itsOutDataHolders[0]->getBuffer()[station1][station2][freq] =
		    DataBufferType (itsOutDataHolders[0]->getBuffer()[station1][station2][freq].real(),
				    itsOutDataHolders[0]->getBuffer()[station1][station2][freq].imag()); 
		}
	    }
	}
    default :
      TRACER(error, "WH_Integrator Error: unknown process mode ");
      break;
    }
  }
}

void WH_Integrator::dump () const
{
//   cout << "WH_Integrator Buffer_" << getIndex () << ":  " << endl;
//   for (int station1 = 0; station1 < STATIONS; station1++)
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
