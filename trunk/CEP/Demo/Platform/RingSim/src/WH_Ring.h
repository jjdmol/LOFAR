// WH_Ring.h: interface for the WH_Ring class.
//
//////////////////////////////////////////////////////////////////////

#ifndef AFX_WH_Ring_H__
#define AFX_WH_Ring_H__

#include <map>
#include <queue>

#include "WorkHolder.h"
#include "DH_Test.h"
#include "DH_Ring.h"
#include "Step.h"
#include "firewalls.h"

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

  ///   The routing information is stored in a map.
    /// The first field is the destination ID, 
    /// the second field the corresponding output channel
  map<int,int> itsRoutingTable;
  
  /// return whether the input slot is stil to be sent(false) or 
  /// is already delivered and thus "empty" (true)
  bool slotIsEmpty(int slotnr=1) const;

  /// return whether the input DH is still to be sent
  bool inputIsEmpty() const;

  void markInputEmpty();
  void markOutputEmpty();
  void markSlotEmpty(int slotnr);
  void copySlot2Output(int slotnr);
  void copyQ2Output();
  void copySlot2OutQ(int slotnr);
  void copyInput2Slot(int slotnr);  
  void copySlot2Slot(int srcslotnr, int destslotnr);

  /// test for data destined for this workholder and handle it.
  bool receiveFromRing();
  
  /// if the cuurent DH slot is not occupied, put input data in it (if available)
  /// return if the slot is occupied after return.
  bool putInputToRing();

  /// copy data from one slot to another if needed according to routing table.
  void reRoute();

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
  
  queue<DH_Ring<T>*> itsOutQ;
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
inline bool     WH_Ring<T>::slotIsEmpty(int slotnr) const {
  return itsInDataHolders[slotnr]->getPacket()->destination == NOTADDRESSED;
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
inline void     WH_Ring<T>::markSlotEmpty(int slotnr) { 
  itsInDataHolders[slotnr]->getPacket()->destination = NOTADDRESSED;
  itsOutDataHolders[slotnr]->getPacket()->destination = NOTADDRESSED;
}

template <class T>
inline void WH_Ring<T>::copySlot2Output(int slotnr){
  memcpy((void*)itsOutDataHolders[0]->getPacket(),
	 (void*)itsInDataHolders[slotnr]->getPacket(),
	 itsInDataHolders[slotnr]->getDataPacketSize());
}

template <class T>
inline void WH_Ring<T>::copyQ2Output(){
  DH_Ring<T>* aDH = itsOutQ.front();
  itsOutQ.pop();
  memcpy((void*)itsOutDataHolders[0]->getPacket(),
	 (void*)aDH->getPacket(),
	 itsInDataHolders[0]->getDataPacketSize());
  delete aDH; // created in copySlot2OutQ()
}

template <class T>
inline void WH_Ring<T>::copySlot2OutQ(int slotnr){
  DH_Ring<T>* aDH = new DH_Ring<T>();
  memcpy((void*)aDH->getPacket(),
	 (void*)itsInDataHolders[slotnr]->getPacket(),
	 itsInDataHolders[slotnr]->getDataPacketSize());
  itsOutQ.push(aDH);
}


template <class T>
inline void WH_Ring<T>::copyInput2Slot(int slotnr) {
  memcpy((void*)itsOutDataHolders[slotnr]->getPacket(),
	 (void*)itsInDataHolders[0]->getPacket(), 
	 itsInDataHolders[slotnr]->getDataPacketSize());
}

template <class T>
inline void WH_Ring<T>::copySlot2Slot(int srcslotnr, int destslotnr) {
  memcpy((void*)itsOutDataHolders[destslotnr]->getPacket(),
	 (void*)itsInDataHolders[srcslotnr]->getPacket(), 
	 itsInDataHolders[srcslotnr]->getDataPacketSize());
}


/****************************************************************************/


template <class T>
inline WH_Ring<T>::WH_Ring ():
WorkHolder (3,3),
itsLastBufferSent(true)
{
  itsInDataHolders.reserve(3);
  itsOutDataHolders.reserve(3);
  
  itsInDataHolders.push_back(new DH_Ring<T>()); // input
  
  DH_Ring<DH_Test>* aDH1 = new DH_Ring<T>();
  DH_Ring<DH_Test>* aDH2 = new DH_Ring<T>();
  itsInDataHolders.push_back(aDH1); // Slot1
  itsInDataHolders.push_back(aDH2); // Slot2
  
  itsOutDataHolders.push_back(new DH_Ring<T>()); // output

  itsOutDataHolders.push_back(aDH1); // Slot1
  itsOutDataHolders.push_back(aDH2); // Slot2
  
  myInstanceCnt = itsInstanceCnt++;
  Firewall::Assert(myInstanceCnt != NOTADDRESSED
		   ,__HERE__,
		   "InstanceCnt == NOTADDRESSED flag %i",NOTADDRESSED);
  
  for (int destination = 0; destination<10; destination++) {
    if (myInstanceCnt <= 4) { // first ring
      if (destination <= 4) itsRoutingTable[destination] = 1; // this ring
      if (destination  > 4) itsRoutingTable[destination] = 2; // other ring
    }
    if (myInstanceCnt >  4) { // second ring
      if (destination >  4) itsRoutingTable[destination] = 1; //this ring
      if (destination <= 4) itsRoutingTable[destination] = 2; // other ring
    }

  }
  markInputEmpty();
  markOutputEmpty();
  markSlotEmpty(1);
  markSlotEmpty(2);
}


template <class T>
inline WH_Ring<T>::~WH_Ring ()
{ 
}

template <class T>
inline void WH_Ring<T>::process ()
{

  // set back the doRead flag which may have been lowered in order to
  // prevent deadlock in the first call to a closed loop first element 
  // 
  
  if (Step::getEventCount() == 1) 
    for (int channel=1; channel<getInputs(); channel++)
      getInHolder(channel)->setRead(true); 

/*   cout << "RingNode " << getInstanceCnt()  << endl */
/*        << "\t Ring INDATA1:  destination "  */
/*        << itsInDataHolders[1]->getPacket()->destination << "  " */
/*        << itsInDataHolders[1]->getBuffer()[0] << endl */
/*        << "\t Ring INDATA2:  destination "  */
/*        << itsInDataHolders[2]->getPacket()->destination << "  " */
/*        << itsInDataHolders[2]->getBuffer()[0] << endl */
/*        << "\t ToRing INDATA1:  destination "  */
/*        << itsInDataHolders[0]->getPacket()->destination << "  " */
/*        << itsInDataHolders[0]->getBuffer()[0] << endl; */

  if (getInHolder(0)->doHandle()) {    
    Firewall::Assert(itsLastBufferSent,
		     __HERE__,
		     "Overwriting undelivered data");
    // reset the flag
    if (!inputIsEmpty())itsLastBufferSent = false;
  }

  markOutputEmpty();
  receiveFromRing();
  reRoute();
  putInputToRing();
  
/*   cout << "\t OUTDATA1:  destination "  */
/*        << itsOutDataHolders[1]->getPacket()->destination << "  " */
/*        << itsOutDataHolders[1]->getBuffer()[0] << endl */
/*        << "\t OUTDATA2:  destination "  */
/*        << itsOutDataHolders[2]->getPacket()->destination << "  " */
/*        << itsOutDataHolders[2]->getBuffer()[0] << endl */
/*        << "\t From Ring DATA: destination "  */
/*        << itsOutDataHolders[0]->getPacket()->destination << "  " */
/*        << itsOutDataHolders[0]->getBuffer()[0] << endl; */
}

template <class T>
inline bool WH_Ring<T>::receiveFromRing() {
  // test all Slots for data to be received and store into queue
  //  then, check for data in queue and copy it into the Output
  //  (therefore, this may be data received in a earlier call
  for (int slotnr=1; slotnr<getInputs(); slotnr++) {
    if (getInstanceCnt() == itsInDataHolders[slotnr]->getPacket()->destination) {
      cout << "Packet arrived at WorkHolder" << getInstanceCnt()
	   << " Destination = " << itsInDataHolders[slotnr]->getPacket()->destination 
	   << " Data = " << itsInDataHolders[slotnr]->getBuffer()[0] << endl;
      //copy the data to the queue
      copySlot2OutQ(slotnr);
      // mark the DH as received
      markSlotEmpty(slotnr);
    }
  } 
  if (itsOutQ.size() > 0) {
    // copy queued data to output
    copyQ2Output();
  }
  return true;
}

template <class T>
inline bool WH_Ring<T>::putInputToRing() {
  if ((slotIsEmpty()) &&  !inputIsEmpty()) {
    // OK we have input data to put on the ring
    int slot = itsRoutingTable[itsInDataHolders[0]->getPacket()->destination];
    copyInput2Slot(slot);
    itsLastBufferSent = true;
    markInputEmpty();
    cout << "Added to ring data for node " 
	 << itsOutDataHolders[slot]->getPacket()->destination  << " " 
	 << itsOutDataHolders[slot]->getBuffer()[0] << endl;
    return true;
  } 
  return false;
}

template <class T>
inline void WH_Ring<T>::reRoute() {
  /** This routine only works for closed loops since rerouting is not
      done if a non-empty slot is found; the package will then be rerouted
      further on in the ring or eventualy after one or more loops by
      this same ring element.
  */
  for (int slotnr=1; slotnr<getInputs(); slotnr++) {
    if (!slotIsEmpty(slotnr)) {
      int destslot = itsRoutingTable[itsInDataHolders[slotnr]->getPacket()->destination];
      if ( destslot != slotnr) {
	if (slotIsEmpty(destslot)) { // only rerout if slot is empty; this only work for closed loops!!!!!
	  cout << "reRoute destination slot element " 
	       << getInstanceCnt() 
	       << " destination " << itsInDataHolders[slotnr]->getPacket()->destination 
	       << " : " << slotnr << " --> " << destslot << endl;
	  copySlot2Slot(slotnr,destslot);
	  markSlotEmpty(slotnr);
	}
      }
    }
  }
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




