// WH_WAV.cpp: implementation of the WH_WAV class.
//
//////////////////////////////////////////////////////////////////////

#include "general.h"
#include "WH_WAV.h"
#include <math.h>
#include <stdio.h>

//float WH_WAV::itsTime=0.;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



WH_WAV::WH_WAV (int inputs,int outputs, int offset):
  WorkHolder (inputs, outputs),
  itsOffset(offset),  
  itsFirstCall(true),
  itsDelay(0),
  itsTime(0.),
  itsTimeStamp(0)
{
  int ch;

  Firewall::Assert(offset < ANTSAMPLES,
		   __HERE__,
		   "Offset must be < ANTSAMPLES");

  itsInDataHolders.reserve(inputs);
  itsOutDataHolders.reserve(outputs);
  for (ch = 0; ch < inputs; ch++)    {
    DH_Antenna* aDH = new DH_Antenna();
    itsInDataHolders.push_back(aDH);
  }
  for (ch = 0; ch < outputs; ch++)    {
    DH_Antenna* aDH = new DH_Antenna();
    itsOutDataHolders.push_back(aDH);
  }
  return;
}


WH_WAV::~WH_WAV () 
{
}

void WH_WAV::readFile(char *filename){
  cout << "Read file " << filename << endl;
  cout << "itsOffset = " << itsOffset << endl;
  FILE *infile;
  char ptr[10];
  int i, sample=0;
  unsigned short currentvalue, in , left, right;
  

  infile = fopen(filename,"r");
  if (infile == NULL) {
    printf("Could not open file \n");
    return;
  }
  /* skip header (or about that size...)*/
  for (i=0; i<24; i++) {
    fread(&in,sizeof(in),1,infile);
  }

  while (   fread(&in,sizeof(in),1,infile)
	 && sample++ < ANTSAMPLES+itsOffset/2) {
#ifdef SWAPWAVVALUES
    /* swap first 2 bytes with last 2 bytes in the word */
    left = in << 8;
    right= in >> 8;
    currentvalue = left | right;
#else 
    currentvalue = max (in-63500 , 0) ;
#endif
    if (sample >= itsOffset/2) {
      itsSampleBuffer[sample - itsOffset/2] = currentvalue;
    }
  }
  //  for (int i=0; i<ANTSAMPLES; i++) {
  //    printf("SampleBuffer[%3i] = %i\n",i,itsSampleBuffer[i]);
  //  }
  fclose(infile);
}

void WH_WAV::process () {
  if (WorkHolder::getProcMode() == Process) {

    if (itsFirstCall) {
      itsFirstCall = false;
      readFile("/home/schaaf/1min-8000Hz.wav");
    }

    if (itsOffset > 0 ) {
      itsOffset--;
    }
    for (int i=0; i<ANTSAMPLES; i++) {
      itsOutDataHolders[0]->getBuffer()[i] = itsSampleBuffer[i+itsOffset];
      //       cout << "Output [" << i << "]"
      // 	   << " = " << itsOutDataHolders[0]->getBuffer()[i] 
      // 	   << endl;
    }

    //ANTSAMPLES
   
    //cout << "WH_WAV::Process " << endl;
    itsTimeStamp++;
    for (int output = 0; output < getOutputs (); output++)
      {
	itsOutDataHolders[output]->getPacket()->timeStamp = itsTimeStamp;
      }
  }   
}

void WH_WAV::dump () const {
//   cout << "WH_WAV Buffer_" << getIndex () << ":  ";
//   int freq;
//   for (freq = 0; freq < FREQS; freq++)
//     {
//       itsOutDataHolders[output]->getBuffer() << " ";
//     }
//   cout << endl;
}









