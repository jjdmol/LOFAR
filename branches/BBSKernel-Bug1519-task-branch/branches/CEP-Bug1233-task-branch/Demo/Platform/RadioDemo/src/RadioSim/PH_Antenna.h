// PH_Antenna
//
//////////////////////////////////////////////////////////////////////

#ifndef _PH_ANTENNA_H_
#define _PH_ANTENNA_H_

#include "ParamHolder.h"

#include <math.h>

/**
   This is the ParameterHolder for the Antenna class; each antenna
   will get an instance of the class. 
 */
class PH_Antenna: ParamHolder
{
  public:

  /// Standard Parameter type class
  class DataType
    { public:
        float Xpos;
	float Ypos;
    };

  PH_Antenna();

  void  setPosition(const float aXpos, const float aYpos);
  float getRadius() const;
  float getAngle() const;

  void Dump () const;
  int getDataPacketSize () const;

 private:
  DataType *dataPacket;
};

inline int   PH_Antenna::getDataPacketSize () const { return sizeof(DataType);}
inline float PH_Antenna::getRadius() const          { return sqrt(dataPacket->Xpos*dataPacket->Xpos
								  +dataPacket->Ypos*dataPacket->Ypos);}
inline float PH_Antenna::getAngle() const           { return atan2(dataPacket->Ypos,dataPacket->Xpos);}
#endif 
