// DH_beam.h: interface for the DH_beam class.
//
//////////////////////////////////////////////////////////////////////

#ifndef AFX_DH_BEAM_H__
#define AFX_DH_BEAM_H__

#include "DataHolder.h"
#include "general.h"

class DH_beam:public DataHolder
{
  public:
  DH_beam (const string& name = "");
  virtual ~ DH_beam ();

  typedef DataBufferType Beam_BType[BEAMS][FREQS];
  class DataPacket: public DataHolder::DataPacket
    { public:
      Beam_BType Buffer;
      unsigned long   timeStamp;
      unsigned long&  getTimeStamp();
    };

  Beam_BType& getBuffer();
  DataPacket*   getPacket();

 private:
  DataPacket itsDataPacket;
};

inline DH_beam::Beam_BType& DH_beam::getBuffer()              { return itsDataPacket.Buffer; }
inline DH_beam::DataPacket*   DH_beam::getPacket()              { return &itsDataPacket; }
inline unsigned long&       DH_beam::DataPacket::getTimeStamp() { return timeStamp;}

#endif 
