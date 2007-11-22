// DH_BeamBand.h: interface for the DH_beam class.
//
//////////////////////////////////////////////////////////////////////

#ifndef AFX_DH_BEAMBAND_H__
#define AFX_DH_BEAMBAND_H__

#include "DataHolder.h"
#include "general.h"

class DH_BeamBand:public DataHolder
{
  public:
  DH_BeamBand (const string& name = "");
  virtual ~ DH_BeamBand ();

  typedef DataBufferType Beam_BType[FREQBANDWIDTH];
  class DataPacket: public DataHolder::DataPacket
    { public:
      Beam_BType      Buffer;
      short           station;
      short           beam;
      short           firstFrequencyChannel;
      unsigned long   timeStamp;
      unsigned long&  getTimeStamp();
    };

  Beam_BType& getBuffer();
  DataPacket*   getPacket();

 private:
  DataPacket itsDataPacket;
};

inline DH_BeamBand::Beam_BType& DH_BeamBand::getBuffer() { 
  return itsDataPacket.Buffer; 
}

inline DH_BeamBand::DataPacket* DH_BeamBand::getPacket() { 
  return &itsDataPacket; 
}

inline unsigned long& DH_BeamBand::DataPacket::getTimeStamp() { 
  return timeStamp;
}

#endif 
