// WH_ToRing.h: interface for the WH_ToRing class.
//
//////////////////////////////////////////////////////////////////////

#ifndef AFX_WH_ToRing_H__
#define AFX_WH_ToRing_H__

#include "WorkHolder.h"
#include "DH_Test.h"
#include "DH_Ring.h"

#include <vector>


class WH_ToRing:public WorkHolder
{
 public:
  WH_ToRing ();
  virtual ~ WH_ToRing ();
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

  static short itsInstanceCnt;
  short        myInstanceCnt;

  static short BeamNr;
};

inline DH_Ring<DH_Test>* WH_ToRing::getInHolder (int channel)  { 
  return itsInDataHolders[channel]; 
}

inline DH_Ring<DH_Test>* WH_ToRing::getOutHolder (int channel) { 
  return itsOutDataHolders[channel];
}

inline short  WH_ToRing::getInstanceCnt()         { 
  return myInstanceCnt; 
}
#endif 

