// DH_freqT.h: interface for the DH_freqT class.
//
//////////////////////////////////////////////////////////////////////

#ifndef AFX_DH_FREQT_H__
#define AFX_DH_FREQT_H__

#include "DataHolder.h"
#include "general.h"

class DH_freqT:public DataHolder
{
  public:
  DH_freqT (const string& name = "");
  virtual ~ DH_freqT ();
  class DataPacket:public DataHolder::DataPacket
    { public:
      DataBufferType Buffer[ELEMENTS];
      unsigned long timeStamp;
    };

  DataBufferType*   getBuffer();
  DataPacket*         getPacket();

 private:
  DataPacket itsDataPacket;

};

inline DataBufferType*     DH_freqT::getBuffer()          { return itsDataPacket.Buffer; }
inline DH_freqT::DataPacket* DH_freqT::getPacket()          { return &itsDataPacket; }

#endif 
