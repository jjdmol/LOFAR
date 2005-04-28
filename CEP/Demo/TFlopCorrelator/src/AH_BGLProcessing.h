//#  AH_BGLProcessing.h: 
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$

#ifndef AH_BGLPROCESSING_H
#define AH_BGLPROCESSING_H

#include <CEPFrame/ApplicationHolder.h>

namespace LOFAR {
// Description of class.
class AH_BGLProcessing: public LOFAR::ApplicationHolder
{
 public:
  AH_BGLProcessing();
  virtual ~AH_BGLProcessing();
  virtual void undefine();
  virtual void define  ();
  virtual void prerun  ();
  virtual void run     (int nsteps);
  virtual void dump    () const;
  virtual void quit    ();
 private:
  vector<WorkHolder*> itsWHs;


};
}
#endif
