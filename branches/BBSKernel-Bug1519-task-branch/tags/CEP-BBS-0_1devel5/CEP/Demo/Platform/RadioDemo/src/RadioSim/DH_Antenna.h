// DH_Antenna.h: interface for the DH_Antenna class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _DH_ANTENNA_H__
#define _DH_ANTENNA_H__

#include "DataHolder.h"
#include "general.h"

class DH_Antenna:public DataHolder
{
  public:
  DH_Antenna (const string& name = "");
  virtual ~ DH_Antenna ();
  void Dump ();
  class DataPacket : public DataHolder::DataPacket
    { 
    public:
      DataBufferType Buffer[ANTSAMPLES];
      unsigned long timeStamp;
    };
  
  DataBufferType*   getBuffer();
  DataPacket*       getPacket();
/*   int               getBufferSize (); */
/*   int               getDataPacketSize (); */

 private:
  DataPacket itsDataPacket;
};

inline DataBufferType*       DH_Antenna::getBuffer()          { return itsDataPacket.Buffer; }
inline DH_Antenna::DataPacket* DH_Antenna::getPacket()          { return &itsDataPacket; }
/* inline int                   DH_Antenna::getBufferSize ()     {  return sizeof (itsDataPacket.Buffer);} */
/* inline int                   DH_Antenna::getDataPacketSize () {  return sizeof (DataType);} */

#endif 
