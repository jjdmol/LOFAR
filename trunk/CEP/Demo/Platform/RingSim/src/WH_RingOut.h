// WH_RingOut.h: interface for the WH_RingOut class.
//
//////////////////////////////////////////////////////////////////////

#ifndef AFX_WH_RingOut_H__
#define AFX_WH_RingOut_H__

#include <queue>

#include "WorkHolder.h"
#include "DH_Test.h"
#include "DH_Ring.h"

class WH_RingOut:public WorkHolder
{
 public:
  WH_RingOut ();
  virtual ~ WH_RingOut ();
  void process ();
  void dump () const;

  short getInstanceCnt();
  /// Retrieve a pointer to the input data holder for the given channel
  DH_Ring<DH_Test>* getInHolder (int channel); 

  /// Retrieve a pointer to the output data holder for the given channel
  DH_Ring<DH_Test>* getOutHolder (int channel); 

 private:
  /** vector with pointers to the input dataholders
	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_Ring<DH_Test>*> itsInDataHolders; 

  /** vector with pointers to the output dataholders
  	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_Ring<DH_Test>*> itsOutDataHolders; 

  static short   itsInstanceCnt;
  short          myInstanceCnt;

  queue<DH_Ring<DH_Test>* >  itsQDataHolders;

};

inline short  WH_RingOut::getInstanceCnt()         { 
  return myInstanceCnt; 
}

inline DH_Ring<DH_Test>* WH_RingOut::getInHolder (int channel)  { 
  return itsInDataHolders[channel]; 
}

inline DH_Ring<DH_Test>* WH_RingOut::getOutHolder (int channel) { 
  return itsOutDataHolders[channel];
}

#endif 

