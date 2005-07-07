//#  AH_Storage.h: 
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$
//#
////////////////////////////////////////////////////////////////////

#ifndef TFC_STORAGE_AH_STORAGE_H
#define TFC_STORAGE_AH_STORAGE_H

#include <CEPFrame/ApplicationHolder.h>

namespace LOFAR {

// This is the ApplicationHolder for the storage section of the TFLopCorrelator demo
// This applicationholder uses the CEPFrame library and is supposed to
// connect to the BGLProcessing application Holder (using only tinyCEP). 
// The interface between these is defined in  the Corr_Stub class.
// 

class Step;
class Stub_Corr;

class AH_Storage: public LOFAR::ApplicationHolder
{
 public:
  AH_Storage();
  virtual ~AH_Storage();
  virtual void undefine();
  virtual void define  (const LOFAR::KeyValueMap&);
  virtual void prerun  ();
  virtual void run     (int nsteps);
  virtual void dump    () const;
  virtual void quit    ();
 private:

  vector<WorkHolder*> itsWHs;
  vector<Step*> itsSteps;
  int itsNSBF;
  Stub_Corr* itsStub;

};
}
#endif
