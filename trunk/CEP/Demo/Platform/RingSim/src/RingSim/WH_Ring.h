// WH_Ring.h: interface for the WH_Ring class.
//
//////////////////////////////////////////////////////////////////////

#ifndef AFX_WH_Ring_H__
#define AFX_WH_Ring_H__

#include "WorkHolder.h"
#include "DH_Test.h"
#include "DH_Ring.h"

template <class T> 
class WH_Ring:public WorkHolder
{
 public:
  WH_Ring ();
  virtual ~ WH_Ring ();
  void process ();
  void dump () const;

  short getInstanceCnt();

  /// Retrieve a pointer to the input data holder for the given channel
  DH_Ring<T>* getInHolder (int channel); 

  /// Retrieve a pointer to the output data holder for the given channel
  DH_Ring<T>* getOutHolder (int channel); 

 private:
  
  /// return whether the input slot is stil to be sent(false) or 
  /// is already delivered and thus "empty" (true)
  bool slotIsEmpty() const;

  /// return whether the input DH is still to be sent
  bool inputIsEmpty() const;

  void markInputEmpty();
  void markOutputEmpty();
  void markSlotEmpty();
  void copySlot2Output();
  void copyInput2Slot();

  /// test for data destined for this workholder and handle it.
  bool receiveFromRing();
  
  /// if the cuurent DH slot is not occupied, put input data in it (if available)
  /// return if the slot is occupied after return.
  bool putInputToRing();

   /** vector with pointers to the input dataholders
	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_Ring<T>*> itsInDataHolders; 

  /** vector with pointers to the output dataholders
  	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_Ring<T>*> itsOutDataHolders; 

  static short itsInstanceCnt;
  short        myInstanceCnt;
  bool         itsLastBufferSent;
};

const unsigned short NOTADDRESSED = 9999;

template <class T>
inline DH_Ring<T>* WH_Ring<T>::getInHolder (int channel)  { 
  return itsInDataHolders[channel]; 
}

template <class T>
inline DH_Ring<T>* WH_Ring<T>::getOutHolder (int channel) { 
  return itsOutDataHolders[channel];
}

template <class T>
inline short    WH_Ring<T>::getInstanceCnt()           { 
  return myInstanceCnt; 
}

template <class T>
inline bool     WH_Ring<T>::slotIsEmpty() const {
  return itsInDataHolders[1]->getPacket()->destination == NOTADDRESSED;
}

template <class T>
inline bool     WH_Ring<T>::inputIsEmpty() const {
  return itsInDataHolders[0]->getPacket()->destination == NOTADDRESSED;
}

template <class T>
inline void     WH_Ring<T>::markInputEmpty() { 
  itsInDataHolders[0]->getPacket()->destination = NOTADDRESSED;
}

template <class T>
inline void     WH_Ring<T>::markOutputEmpty() { 
  itsOutDataHolders[0]->getPacket()->destination = NOTADDRESSED;
}

template <class T>
inline void     WH_Ring<T>::markSlotEmpty() { 
  itsInDataHolders[1]->getPacket()->destination = NOTADDRESSED;
  itsOutDataHolders[1]->getPacket()->destination = NOTADDRESSED;
}

template <class T>
inline void WH_Ring<T>::copySlot2Output(){
  memcpy((void*)itsOutDataHolders[0]->getPacket(),
	 (void*)itsInDataHolders[1]->getPacket(),
	 itsInDataHolders[1]->getDataPacketSize());
}


template <class T>
inline void WH_Ring<T>::copyInput2Slot() {
  memcpy((void*)itsOutDataHolders[1]->getPacket(),
	 (void*)itsInDataHolders[0]->getPacket(), 
	 itsInDataHolders[1]->getDataPacketSize());
}


/****************************************************************************/


template <class T>
inline WH_Ring<T>::WH_Ring ():
WorkHolder (2,2),
itsLastBufferSent(true)
{
  itsInDataHolders.reserve(2);
  itsOutDataHolders.reserve(2);
  
  itsInDataHolders.push_back(new DH_Ring<DH_Test>()); // input
  
  DH_Ring<DH_Test>* aDH = new DH_Ring<DH_Test>();
  itsInDataHolders.push_back(aDH); // Slot
  
  itsOutDataHolders.push_back(new DH_Ring<DH_Test>()); // output

  itsOutDataHolders.push_back(aDH); // Slot
  
  myInstanceCnt = itsInstanceCnt++;
  Firewall::Assert(myInstanceCnt != NOTADDRESSED
		   ,__HERE__,
		   "InstanceCnt == NOTADDRESSED flag %i",NOTADDRESSED);
}


template <class T>
inline WH_Ring<T>::~WH_Ring ()
{ 
}

template <class T>
inline void WH_Ring<T>::process ()
{

  TRACER(monitor,"RingNode " << getInstanceCnt() << " Ring INDATA:  destination " 
	 << itsInDataHolders[1]->getPacket()->destination << "  "
	 << itsInDataHolders[1]->getBuffer()[0] << endl
	 << "RingNode " << getInstanceCnt() << " ToRing INDATA:  destination " 
	 << itsInDataHolders[0]->getPacket()->destination << "  "
	 << itsInDataHolders[0]->getBuffer()[0]);

 
  if (getInHolder(0)->doHandle()) {    
    Firewall::Assert(itsLastBufferSent,
		     __HERE__,
		     "Overwriting undelivered data");
    // reset the flag
    if (!inputIsEmpty())itsLastBufferSent = false;
  }

  // make both outholders invallid
  
  markOutputEmpty();
  receiveFromRing();
  putInputToRing();
  
  TRACER(monitor,"RingNode " << getInstanceCnt() << " OUTDATA:  destination " 
	 << itsOutDataHolders[1]->getPacket()->destination << "  "
	 << itsOutDataHolders[1]->getBuffer()[0] << endl
	 << "RingNode " << getInstanceCnt() << " From Ring DATA:  destination " 
	 << itsOutDataHolders[0]->getPacket()->destination << "  "
	 << itsOutDataHolders[0]->getBuffer()[0]);
}

template <class T>
inline bool WH_Ring<T>::receiveFromRing() {
  if (getInstanceCnt() == itsInDataHolders[1]->getPacket()->destination) {
    TRACER(monitor, "Packet arrived at WorkHolder" << getInstanceCnt()
	   << " Destination = " << itsInDataHolders[1]->getPacket()->destination 
	   << " Data = " << itsInDataHolders[1]->getBuffer()[0]);
    //copy the data to the outholder
    copySlot2Output();

    // mark the DH as received
    markSlotEmpty();
   return true;
  } else {
  }
  return false;
}

template <class T>
inline bool WH_Ring<T>::putInputToRing() {
  if ((slotIsEmpty()) &&  !inputIsEmpty()) {
    // OK we have input data to put on the ring
    copyInput2Slot();
    itsLastBufferSent = true;
    markInputEmpty();
    TRACER(monitor,"Added to ring data for node " 
	   << itsOutDataHolders[1]->getPacket()->destination  << " " 
	   << itsOutDataHolders[1]->getBuffer()[0] );
    return true;
  } 
  return false;
}

template <class T>
inline void WH_Ring<T>::dump () const
{
//   cout << "WH_Ring Buffer " << getInstanceCnt() << " Timestamp  = " 
//        << itsOutDataHolders[1]->getPacket()->timeStamp 
//        << "\t Buffer[0] = " <<  itsOutDataHolders[1]->getBuffer()[0] 
//        << "\t Destination : " << itsOutDataHolders[1]->getPacket()->destination  
//        << endl;
}

template <class T>
short WH_Ring<T>::itsInstanceCnt = 0;


#endif 

