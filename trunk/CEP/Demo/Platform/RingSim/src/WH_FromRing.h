// WH_FromRing.h: interface for the WH_FromRing class.
//
//////////////////////////////////////////////////////////////////////

#ifndef AFX_WH_FromRing_H__
#define AFX_WH_FromRing_H__

#include "WorkHolder.h"
#include "DH_Test.h"
#include "DH_Ring.h"

#include <vector>


class WH_FromRing:public WorkHolder
{
 public:
  WH_FromRing ();
  virtual ~ WH_FromRing ();
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

};

inline DH_Ring<DH_Test>* WH_FromRing::getInHolder (int channel)  { 
  return itsInDataHolders[channel]; 
}

inline DH_Ring<DH_Test>* WH_FromRing::getOutHolder (int channel) { 
  return itsOutDataHolders[channel];
}

inline short  WH_FromRing::getInstanceCnt()         { 
  return myInstanceCnt; 
}
#endif 




