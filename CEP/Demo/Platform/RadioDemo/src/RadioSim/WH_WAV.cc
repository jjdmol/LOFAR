// WH_WAV.cpp: implementation of the WH_WAV class.
//
//////////////////////////////////////////////////////////////////////

#include "general.h"
#include "WH_WAV.h"
#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

//float WH_WAV::itsTime=0.;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



WH_WAV::WH_WAV (int inputs,int outputs, int channel):
  WorkHolder (inputs, outputs),
  itsChannel(channel),
  itsOffset(0),
  itsFirstCall(true),
  itsDelay(0),
  itsTime(0.),
  itsTimeStamp(0)
{
  int ch;

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

  FILE *infile;
  char ptr[10];
  int i, sample=0;
  unsigned short currentvalue, in , left, right;
  char ready_name[256];
  struct stat ready_stat;

  system("record_sample_sim antenna1");
  system("touch ./WAVE/antenna1.wav.ready");
  cout << "Size of (in) = " << sizeof(in) << endl;
  sprintf(ready_name, "%s.ready", filename);
  
  // wait for ready file
//    do {
//      errno = 0;
//      (void)stat(ready_name, &ready_stat);
//    } while (errno == ENOENT);

  infile = fopen(filename,"r");
  if (infile == NULL) {
    cout << "Could not open file" << endl;
    return;
  }
  /* skip header (or about that size...)*/
  for (i=0; i< 0x2c; i++) {
    fread(&in,sizeof(in),1,infile);
  }

  // skip one sample; i.e. go to the second inpu channel of the stereo WAV file
  if (itsChannel == 0) fread(&in,sizeof(in),1,infile);

  while (   fread(&in,sizeof(in),1,infile)
	 && sample++ < 2*ANTSAMPLES) {
    float currentvalue=0.;
      
#ifdef SWAPWAVVALUES
      /* swap first 2 bytes with last 2 bytes in the word */
      left = in << 8;
      right= in >> 8;
      currentvalue = left | right;
#endif
      currentvalue = in;
      itsSampleBuffer[sample] = currentvalue;
    // skip the next sample since that is the other channel;
    fread(&in,sizeof(in),1,infile);
  }

//    for (int i=0; i<10; i++) {
//      cout << "Output [" << i << "]"
//  	 << " = " << itsSampleBuffer[i] 
//  	 << endl;
//    }
//    cout << "*************************************" << endl;

  itsOffset = 0;
  fclose(infile);
}

void WH_WAV::process () {
  if (WorkHolder::getProcMode() == Process) {

//      if (itsLags == 0) {
//        itsOffset++;      
//      }

    for (int i=0; i<ANTSAMPLES; i++) {
      itsOutDataHolders[0]->getBuffer()[i] = itsSampleBuffer[i+itsOffset];
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









