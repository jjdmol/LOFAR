// DH_Ring.h: interface for the DH_Ring class.
//
// This class is a dummy for Workholders wich doesn't need data
//////////////////////////////////////////////////////////////////////

#ifndef _DH_Ring_H__
#define _DH_Ring_H__

#include "DataHolder.h"
#include "BaseSim.h"

template <class T> class DH_Ring:public DataHolder
{
 public:
  DH_Ring ();
  virtual ~ DH_Ring ();
  class DataPacket: public T::DataPacket
    {
    public:
      unsigned short destination;
      short          SourceID;
    };
  
  /// Accessor function
      
  short  *           getBuffer();
  DataPacket*        getPacket();
  
private:
  DataPacket itsDataPacket;

};

template <class T> 
inline DH_Ring<T>::DH_Ring () {
  setDataPacket(&itsDataPacket,sizeof(itsDataPacket));
}

template <class T> 
inline DH_Ring<T>::~DH_Ring () {

}

template <class T> 
inline short* DH_Ring<T>::getBuffer() {
  return itsDataPacket.Buffer;
}

template <class T> 
inline DH_Ring<T>::DataPacket* DH_Ring<T>::getPacket() {
  return &itsDataPacket;
}


#endif 

