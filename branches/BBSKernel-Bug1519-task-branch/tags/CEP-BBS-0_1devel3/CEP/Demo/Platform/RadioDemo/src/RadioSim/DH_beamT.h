// DH_beamT.h: interface for the DH_beamT class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _DH_BEAMT_H__
#define _DH_BEAMT_H__

#include "DataHolder.h"
#include "general.h"

class DH_beamT:public DataHolder
{
  public:
  DH_beamT (const string& name = "");
  virtual ~ DH_beamT ();
  class DataPacket: public DataHolder::DataPacket
    { 
    public:
      DataBufferType Buffer[BEAMS];
      unsigned long timeStamp;
    };

  DataBufferType*   getBuffer();
  DataPacket*         getPacket();

 private:
  DataPacket itsDataPacket;
};

inline DataBufferType*     DH_beamT::getBuffer()          { return itsDataPacket.Buffer; }
inline DH_beamT::DataPacket* DH_beamT::getPacket()          { return &itsDataPacket; }
#endif 
