// WH_Antenna.cpp: implementation of the WH_Antenna class.
//
//////////////////////////////////////////////////////////////////////

#include "general.h"
#include "WH_Antenna.h"
#include <math.h>

//float WH_Antenna::itsTime=0.;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



WH_Antenna::WH_Antenna (int inputs,int outputs):
  WorkHolder (inputs, outputs),
  itsPH(),
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


WH_Antenna::~WH_Antenna () 
{
}


void WH_Antenna::setPosition(float aXpos, float aYpos) {
  itsPH.setPosition(aXpos, aYpos);
  itsDelay  = itsPH.getRadius() * cos(INPUT_ELEVATION/180.*PI)
    *sin((INPUT_AZIMUTH/180.*PI) - itsPH.getAngle())/LIGHT_SPEED;   // In sec
}

void WH_Antenna::process () {
  if (WorkHolder::getProcMode() == Process) {
    float iStep;
    // Calculate the difference of arrival time for an incoming wavefront
    // with respect to the arrival time at location 0,0
    iStep=itsTime;
    for (int sample = 0; sample < ANTSAMPLES ; sample ++) {
      iStep+=ANTSAMPLETIME;
      itsOutDataHolders[0]->getBuffer()[sample]=cos(INPUT_FREQ*(iStep+itsDelay));
    }
    
    itsTime=itsTime+(ANTSAMPLES*ANTSAMPLETIME);
    itsTimeStamp++;

    for (int output = 0; output < getOutputs (); output++)
      {
	itsOutDataHolders[output]->getPacket()->timeStamp = itsTimeStamp;
      }
  } else { // zeroes,ones,infile,skip
    switch (WorkHolder::getProcMode()) {
    case Zeroes :
      // set all output to 0
      for (int output=0; output<getOutputs(); output++) {
	for (int freq = 0; freq < FREQS; freq++) {
	   itsOutDataHolders[output]->getBuffer()[freq] = 0;
	}
      }
      break;
    case Ones :
      // set all output to 1
      for (int output=0; output<getOutputs(); output++) {
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
      // set all output to input 
      for (int output=0; output<getOutputs(); output++) {
	for (int freq = 0; freq < FREQS; freq++) {
	  itsOutDataHolders[output]->getBuffer()[freq] = itsInDataHolders[0]->getBuffer()[freq];
	}
      }
      break;
    default :
      TRACER(error,"WH_Beam Error: unknown process mode ");
    }
  }  
}

void WH_Antenna::dump () const {
//   cout << "WH_Antenna Buffer_" << getIndex () << ":  ";
//   int freq;
//   for (freq = 0; freq < FREQS; freq++)
//     {
//       itsOutDataHolders[output]->getBuffer() << " ";
//     }
//   cout << endl;
}









