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
  WH_FromRing (int seqNr);
  virtual ~ WH_FromRing ();

  /// Make a fresh copy of the WH object.
  virtual WH_FromRing* make (const string& name) const;

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

  /// Sequence number
  int itsSeqNr;
};


#endif 




