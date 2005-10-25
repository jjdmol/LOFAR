// DH_Corr.h: interface for the DH_Corr class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _DH_CORR_H__
#define _DH_CORR_H__

#include "DataHolder.h"
#include "general.h"

class DH_Corr:public DataHolder
{
  public:
  DH_Corr (const string& name = "");
  virtual ~ DH_Corr ();

  typedef DataBufferType Corr_BType[STATIONS][STATIONS][CORRFREQS];
  class DataPacket: public DataHolder::DataPacket
    { 
    public:
      Corr_BType    Buffer;
      unsigned long timeStamp;
    };
  Corr_BType& getBuffer();
  DataPacket*   getPacket();
 private:
  DataPacket itsDataPacket;
};

inline DH_Corr::Corr_BType& DH_Corr::getBuffer()         { return itsDataPacket.Buffer; }
inline DH_Corr::DataPacket* DH_Corr::getPacket()         { return &itsDataPacket; }

#endif // 


