// DH_Test.h: interface for the DH_Test class.
//
// This class is a dummy for Workholders wich doesn't need data
//////////////////////////////////////////////////////////////////////

#ifndef _DH_Test_H__
#define _DH_Test_H__

#include "DataHolder.h"
#include "BaseSim.h"

class DH_Test:public DataHolder
{
 public:
  DH_Test ();
  virtual ~ DH_Test ();
  class DataPacket: public DataHolder::DataPacket
    {
    public:
      short          Buffer[10];
      unsigned long  timeStamp;
    };
  
  /// Accessor function
      
  short*           getBuffer();
  DataPacket*        getPacket();
  
private:
  DataPacket itsDataPacket;

};

inline short*               DH_Test::getBuffer() {return itsDataPacket.Buffer;}
inline DH_Test::DataPacket* DH_Test::getPacket() {return &itsDataPacket;}


#endif 

