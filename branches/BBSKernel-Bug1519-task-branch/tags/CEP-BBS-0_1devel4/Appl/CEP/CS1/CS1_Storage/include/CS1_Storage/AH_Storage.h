//#  AH_Storage.h: 
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$
//#
////////////////////////////////////////////////////////////////////

#ifndef CS1_STORAGE_AH_STORAGE_H
#define CS1_STORAGE_AH_STORAGE_H

#include <CEPFrame/ApplicationHolder.h>
#include <CS1_Interface/Stub_BGL.h>
#include <CS1_Interface/CS1_Parset.h>

namespace LOFAR
{
  namespace CS1
  {

    // This is the ApplicationHolder for the storage section of the CS1 application
    // This applicationholder uses the CEPFrame library and is supposed to
    // connect to the BGLProcessing application Holder (using only tinyCEP). 
    // The interface between these is defined in  the Corr_Stub class.
    // 
    class AH_Storage: public LOFAR::ApplicationHolder
    {
    public:
      AH_Storage();
      virtual ~AH_Storage();
      virtual void define  (const LOFAR::KeyValueMap&);
      virtual void undefine();
      virtual void prerun  ();
      virtual void run     (int nsteps);
      virtual void dump    () const;
      virtual void quit    ();
    private:
      CS1_Parset       *itsCS1PS;
      Stub_BGL         *itsStub;

    };

  } // namespace CS1

} // namespace LOFAR

#endif
