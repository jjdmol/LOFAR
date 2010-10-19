// WH_WAV.h: interface for the WH_WAV class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _WH_WAV_H__
#define _WH_WAV_H__


#include "WorkHolder.h"
#include "DH_Antenna.h"

#include <vector>
#include <stdio.h>


class WH_WAV:public WorkHolder
{
public:
  WH_WAV (int inputs, int outputs, int offset=0);

  virtual ~ WH_WAV ();

  void process ();
  void dump () const;
  void takeSamples();

  /// Retrieve a pointer to the input data holder for the given channel
  DH_Antenna* getInHolder (int channel); 

  /// Retrieve a pointer to the output data holder for the given channel
  DH_Antenna* getOutHolder (int channel); 

 private:
  int itsOffset;
  int itsChannel;
  int itsWAVArray[1000];
  unsigned short itsSampleBuffer[2*ANTSAMPLES];

  float itsDelay;
  float itsTime;
  unsigned int itsTimeStamp;
  FILE *itsInfile;

  /** vector with pointers to the input dataholders
	  The derived classes should add a similar typed vector for their 
	  DataHolders
  */
  vector<DH_Antenna*> itsInDataHolders; 

  /** vector with pointers to the output dataholders
  	  The derived classes should add a similar typed vector for their 
	  DataHolders
  */
  vector<DH_Antenna*> itsOutDataHolders; 
 
};

inline DH_Antenna* WH_WAV::getInHolder (int channel)  { 
  return itsInDataHolders[channel]; 
}

inline DH_Antenna* WH_WAV::getOutHolder (int channel) { 
  return itsOutDataHolders[channel];
}

#endif 










