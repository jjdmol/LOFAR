//#  AH_BGLProcessing.h: 
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$

#ifndef AH_BGLPROCESSING_H
#define AH_BGLPROCESSING_H

#include <tinyCEP/TinyApplicationHolder.h>
#include <Transport/TransportHolder.h>
#include <Transport/Connection.h>
#include <tinyCEP/WorkHolder.h>

namespace LOFAR {
// Description of class.
class AH_BGLProcessing: public LOFAR::TinyApplicationHolder
{
 public:
  AH_BGLProcessing();
  virtual ~AH_BGLProcessing();
  virtual void undefine();
  virtual void define  (const LOFAR::KeyValueMap&);
  virtual void prerun  ();
  virtual void run     (int nsteps);
  virtual void postrun  ();
  virtual void dump    () const;
  virtual void quit    ();

 private:

  void connectWHs(WorkHolder* srcWH, int srcDH, WorkHolder* dstWH, int dstDH);
  vector<WorkHolder*> itsWHs;
  vector<Connection*> itsConnections;
  vector<TransportHolder*> itsTHs;
};
}
#endif
