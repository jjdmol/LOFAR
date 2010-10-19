// DH_freq.h: interface for the DH_freq class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _DH_FREQ_H__
#define _DH_FREQ_H__

#include "DataHolder.h"
#include "general.h"

class DH_freq:public DataHolder
{
  public:
  DH_freq (const string& name = "");
  virtual ~ DH_freq ();
  class DataPacket: public DataHolder::DataPacket
    { 
    public:
      DataBufferType Buffer[FREQS];
      unsigned long timeStamp;
    };

  /// Accessor function
  DataBufferType* getBuffer();
  DataPacket*       getPacket();

private:
  DataPacket itsDataPacket;
};

inline DataBufferType* DH_freq::getBuffer() {
  return itsDataPacket.Buffer;
}

inline DH_freq::DataPacket* DH_freq::getPacket() {
  return &itsDataPacket;
}
#endif 
