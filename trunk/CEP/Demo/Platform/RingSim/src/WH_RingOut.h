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
  WH_RingOut (int seqNr);
  virtual ~ WH_RingOut ();

  /// Make a fresh copy of the WH object.
  virtual WH_RingOut* make (const string& name) const;

  virtual void process ();
  virtual void dump () const;

  /// Retrieve a pointer to the input data holder for the given channel
  virtual DH_Ring<DH_Test>* getInHolder (int channel); 

  /// Retrieve a pointer to the output data holder for the given channel
  virtual DH_Ring<DH_Test>* getOutHolder (int channel); 

 private:
  /** vector with pointers to the input dataholders
	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_Ring<DH_Test>*> itsInDataHolders; 

  /** vector with pointers to the output dataholders
  	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_Ring<DH_Test>*> itsOutDataHolders; 

  int itsSeqNr;

  queue<DH_Ring<DH_Test>* >  itsQDataHolders;

};


#endif 

