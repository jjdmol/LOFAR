// WH_Beam.cpp: implementation of the WH_Beam class.
//
//////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <math.h>
#include <fstream.h>

#include "general.h"
#include "WH_Beam.h"
#include "fourn.c"


short WH_Beam::itsInstanceCnt = 0;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



WH_Beam::WH_Beam (int inputs, int outputs):
  WorkHolder (inputs, outputs)
{
  Firewall::Assert(inputs == outputs,
		   __HERE__,
		   "WH_Beam C'tor: inputs and outputs must be the same!");
  
  // check BEAMS for fourn();
  Firewall::Assert((((float) (log(BEAMS)/log(4)))==(floor(log(BEAMS)/log(4)))),
		   __HERE__,
		   "WH_Beam C'tor: BEAMS must be a power of 4 !");
  
  int ch;
  itsInDataHolders.reserve(inputs);
  itsOutDataHolders.reserve(outputs);
  for (ch = 0; ch < inputs; ch++) {
	DH_freqT* aDH = new DH_freqT();
    itsInDataHolders.push_back(aDH);
  }
  for (ch = 0; ch < outputs; ch++) {
 	DH_beamT* aDH = new DH_beamT();
    itsOutDataHolders.push_back(aDH);
  }
  myInstanceCnt = itsInstanceCnt++;
  return;
}


WH_Beam::~WH_Beam ()
{
  int ch;
  for (ch=0;ch<getInputs();ch++) {
   
  }
  for (ch=0;ch<getOutputs();ch++) {
    
  }

}

void WH_Beam::process () {
  // static-definition of the pattern
  double patr_00[16]={1.,0.,1.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.};
  double patr_01[16]={0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.};
  double patr_02[16]={1.,0.,1.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.};
  double patr_03[16]={0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.};
  double patr_04[16]={0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.};
  double patr_05[16]={0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.};
  double patr_06[16]={0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.};
  double patr_07[16]={0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.};
  double patr_08[16]={0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.};
  double patr_09[16]={0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.};
  double patr_10[16]={0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.};
  double patr_11[16]={0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.};
  double patr_12[16]={0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.};
  double patr_13[16]={0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.};
  double patr_14[16]={0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.};
  double patr_15[16]={0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.};
  
  double *patr[16] = {patr_00,patr_01,patr_02,patr_03,
		      patr_04,patr_05,patr_06,patr_07,
		      patr_08,patr_09,patr_10,patr_11,
		      patr_12,patr_13,patr_14,patr_15};



  if (WorkHolder::getProcMode() == Process) {
    int input, element;		// loop counters
    //      cout << "WH_Beam " << getIndex() << " Process"  << endl;
    
    double weights[ELEMENTS];
    double signals_real[ELEMENTS];
    double signals_imag[ELEMENTS];
    
    for (input = 0; input < getInputs (); input++) {
	for (element = 0; element < ELEMENTS; element++) 	  {
	  signals_real[element] = itsInDataHolders[input]->getBuffer()[element].real ();
	  signals_imag[element] = itsInDataHolders[input]->getBuffer()[element].imag ();
	  weights[element] = 1.;
	}
	
	int cnt    = 0;
	float fft_data[2*BEAMS];
        int base = (int) sqrt(BEAMS);
	for (int y=0;y < base; y++) {
	  for (int x=0; x < base; x++) {
	    if (patr[y][x]==1.) {
	      patr[y][x]= weights[cnt]*signals_real[cnt];
	      cnt++;
	    }
	    fft_data[(y*2*base)+(x*2)]= patr[y][x];
	    fft_data[(y*2*base)+(x*2)+1]=0.;
	  }
	}
	long unsigned int s[2] = {base,base};

	// 2D-Fourier transform	
        fourn(fft_data-1,s-1,2,1);
	
	for (int y = 0; y < base; y ++) {
	  for (int x = 0; x < base; x ++) {
	    itsOutDataHolders[input]->getBuffer()[(y*(base))+x] = 
	      DataBufferType(fft_data[(y*2*base)+(2*x)],fft_data[(y*2*base)+(2*x)+1]);
	  }
	}
      }
    for (int output = 0; output < getOutputs (); output++)      {
	//	getOutDataPacket(output)->timeStamp=getInDataPacket(0)->timeStamp;
	// mind the parentheses!
	itsOutDataHolders[output]->getPacket()->timeStamp=
	  itsInDataHolders[0]->getPacket()->timeStamp; //copy timestamp
      }
  }
  else { // zeroes,ones,infile,skip
    switch (WorkHolder::getProcMode()) {
    case Zeroes :
      // set all output to 0
      for (int output = 0; output < getOutputs (); output++) {
	for (int beam = 0; beam < BEAMS; beam++) {
	    itsOutDataHolders[output]->getBuffer()[beam] = 0;
	  }
      }
      break;
    case Ones :
      // set all output to 1
      for (int output = 0; output < getOutputs (); output++) {
	for (int beam = 0; beam < BEAMS; beam++) {
	    itsOutDataHolders[output]->getBuffer()[beam] = 1;
	  }
      }
      break;
    case Infile :
      {
	// read some infile ..
      }     
      break;
    case Skip :
      TRACER(error, "WH_Beam Error: cannot skip proces");
      break; 
    default :
      TRACER(error, "WH_Beam Error: unknown process mode ");
      break;
    }
    
  }
  //  cout << "Beam Time    = " << itsOutDataHolders[0]->getPacket()->timeStamp << endl;
}

void WH_Beam::dump () const
{
//  cout << "WH_Beam Buffer_" << getIndex () << ":  ";
//  int beam;
//   for (beam = 0; beam < BEAMS; beam++)
//     {
//       cout << itsOutDataHolders[0]->getBuffer()[beam] << " ";
//     }
//  cout << endl;

  // Output file

  if (getInstanceCnt() == BEAM_GRAPH) {
    cout << "Write output of Beamformer " << BEAM_GRAPH << "in file: outData/Beam" << endl;
    cout << "Plot in Matlab with: surf(load('outdata/beam.ext'))" << endl;
    ofstream outfile("outData/Beam.ext");
    int base = (int) sqrt(BEAMS);
    for (int x = 0; x < base; x ++) {
      for (int y = 0; y < base; y ++) {
	outfile <<  abs(itsOutDataHolders[0]->getBuffer()[(y*(base))+x]) << " ";
      }
      outfile << endl;
    }
    outfile.close();
  }

}


