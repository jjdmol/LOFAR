// WH_WAV.cpp: implementation of the WH_WAV class.
//
//////////////////////////////////////////////////////////////////////

#include "general.h"
#include "WH_WAV.h"
#include <math.h>
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
  itsDelay(0),
  itsTime(0.),
  itsTimeStamp(0),
  itsInfile(NULL)
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
  fclose(itsInfile);
}

void WH_WAV::takeSamples(){
    cout << "WH_WAV::takeSamples;  channel = " << itsChannel << endl;
    unsigned short in;

    //system ("rm -f ./WAVE/antenna1.wav");
    //system ("rm -f ./WAVE/antenna1.wav.ready");
    if (itsChannel == 0) { 
      // record 1 second of sound into WAV file
      system("rsh astron5 ~/RadioDemo/record_sample.csh antenna1");
  }
  
  // try to close file
  if (itsInfile != NULL) {
      fclose(itsInfile);
  }

  struct stat stat_result;
    
  cout << "wait for wav file" << endl;
  do {
      errno = 0;
      //(void)stat("./WAVE/antenna1.wav.ready", &stat_result);
      (void)stat("./WAVE/antenna1.wav", &stat_result);
  } while (errno == ENOENT);
  system("touch ./WAVE/antenna1.wav.ready");

  cout << "open WAV file " << endl;
  itsInfile = fopen("./WAVE/antenna1.wav","r");
  if (itsInfile == NULL) {
    cout << "Could not open file" << endl;
    return;
  }
  /* skip header (or about that size...)*/
  for (int i=0; i< 0x2c; i++) {
    fread(&in,sizeof(in),1,itsInfile);
  }

  // skip one sample; i.e. go to the second input channel of the stereo WAV file
  if (itsChannel == 0) fread(&in,sizeof(in),1,itsInfile);
  
}


void WH_WAV::process () {


    char ptr[10];
    int i, sample=0;
    unsigned short in, tmp , left, right;
    
    if (WorkHolder::getProcMode() == Process) {
	
	
	
	bool readSamples = false;
	while (!readSamples) {
	    if (itsInfile == NULL) {
		takeSamples();
	    }
	    int i=0;
	    while (   i<ANTSAMPLES 
		      && fread(&in,sizeof(in),1,itsInfile)) {
		i++;
#ifdef SWAPWAVVALUES
		/* swap first 2 bytes with last 2 bytes in the word */
		left = in << 8;
		right= in >> 8;
		tmp  = left | right;
		in = tmp;
#endif
		itsOutDataHolders[0]->getBuffer()[i] = in;
		
		// skip the next sample since that is the other channel;
		if (!fread(&in,sizeof(in),1,itsInfile)){
		    cout << "Could not skip channel" << endl;
		}
	    }
	    if (i < ANTSAMPLES) {
		readSamples = false;
		takeSamples();
	    } else {
		readSamples = true;
	    }
	}
      
//  	    for (int i=0; i<10; i++) {
//  	     cout << "Output [" << i << "]"
//  	  	 << " = " << itsOutDataHolders[0]->getBuffer()[i] 
//  	  	 << endl;
//  	    }
//  	    cout << "*************************************" << endl;
	dump();
    
    }
    //cout << "WH_WAV::Process " << endl;
    itsTimeStamp++;
    for (int output = 0; output < getOutputs (); output++)
	{
	    itsOutDataHolders[output]->getPacket()->timeStamp = itsTimeStamp;
	}   
    if (itsChannel == 0) system("touch ./WAVE/antenna1.wav.ready");
}

void WH_WAV::dump () const {
//   cout << "WH_WAV Buffer_" << getIndex () << ":  ";
//   int freq;
//   for (freq = 0; freq < FREQS; freq++)
//     {
//       itsOutDataHolders[output]->getBuffer() << " ";
//     }
//   cout << endl;

  cout << "Dump Antenna samples" << endl;
    char filename[80];
    sprintf(filename,".WAVE/antenna%1i.dat",itsChannel);
    ofstream outfile(filename);
    for (int ch = 0; ch < ANTSAMPLES; ch++)
      { outfile << ch << " , "
		<< sqrt(itsOutDataHolders[0]->getBuffer()[ch].real () * 
			itsOutDataHolders[0]->getBuffer()[ch].real () +
			itsOutDataHolders[0]->getBuffer()[ch].imag () *
			itsOutDataHolders[0]->getBuffer()[ch].imag ()) << endl;
      }
    return;
}









